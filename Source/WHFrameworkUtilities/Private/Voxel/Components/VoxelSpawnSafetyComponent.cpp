#include "Voxel/Components/VoxelSpawnSafetyComponent.h"

#include "Character/Components/CharacterMovementComponentBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Voxel/Agent/VoxelAgentComponent.h"
#include "Voxel/Chunks/VoxelSectionKey.h"
#include "Voxel/Runtime/VoxelRegistry.h"
#include "Voxel/Runtime/VoxelWorldRuntime.h"
#include "Voxel/Streaming/VoxelStreamingSource.h"
#include "Voxel/VoxelModule.h"
#include "Voxel/VoxelModuleStatics.h"
#include "Voxel/VoxelSpawnPrewarm.h"

UVoxelSpawnSafetyComponent::UVoxelSpawnSafetyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UVoxelSpawnSafetyComponent::BeginPlay()
{
	Super::BeginPlay();
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !Character->GetCharacterMovement())
	{
		UE_LOG(LogTemp, Error, TEXT("体素出生保护需要角色和角色移动组件。"));
		SetComponentTickEnabled(false);
		return;
	}
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	bGuardMovement = !Movement->IsA<UCharacterMovementComponentBase>() && Character->GetLocalRole() != ROLE_SimulatedProxy;
	if (bGuardMovement)
	{
		Movement->AddTickPrerequisiteComponent(this);
		if (UVoxelAgentComponent* Agent = Character->FindComponentByClass<UVoxelAgentComponent>())
		{
			AddTickPrerequisiteComponent(Agent);
		}
	}
	PreviousMovementMode = Movement->MovementMode;
	PreviousCustomMovementMode = Movement->CustomMovementMode;
	bPreviousCollisionEnabled = Character->GetActorEnableCollision();
	Movement->DisableMovement();
	Character->SetActorEnableCollision(false);
}

void UVoxelSpawnSafetyComponent::EndPlay(const EEndPlayReason::Type InReason)
{
	ReleaseSource();
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		RestoreMovementTick(*Character);
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->RemoveTickPrerequisiteComponent(this);
		}
	}
	Super::EndPlay(InReason);
}

void UVoxelSpawnSafetyComponent::TickComponent(const float InDeltaSeconds, const ELevelTick InTickType,
	FActorComponentTickFunction* InTickFunction)
{
	Super::TickComponent(InDeltaSeconds, InTickType, InTickFunction);
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;
	if (bFinished)
	{
		if (bGuardMovement) RefreshMovementSafety(*Character, InDeltaSeconds);
		return;
	}
	if (!Character->GetCapsuleComponent())
	{
		FailPreparation(TEXT("体素出生保护需要角色胶囊。"));
		return;
	}
	if (!Module.IsValid()) Module = UVoxelModule::GetPtr(GetWorld());
	UVoxelModule* Voxel = Module.Get();
	if (!Voxel) return;
	if (Voxel->GetWorldState() == EVoxelWorldState::Failed)
	{
		FailPreparation(TEXT("体素世界初始化失败。"));
		return;
	}
	if (Voxel->HasInterestBuildFailed())
	{
		FailPreparation(FString::Printf(TEXT("出生区域的体素兴趣规划失败：%s"), *Voxel->GetInterestBuildError()));
		return;
	}
	const FVoxelWorldReadinessSnapshot& Readiness = Voxel->GetReadiness();
	if (Readiness.WorldEpoch == static_cast<int64>(Voxel->GetWorldEpoch()) && Readiness.bFailed)
	{
		FailPreparation(TEXT("出生区域的体素地形准备失败。"));
		return;
	}
	if (!Voxel->IsReady()) return;
	if (Voxel->GetManifest().TerrainType != EVoxelTerrainType::Superflat)
	{
		Character->SetActorEnableCollision(bPreviousCollisionEnabled);
		RestoreMovement();
		bFinished = true;
		bGuardMovement = false;
		Character->GetCharacterMovement()->RemoveTickPrerequisiteComponent(this);
		SetComponentTickEnabled(false);
		return;
	}
	if (SourceEpoch != Voxel->GetWorldEpoch())
	{
		ReleaseSource();
		SourceEpoch = Voxel->GetWorldEpoch();
		bPlaced = false;
	}
	if (!bPlaced)
	{
		const double CellSize = Voxel->BlockSize();
		FVector GroundLocation = Character->GetActorLocation();
		GroundLocation.Z = VoxelGeneration::SuperflatSurfaceZ * CellSize;
		FIntVector GroundCell;
		if (!VoxelCoord::FromWorld(GroundLocation, CellSize, GroundCell))
		{
			FailPreparation(TEXT("出生位置超出体素世界坐标。"));
			return;
		}
		const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
		const float Radius = Capsule->GetScaledCapsuleRadius();
		const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		const float SearchDistance = FMath::Clamp(float(CellSize) + 4.f, 4.f, 4096.f);
		FVector Desired = Character->GetActorLocation();
		Desired.Z = (GroundCell.Z + 1.0) * CellSize + HalfHeight + 2.0;
		FVoxelStreamingSource Source;
		Source.Purpose = EVoxelStreamingSourcePurpose::InitialSpawn;
		Source.Center = GroundCell;
		Source.ExactRadius = 16;
		Source.Capabilities = EVoxelStreamingCapability::Data | EVoxelStreamingCapability::Collision |
			EVoxelStreamingCapability::Readiness;
		Source.Normalize();
		if (!SourceId.IsValid())
		{
			FString RadiusError;
			if (!FVoxelSpawnPrewarm::CalculateCollisionRadius(*Voxel, GroundCell, Desired, Radius, HalfHeight,
				SearchDistance, Source.CollisionRadius, RadiusError))
			{
				FailPreparation(RadiusError);
				return;
			}
			SourceId = Voxel->RegisterSource(this, Source);
			if (!SourceId.IsValid())
			{
				FailPreparation(TEXT("无法注册出生位置的体素兴趣源。"));
				return;
			}
		}
		if (!Voxel->IsSourceAdmittedAt(SourceId, GroundCell, SourceEpoch)) return;
		FVoxelBlockState Ground;
		if (!Voxel->GetRuntime() || !Voxel->GetRuntime()->TryGetBlock(GroundCell, Ground)) return;
		const FVoxelRuntimeDefinition* Definition = Voxel->GetRegistry() ? Voxel->GetRegistry()->Find(Ground.TypeId) : nullptr;
		if (!Definition || !Definition->bSolid)
		{
			FailPreparation(TEXT("出生位置缺少可站立的体素地面。"));
			return;
		}
		FVector Stand;
		if (!UVoxelModuleStatics::FindStandLocation(this, Desired, Radius, HalfHeight, SearchDistance, Stand)) return;
		Character->SetActorLocation(Stand, false, nullptr, ETeleportType::TeleportPhysics);
		Character->SetActorEnableCollision(bPreviousCollisionEnabled);
		UVoxelAgentComponent* Agent = Character->FindComponentByClass<UVoxelAgentComponent>();
		if (!Agent)
		{
			FailPreparation(TEXT("体素出生保护需要角色的 VoxelAgentComponent。"));
			return;
		}
		Agent->SetAgentEnabled(true);
		bPlaced = true;
	}
	UVoxelAgentComponent* Agent = Character->FindComponentByClass<UVoxelAgentComponent>();
	FIntVector CharacterCell;
	if (!Agent || !VoxelCoord::FromWorld(Character->GetActorLocation(), Voxel->BlockSize(), CharacterCell) ||
		!Agent->IsStreamingLocationAdmitted(CharacterCell, SourceEpoch)) return;
	RestoreMovement();
	ReleaseSource();
	bFinished = true;
	if (bGuardMovement) RefreshMovementSafety(*Character, InDeltaSeconds);
	else SetComponentTickEnabled(false);
}

void UVoxelSpawnSafetyComponent::FailPreparation(const FString& InError)
{
	UE_LOG(LogTemp, Error, TEXT("体素出生保护失败：%s"), *InError);
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement()) Movement->DisableMovement();
		Character->SetActorEnableCollision(false);
	}
	ReleaseSource();
	bFinished = true;
	SetComponentTickEnabled(false);
}

void UVoxelSpawnSafetyComponent::ReleaseSource()
{
	if (UVoxelModule* Voxel = Module.Get(); Voxel && SourceId.IsValid() && Voxel->IsSourceOwnedBy(SourceId, this))
	{
		Voxel->UnregisterSource(SourceId);
	}
	SourceId.Invalidate();
	SourceEpoch = 0;
}

void UVoxelSpawnSafetyComponent::RestoreMovement()
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			const EMovementMode Mode = PreviousMovementMode == MOVE_None || PreviousMovementMode == MOVE_Falling
				? MOVE_Walking : PreviousMovementMode.GetValue();
			Movement->SetMovementMode(Mode, PreviousCustomMovementMode);
		}
	}
}

void UVoxelSpawnSafetyComponent::RefreshMovementSafety(ACharacter& InCharacter, const float InDeltaSeconds)
{
	UCharacterMovementComponent* Movement = InCharacter.GetCharacterMovement();
	const UCapsuleComponent* Capsule = InCharacter.GetCapsuleComponent();
	if (!Movement || !Capsule) return;
	if (!Movement->IsActive())
	{
		bMovementTickPausedForTerrain = false;
		return;
	}
	if (Movement->MovementMode == MOVE_None)
	{
		RestoreMovementTick(InCharacter);
		return;
	}
	if (!bMovementTickPausedForTerrain && !Movement->IsComponentTickEnabled()) return;
	if (!Module.IsValid()) Module = UVoxelModule::GetPtr(GetWorld());
	UVoxelModule* Voxel = Module.Get();
	if (Voxel && Voxel->IsReady() && Voxel->GetManifest().TerrainType != EVoxelTerrainType::Superflat)
	{
		RestoreMovementTick(InCharacter);
		bGuardMovement = false;
		Movement->RemoveTickPrerequisiteComponent(this);
		SetComponentTickEnabled(false);
		return;
	}
	UVoxelAgentComponent* Agent = InCharacter.FindComponentByClass<UVoxelAgentComponent>();
	const FVector Location = Capsule->GetComponentLocation();
	const FVector Velocity = Movement->Velocity;
	const double Delta = FMath::Max(0.0, static_cast<double>(InDeltaSeconds));
	const double MaxSpeed = FMath::Max(0.0, static_cast<double>(Movement->GetMaxSpeed()));
	const double Acceleration = FMath::Max(0.0, static_cast<double>(Movement->GetMaxAcceleration()));
	const double Gravity = FMath::Abs(static_cast<double>(Movement->GetGravityZ()));
	const double JumpSpeed = FMath::Abs(static_cast<double>(Movement->JumpZVelocity));
	const double StepHeight = FMath::Max(0.0, static_cast<double>(Movement->MaxStepHeight));
	const double Radius = Capsule->GetScaledCapsuleRadius();
	const double HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	bool bSafe = false;
	if (Voxel && Voxel->IsReady() && Agent && Agent->GetStreamingSourceId().IsValid() &&
		!Location.ContainsNaN() && !Velocity.ContainsNaN() && FMath::IsFinite(Delta) &&
		FMath::IsFinite(MaxSpeed) && FMath::IsFinite(Acceleration) && FMath::IsFinite(Gravity) &&
		FMath::IsFinite(JumpSpeed) && FMath::IsFinite(StepHeight) && FMath::IsFinite(Radius) &&
		FMath::IsFinite(HalfHeight) && Radius > 0.0 && HalfHeight >= Radius)
	{
		const double HorizontalTravel = FMath::Max(MaxSpeed, Velocity.Size2D()) * Delta + Acceleration * Delta * Delta;
		const double VerticalTravel = FMath::Max(FMath::Abs(Velocity.Z), JumpSpeed) * Delta + Gravity * Delta * Delta;
		const FVector Extent(Radius + HorizontalTravel + 1.0, Radius + HorizontalTravel + 1.0,
			HalfHeight + VerticalTravel + StepHeight + 2.0);
		FBox Bounds(Location - Extent, Location + Extent);
		const FVoxelGenerationSettings& Settings = Voxel->GetManifest().Settings;
		Bounds.Min.Z = FMath::Max(Bounds.Min.Z, Settings.MinZ * Voxel->BlockSize());
		Bounds.Max.Z = FMath::Min(Bounds.Max.Z, Settings.MaxZ * Voxel->BlockSize());
		bSafe = Voxel->IsTerrainSafeAt(Agent->GetStreamingSourceId(), Bounds);
	}
	if (bSafe) RestoreMovementTick(InCharacter);
	else PauseMovementTick(InCharacter);
}

void UVoxelSpawnSafetyComponent::PauseMovementTick(ACharacter& InCharacter)
{
	UCharacterMovementComponent* Movement = InCharacter.GetCharacterMovement();
	if (Movement && Movement->IsComponentTickEnabled())
	{
		Movement->SetComponentTickEnabled(false);
		bMovementTickPausedForTerrain = true;
	}
}

void UVoxelSpawnSafetyComponent::RestoreMovementTick(ACharacter& InCharacter)
{
	if (!bMovementTickPausedForTerrain) return;
	bMovementTickPausedForTerrain = false;
	if (UCharacterMovementComponent* Movement = InCharacter.GetCharacterMovement(); Movement && Movement->IsActive())
	{
		Movement->SetComponentTickEnabled(true);
	}
}
