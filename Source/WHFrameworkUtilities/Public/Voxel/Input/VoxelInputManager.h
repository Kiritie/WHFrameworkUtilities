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
	UFUNCTION()
	virtual void OnPrimaryPressed();

	UFUNCTION()
	virtual void OnPrimaryRepeated();

	UFUNCTION()
	virtual void OnPrimaryReleased();

	UFUNCTION()
	virtual void OnSecondaryPressed();

	UFUNCTION()
	virtual void OnSecondaryRepeated();

	UFUNCTION()
	virtual void OnSecondaryReleased();

	UFUNCTION()
	virtual void PrevInventoryItem();

	UFUNCTION()
	virtual void NextInventoryItem();
};

namespace GameplayTags
{
	////////////////////////////////////////////////////
	// Input_Player
	WHFRAMEWORKUTILITIES_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Primary);
	WHFRAMEWORKUTILITIES_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Secondary);
	
	WHFRAMEWORKUTILITIES_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_PrevInventoryItem);
	WHFRAMEWORKUTILITIES_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_NextInventoryItem);
};
