#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VoxelSpawnSafetyComponent.generated.h"

class UVoxelModule;
class ACharacter;

UCLASS(ClassGroup = Voxel, meta = (BlueprintSpawnableComponent))
class WHFRAMEWORKUTILITIES_API UVoxelSpawnSafetyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelSpawnSafetyComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type InReason) override;
	virtual void TickComponent(float InDeltaSeconds, ELevelTick InTickType, FActorComponentTickFunction* InTickFunction) override;

private:
	void FailPreparation(const FString& InError);
	void ReleaseSource();
	void RestoreMovement();
	void RefreshMovementSafety(ACharacter& InCharacter, float InDeltaSeconds);
	void PauseMovementTick(ACharacter& InCharacter);
	void RestoreMovementTick(ACharacter& InCharacter);

	TWeakObjectPtr<UVoxelModule> Module;
	FGuid SourceId;
	uint64 SourceEpoch = 0;
	TEnumAsByte<EMovementMode> PreviousMovementMode = MOVE_None;
	uint8 PreviousCustomMovementMode = 0;
	bool bPreviousCollisionEnabled = false;
	bool bPlaced = false;
	bool bFinished = false;
	bool bGuardMovement = false;
	bool bMovementTickPausedForTerrain = false;
};
