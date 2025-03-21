// Fill out your copyright notice in the Description page of Project Settings.

#include "Voxel/Input/VoxelInputManager.h"

#include "Camera/CameraModuleStatics.h"
#include "Camera/Actor/RoamCameraActor.h"
#include "Common/CommonStatics.h"
#include "Input/Components/InputComponentBase.h"
#include "Voxel/VoxelModuleStatics.h"
#include "Voxel/Voxels/Voxel.h"
#include "Voxel/Widget/WidgetVoxelInventory.h"
#include "Widget/WidgetModuleStatics.h"

// ParamSets default values
UVoxelInputManager::UVoxelInputManager()
{
	
}

void UVoxelInputManager::OnInitialize()
{
	Super::OnInitialize();
}

void UVoxelInputManager::OnBindAction(UInputComponentBase* InInputComponent)
{
	Super::OnBindAction(InInputComponent);

	InInputComponent->BindInputAction(GameplayTags::Input_Primary, ETriggerEvent::Started, this, &UVoxelInputManager::OnPrimaryPressed);
	InInputComponent->BindInputAction(GameplayTags::Input_Primary, ETriggerEvent::Triggered, this, &UVoxelInputManager::OnPrimaryRepeated);
	InInputComponent->BindInputAction(GameplayTags::Input_Primary, ETriggerEvent::Completed, this, &UVoxelInputManager::OnPrimaryReleased);

	InInputComponent->BindInputAction(GameplayTags::Input_Secondary, ETriggerEvent::Started, this, &UVoxelInputManager::OnSecondaryPressed);
	InInputComponent->BindInputAction(GameplayTags::Input_Secondary, ETriggerEvent::Triggered, this, &UVoxelInputManager::OnSecondaryRepeated);
	InInputComponent->BindInputAction(GameplayTags::Input_Secondary, ETriggerEvent::Completed, this, &UVoxelInputManager::OnSecondaryReleased);

	InInputComponent->BindInputAction(GameplayTags::Input_PrevInventoryItem, ETriggerEvent::Started, this, &UVoxelInputManager::PrevInventoryItem);
	InInputComponent->BindInputAction(GameplayTags::Input_NextInventoryItem, ETriggerEvent::Started, this, &UVoxelInputManager::NextInventoryItem);
}

void UVoxelInputManager::OnPrimaryPressed()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();
	
	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, 100000.f, {}, VoxelHitResult))
	{
		VoxelHitResult.GetVoxel().OnAgentInteract(RoamCamera, EInputInteractAction::Primary, EInputInteractEvent::Started, VoxelHitResult);
	}
}

void UVoxelInputManager::OnPrimaryRepeated()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();
	
	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, 100000.f, {}, VoxelHitResult))
	{
		VoxelHitResult.GetVoxel().OnAgentInteract(RoamCamera, EInputInteractAction::Primary, EInputInteractEvent::Triggered, VoxelHitResult);
	}
}

void UVoxelInputManager::OnPrimaryReleased()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();
	
	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, 100000.f, {}, VoxelHitResult))
	{
		VoxelHitResult.GetVoxel().OnAgentInteract(RoamCamera, EInputInteractAction::Primary, EInputInteractEvent::Completed, VoxelHitResult);
	}
}

void UVoxelInputManager::OnSecondaryPressed()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();

	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, 100000.f, {}, VoxelHitResult))
	{
		VoxelHitResult.GetVoxel().OnAgentInteract(RoamCamera, EInputInteractAction::Secondary, EInputInteractEvent::Started, VoxelHitResult);
	}
}

void UVoxelInputManager::OnSecondaryRepeated()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();

	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, 100000.f, {}, VoxelHitResult))
	{
		VoxelHitResult.GetVoxel().OnAgentInteract(RoamCamera, EInputInteractAction::Secondary, EInputInteractEvent::Triggered, VoxelHitResult);
	}
}

void UVoxelInputManager::OnSecondaryReleased()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();

	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, 100000.f, {}, VoxelHitResult))
	{
		VoxelHitResult.GetVoxel().OnAgentInteract(RoamCamera, EInputInteractAction::Secondary, EInputInteractEvent::Completed, VoxelHitResult);
	}
}

void UVoxelInputManager::PrevInventoryItem()
{
	UWidgetModuleStatics::GetUserWidget<UWidgetVoxelInventory>()->PrevInventoryItem();
}

void UVoxelInputManager::NextInventoryItem()
{
	UWidgetModuleStatics::GetUserWidget<UWidgetVoxelInventory>()->NextInventoryItem();
}

namespace GameplayTags
{
	////////////////////////////////////////////////////
	// Input_Player
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Primary, "Input.Player.Primary", "PrimaryAction");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Secondary, "Input.Player.Secondary", "Secondary");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_PrevInventoryItem, "Input.Player.PrevInventoryItem", "Prev Inventory Item");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_NextInventoryItem, "Input.Player.NextInventoryItem", "Next Inventory Item");
}
