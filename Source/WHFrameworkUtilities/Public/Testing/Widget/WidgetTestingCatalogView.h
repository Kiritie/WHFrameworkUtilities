#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Testing/TestingCatalogTypes.h"
#include "Widget/Sub/SubWidgetBase.h"
#include "WidgetTestingCatalogView.generated.h"

class UCommonButton;
class UCommonInput;
class UCommonTextBlockN;
class UImage;
class UHorizontalBox;
class UPanelWidget;
class UScrollBox;

/** Shared catalog presentation; callers provide entries and handle the selected action. */
UCLASS(Blueprintable)
class WHFRAMEWORKUTILITIES_API UWidgetTestingCatalogView : public USubWidgetBase
{
	GENERATED_BODY()

public:
	UWidgetTestingCatalogView(const FObjectInitializer& InInitializer);
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Tool Catalog")
	void SetEntries(const TArray<FToolCatalogEntry>& InEntries);

	UFUNCTION(BlueprintCallable, Category = "Tool Catalog")
	void SetCategory(FName InCategory);

	void SetCategories(const TArray<FToolCatalogCategory>& InCategories);

	UFUNCTION(BlueprintCallable, Category = "Tool Catalog")
	void ClearSelection();

	UFUNCTION(BlueprintPure, Category = "Tool Catalog")
	bool GetSelection(FToolCatalogEntry& OutEntry) const;
	void SetAddToInventoryEnabled(bool bInEnabled);
	void SetCreateEnabled(bool bInEnabled);
	static void ApplyIcon(UImage* InImage, const TSoftObjectPtr<UObject>& InIcon);

	UPROPERTY(BlueprintAssignable, Category = "Tool Catalog")
	FToolCatalogSelectionChanged OnSelectionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tool Catalog")
	FToolCatalogCategoryChanged OnCategoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tool Catalog")
	FToolCatalogSelectionChanged OnAddToInventory;

	UPROPERTY(BlueprintAssignable, Category = "Tool Catalog")
	FToolCatalogSelectionChanged OnCreateRequested;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Catalog")
	TSubclassOf<UCommonButton> EntryButtonClass;

	UPROPERTY(EditDefaultsOnly, Category = "Style", meta = (Categories = "UI.Style.Button"))
	FGameplayTag EntryButtonStyle;

	UPROPERTY(EditDefaultsOnly, Category = "Style", meta = (Categories = "UI.Style.Button"))
	FGameplayTag CategoryButtonStyle;

	UPROPERTY(EditDefaultsOnly, Category = "Style", meta = (Categories = "UI.Style.Text"))
	FGameplayTag EntryTextStyle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonInput> Input_Search;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> Categories;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> List_Entries;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlockN> Text_Selected;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlockN> Text_Description;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Selected;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButton> Btn_AddToInventory;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButton> Btn_Create;

private:
	UFUNCTION()
	void OnSearchChanged(UCommonInput* InInput, const FParameter& InValue);

	void RefreshEntries();
	void SelectEntry(int32 InIndex, UCommonButton* InButton);

	UPROPERTY(Transient)
	TArray<FToolCatalogEntry> Entries;

	FName Category;
	int32 SelectedIndex = INDEX_NONE;
	TWeakObjectPtr<UCommonButton> SelectedButton;
	TMap<FName, TWeakObjectPtr<UCommonButton>> CategoryButtons;
	bool bAddToInventoryEnabled = false;
	bool bCreateEnabled = false;
};
