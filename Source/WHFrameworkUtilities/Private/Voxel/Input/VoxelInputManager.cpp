// Fill out your copyright notice in the Description page of Project Settings.

#include "Voxel/Input/VoxelInputManager.h"

#include "Camera/CameraModuleStatics.h"
#include "Camera/Actor/RoamCameraActor.h"
#include "Character/CharacterModuleStatics.h"
#include "Character/Base/CharacterBase.h"
#include "Common/CommonModuleStatics.h"
#include "Input/InputModuleStatics.h"
#include "Input/Components/InputComponentBase.h"
#include "Voxel/VoxelModuleStatics.h"
#include "Voxel/Widget/WidgetVoxelInventory.h"
#include "Widget/WidgetModuleStatics.h"

// ParamSets default values
UVoxelInputManager::UVoxelInputManager()
{
	InteractionDistance = 1000.f;
}

void UVoxelInputManager::OnInitialize()
{
	Super::OnInitialize();
}

void UVoxelInputManager::OnBindAction(UInputComponentBase* InInputComponent)
{
	Super::OnBindAction(InInputComponent);

	InInputComponent->BindInputAction(GameplayTags::Input_PrevInventoryItem, ETriggerEvent::Started, this, &UVoxelInputManager::PrevInventoryItem);
	InInputComponent->BindInputAction(GameplayTags::Input_NextInventoryItem, ETriggerEvent::Started, this, &UVoxelInputManager::NextInventoryItem);
	InInputComponent->BindInputAction(GameplayTags::Input_SwitchView, ETriggerEvent::Started, this, &UVoxelInputManager::SwitchView);
}

void UVoxelInputManager::SystemOperation_Implementation()
{
	Super::SystemOperation_Implementation();

	UInputModuleStatics::SetNativeInputMode(EInputMode::UIOnly);
}

void UVoxelInputManager::OnPrimaryPressed_Implementation()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();
	
	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, InteractionDistance, {}, VoxelHitResult))
	{
		RoamCamera->InteractVoxel(EInputInteractAction::Primary, EInputInteractEvent::Started, VoxelHitResult);
	}
}

void UVoxelInputManager::OnPrimaryRepeated_Implementation()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();
	
	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, InteractionDistance, {}, VoxelHitResult))
	{
		RoamCamera->InteractVoxel(EInputInteractAction::Primary, EInputInteractEvent::Triggered, VoxelHitResult);
	}
}

void UVoxelInputManager::OnPrimaryReleased_Implementation()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();
	
	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, InteractionDistance, {}, VoxelHitResult))
	{
		RoamCamera->InteractVoxel(EInputInteractAction::Primary, EInputInteractEvent::Completed, VoxelHitResult);
	}
	else
	{
		RoamCamera->UnInteractVoxel(EInputInteractAction::Primary, EInputInteractEvent::Completed);
	}
}

void UVoxelInputManager::OnSecondaryPressed_Implementation()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();

	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, InteractionDistance, {}, VoxelHitResult))
	{
		RoamCamera->InteractVoxel(EInputInteractAction::Secondary, EInputInteractEvent::Started, VoxelHitResult);
	}
}

void UVoxelInputManager::OnSecondaryRepeated_Implementation()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();

	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, InteractionDistance, {}, VoxelHitResult))
	{
		RoamCamera->InteractVoxel(EInputInteractAction::Secondary, EInputInteractEvent::Triggered, VoxelHitResult);
	}
}

void UVoxelInputManager::OnSecondaryReleased_Implementation()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();

	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, InteractionDistance, {}, VoxelHitResult))
	{
		RoamCamera->InteractVoxel(EInputInteractAction::Secondary, EInputInteractEvent::Completed, VoxelHitResult);
	}
	else
	{
		RoamCamera->UnInteractVoxel(EInputInteractAction::Secondary, EInputInteractEvent::Completed);
	}
}

void UVoxelInputManager::OnThirdPressed_Implementation()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();

	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, InteractionDistance, {}, VoxelHitResult))
	{
		RoamCamera->InteractVoxel(EInputInteractAction::Third, EInputInteractEvent::Started, VoxelHitResult);
	}
}

void UVoxelInputManager::OnThirdRepeated_Implementation()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();

	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, InteractionDistance, {}, VoxelHitResult))
	{
		RoamCamera->InteractVoxel(EInputInteractAction::Third, EInputInteractEvent::Triggered, VoxelHitResult);
	}
}

void UVoxelInputManager::OnThirdReleased_Implementation()
{
	ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>();

	if(!RoamCamera) return;

	FVoxelHitResult VoxelHitResult;
	if(UVoxelModuleStatics::VoxelRaycastSinge(EVoxelRaycastType::FromAimPoint, InteractionDistance, {}, VoxelHitResult))
	{
		RoamCamera->InteractVoxel(EInputInteractAction::Third, EInputInteractEvent::Completed, VoxelHitResult);
	}
	else
	{
		RoamCamera->UnInteractVoxel(EInputInteractAction::Third, EInputInteractEvent::Completed);
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

void UVoxelInputManager::SwitchView()
{
	if (UCharacterModuleStatics::GetCurrentCharacter())
	{
		UCharacterModuleStatics::SwitchCharacter(nullptr);
	}
	else if (UCharacterModuleStatics::GetAllCharacter().Num() > 0)
	{
		ACharacterBase* Character = UCharacterModuleStatics::GetAllCharacter()[0];
		Character->SetActorLocationAndRotation(UCameraModuleStatics::GetCameraLocation(), FRotator(0.f, UCameraModuleStatics::GetCameraRotation().Yaw, 0.f));
		UCharacterModuleStatics::SwitchCharacter(Character);
	}
}

namespace GameplayTags
{
	////////////////////////////////////////////////////
	// Input_Player
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_PrevInventoryItem, "Input.Player.PrevInventoryItem", "Prev Inventory Item");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_NextInventoryItem, "Input.Player.NextInventoryItem", "Next Inventory Item");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_SwitchView, "Input.Player.SwitchView", "Switch View");
}
