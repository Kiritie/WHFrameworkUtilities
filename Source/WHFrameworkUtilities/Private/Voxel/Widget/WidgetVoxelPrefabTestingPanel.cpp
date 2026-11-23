#include "Voxel/Widget/WidgetVoxelPrefabTestingPanel.h"

#include "Camera/CameraModule.h"
#include "Camera/CameraModuleStatics.h"
#include "Camera/Manager/CameraManagerBase.h"
#include "Character/Base/CharacterBase.h"
#include "Character/CharacterModuleStatics.h"
#include "Components/HorizontalBox.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Asset/AssetModuleStatics.h"
#include "Ability/Inventory/InventoryItemRegistry.h"
#include "Ability/Inventory/InventoryModule.h"
#include "Scene/Object/Voxel/VoxelSceneObjectData.h"
#include "Scene/Object/Voxel/VoxelSceneObjectRegistry.h"
#include "Voxel/Runtime/VoxelRegistry.h"
#include "Voxel/Prefabs/Data/VoxelPrefabData.h"
#include "Voxel/Prefabs/VoxelPrefabEditCapture.h"
#include "Voxel/Chunks/VoxelSectionKey.h"
#include "Voxel/Agent/VoxelAgentComponent.h"
#include "Voxel/VoxelModule.h"
#include "Voxel/VoxelModuleStatics.h"
#include "Testing/TestingModule.h"
#include "Voxel/VoxelPrefabTestingModule.h"
#include "Testing/Widget/WidgetTestingInventory.h"
#include "Voxel/Voxels/Data/VoxelData.h"
#include "Widget/Common/CommonButton.h"
#include "Widget/Common/CommonInput.h"
#include "Widget/Common/CommonTextBlockN.h"
#include "Widget/WidgetModule.h"
#include "Widget/ScreenWidgetTypes.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#endif

#define LOCTEXT_NAMESPACE "WidgetVoxelPrefabTestingPanel"

#if WITH_EDITOR
const FString DefaultVoxelPrefabPath = TEXT("/Game");
#endif

UWidgetVoxelPrefabTestingPanel::UWidgetVoxelPrefabTestingPanel(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , Btn_LoadData(nullptr)
    , Btn_SaveData(nullptr)
    , Btn_ResetData(nullptr)
    , CurrentPrefabAsset(nullptr)
{
	CatalogCategories.Emplace(TEXT("Blocks"), FText::FromString(TEXT("方块")));
	CatalogCategories.Emplace(TEXT("Facilities"), FText::FromString(TEXT("设施")));
	CatalogCategories.Emplace(TEXT("Furniture"), FText::FromString(TEXT("家具")));
}

void UWidgetVoxelPrefabTestingPanel::OnCreate(const FParameter& InParam)
{
	Super::OnCreate(InParam);

	if (Btn_LoadData)
	{
		Btn_LoadData->OnClicked().AddUObject(this, &UWidgetVoxelPrefabTestingPanel::LoadData);
	}
	if (Btn_SaveData)
	{
		Btn_SaveData->OnClicked().AddUObject(this, &UWidgetVoxelPrefabTestingPanel::SaveData);
	}
	if (Btn_ResetData)
	{
		Btn_ResetData->OnClicked().AddUObject(this, &UWidgetVoxelPrefabTestingPanel::ResetData);
	}
	if (!CatalogCategories.IsEmpty()) CatalogView->SetCategory(CatalogCategories[0].Name);
}

void UWidgetVoxelPrefabTestingPanel::SetCatalogEntries(const TArray<FToolCatalogEntry>& InEntries)
{
	if (CatalogView) CatalogView->SetEntries(InEntries);
}

void UWidgetVoxelPrefabTestingPanel::SetAdditionalCatalogEntries(const TArray<FToolCatalogEntry>& InEntries)
{
	AdditionalCatalogEntries = InEntries;
	RefreshCatalog();
}

void UWidgetVoxelPrefabTestingPanel::OnOpen(const FParameter& InParam, bool bInstant)
{
	if (UVoxelModule* Voxel = UVoxelModule::GetPtr(GetWorld())) Voxel->OnWorldInitialized.AddUniqueDynamic(this, &ThisClass::RefreshCatalog);
	if (UTestingModule* Testing = UTestingModule::GetPtr(GetWorld()))
	{
		Testing->OnCatalogReady.AddUniqueDynamic(this, &ThisClass::RefreshCatalog);
	}
	Super::OnOpen(InParam, bInstant);
	RefreshCatalog();
}

void UWidgetVoxelPrefabTestingPanel::OnClose(bool bInstant)
{
	if (UVoxelModule* Voxel = UVoxelModule::GetPtr(GetWorld())) Voxel->OnWorldInitialized.RemoveDynamic(this, &ThisClass::RefreshCatalog);
	if (UTestingModule* Testing = UTestingModule::GetPtr(GetWorld()))
	{
		Testing->OnCatalogReady.RemoveDynamic(this, &ThisClass::RefreshCatalog);
	}
	Super::OnClose(bInstant);
}

void UWidgetVoxelPrefabTestingPanel::RefreshCatalog()
{
	CatalogView->SetAddToInventoryEnabled(false);
	CatalogView->SetCreateEnabled(false);
	const UVoxelPrefabTestingModule* PrefabTesting = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld()));
	const bool bInventoryReady = PrefabTesting && PrefabTesting->IsInventoryReady();
	const FText InventoryStatus = PrefabTesting && !PrefabTesting->GetStartupError().IsEmpty()
		? FText::FromString(PrefabTesting->GetStartupError()) : bInventoryReady ? FText::GetEmpty() : FText::FromString(TEXT("正在准备物品目录。"));
	if (Text_CatalogStatus)
	{
		Text_CatalogStatus->SetText(InventoryStatus);
	}
	TArray<FToolCatalogEntry> Entries;
	for (const UVoxelData* Data : UAssetModuleStatics::LoadPrimaryAssets<UVoxelData>(FName(TEXT("Voxel"))))
	{
		if (!IsValid(Data) || !Data->bRegisterBlock) continue;
		const FInventoryItemDefinition Definition = Data->BuildInventoryDefinition();
		if (!Definition.ItemId.IsValid() || !Definition.PlaceableVoxelId.IsValid()) continue;
		FToolCatalogEntry& Entry = Entries.AddDefaulted_GetRef();
		Entry.ItemId = Definition.ItemId;
		Entry.bPlaceable = true;
		Entry.Asset = const_cast<UVoxelData*>(Data);
		Entry.Category = TEXT("Blocks");
		Entry.DisplayName = Definition.DisplayName.IsEmptyOrWhitespace() ? FText::FromName(Data->GetFName()) : Definition.DisplayName;
		Entry.Description = Data->Detail;
		Entry.Icon = TSoftObjectPtr<UObject>(Definition.Icon);
	}
	Entries.Sort([](const FToolCatalogEntry& A, const FToolCatalogEntry& B) { return A.DisplayName.ToString() < B.DisplayName.ToString(); });
	if (const UTestingModule* Testing = UTestingModule::GetPtr(GetWorld()))
	{
		TArray<FToolCatalogEntry> Configured;
		Testing->GetConfiguredCatalogEntries(Configured);
		Entries.Append(Configured);
	}
	Entries.Append(AdditionalCatalogEntries);
	const UVoxelModule* Voxel = UVoxelModule::GetPtr(GetWorld());
	const UInventoryModule* Inventory = UInventoryModule::GetPtr(GetWorld());
	const auto Blocks = Voxel ? Voxel->GetRegistry() : nullptr;
	for (FToolCatalogEntry& Entry : Entries)
	{
		const UVoxelSceneObjectData* Object = Cast<UVoxelSceneObjectData>(Entry.Asset.LoadSynchronous());
		const USceneObjectData* SceneData = Object;
		if (SceneData)
		{
			Entry.AssetId = SceneData->GetPrimaryAssetId();
			if (!SceneData->DisplayName.IsEmptyOrWhitespace()) Entry.DisplayName = SceneData->DisplayName;
		}
		if (!bInventoryReady)
		{
			Entry.bAvailable = false;
			Entry.UnavailableReason = InventoryStatus;
			continue;
		}
		if (SceneData) Entry.ItemId = SceneData->GetPrimaryAssetId();
		const FInventoryItemDefinition* Item = Inventory && Entry.ItemId.IsValid() ? Inventory->GetRegistry().Find(Entry.ItemId) : nullptr;
		const FVoxelRuntimeDefinition* Block = Item && Blocks ? Blocks->Find(Item->PlaceableVoxelId) : nullptr;
		const bool bPlaceable = Item && (SceneData ? Item->UsePolicy == EInventoryUsePolicy::PlaceSceneObject && Item->PlaceableSceneObject == Entry.AssetId :
			Item->UsePolicy == EInventoryUsePolicy::PlaceVoxel && Block);
		if (!bPlaceable)
		{
			Entry.bAvailable = false;
			if (Entry.UnavailableReason.IsEmpty()) Entry.UnavailableReason = FText::FromString(TEXT("No placeable item is configured."));
		}
		else
		{
			if ((!SceneData || SceneData->DisplayName.IsEmptyOrWhitespace()) && !Item->DisplayName.IsEmptyOrWhitespace()) Entry.DisplayName = Item->DisplayName;
			if (!Item->Icon.IsNull()) Entry.Icon = TSoftObjectPtr<UObject>(Item->Icon);
		}
	}
	TMap<FName, TSet<FPrimaryAssetId>> Seen;
	Entries.RemoveAll([&Seen](const FToolCatalogEntry& Entry)
	{
		TSet<FPrimaryAssetId>& Category = Seen.FindOrAdd(Entry.Category);
		const FPrimaryAssetId AssetId = Entry.GetAssetId();
		if (!AssetId.IsValid()) return false;
		if (Category.Contains(AssetId)) return true;
		Category.Add(AssetId);
		return false;
	});
	SetCatalogEntries(Entries);
	FToolCatalogEntry Selection;
	if (CatalogView->GetSelection(Selection))
	{
		CatalogView->SetAddToInventoryEnabled(Selection.bAvailable && Selection.ItemId.IsValid());
		CatalogView->SetCreateEnabled(Selection.bAvailable && Selection.ItemId.IsValid());
	}
}

bool UWidgetVoxelPrefabTestingPanel::GetSelectedCatalogEntry(FToolCatalogEntry& OutEntry) const
{
	return CatalogView && CatalogView->GetSelection(OutEntry);
}

void UWidgetVoxelPrefabTestingPanel::HandleCatalogSelection(const FToolCatalogEntry& InEntry)
{
	CatalogView->SetAddToInventoryEnabled(InEntry.bAvailable && InEntry.ItemId.IsValid());
	CatalogView->SetCreateEnabled(InEntry.bAvailable && InEntry.ItemId.IsValid());
	OnCatalogSelectionChanged.Broadcast(InEntry);
}

void UWidgetVoxelPrefabTestingPanel::HandleCreate(const FToolCatalogEntry& InEntry)
{
	UVoxelPrefabTestingModule* Testing = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld()));
	const bool bCreated = Testing && Testing->CreatePlacement(GetOwningPlayer(), InEntry.ItemId);
	if (Text_CatalogStatus)
	{
		Text_CatalogStatus->SetText(bCreated ? FText::FromString(TEXT("已提交创建请求。")) : FText::FromString(TEXT("无法在当前视线位置创建该物体。")));
	}
}

void UWidgetVoxelPrefabTestingPanel::HandleAddToInventory(const FToolCatalogEntry& InEntry)
{
	APlayerController* Controller = GetOwningPlayer();
	UVoxelPrefabTestingModule* Testing = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld()));
	if (!Testing || !Testing->AddPlacement(Controller, InEntry))
	{
		if (Text_CatalogStatus) Text_CatalogStatus->SetText(FText::FromString(TEXT("Creative placement is not available for this player.")));
		return;
	}
	if (Text_CatalogStatus) Text_CatalogStatus->SetText(FText::GetEmpty());
	if (UWidgetModule* Widgets = UWidgetModule::GetPtr(GetWorld()))
	{
		if (Controller) Widgets->OpenUserWidget<UWidgetTestingInventory>(Controller);
	}
}

void UWidgetVoxelPrefabTestingPanel::LoadData()
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
	ContentBrowserModule.Get().CreateOpenAssetDialog(OpenAssetConfig, FOnAssetsChosenForOpen::CreateWeakLambda(this, [this](const TArray<FAssetData>& SelectedAssets)
	{
		if (SelectedAssets.IsEmpty() || !IsWidgetOpened() || !CanAccessPrefabWorld()) return;
		UVoxelPrefabData* PrefabAsset = Cast<UVoxelPrefabData>(SelectedAssets[0].GetAsset());
		FString Error;
		UVoxelModule* Voxel = UVoxelModule::GetPtr(GetWorld());
		FVoxelPrefabSaveData PrefabData;
		if (!IsValid(PrefabAsset) || !Voxel || !Voxel->IsReady() || !PrefabAsset->DecodePlacementCells(PrefabData, Error) || !Voxel->ApplyPrefab(PrefabData, FIntVector::ZeroValue, Error))
		{
			ShowNotification(FText::FromString(Error.IsEmpty() ? TEXT("无法加载 VoxelPrefab。") : Error));
			return;
		}
		ResetData();
		CurrentPrefabAsset = PrefabAsset;
		ShowNotification(FText::Format(LOCTEXT("LoadSucceeded", "已加载 VoxelPrefab：{0}"), FText::FromString(PrefabAsset->GetName())));
	}), FOnAssetDialogCancelled());
#endif
}

void UWidgetVoxelPrefabTestingPanel::SaveData()
{
#if WITH_EDITOR
	if (!CanAccessPrefabWorld()) return;
	if (PrefabCapture) { ShowNotification(LOCTEXT("CaptureBusy", "Prefab capture is already running.")); return; }
	if (!IsValid(CurrentPrefabAsset))
	{
		FSaveAssetDialogConfig SaveConfig;
		SaveConfig.DialogTitleOverride = LOCTEXT("SaveDialogTitle", "Save Voxel Prefab");
		SaveConfig.DefaultPath = DefaultVoxelPrefabPath;
		SaveConfig.DefaultAssetName = TEXT("VoxelPrefab");
		SaveConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser")).Get().CreateSaveAssetDialog(
			SaveConfig, FOnObjectPathChosenForSave::CreateWeakLambda(this, [this](const FString& ObjectPath)
			{
				if (!ObjectPath.IsEmpty() && IsWidgetOpened()) CapturePrefab(FPackageName::ObjectPathToPackageName(ObjectPath));
			}), FOnAssetDialogCancelled());
		return;
	}
	CapturePrefab(FString());
#endif
}

#if WITH_EDITOR
void UWidgetVoxelPrefabTestingPanel::CapturePrefab(const FString& NewPackageName)
{
	if (!CanAccessPrefabWorld() || PrefabCapture) return;
	UVoxelModule* Voxel = UVoxelModule::GetPtr(GetWorld());
	const TWeakObjectPtr<UWidgetVoxelPrefabTestingPanel> WeakThis(this);
	const TWeakObjectPtr<UVoxelPrefabData> WeakAsset(CurrentPrefabAsset);
	const TWeakObjectPtr<UVoxelModule> WeakVoxel(Voxel);
	FString Error;
	PrefabCapture = FVoxelPrefabEditCapture::Start(*Voxel, [WeakThis, WeakAsset, WeakVoxel, NewPackageName](bool bSuccess, TArray<FVoxelCellEdit> Cells, TSharedPtr<const FVoxelOverlaySnapshotSet, ESPMode::ThreadSafe> Snapshot, FString Message)
	{
		UWidgetVoxelPrefabTestingPanel* Panel = WeakThis.Get();
		UVoxelPrefabData* Asset = WeakAsset.Get();
		UVoxelModule* Module = WeakVoxel.Get();
		if (!Panel) return;
		Panel->PrefabCapture.Reset();
		if (!bSuccess || !Module || !Snapshot || (NewPackageName.IsEmpty() ? !Asset || Asset != Panel->CurrentPrefabAsset : IsValid(Panel->CurrentPrefabAsset)))
		{
			Panel->ShowNotification(FText::FromString(Message.IsEmpty() ? TEXT("Prefab capture failed.") : Message));
			return;
		}
		FVoxelPrefabSaveData Data;
		FIntVector Offset = FIntVector::ZeroValue;
		if (!Module->BuildPrefabFromEdits(Cells, *Snapshot, Data, Message) || (Asset && !Asset->GetPlacementOffset(Offset, Message)))
		{
			Panel->ShowNotification(FText::FromString(Message));
			return;
		}
		for (FVoxelPrefabCell& Cell : Data.Cells)
		{
			for (int32 Axis = 0; Axis < 3; ++Axis)
			{
				const int64 Value = int64(Cell.Offset[Axis]) - Offset[Axis];
				if (Value < -int64(VoxelBlock::MaxAbsCoordinate) || Value >= VoxelBlock::MaxAbsCoordinate)
				{
					Panel->ShowNotification(LOCTEXT("OffsetInvalid", "Prefab center offset exceeds voxel bounds."));
					return;
				}
				Cell.Offset[Axis] = int32(Value);
			}
		}
		if (!Asset)
		{
			UPackage* NewPackage = CreatePackage(*NewPackageName);
			const FName AssetName(*FPackageName::GetLongPackageAssetName(NewPackageName));
			Asset = NewObject<UVoxelPrefabData>(NewPackage, AssetName, RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(Asset);
			Panel->CurrentPrefabAsset = Asset;
		}
		Asset->Modify();
		Asset->PackedCells.Reset();
		Asset->PackedPalette.Reset();
		Asset->Data = MoveTemp(Data);
		Asset->PostEditChange();
		UPackage* Package = Asset->GetOutermost();
		Package->MarkPackageDirty();
		const FString Filename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs Args;
		Args.TopLevelFlags = RF_Standalone;
		Args.SaveFlags = SAVE_NoError;
		const bool bSaved = UPackage::SavePackage(Package, nullptr, *Filename, Args) && !Package->IsDirty();
		Panel->ShowNotification(bSaved ? FText::Format(LOCTEXT("PrefabSaved", "Saved prefab: {0}"), FText::FromString(Asset->GetName())) : LOCTEXT("PrefabSaveFailed", "Prefab updated but could not be saved."));
	}, Error);
	if (!PrefabCapture) ShowNotification(FText::FromString(Error));
}
#endif

void UWidgetVoxelPrefabTestingPanel::ResetData()
{
#if WITH_EDITOR
	if (PrefabCapture) { PrefabCapture->Cancel(); PrefabCapture.Reset(); }
#endif
	CurrentPrefabAsset = nullptr;
}

#if WITH_EDITOR
bool UWidgetVoxelPrefabTestingPanel::CanAccessPrefabWorld() const
{
	const UVoxelModule* Voxel = UVoxelModule::GetPtr(GetWorld());
	if (!Voxel || !Voxel->IsReady())
	{
		ShowNotification(LOCTEXT("WorldNotReady", "Prefab 世界尚未准备完成。"));
		return false;
	}
	return true;
}

void UWidgetVoxelPrefabTestingPanel::ShowNotification(const FText& InText) const
{
	if (Text_CatalogStatus) Text_CatalogStatus->SetText(InText);
}
#endif

#undef LOCTEXT_NAMESPACE
