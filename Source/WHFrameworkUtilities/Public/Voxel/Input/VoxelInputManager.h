// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/Manager/DefaultInputManagerBase.h"
#include "Voxel/VoxelModuleTypes.h"

#include "VoxelInputManager.generated.h"

UCLASS()
class WHFRAMEWORKUTILITIES_API UVoxelInputManager : public UDefaultInputManagerBase
{
	GENERATED_BODY()
	
public:	
	// ParamSets default values for this actor's properties
	UVoxelInputManager();
	
public:
	virtual void OnInitialize() override;

	virtual void OnBindAction(UInputComponentBase* InInputComponent) override;

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
	virtual void PrevInventoryItem();

	UFUNCTION()
	virtual void NextInventoryItem();

	UFUNCTION()
	virtual void SwitchView();

protected:
	UPROPERTY(EditAnywhere)
	float InteractionDistance;
};

namespace GameplayTags
{
	////////////////////////////////////////////////////
	// Input_Player
	WHFRAMEWORKUTILITIES_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_PrevInventoryItem);
	WHFRAMEWORKUTILITIES_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_NextInventoryItem);
	WHFRAMEWORKUTILITIES_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_SwitchView);
};
