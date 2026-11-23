#include "Testing/Widget/WidgetTestingKeyHint.h"
#include "Input/InputTags.h"

#include "Input/Widget/WidgetInputKeyTips.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Testing/TestingModule.h"

UWidgetTestingKeyHint::UWidgetTestingKeyHint(const FObjectInitializer& InInitializer)
	: Super(InInitializer)
{
}

void UWidgetTestingKeyHint::OnOpen(const FParameter& InParam, bool bInstant)
{
	const UTestingModule* Testing = UTestingModule::GetPtr(GetWorld());
	if (Testing && !Testing->UsesStandaloneKeyHints())
	{
		Super::OnOpen(InParam, bInstant);
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	FInputKeyTipsPresentation Presentation;
	Presentation.KeyTextStyle = FGameplayTag::RequestGameplayTag(TEXT("UI.Style.Text.Key"));
	Presentation.NameTextStyle = FGameplayTag::RequestGameplayTag(TEXT("UI.Style.Text.Action.Compact"));
	Presentation.KeyBorderStyle = FGameplayTag::RequestGameplayTag(TEXT("UI.Style.Brush.Frame.R4"));
	KeyTip->SetPresentation(Presentation);
	KeyTip->SetActionTags({GameplayTags::Input_SystemOperation});
	KeyTip->SetKeyDisplayName(NSLOCTEXT("Testing", "OpenPanel", "打开工具"));
	if (UVerticalBoxSlot* HintSlot = Cast<UVerticalBoxSlot>(KeyTip->Slot))
	{
		HintSlot->SetHorizontalAlignment(HAlign_Right);
	}
	if (Hints)
	{
		while (Hints->GetChildrenCount() > 1) Hints->RemoveChildAt(Hints->GetChildrenCount() - 1);
		if (Testing)
		{
			for (const FTestingInputHint& Hint : Testing->GetInputHints())
			{
				UWidgetInputKeyTips* Item = CreateWidget<UWidgetInputKeyTips>(GetOwningPlayer(), UWidgetInputKeyTips::StaticClass());
				Item->SetPresentation(Presentation);
				Item->SetActionTags(Hint.Actions);
				Item->SetKeyDisplayName(Hint.Label);
				Hints->AddChildToVerticalBox(Item)->SetHorizontalAlignment(HAlign_Right);
			}
		}
	}
	Super::OnOpen(InParam, bInstant);
}
