#include "Testing/TestingInputBinding.h"

#include "Input/Components/InputComponentBase.h"
#include "Input/InputModule.h"
#include "Input/InputTags.h"
#include "Testing/TestingModule.h"

UTestingInputBinding::UTestingInputBinding()
{
	InputBindingName = TEXT("Testing");
}

void UTestingInputBinding::OnBindInput(UInputComponentBase* InInputComponent)
{
	Super::OnBindInput(InInputComponent);
	if (InInputComponent)
	{
		UTestingModule* Testing = UTestingModule::GetPtr(GetWorld());
		if (Testing && Testing->UsesPlacementList())
		{
			InInputComponent->BindInputAction(FGameplayTag::RequestGameplayTag(TEXT("Input.Player.PrevInventoryItem")), ETriggerEvent::Started, this, &ThisClass::PreviousPlacement);
			InInputComponent->BindInputAction(FGameplayTag::RequestGameplayTag(TEXT("Input.Player.NextInventoryItem")), ETriggerEvent::Started, this, &ThisClass::NextPlacement);
		}
	}
}

void UTestingInputBinding::PreviousPlacement()
{
	CyclePlacement(-1);
}

void UTestingInputBinding::NextPlacement()
{
	CyclePlacement(1);
}

void UTestingInputBinding::CyclePlacement(int32 InDirection)
{
	const UInputModule* Input = UInputModule::GetPtr(GetWorld());
	const int32 PlayerIndex = Input ? Input->FindPlayerRuntimeIndex(GetOwningPlayerController()) : INDEX_NONE;
	if (Input && Input->IsInputActionActive(GameplayTags::Input_CameraZoomModifier, PlayerIndex)) return;
	if (UTestingModule* Testing = UTestingModule::GetPtr(GetWorld()))
	{
		Testing->CyclePlacement(GetOwningPlayerController(), InDirection);
	}
}

void UTestingInputBinding::OnUnbindInput()
{
	Super::OnUnbindInput();
}
