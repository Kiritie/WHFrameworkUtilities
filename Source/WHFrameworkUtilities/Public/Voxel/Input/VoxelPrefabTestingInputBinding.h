// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Input/Manager/DefaultInputBinding.h"
#include "Voxel/VoxelModuleTypes.h"
#include "Voxel/Interaction/VoxelEditTypes.h"

#include "VoxelPrefabTestingInputBinding.generated.h"

UCLASS()
class WHFRAMEWORKUTILITIES_API UVoxelPrefabTestingInputBinding : public UDefaultInputBinding
{
	GENERATED_BODY()

public:
	// ParamSets default values for this actor's properties
	UVoxelPrefabTestingInputBinding();

public:
	virtual void OnBindInput(UInputComponentBase* InInputComponent) override;
	virtual void OnRefresh(float InDeltaSeconds) override;
	virtual void OnReset() override;
	virtual void OnUnbindInput() override;

protected:
	virtual void SystemOperation_Implementation() override;

protected:
	virtual void OnPrimaryPressed_Implementation() override;

	virtual void OnPrimaryRepeated_Implementation() override;

	virtual void OnPrimaryReleased_Implementation() override;

	virtual void OnSecondaryPressed_Implementation() override;

	virtual void OnSecondaryRepeated_Implementation() override;

	virtual void OnSecondaryReleased_Implementation() override;

	virtual void OnThirdPressed_Implementation() override;

	virtual void OnThirdRepeated_Implementation() override;

	virtual void OnThirdReleased_Implementation() override;

	UFUNCTION()
	virtual void SwitchView();

	class UVoxelAgentComponent* GetVoxelAgentComponent() const;
	void ToggleSceneAI();
	void RefreshPlacement();
	void CancelPlacement();

	TWeakObjectPtr<class AScenePlacementPreview> PlacementPreview;
	TWeakObjectPtr<UVoxelAgentComponent> PlacementAgent;
	FVoxelEditIntent PlacementIntent;
	bool bPlacing = false;
	bool bPlacementValid = false;
	bool bRotatedPlacement = false;

};

namespace GameplayTags
{
////////////////////////////////////////////////////
// Input_Player
WHFRAMEWORKUTILITIES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_SwitchView);
};
