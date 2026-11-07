// Fill out your copyright notice in the Description page of Project Settings.

#include "Voxel/Input/VoxelInputBinding.h"

#include "Camera/CameraModuleStatics.h"
#include "Character/CharacterModuleStatics.h"
#include "Character/CharacterModule.h"
#include "Character/Base/CharacterBase.h"
#include "Common/CommonModuleStatics.h"
#include "Input/InputModuleStatics.h"
#include "Input/InputModule.h"
#include "Input/Components/InputComponentBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Voxel/VoxelModuleStatics.h"
#include "Voxel/Agent/VoxelAgentComponent.h"
#include "Voxel/Widget/WidgetVoxelInventory.h"
#include "Widget/WidgetModuleStatics.h"
#include "Widget/WidgetModule.h"

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
UVoxelInputBinding::UVoxelInputBinding()
{
	InteractionDistance = 1000.f;
}

void UVoxelInputBinding::OnBindInput(UInputComponentBase* InInputComponent)
{
	Super::OnBindInput(InInputComponent);

	InInputComponent->BindInputAction(GameplayTags::Input_PrevInventoryItem, ETriggerEvent::Started, this, &UVoxelInputBinding::PrevInventoryItem);
	InInputComponent->BindInputAction(GameplayTags::Input_NextInventoryItem, ETriggerEvent::Started, this, &UVoxelInputBinding::NextInventoryItem);
	InInputComponent->BindInputAction(GameplayTags::Input_SwitchView, ETriggerEvent::Started, this, &UVoxelInputBinding::SwitchView);
}

void UVoxelInputBinding::SystemOperation_Implementation()
{
	Super::SystemOperation_Implementation();

	APlayerController* Player = ResolveInputPlayer(BoundInputComponent);
	if (UInputModule* Input = Player ? UInputModule::GetPtr(Player->GetWorld()) : nullptr)
	{
		Input->SetPlayerDefaultInputMode(Player, EInputMode::UIOnly);
	}
}

void UVoxelInputBinding::OnPrimaryPressed_Implementation()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->BeginBreak();
	}
}

void UVoxelInputBinding::OnPrimaryRepeated_Implementation()
{
}

void UVoxelInputBinding::OnPrimaryReleased_Implementation()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->EndBreak();
	}
}

void UVoxelInputBinding::OnSecondaryPressed_Implementation()
{
}

void UVoxelInputBinding::OnSecondaryRepeated_Implementation()
{
}

void UVoxelInputBinding::OnSecondaryReleased_Implementation()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->PlaceSelected();
	}
}

void UVoxelInputBinding::OnThirdPressed_Implementation()
{
}

void UVoxelInputBinding::OnThirdRepeated_Implementation()
{
}

void UVoxelInputBinding::OnThirdReleased_Implementation()
{
	if (UVoxelAgentComponent* Agent = GetVoxelAgentComponent())
	{
		Agent->UseTarget();
	}
}

void UVoxelInputBinding::PrevInventoryItem()
{
	APlayerController* Player = ResolveInputPlayer(BoundInputComponent);
	const UWidgetModule* Widgets = Player ? UWidgetModule::GetPtr(Player->GetWorld()) : nullptr;
	UWidgetVoxelInventory* Inventory = Widgets ? Widgets->GetUserWidget<UWidgetVoxelInventory>(Player) : nullptr;
	if (Inventory && Inventory->GetOwningPlayer() == Player) Inventory->PrevInventoryItem();
}

void UVoxelInputBinding::NextInventoryItem()
{
	APlayerController* Player = ResolveInputPlayer(BoundInputComponent);
	const UWidgetModule* Widgets = Player ? UWidgetModule::GetPtr(Player->GetWorld()) : nullptr;
	UWidgetVoxelInventory* Inventory = Widgets ? Widgets->GetUserWidget<UWidgetVoxelInventory>(Player) : nullptr;
	if (Inventory && Inventory->GetOwningPlayer() == Player) Inventory->NextInventoryItem();
}

void UVoxelInputBinding::SwitchView()
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

UVoxelAgentComponent* UVoxelInputBinding::GetVoxelAgentComponent() const
{
	AActor* Owner = BoundInputComponent ? BoundInputComponent->GetOwner() : nullptr;
	const APlayerController* Player = Cast<APlayerController>(Owner);
	ACharacterBase* Character = Player ? Cast<ACharacterBase>(Player->GetPawn()) : Cast<ACharacterBase>(Owner);
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
