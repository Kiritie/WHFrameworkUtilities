#pragma once

#include "Testing/Widget/WidgetTestingCatalogView.h"
#include "Widget/Screen/UserWidgetBase.h"
#include "WidgetTestingPanel.generated.h"

class UCommonWindow;

/** Shared catalog and input behavior for testing panels. */
UCLASS(Abstract)
class WHFRAMEWORKUTILITIES_API UWidgetTestingPanel : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	UWidgetTestingPanel(const FObjectInitializer& InInitializer);
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent) override;

protected:
	virtual void HandleCatalogSelection(const FToolCatalogEntry& InEntry) {}
	virtual void HandleAddToInventory(const FToolCatalogEntry& InEntry) {}
	virtual void HandleCreate(const FToolCatalogEntry& InEntry) {}
	virtual void HandleCategoryChanged(FName InCategory) {}

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonWindow> Window_Content;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetTestingCatalogView> CatalogView;

	UPROPERTY(EditDefaultsOnly, Category = "CatalogView")
	TArray<FToolCatalogCategory> CatalogCategories;

private:
	UFUNCTION()
	void OnCatalogSelection(const FToolCatalogEntry& InEntry);
	UFUNCTION()
	void OnAddToInventory(const FToolCatalogEntry& InEntry);
	UFUNCTION()
	void OnCreateRequested(const FToolCatalogEntry& InEntry);
	UFUNCTION()
	void OnCategoryChanged(FName InCategory);
	UFUNCTION()
	void ClosePanel();
};
