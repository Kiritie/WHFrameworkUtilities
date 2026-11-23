#pragma once

#include "CoreMinimal.h"
#include "Testing/Widget/WidgetTestingCatalogView.h"
#include "Widget/Screen/UserWidgetBase.h"
#include "WidgetTestingInventory.generated.h"

class UCommonTextBlockN;
class UImage;
class UTestingModule;

/** Compact selected-item indicator for a tool HUD. */
UCLASS(Blueprintable)
class WHFRAMEWORKUTILITIES_API UWidgetTestingInventory : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void OnOpen(const FParameter& InParam, bool bInstant) override;
	virtual void OnClose(bool bInstant) override;

	UFUNCTION(BlueprintCallable, Category = "Tool Catalog")
	void SetSelection(const FToolCatalogEntry& InEntry);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlockN> Text_Name;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

private:
	TWeakObjectPtr<UTestingModule> TestingModule;
};
