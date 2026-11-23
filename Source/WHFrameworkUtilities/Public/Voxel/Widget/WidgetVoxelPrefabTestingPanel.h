#pragma once

#include "Testing/Widget/WidgetTestingPanel.h"

#include "WidgetVoxelPrefabTestingPanel.generated.h"

class UCommonButton;
class UVoxelPrefabData;
class FVoxelPrefabEditCapture;
class UCommonInput;
class UCommonTextBlockN;
class UHorizontalBox;

/** 体素预制体控制面板 */
UCLASS()
class WHFRAMEWORKUTILITIES_API UWidgetVoxelPrefabTestingPanel : public UWidgetTestingPanel
{
	GENERATED_BODY()

public:
	UWidgetVoxelPrefabTestingPanel(const FObjectInitializer& ObjectInitializer);

public:
	virtual void OnCreate(const FParameter& InParam) override;
	virtual void OnOpen(const FParameter& InParam, bool bInstant) override;
	virtual void OnClose(bool bInstant = false) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Prefab")
	void LoadData();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Prefab")
	void SaveData();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Prefab")
	void ResetData();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Catalog")
	void SetCatalogEntries(const TArray<FToolCatalogEntry>& InEntries);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Catalog")
	void SetAdditionalCatalogEntries(const TArray<FToolCatalogEntry>& InEntries);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Catalog")
	void RefreshCatalog();

	UFUNCTION(BlueprintPure, Category = "Voxel|Catalog")
	bool GetSelectedCatalogEntry(FToolCatalogEntry& OutEntry) const;

	UPROPERTY(BlueprintAssignable, Category = "Voxel|Catalog")
	FToolCatalogSelectionChanged OnCatalogSelectionChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget, OptionalWidget = false))
	UCommonButton* Btn_LoadData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget, OptionalWidget = false))
	UCommonButton* Btn_SaveData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget, OptionalWidget = false))
	UCommonButton* Btn_ResetData;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlockN> Text_CatalogStatus;

protected:
	virtual void HandleCatalogSelection(const FToolCatalogEntry& InEntry) override;
	virtual void HandleAddToInventory(const FToolCatalogEntry& InEntry) override;
	virtual void HandleCreate(const FToolCatalogEntry& InEntry) override;

private:
	UPROPERTY(Transient)
	TArray<FToolCatalogEntry> AdditionalCatalogEntries;

private:
	UPROPERTY(Transient)
	UVoxelPrefabData* CurrentPrefabAsset;

#if WITH_EDITOR
private:
	bool CanAccessPrefabWorld() const;

	void CapturePrefab(const FString& NewPackageName);

	void ShowNotification(const FText& InText) const;

	TSharedPtr<FVoxelPrefabEditCapture, ESPMode::ThreadSafe> PrefabCapture;
#endif
};
