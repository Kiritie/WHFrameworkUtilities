#include "Voxel/Widget/WidgetVoxelControlPanel.h"

#include "Camera/CameraModule.h"
#include "Camera/CameraModuleStatics.h"
#include "Camera/Manager/CameraManagerBase.h"
#include "Character/Base/CharacterBase.h"
#include "Character/CharacterModuleStatics.h"
#include "Voxel/Prefabs/Data/VoxelPrefabData.h"
#include "Voxel/VoxelModule.h"
#include "Voxel/VoxelModuleStatics.h"
#include "Widget/Common/CommonButton.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "Framework/Notifications/NotificationManager.h"
#include "IContentBrowserSingleton.h"
#include "Misc/PackageName.h"
#include "Styling/AppStyle.h"
#include "UObject/SavePackage.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif

#define LOCTEXT_NAMESPACE "WidgetVoxelControlPanel"

#if WITH_EDITOR
const FString DefaultVoxelPrefabPath = TEXT("/WHFramework/Voxel/DataAssets/Prefab");
#endif

UWidgetVoxelControlPanel::UWidgetVoxelControlPanel(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , Btn_LoadData(nullptr)
    , Btn_SaveData(nullptr)
    , Btn_ResetData(nullptr)
    , CurrentPrefabAsset(nullptr)
{
}

void UWidgetVoxelControlPanel::OnCreate(const FParameter& InParam)
{
	Super::OnCreate(InParam);

	if (Btn_LoadData)
	{
		Btn_LoadData->OnClicked().AddUObject(this, &UWidgetVoxelControlPanel::LoadData);
	}
	if (Btn_SaveData)
	{
		Btn_SaveData->OnClicked().AddUObject(this, &UWidgetVoxelControlPanel::SaveData);
	}
	if (Btn_ResetData)
	{
		Btn_ResetData->OnClicked().AddUObject(this, &UWidgetVoxelControlPanel::ResetData);
	}
}

void UWidgetVoxelControlPanel::LoadData()
{
#if WITH_EDITOR
	if (!CanAccessPrefabWorld())
	{
		return;
	}

	FOpenAssetDialogConfig OpenAssetConfig;
	OpenAssetConfig.DialogTitleOverride = LOCTEXT("LoadDialogTitle", "Load Voxel Prefab");
	OpenAssetConfig.DefaultPath = DefaultVoxelPrefabPath;
	OpenAssetConfig.AssetClassNames.Add(UVoxelPrefabData::StaticClass()->GetClassPathName());
	OpenAssetConfig.bAllowMultipleSelection = false;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	const TArray<FAssetData> SelectedAssets = ContentBrowserModule.Get().CreateModalOpenAssetDialog(OpenAssetConfig);
	if (SelectedAssets.IsEmpty())
	{
		return;
	}

	UVoxelPrefabData* PrefabAsset = Cast<UVoxelPrefabData>(SelectedAssets[0].GetAsset());
	FString Error;
	if (!IsValid(PrefabAsset) || !UVoxelModule::Get().ApplyPrefab(PrefabAsset->Data, FIntVector::ZeroValue, Error))
	{
		ShowNotification(FText::FromString(Error.IsEmpty() ? TEXT("无法加载 VoxelPrefab。") : Error), false);
		return;
	}

	CurrentPrefabAsset = PrefabAsset;
	ShowNotification(FText::Format(LOCTEXT("LoadSucceeded", "已加载 VoxelPrefab：{0}"), FText::FromString(PrefabAsset->GetName())), true);
#endif
}

void UWidgetVoxelControlPanel::SaveData()
{
#if WITH_EDITOR
	if (!CanAccessPrefabWorld() || !IsValid(CurrentPrefabAsset))
	{
		return;
	}

	const FBox Bounds = CurrentPrefabAsset->GetVoxelBounds();
	FVoxelPrefabSaveData PrefabData;
	FString Error;
	if (!Bounds.IsValid || !UVoxelModule::Get().ExportPrefab(FIntVector(Bounds.Min), FIntVector(Bounds.Max) - FIntVector(1, 1, 1), PrefabData, Error))
	{
		ShowNotification(FText::FromString(Error.IsEmpty() ? TEXT("无法导出 VoxelPrefab。") : Error), false);
		return;
	}

	CurrentPrefabAsset->Modify();
	CurrentPrefabAsset->Data = MoveTemp(PrefabData);
	CurrentPrefabAsset->PostEditChange();

	UPackage* Package = CurrentPrefabAsset->GetOutermost();
	Package->MarkPackageDirty();
	Package->GetMetaData();

	const FString PackageFilename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	if (!UPackage::SavePackage(Package, nullptr, *PackageFilename, SaveArgs) || Package->IsDirty())
	{
		ShowNotification(LOCTEXT("SaveFailed", "VoxelPrefab 数据已更新，但资产未能保存到磁盘。"), false);
		return;
	}

	ShowNotification(FText::Format(LOCTEXT("SaveSucceeded", "已更新 VoxelPrefab：{0}"), FText::FromString(CurrentPrefabAsset->GetName())), true);
#endif
}

void UWidgetVoxelControlPanel::ResetData()
{
	CurrentPrefabAsset = nullptr;
}

#if WITH_EDITOR
bool UWidgetVoxelControlPanel::CanAccessPrefabWorld() const
{
	if (!UVoxelModule::IsValid() || !UVoxelModule::Get().IsReady())
	{
		ShowNotification(LOCTEXT("WorldNotReady", "Prefab 世界尚未准备完成。"), false);
		return false;
	}
	return true;
}

void UWidgetVoxelControlPanel::ShowNotification(const FText& InText, bool bSuccess) const
{
	FNotificationInfo NotificationInfo(InText);
	NotificationInfo.ExpireDuration = 4.f;
	NotificationInfo.Image = FAppStyle::Get().GetBrush(bSuccess ? "Icons.SuccessWithColor" : "Icons.ErrorWithColor");
	FSlateNotificationManager::Get().AddNotification(NotificationInfo);
}
#endif

#undef LOCTEXT_NAMESPACE
