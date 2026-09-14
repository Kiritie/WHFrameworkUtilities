// Fill out your copyright notice in the Description page of Project Settings.

#include "Voxel/Widget/WidgetVoxelInventory.h"

#include "Ability/Item/Widget/WidgetAbilityItemBase.h"
#include "Asset/AssetModuleStatics.h"
#include "Voxel/Voxels/Data/VoxelData.h"

UWidgetVoxelInventory::UWidgetVoxelInventory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	

	PreviewItem = nullptr;
	SelectedItemIndex = 0;
}

void UWidgetVoxelInventory::OnCreate(const FParameter& InParam)
{
	Super::OnCreate(InParam);

	auto VoxelDatas = UAssetModuleStatics::LoadPrimaryAssets<UVoxelData>(FName("Voxel"));
	Algo::Sort(VoxelDatas, [](const UVoxelData* A, const UVoxelData* B){
		return A->VoxelType < B->VoxelType;
	});
	for (int32 i = 0; i < VoxelDatas.Num(); i++)
	{
		if(!VoxelDatas[i]->IsEmpty() && !VoxelDatas[i]->IsUnknown() && !VoxelDatas[i]->IsCustom() && VoxelDatas[i]->IsMainPart())
		{
			VoxelItems.Add(VoxelDatas[i]->GetPrimaryAssetId());
		}
	}
}

void UWidgetVoxelInventory::OnOpen(const FParameter& InParam, bool bInstant)
{
	Super::OnOpen(InParam, bInstant);

	SelectInventoryItem(0);
}

void UWidgetVoxelInventory::OnClose(bool bInstant)
{
	Super::OnClose(bInstant);
}

void UWidgetVoxelInventory::OnRefresh()
{
	Super::OnRefresh();
}

void UWidgetVoxelInventory::PrevInventoryItem()
{
	if(SelectedItemIndex > 0)
	{
		SelectInventoryItem(SelectedItemIndex - 1);
	}
	else
	{
		SelectInventoryItem(VoxelItems.Num() - 1);
	}
}

void UWidgetVoxelInventory::NextInventoryItem()
{
	if (SelectedItemIndex < VoxelItems.Num() - 1)
	{
		SelectInventoryItem(SelectedItemIndex + 1);
	}
	else
	{
		SelectInventoryItem(0);
	}
}

void UWidgetVoxelInventory::SelectInventoryItem(int32 InItemIndex)
{
	if(!VoxelItems.IsValidIndex(InItemIndex)) return;

	SelectedItemIndex = InItemIndex;

	if(PreviewItem)
	{
		PreviewItem->Init(FAbilityWidgetSpawnParameter(VoxelItems[InItemIndex]));
	}

}

FAbilityItem UWidgetVoxelInventory::GetSelectedItem() const
{
	if(VoxelItems.IsValidIndex(SelectedItemIndex))
	{
		return VoxelItems[SelectedItemIndex];
	}
	return FAbilityItem::Empty;
}
