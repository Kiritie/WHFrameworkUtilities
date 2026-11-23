// Fill out your copyright notice in the Description page of Project Settings.

#include "Voxel/Input/VoxelPrefabTestingInputBinding.h"
#include "Testing/TestingModule.h"

#include "Ability/Inventory/InventoryItemRegistry.h"
#include "Ability/Inventory/InventoryModule.h"
#include "Camera/CameraModuleStatics.h"
#include "Character/CharacterModuleStatics.h"
#include "Character/CharacterModule.h"
#include "Character/Base/CharacterBase.h"
#include "Common/CommonModuleStatics.h"
#include "Input/InputModuleStatics.h"
#include "Input/InputModule.h"
#include "Input/InputTags.h"
#include "Input/Components/InputComponentBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameViewportClient.h"
#include "Scene/Object/ScenePlacementPreview.h"
#include "UnrealClient.h"
#include "Voxel/Runtime/VoxelRegistry.h"
#include "Voxel/VoxelModule.h"
#include "Voxel/VoxelModuleStatics.h"
#include "Voxel/Agent/VoxelAgentComponent.h"
#include "Voxel/VoxelPrefabTestingModule.h"

namespace
{
	APlayerController* ResolveInputPlayer(const UInputComponentBase* Component)
	{
		AActor* Owner = Component ? Component->GetOwner() : nullptr;
		if (APlayerController* Controller = Cast<APlayerController>(Owner)) return Controller;
		const APawn* Pawn = Cast<APawn>(Owner);
		return Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	}
}

// ParamSets default values
UVoxelPrefabTestingInputBinding::UVoxelPrefabTestingInputBinding()
{
}

void UVoxelPrefabTestingInputBinding::OnBindInput(UInputComponentBase* InInputComponent)
{
	Super::OnBindInput(InInputComponent);

	InInputComponent->BindInputAction(GameplayTags::Input_SwitchView, ETriggerEvent::Started, this, &UVoxelPrefabTestingInputBinding::SwitchView);
	InInputComponent->BindInputAction(FGameplayTag::RequestGameplayTag(TEXT("Input.Testing.ToggleSceneAI")), ETriggerEvent::Started, this, &ThisClass::ToggleSceneAI);
}

void UVoxelPrefabTestingInputBinding::OnRefresh(float InDeltaSeconds)
{
	Super::OnRefresh(InDeltaSeconds);
	if (bPlacing) RefreshPlacement();
}

void UVoxelPrefabTestingInputBinding::OnReset()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent()) Agent->EndBreak();
	CancelPlacement();
	bRotatedPlacement = false;
	Super::OnReset();
}

void UVoxelPrefabTestingInputBinding::OnUnbindInput()
{
	OnReset();
	Super::OnUnbindInput();
}

void UVoxelPrefabTestingInputBinding::SystemOperation_Implementation()
{
	CancelPlacement();
	APlayerController* Player = ResolveInputPlayer(BoundInputComponent);
	if (UTestingModule* Testing = Player ? UTestingModule::GetPtr(Player->GetWorld()) : nullptr)
	{
		Testing->TogglePanel(Player);
	}
}

void UVoxelPrefabTestingInputBinding::OnPrimaryPressed_Implementation()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->BeginBreak();
	}
}

void UVoxelPrefabTestingInputBinding::OnPrimaryRepeated_Implementation()
{
}

void UVoxelPrefabTestingInputBinding::OnPrimaryReleased_Implementation()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->EndBreak();
	}
}

void UVoxelPrefabTestingInputBinding::OnSecondaryPressed_Implementation()
{
	CancelPlacement();
	UVoxelAgentComponent* Agent = GetVoxelAgentComponent();
	if (!Agent) return;
	PlacementAgent = Agent;
	Agent->ResetPlacementRotation();
	bPlacing = true;
	RefreshPlacement();
}

void UVoxelPrefabTestingInputBinding::OnSecondaryRepeated_Implementation()
{
}

void UVoxelPrefabTestingInputBinding::OnSecondaryReleased_Implementation()
{
	if (bPlacing) RefreshPlacement();
	UVoxelAgentComponent* Agent = PlacementAgent.Get();
	const FVoxelEditIntent Intent = PlacementIntent;
	const bool bSubmit = bPlacementValid && Agent && Agent == GetVoxelAgentComponent();
	CancelPlacement();
	if (bSubmit) Agent->SubmitPlacement(Intent);
}

void UVoxelPrefabTestingInputBinding::OnThirdPressed_Implementation()
{
	if (UVoxelAgentComponent* Agent = PlacementAgent.Get(); bPlacing && Agent)
	{
		bRotatedPlacement = true;
		if (Agent->RotateSelected(1)) RefreshPlacement();
	}
}

void UVoxelPrefabTestingInputBinding::OnThirdRepeated_Implementation()
{
}

void UVoxelPrefabTestingInputBinding::OnThirdReleased_Implementation()
{
	if (bRotatedPlacement)
	{
		bRotatedPlacement = false;
		return;
	}
	if (bPlacing) return;
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->UseTarget();
	}
}

void UVoxelPrefabTestingInputBinding::RefreshPlacement()
{
	bPlacementValid = false;
	UVoxelAgentComponent* Agent = PlacementAgent.Get();
	APlayerController* Controller = GetOwningPlayerController();
	AActor* Source = Agent ? Agent->GetOwner() : nullptr;
	UWorld* World = GetWorld();
	UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
	if (!Agent || Agent != GetVoxelAgentComponent() || !Source || !Viewport || !Viewport->Viewport || !Viewport->Viewport->HasFocus())
	{
		CancelPlacement();
		return;
	}
	UVoxelModule* Voxel = UVoxelModule::GetPtr(World);
	const UInventoryModule* Inventory = UInventoryModule::GetPtr(World);
	FVoxelEditIntent Intent;
	if (!Voxel || !Voxel->IsReady() || !Inventory || !Agent->CapturePlacement(Intent))
	{
		if (PlacementPreview.IsValid()) PlacementPreview->SetActorHiddenInGame(true);
		return;
	}
	const FInventoryItemDefinition* Item = Inventory->GetRegistry().Find(Intent.ExpectedItemID);
	const auto Registry = Voxel->GetRegistry();
	const FVoxelRuntimeDefinition* Block = Item && Registry ? Registry->Find(Item->PlaceableVoxelId) : nullptr;
	if (!Block) { CancelPlacement(); return; }
	if (PlacementIntent.ExpectedItemID.IsValid() && !PlacementIntent.HasSamePlacementSource(Intent))
	{
		CancelPlacement();
		return;
	}
	if (!PlacementPreview.IsValid())
	{
		FActorSpawnParameters Parameters;
		Parameters.Owner = Source;
		Parameters.ObjectFlags |= RF_Transient;
		PlacementPreview = World->SpawnActor<AScenePlacementPreview>(AScenePlacementPreview::StaticClass(), FTransform::Identity, Parameters);
	}
	if (!PlacementPreview.IsValid()) return;
	PlacementIntent = Intent;
	EVoxelEditCode Result;
	FString Error;
	bPlacementValid = PlacementPreview->PreviewVoxelPlacement(*Voxel, *Block, *Source, Intent, Result, Error);
}

void UVoxelPrefabTestingInputBinding::CancelPlacement()
{
	if (AScenePlacementPreview* Current = PlacementPreview.Get()) Current->Destroy();
	PlacementPreview.Reset();
	PlacementAgent.Reset();
	PlacementIntent = FVoxelEditIntent();
	bPlacing = false;
	bPlacementValid = false;
}

void UVoxelPrefabTestingInputBinding::ToggleSceneAI()
{
	if (UVoxelPrefabTestingModule* Testing = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld())))
	{
		Testing->ToggleAIPanel(GetOwningPlayerController());
	}
}
void UVoxelPrefabTestingInputBinding::SwitchView()
{
	APlayerController* Player = ResolveInputPlayer(BoundInputComponent);
	UCharacterModule* Characters = Player ? UCharacterModule::GetPtr(Player->GetWorld()) : nullptr;
	if (!Characters) return;
	if (Characters->GetCurrentCharacter(Player))
	{
		UCharacterModuleStatics::SwitchCharacter(Player, nullptr);
	}
	else
	{
		const TArray<ACharacterBase*> Available = Characters->GetAllCharacter();
		ACharacterBase* Character = nullptr;
		for (ACharacterBase* Candidate : Available)
		{
			if (IsValid(Candidate) && Candidate->GetOwner() == Player && !Candidate->IsPlayerControlled()) { Character = Candidate; break; }
		}
		if (!Character) return;
		UCharacterModuleStatics::SwitchCharacter(Player, Character);
	}
}

UVoxelAgentComponent* UVoxelPrefabTestingInputBinding::GetVoxelAgentComponent() const
{
	UVoxelPrefabTestingModule* Testing = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld()));
	return Testing ? Testing->GetPlacementAgent(GetOwningPlayerController()) : nullptr;
}

namespace GameplayTags
{
////////////////////////////////////////////////////
// Input_Player
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_SwitchView, "Input.Player.SwitchView", "Switch View");
}
