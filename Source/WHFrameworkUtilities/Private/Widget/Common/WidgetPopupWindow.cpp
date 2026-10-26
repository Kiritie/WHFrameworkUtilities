#include "Widget/Common/WidgetPopupWindow.h"

#include "Components/PanelWidget.h"
#include "InputCoreTypes.h"
#include "Widget/Common/CommonButton.h"
#include "Widget/Common/CommonTextBlockN.h"
#include "Widget/Common/CommonWindow.h"
#include "Widget/WidgetModule.h"

UWidgetPopupWindow::UWidgetPopupWindow(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DefaultWidgetTag = FGameplayTag::RequestGameplayTag(TEXT("Widget.Screen.PopupWindow"));
	WidgetInputMode = EWidgetInputMode::UIOnly;
	bWidgetActivatable = true;
	bWidgetAutoFocus = true;
	bIsModal = true;
	SetIsFocusable(true);
}


void UWidgetPopupWindow::OnOpen(const FParameter& InParam, bool bInstant)
{
	const FWidgetPopupWindowOpenParameter& Parameter = InParam.GetRef<FWidgetPopupWindowOpenParameter>();
	bResolved = false;
	ConfirmTrigger = Parameter.ConfirmTrigger;
	CancelTrigger = Parameter.CancelTrigger;
	Btn_Confirm->SetIsEnabled(true);
	Btn_Confirm->SetStyleTag(FGameplayTag::RequestGameplayTag(Parameter.bDestructive
		? TEXT("UI.Style.Button.Dialog.Destructive") : TEXT("UI.Style.Button.Dialog.Primary")));
	Btn_Cancel->SetStyleTag(FGameplayTag::RequestGameplayTag(Parameter.bDestructive
		? TEXT("UI.Style.Button.Dialog.Primary") : TEXT("UI.Style.Button.Dialog.Secondary")));
	Window_Content->SetTitle(Parameter.Title);
	SetText(Parameter.Text);
	Btn_Confirm->SetTitle(Parameter.ConfirmText.IsEmpty() ? NSLOCTEXT("WH.Popup", "Confirm", "确认") : Parameter.ConfirmText);
	Btn_Cancel->SetTitle(Parameter.CancelText.IsEmpty() ? NSLOCTEXT("WH.Popup", "Cancel", "取消") : Parameter.CancelText);
	Btn_Cancel->SetVisibility(Parameter.bShowCancel ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	PopupContentHost->ClearChildren();
	ContentWidget = Parameter.ContentClass ? CreateWidget<UUserWidget>(GetOwningPlayer(), Parameter.ContentClass) : nullptr;
	if (ContentWidget)
	{
		PopupContentHost->AddChild(ContentWidget);
	}
	PopupContentHost->SetVisibility(ContentWidget ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	Super::OnOpen(InParam, bInstant);
}

void UWidgetPopupWindow::OnClose(bool bInstant)
{
	if (!bResolved)
	{
		Resolve(false);
		return;
	}
	Super::OnClose(bInstant);
}

void UWidgetPopupWindow::OnDestroy(EObjectDespawnMode InMode)
{
	bResolved = true;
	ConfirmTrigger.Unbind();
	CancelTrigger.Unbind();
	OnResult.Clear();
	ContentWidget = nullptr;
	Super::OnDestroy(InMode);
}

void UWidgetPopupWindow::NativeConstruct()
{
	Super::NativeConstruct();
	Window_Content->OnCloseRequested.AddUniqueDynamic(this, &ThisClass::Cancel);
	Btn_Confirm->OnClicked().RemoveAll(this);
	Btn_Cancel->OnClicked().RemoveAll(this);
	Btn_Confirm->OnClicked().AddUObject(this, &ThisClass::Confirm);
	Btn_Cancel->OnClicked().AddUObject(this, &ThisClass::Cancel);
}

void UWidgetPopupWindow::NativeDestruct()
{
	Window_Content->OnCloseRequested.RemoveDynamic(this, &ThisClass::Cancel);
	Btn_Confirm->OnClicked().RemoveAll(this);
	Btn_Cancel->OnClicked().RemoveAll(this);
	if (!bResolved)
	{
		Resolve(false);
	}
	Super::NativeDestruct();
}

void UWidgetPopupWindow::SetText(const FText& InText)
{
	Txt_Content->SetText(InText);
	Txt_Content->SetVisibility(InText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

void UWidgetPopupWindow::SetConfirmEnabled(bool bInEnabled)
{
	Btn_Confirm->SetIsEnabled(bInEnabled);
}

UUserWidget* UWidgetPopupWindow::GetContentWidget() const
{
	return ContentWidget;
}

void UWidgetPopupWindow::Confirm()
{
	if (Btn_Confirm->GetIsEnabled())
	{
		Resolve(true);
	}
}

void UWidgetPopupWindow::Cancel()
{
	Resolve(false);
}

void UWidgetPopupWindow::Resolve(bool bConfirmed)
{
	const FScopedWorldContext WorldContext(GetWorld());
	if (bResolved)
	{
		return;
	}
	bResolved = true;
	FSimpleDynamicDelegate Trigger = bConfirmed ? ConfirmTrigger : CancelTrigger;
	FWidgetPopupWindowResult Result = OnResult;
	ConfirmTrigger.Unbind();
	CancelTrigger.Unbind();
	OnResult.Clear();
	Close(true);
	Result.Broadcast(this, bConfirmed);
	Trigger.ExecuteIfBound();
}

void UWidgetPopupWindow::Dismiss()
{
	const FScopedWorldContext WorldContext(GetWorld());
	bResolved = true;
	ConfirmTrigger.Unbind();
	CancelTrigger.Unbind();
	OnResult.Clear();
	Close(true);
}

UWidget* UWidgetPopupWindow::NativeGetDesiredFocusTarget() const
{
	return Btn_Confirm;
}

FReply UWidgetPopupWindow::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Right)
	{
		Cancel();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
