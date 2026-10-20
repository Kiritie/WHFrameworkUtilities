#pragma once

#include "Setting/Widget/WidgetSettingPanelBase.h"
#include "WidgetSettingPanel.generated.h"

class UWidgetPopupWindow;

/** 使用通用弹窗确认设置 */
UCLASS()
class WHFRAMEWORKUTILITIES_API UWidgetSettingPanel : public UWidgetSettingPanelBase
{
	GENERATED_BODY()

public:
	UWidgetSettingPanel(const FObjectInitializer& ObjectInitializer);
	virtual void OnClose(bool bInstant) override;
	virtual void OnDestroy(EObjectDespawnMode InMode) override;
	virtual void NativeDestruct() override;

protected:
	virtual void RefreshConfirmation() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UWidgetPopupWindow> PopupWindowClass;

	UPROPERTY(Transient)
	TObjectPtr<UWidgetPopupWindow> ConfirmationWindow;

private:
	void DismissConfirmation();

	UFUNCTION()
	void OnConfirmed();

	UFUNCTION()
	void OnCancelled();
};
