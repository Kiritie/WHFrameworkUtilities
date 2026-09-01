#pragma once

#include "Widget/Screen/UserWidgetBase.h"

#include "WidgetVoxelControlPanel.generated.h"

class UCommonButton;
class UVoxelPrefabData;

/** 体素预制体控制面板 */
UCLASS()
class WHFRAMEWORKUTILITIES_API UWidgetVoxelControlPanel : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	UWidgetVoxelControlPanel(const FObjectInitializer& ObjectInitializer);

public:
	virtual void OnCreate(UObject* InOwner, const TArray<FParameter>& InParams) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Prefab")
	void LoadData();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Prefab")
	void SaveData();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Prefab")
	void ResetData();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget, OptionalWidget = false))
	UCommonButton* Btn_LoadData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget, OptionalWidget = false))
	UCommonButton* Btn_SaveData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget, OptionalWidget = false))
	UCommonButton* Btn_ResetData;

private:
	UPROPERTY(Transient)
	UVoxelPrefabData* CurrentPrefabAsset;

#if WITH_EDITOR
private:
	bool CanAccessPrefabWorld() const;

	void ShowNotification(const FText& InText, bool bSuccess) const;
#endif
};
