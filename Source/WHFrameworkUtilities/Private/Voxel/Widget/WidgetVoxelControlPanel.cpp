#include "Voxel/Widget/WidgetVoxelControlPanel.h"

#include "Camera/Actor/RoamCameraActor.h"
#include "Camera/CameraModuleStatics.h"
#include "Character/Base/CharacterBase.h"
#include "Character/CharacterModuleStatics.h"
#include "Voxel/Prefabs/Data/VoxelPrefabData.h"
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

UWidgetVoxelControlPanel::UWidgetVoxelControlPanel(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WidgetName = FName("VoxelControlPanel");

	Btn_LoadData = nullptr;
	Btn_SaveData = nullptr;
	Btn_ResetData = nullptr;
	CurrentPrefabAsset = nullptr;
}

void UWidgetVoxelControlPanel::OnCreate(UObject* InOwner, const TArray<FParameter>& InParams)
{
	Super::OnCreate(InOwner, InParams);

	if(Btn_LoadData)
	{
		Btn_LoadData->OnClicked().AddUObject(this, &UWidgetVoxelControlPanel::LoadData);
	}
	if(Btn_SaveData)
	{
		Btn_SaveData->OnClicked().AddUObject(this, &UWidgetVoxelControlPanel::SaveData);
	}
	if(Btn_ResetData)
	{
		Btn_ResetData->OnClicked().AddUObject(this, &UWidgetVoxelControlPanel::ResetData);
	}
}

void UWidgetVoxelControlPanel::LoadData()
{
#if WITH_EDITOR
	if(!CanAccessPrefabWorld())
	{
		return;
	}

	FOpenAssetDialogConfig OpenAssetConfig;
	OpenAssetConfig.DialogTitleOverride = LOCTEXT("LoadDialogTitle", "Load Voxel Prefab");
	OpenAssetConfig.DefaultPath = IsValid(CurrentPrefabAsset)
		? FPackageName::GetLongPackagePath(CurrentPrefabAsset->GetOutermost()->GetName())
		: DefaultVoxelPrefabPath;
	OpenAssetConfig.AssetClassNames.Add(UVoxelPrefabData::StaticClass()->GetClassPathName());
	OpenAssetConfig.bAllowMultipleSelection = false;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	const TArray<FAssetData> SelectedAssets = ContentBrowserModule.Get().CreateModalOpenAssetDialog(OpenAssetConfig);
	if(SelectedAssets.IsEmpty())
	{
		return;
	}

	UVoxelPrefabData* PrefabAsset = Cast<UVoxelPrefabData>(SelectedAssets[0].GetAsset());
	if(!IsValid(PrefabAsset))
	{
		ShowNotification(LOCTEXT("InvalidLoadAsset", "选择的资产不是有效的 VoxelPrefab。"), false);
		return;
	}

	FVoxelPrefabSaveData PrefabData;
	PrefabData.VoxelDatas = PrefabAsset->VoxelDatas;
	const FBox PrefabBounds = PrefabAsset->GetVoxelBounds();
	const FVector BlockSize = UVoxelModuleStatics::VoxelIndexToLocation(FIndex::OneIndex);
	const FVector PrefabCenterLocation = PrefabBounds.GetCenter() * BlockSize;
	const float SafeLocationX = (PrefabBounds.Min.X - 5.f) * BlockSize.X;
	const float SafeLocationY = PrefabBounds.GetCenter().Y * BlockSize.Y;
	
	if(ACharacterBase* CurrentCharacter = UCharacterModuleStatics::GetCurrentCharacter())
	{
		const FVector SafeLocation(
			SafeLocationX,
			SafeLocationY,
			FMath::Max(CurrentCharacter->GetActorLocation().Z, BlockSize.Z * 2.f));
		const float ViewYaw = (PrefabCenterLocation - SafeLocation).Rotation().Yaw;
		CurrentCharacter->SetActorLocationAndRotation(
			SafeLocation,
			FRotator(0.f, ViewYaw, 0.f),
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
		UCameraModuleStatics::SetCameraRotation(ViewYaw, 0.f, true);
	}
	else
	{
		const FVector SafeLocation(
			SafeLocationX,
			SafeLocationY,
			FMath::Max(UCameraModuleStatics::GetCameraLocation().Z, BlockSize.Z * 2.f));
		const FRotator ViewRotation = (PrefabCenterLocation - SafeLocation).Rotation();
		UCameraModuleStatics::SetCameraLocation(SafeLocation, true);
		UCameraModuleStatics::SetCameraRotation(ViewRotation.Yaw, ViewRotation.Pitch, true);
	}
	
	UVoxelModuleStatics::LoadVoxelPrefabData(PrefabData);
	CurrentPrefabAsset = PrefabAsset;

	ShowNotification(FText::Format(LOCTEXT("LoadSucceeded", "已加载 VoxelPrefab：{0}"), FText::FromString(PrefabAsset->GetName())), true);
#endif
}

void UWidgetVoxelControlPanel::SaveData()
{
#if WITH_EDITOR
	if(!CanAccessPrefabWorld())
	{
		return;
	}

	FSaveAssetDialogConfig SaveAssetConfig;
	SaveAssetConfig.DialogTitleOverride = LOCTEXT("SaveDialogTitle", "Save Voxel Prefab");
	SaveAssetConfig.DefaultPath = IsValid(CurrentPrefabAsset)
		? FPackageName::GetLongPackagePath(CurrentPrefabAsset->GetOutermost()->GetName())
		: DefaultVoxelPrefabPath;
	SaveAssetConfig.DefaultAssetName = IsValid(CurrentPrefabAsset) ? CurrentPrefabAsset->GetName() : TEXT("DA_VoxelPrefab");
	SaveAssetConfig.AssetClassNames.Add(UVoxelPrefabData::StaticClass()->GetClassPathName());
	SaveAssetConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	const FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetConfig);
	if(SaveObjectPath.IsEmpty())
	{
		return;
	}

	UVoxelPrefabData* PrefabAsset = nullptr;
	bool bCreatedAsset = false;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const FAssetData ExistingAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(SaveObjectPath));
	if(ExistingAssetData.IsValid())
	{
		PrefabAsset = Cast<UVoxelPrefabData>(ExistingAssetData.GetAsset());
		if(!IsValid(PrefabAsset))
		{
			ShowNotification(LOCTEXT("InvalidSaveAsset", "选择的已有资产不是 VoxelPrefab。"), false);
			return;
		}
	}
	else
	{
		const FString PackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
		const FString AssetName = FPackageName::ObjectPathToObjectName(SaveObjectPath);
		if(!FPackageName::IsValidLongPackageName(PackageName) || AssetName.IsEmpty())
		{
			ShowNotification(LOCTEXT("InvalidSavePath", "选择的 VoxelPrefab 资产路径无效。"), false);
			return;
		}

		UPackage* Package = CreatePackage(*PackageName);
		PrefabAsset = NewObject<UVoxelPrefabData>(Package, UVoxelPrefabData::StaticClass(), *AssetName, RF_Public | RF_Standalone | RF_Transactional);
		if(!IsValid(PrefabAsset))
		{
			ShowNotification(LOCTEXT("CreateFailed", "创建 VoxelPrefab 资产失败。"), false);
			return;
		}

		PrefabAsset->DisplayName = FText::FromString(FName::NameToDisplayString(AssetName, false));
		FAssetRegistryModule::AssetCreated(PrefabAsset);
		bCreatedAsset = true;
	}

	const FVoxelPrefabSaveData PrefabData = UVoxelModuleStatics::GetVoxelPrefabData();
	TArray<FIndex> Indices;
	TArray<FString> ItemDatas;
	PrefabData.VoxelDatas.ParseIntoArray(ItemDatas, TEXT("|"), true);
	int32 MinX = MAX_int32;
	int32 MinY = MAX_int32;
	int32 MinZ = MAX_int32;
	int32 MaxX = MIN_int32;
	int32 MaxY = MIN_int32;
	int32 MaxZ = MIN_int32;
	for(const FString& ItemData : ItemDatas)
	{
		TArray<FString> Fields;
		if(ItemData.ParseIntoArray(Fields, TEXT(";"), false) >= 3)
		{
			const FIndex Index = UVoxelModuleStatics::NumberToVoxelIndex(FCString::Atoi64(*Fields[1]), true);
			Indices.Add(Index);
			MinX = FMath::Min(MinX, Index.X);
			MinY = FMath::Min(MinY, Index.Y);
			MinZ = FMath::Min(MinZ, Index.Z);
			MaxX = FMath::Max(MaxX, Index.X);
			MaxY = FMath::Max(MaxY, Index.Y);
			MaxZ = FMath::Max(MaxZ, Index.Z);
		}
	}

	FVector VoxelSize = FVector::ZeroVector;
	FVector CenterOffset = FVector::ZeroVector;
	if(!Indices.IsEmpty())
	{
		VoxelSize = FVector(MaxX - MinX + 1, MaxY - MinY + 1, MaxZ - MinZ + 1);
		CenterOffset = FVector(
			(MinX + MaxX + 1) * 0.5f,
			(MinY + MaxY + 1) * 0.5f,
			MinZ);
	}

	PrefabAsset->Modify();
	PrefabAsset->VoxelDatas = PrefabData.VoxelDatas;
	PrefabAsset->VoxelSize = VoxelSize;
	PrefabAsset->CenterOffset = CenterOffset;
	PrefabAsset->PostEditChange();
	UPackage* PrefabPackage = PrefabAsset->GetOutermost();
	PrefabPackage->MarkPackageDirty();
	PrefabPackage->GetMetaData();

	const FString PackageFilename = FPackageName::LongPackageNameToFilename(
		PrefabPackage->GetName(),
		FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	if(!UPackage::SavePackage(PrefabPackage, nullptr, *PackageFilename, SaveArgs) || PrefabPackage->IsDirty())
	{
		ShowNotification(LOCTEXT("SaveFailed", "VoxelPrefab 数据已更新，但资产未能保存到磁盘。"), false);
		return;
	}

	CurrentPrefabAsset = PrefabAsset;
	ShowNotification(
		FText::Format(
			bCreatedAsset ? LOCTEXT("CreateSucceeded", "已创建 VoxelPrefab：{0}") : LOCTEXT("OverwriteSucceeded", "已更新 VoxelPrefab：{0}"),
			FText::FromString(PrefabAsset->GetName())),
		true);
#endif
}

void UWidgetVoxelControlPanel::ResetData()
{
	if(UVoxelModuleStatics::GetVoxelWorldMode() == EVoxelWorldMode::Prefab && UVoxelModuleStatics::GetWorldGeneratePercent() >= 1.f)
	{
		UVoxelModuleStatics::LoadVoxelPrefabData(FVoxelPrefabSaveData());
	}
}

#if WITH_EDITOR
bool UWidgetVoxelControlPanel::CanAccessPrefabWorld() const
{
	if(UVoxelModuleStatics::GetVoxelWorldMode() != EVoxelWorldMode::Prefab)
	{
		ShowNotification(LOCTEXT("NotPrefabWorld", "只能在 Prefab 世界模式中编辑 VoxelPrefab 数据。"), false);
		return false;
	}
	if(UVoxelModuleStatics::GetWorldGeneratePercent() < 1.f)
	{
		ShowNotification(LOCTEXT("WorldNotReady", "体素世界仍在生成，请稍后重试。"), false);
		return false;
	}
	return true;
}

void UWidgetVoxelControlPanel::ShowNotification(const FText& InText, bool bSuccess) const
{
	FNotificationInfo NotificationInfo(InText);
	NotificationInfo.ExpireDuration = 4.f;
	NotificationInfo.bUseThrobber = false;
	NotificationInfo.bUseSuccessFailIcons = false;
	NotificationInfo.Image = FAppStyle::Get().GetBrush(bSuccess ? "Icons.SuccessWithColor" : "Icons.ErrorWithColor");

	if(const TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(NotificationInfo))
	{
		Notification->SetCompletionState(SNotificationItem::CS_None);
	}
}
#endif

#undef LOCTEXT_NAMESPACE
