#pragma once

#include "Widget/Screen/UserWidgetBase.h"
#include "WidgetPopupWindow.generated.h"

class UCommonWindow;
class UCommonButton;
class UCommonTextBlockN;
class UPanelWidget;
class UWidgetPopupWindow;

USTRUCT(BlueprintType)
struct WHFRAMEWORKUTILITIES_API FWidgetPopupWindowOpenParameter : public FWidgetOpenParameter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> ContentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ConfirmText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CancelText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCancel = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSimpleDynamicDelegate ConfirmTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSimpleDynamicDelegate CancelTrigger;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWidgetPopupWindowResult, UWidgetPopupWindow*, InWindow, bool, bConfirmed);

/** 使用基础窗体承载内容与确认、取消操作 */
UCLASS(BlueprintType)
class WHFRAMEWORKUTILITIES_API UWidgetPopupWindow : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	UWidgetPopupWindow(const FObjectInitializer& ObjectInitializer);

	virtual void OnOpen(const FParameter& InParam, bool bInstant) override;
	virtual void OnClose(bool bInstant) override;
	virtual void OnDestroy(EObjectDespawnMode InMode) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable)
	void Dismiss();

	UFUNCTION(BlueprintCallable)
	void SetText(const FText& InText);

	UFUNCTION(BlueprintCallable)
	void SetConfirmEnabled(bool bInEnabled);

	UFUNCTION(BlueprintPure)
	UUserWidget* GetContentWidget() const;

	UPROPERTY(BlueprintAssignable)
	FWidgetPopupWindowResult OnResult;

protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonWindow> Window_Content;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlockN> Txt_Content;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PopupContentHost;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButton> Btn_Confirm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButton> Btn_Cancel;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ContentWidget;

	UPROPERTY(Transient)
	FSimpleDynamicDelegate ConfirmTrigger;

	UPROPERTY(Transient)
	FSimpleDynamicDelegate CancelTrigger;

private:
	void Confirm();
	UFUNCTION()
	void Cancel();
	void Resolve(bool bConfirmed);
	bool bResolved = true;
};
