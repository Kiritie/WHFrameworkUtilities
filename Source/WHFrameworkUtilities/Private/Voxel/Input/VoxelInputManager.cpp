// Fill out your copyright notice in the Description page of Project Settings.

#include "Voxel/Input/VoxelInputManager.h"

#include "Camera/CameraModuleStatics.h"
#include "Character/CharacterModuleStatics.h"
#include "Character/Base/CharacterBase.h"
#include "Common/CommonModuleStatics.h"
#include "Input/InputModuleStatics.h"
#include "Input/Components/InputComponentBase.h"
#include "Voxel/VoxelModuleStatics.h"
#include "Voxel/Agent/VoxelAgentComponent.h"
#include "Voxel/Widget/WidgetVoxelInventory.h"
#include "Widget/WidgetModuleStatics.h"

// ParamSets default values
UVoxelInputManager::UVoxelInputManager()
{
	InteractionDistance = 1000.f;
}

void UVoxelInputManager::OnInitialize(int32 InPlayerIndex)
{
	Super::OnInitialize(InPlayerIndex);
}

void UVoxelInputManager::OnBindInput(UInputComponentBase* InInputComponent)
{
	Super::OnBindInput(InInputComponent);

	InInputComponent->BindInputAction(GameplayTags::Input_PrevInventoryItem, ETriggerEvent::Started, this, &UVoxelInputManager::PrevInventoryItem);
	InInputComponent->BindInputAction(GameplayTags::Input_NextInventoryItem, ETriggerEvent::Started, this, &UVoxelInputManager::NextInventoryItem);
	InInputComponent->BindInputAction(GameplayTags::Input_SwitchView, ETriggerEvent::Started, this, &UVoxelInputManager::SwitchView);
}

void UVoxelInputManager::SystemOperation_Implementation()
{
	Super::SystemOperation_Implementation();

	UInputModuleStatics::SetDefaultInputMode(EInputMode::UIOnly);
}

void UVoxelInputManager::OnPrimaryPressed_Implementation()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->BeginBreak();
	}
}

void UVoxelInputManager::OnPrimaryRepeated_Implementation()
{
}

void UVoxelInputManager::OnPrimaryReleased_Implementation()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->EndBreak();
	}
}

void UVoxelInputManager::OnSecondaryPressed_Implementation()
{
}

void UVoxelInputManager::OnSecondaryRepeated_Implementation()
{
}

void UVoxelInputManager::OnSecondaryReleased_Implementation()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->PlaceSelected();
	}
}

void UVoxelInputManager::OnThirdPressed_Implementation()
{
}

void UVoxelInputManager::OnThirdRepeated_Implementation()
{
}

void UVoxelInputManager::OnThirdReleased_Implementation()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->UseTarget();
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
		const FTransform ViewTransform = UCameraModuleStatics::GetViewTransform();
		Character->SetActorLocationAndRotation(ViewTransform.GetLocation(), FRotator(0.f, ViewTransform.Rotator().Yaw, 0.f));
		UCharacterModuleStatics::SwitchCharacter(Character);
	}
}

UVoxelAgentComponent* UVoxelInputManager::GetVoxelAgentComponent() const
{
	ACharacterBase* Character = UCharacterModuleStatics::GetCurrentCharacter();
	return Character ? Character->GetVoxelAgentComponent() : nullptr;
}

namespace GameplayTags
{
////////////////////////////////////////////////////
// Input_Player
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_PrevInventoryItem, "Input.Player.PrevInventoryItem", "Prev Inventory Item");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_NextInventoryItem, "Input.Player.NextInventoryItem", "Next Inventory Item");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_SwitchView, "Input.Player.SwitchView", "Switch View");
}
