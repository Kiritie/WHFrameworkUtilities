#include "Widget/Setting/WidgetSettingPanel.h"

#include "Widget/Common/WidgetPopupWindow.h"
#include "Setting/SettingModule.h"
#include "Widget/WidgetModuleStatics.h"

UWidgetSettingPanel::UWidgetSettingPanel(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UWidgetSettingPanel::OnClose(bool bInstant)
{
	ReleaseEditSession();
	DismissConfirmation();
	Super::OnClose(bInstant);
}

void UWidgetSettingPanel::OnDestroy(EObjectDespawnMode InMode)
{
	ReleaseEditSession();
	DismissConfirmation();
	Super::OnDestroy(InMode);
}

void UWidgetSettingPanel::NativeDestruct()
{
	DismissConfirmation();
	Super::NativeDestruct();
}

void UWidgetSettingPanel::DismissConfirmation()
{
	if (ConfirmationWindow)
	{
		ConfirmationWindow->Dismiss();
		ConfirmationWindow = nullptr;
	}
}

void UWidgetSettingPanel::RefreshConfirmation()
{
	Super::RefreshConfirmation();
	const USettingModule* Settings = GetSettingSource();
	const bool bPending = Settings && Settings->HasPendingConfirmation();
	if (!bPending)
	{
		DismissConfirmation();
		return;
	}
	const FText Message = FText::Format(NSLOCTEXT("WH.Setting", "ConfirmSettings", "是否保留这些设置？{0} 秒后自动还原。"),
		FText::AsNumber(FMath::CeilToInt(Settings->GetConfirmationTimeRemaining())));
	if (!ConfirmationWindow)
	{
		FWidgetPopupWindowOpenParameter Parameter;
		Parameter.Title = NSLOCTEXT("WH.Setting", "WindowTitle", "确认设置");
		Parameter.Text = Message;
		Parameter.ConfirmText = NSLOCTEXT("WH.Setting", "KeepSettings", "保留");
		Parameter.CancelText = NSLOCTEXT("WH.Setting", "RevertSettings", "还原");
		Parameter.ConfirmTrigger.BindDynamic(this, &ThisClass::OnConfirmed);
		Parameter.CancelTrigger.BindDynamic(this, &ThisClass::OnCancelled);
		Parameter.OwnerObject = this;
		const FScopedWorldContext WorldContext(GetWorld());
		const TSubclassOf<UUserWidgetBase> ConfiguredClass = PopupWindowClass ? PopupWindowClass.Get() : UWidgetPopupWindow::StaticClass();
		ConfirmationWindow = UWidgetModuleStatics::OpenUserWidget(ConfiguredClass, FParameter(Parameter), true, true)
			? Cast<UWidgetPopupWindow>(UWidgetModuleStatics::GetUserWidget(ConfiguredClass)) : nullptr;
		if (!ConfirmationWindow)
		{
			RejectPendingSettings();
		}
	}
	else
	{
		ConfirmationWindow->SetText(Message);
	}
}

void UWidgetSettingPanel::OnConfirmed()
{
	ConfirmationWindow = nullptr;
	ConfirmPendingSettings();
	RefreshConfirmation();
}

void UWidgetSettingPanel::OnCancelled()
{
	ConfirmationWindow = nullptr;
	RejectPendingSettings();
	RefreshConfirmation();
}
