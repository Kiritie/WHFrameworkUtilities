// Fill out your copyright notice in the Description page of Project Settings.

#include "Voxel/Widget/WidgetVoxelInventory.h"

#include "Ability/Item/Widget/WidgetAbilityItemBase.h"
#include "Asset/AssetModuleStatics.h"
#include "Camera/CameraModuleStatics.h"
#include "Camera/Actor/RoamCameraActor.h"
#include "Voxel/Voxels/Data/VoxelData.h"

UWidgetVoxelInventory::UWidgetVoxelInventory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WidgetName = FName("VoxelInventory");
	
	WidgetCreateType = EWidgetCreateType::AutoCreateAndOpen;

	WidgetZOrder = 2;

	PreviewItem = nullptr;
	SelectedItemIndex = 0;
}

void UWidgetVoxelInventory::OnCreate(UObject* InOwner, const TArray<FParameter>& InParams)
{
	Super::OnCreate(InOwner, InParams);

	auto VoxelDatas = UAssetModuleStatics::LoadPrimaryAssets<UVoxelData>(FName("Voxel"));
	for (int32 i = 0; i < VoxelDatas.Num(); i++)
	{
		if(!VoxelDatas[i]->IsEmpty() && !VoxelDatas[i]->IsUnknown() && VoxelDatas[i]->IsMainPart())
		{
			VoxelItems.Add(VoxelDatas[i]->VoxelType);
		}
	}
}

void UWidgetVoxelInventory::OnOpen(const TArray<FParameter>& InParams, bool bInstant)
{
	Super::OnOpen(InParams, bInstant);

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
		PreviewItem->Init({ &VoxelItems[InItemIndex] });
	}

	if(ARoamCameraActor* RoamCamera = UCameraModuleStatics::GetCurrentCamera<ARoamCameraActor>())
	{
		RoamCamera->SetGenerateVoxelID(VoxelItems[InItemIndex].ID);
	}
}

FVoxelItem UWidgetVoxelInventory::GetSelectedItem() const
{
	if(VoxelItems.IsValidIndex(SelectedItemIndex))
	{
		return VoxelItems[SelectedItemIndex];
	}
	return FVoxelItem::Empty;
}
