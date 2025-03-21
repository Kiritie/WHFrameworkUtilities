// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Voxel/VoxelModuleTypes.h"
#include "Widget/Screen/UserWidgetBase.h"

#include "WidgetVoxelInventory.generated.h"

class UWidgetAbilityItemBase;
/**
 * ��Ʒ��
 */
UCLASS()
class WHFRAMEWORKUTILITIES_API UWidgetVoxelInventory : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	UWidgetVoxelInventory(const FObjectInitializer& ObjectInitializer);

public:
	virtual void OnCreate(UObject* InOwner, const TArray<FParameter>& InParams) override;

	virtual void OnOpen(const TArray<FParameter>& InParams, bool bInstant) override;

	virtual void OnClose(bool bInstant) override;

	virtual void OnRefresh() override;

public:
	UFUNCTION(BlueprintCallable)
	void PrevInventoryItem();

	UFUNCTION(BlueprintCallable)
	void NextInventoryItem();

	UFUNCTION(BlueprintCallable)
	void SelectInventoryItem(int32 InItemIndex);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (BindWidget, OptionalWidget = false))
	UWidgetAbilityItemBase* PreviewItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FVoxelItem> VoxelItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 SelectedItemIndex;

public:
	UFUNCTION(BlueprintPure)
	FVoxelItem GetSelectedItem() const;

	UFUNCTION(BlueprintPure)
	int32 GetSelectedItemIndex() const { return SelectedItemIndex; }
};
