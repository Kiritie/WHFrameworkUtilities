#include "Voxel/VoxelPrefabTestingModule.h"
#include "WHFrameworkCoreMacros.h"

#include "Ability/Inventory/InventoryModule.h"
#include "AI/Voxel/VoxelSceneAI.h"
#include "Ability/Inventory/InventoryProfile.h"
#include "Ability/Item/AbilityItemDataBase.h"
#include "Asset/AssetModuleStatics.h"
#include "Common/LifecycleTypes.h"
#include "Camera/Manager/CameraManagerBase.h"
#include "Engine/GameInstance.h"
#include "Engine/AssetManager.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Voxel/Agent/VoxelAgentComponent.h"
#include "Scene/Object/Voxel/VoxelSceneObjectData.h"
#include "Scene/Object/SceneObjectInventoryDomainBridge.h"
#include "Scene/Object/SceneObjectPlacement.h"
#include "Scene/SceneModule.h"
#include "Voxel/Generation/VoxelWorldManifest.h"
#include "Voxel/Interaction/VoxelInventoryDomainBridge.h"
#include "Voxel/VoxelModule.h"
#include "Voxel/Voxels/Data/VoxelData.h"
#include "Input/InputTags.h"
#include "Voxel/Input/VoxelPrefabTestingInputBinding.h"
#include "Widget/WidgetModule.h"
#include "Widget/Screen/UserWidgetBase.h"

UVoxelPrefabTestingModule::UVoxelPrefabTestingModule()
{
	FTestingInputHint& AI = InputHints.AddDefaulted_GetRef();
	AI.Label = NSLOCTEXT("Testing", "OpenAI", "AI 场景编辑");
	AI.Actions = {FGameplayTag::RequestGameplayTag(TEXT("Input.Testing.ToggleSceneAI"))};
	FTestingInputHint& View = InputHints.AddDefaulted_GetRef();
	View.Label = NSLOCTEXT("Testing", "SwitchView", "切换人称");
	View.Actions = {GameplayTags::Input_SwitchView};
	FTestingInputHint& Break = InputHints.AddDefaulted_GetRef();
	Break.Label = NSLOCTEXT("Testing", "Break", "摧毁");
	Break.Actions = {GameplayTags::Input_Primary};
	FTestingInputHint& Place = InputHints.AddDefaulted_GetRef();
	Place.Label = NSLOCTEXT("Testing", "Place", "创建");
	Place.Actions = {GameplayTags::Input_Secondary};
	FTestingInputHint& Vertical = InputHints.AddDefaulted_GetRef();
	Vertical.Label = NSLOCTEXT("Testing", "MoveVertical", "升降");
	Vertical.Actions = {GameplayTags::Input_MoveUpPlayer};
	FTestingInputHint& Move = InputHints.AddDefaulted_GetRef();
	Move.Label = NSLOCTEXT("Testing", "Move", "移动");
	Move.Actions = {GameplayTags::Input_MoveForwardPlayer, GameplayTags::Input_MoveRightPlayer};
}

UVoxelSceneAI* UVoxelPrefabTestingModule::InitializeVoxelSceneAI()
{
	if (!VoxelSceneAI) VoxelSceneAI = NewObject<UVoxelSceneAI>(this);
	return VoxelSceneAI;
}

void UVoxelPrefabTestingModule::ToggleAIPanel(APlayerController* InController)
{
	if (!InController || !InController->IsLocalController() || !AIPanelClass) return;
	UWidgetModule* Widgets = UWidgetModule::GetPtr(GetWorld());
	if (!Widgets) return;
	UUserWidgetBase* Panel = Widgets->GetUserWidget(InController, AIPanelClass);
	if (Panel && Panel->IsWidgetOpened()) Widgets->CloseUserWidget(InController, AIPanelClass);
	else Widgets->OpenUserWidget(InController, AIPanelClass);
}

bool UVoxelPrefabTestingModule::SelectPlacement(APlayerController* InController, FPrimaryAssetId InItemId)
{
	UVoxelAgentComponent* Agent = GetPlacementAgent(InController);
	if (!Agent) return false;
	return InItemId.PrimaryAssetType == FPrimaryAssetType(TEXT("SceneObject")) || Agent->SetCreativeSelection(InItemId);
}

bool UVoxelPrefabTestingModule::CreatePlacement(APlayerController* InController, FPrimaryAssetId InItemId)
{
	if (InItemId.PrimaryAssetType == FPrimaryAssetType(TEXT("SceneObject")))
	{
		if (!IsInventoryReady() || !InController || !InController->IsLocalController() || InController->GetWorld() != GetWorld() || GetWorld()->GetNetMode() != NM_Standalone) return false;
		UInventoryModule* Store = UInventoryModule::GetPtr(GetWorld());
		const FInventoryItemDefinition* Item = Store ? Store->GetRegistry().Find(InItemId) : nullptr;
		const USceneObjectData* Data = USceneObjectData::ResolveAsset(InItemId);
		if (!Data || !Item || Item->UsePolicy != EInventoryUsePolicy::PlaceSceneObject || Item->PlaceableSceneObject != InItemId) return false;
		FVector Eye, Point, Normal;
		FRotator Rotation;
		double GridSize;
		FIntVector GridAnchor;
		InController->GetPlayerViewPoint(Eye, Rotation);
		FSceneObjectRecord Record;
		const UVoxelSceneObjectData* GridData = Cast<UVoxelSceneObjectData>(Data);
		if (!FSceneObjectPlacement::TraceSurface(*InController, Eye, Rotation.Vector(), 600., Data->LocalBounds, 0.f,
			GridData && GridData->UsesFloorAnchor(), Record.Transform, Point, Normal, GridSize, GridAnchor)) return false;
		Record.Id = FGuid::NewGuid();
		Record.ObjectData = InItemId;
		Record.ActorClass = Item->SceneObjectClass;
		Record.bVoxelGrid = GridSize > 0.0;
		if (Record.bVoxelGrid) Record.VoxelAnchor = GridAnchor;
		FInventoryBatchRequest Batch;
		FInventoryDomainRedo& Change = Batch.DomainChanges.AddDefaulted_GetRef();
		Change.Domain = SceneBridge->GetDomain();
		Change.EntityId = Record.Id;
		Change.TargetRevision = 1;
		FString Error;
		return FSceneObjectRecordCodec::Encode(Record, Change.AfterBytes, Error) &&
			Store->ExecuteAuthorizedBatch(Batch, Store->MakeServerAuthority(Store->GetWorldId(), TEXT("Prefab.SceneObject"))).IsSuccess();
	}
	UVoxelAgentComponent* Agent = GetPlacementAgent(InController);
	if (!Agent) return false;
	return Agent->PlaceCreativeItem(InItemId);
}

UVoxelAgentComponent* UVoxelPrefabTestingModule::GetPlacementAgent(APlayerController* InController)
{
	if (!IsInventoryReady() || !InController || !InController->IsLocalController() ||
		InController->GetWorld() != GetWorld() || GetWorld()->GetNetMode() != NM_Standalone) return nullptr;
	if (PlacementAgent && PlacementAgent->GetOwner() != InController)
	{
		PlacementAgent->DestroyComponent();
		PlacementAgent = nullptr;
	}
	if (!PlacementAgent)
	{
		PlacementAgent = NewObject<UVoxelAgentComponent>(InController);
		PlacementAgent->bCreativeInStandalone = true;
		PlacementAgent->RegisterComponent();
		PlacementAgent->BindController(InController);
	}
	return PlacementAgent;
}

void UVoxelPrefabTestingModule::OnRefresh(float InDeltaSeconds, bool bInEditor)
{
	Super::OnRefresh(InDeltaSeconds, bInEditor);
	if (!bInEditor && VoxelSceneAI) VoxelSceneAI->Refresh(InDeltaSeconds);
	if (bInEditor) return;
	const UVoxelModule* Voxel = UVoxelModule::GetPtr(GetWorld());
	if (!Voxel || !Voxel->IsReady()) return;
	if (!bInventoryStarted)
	{
		bInventoryStarted = true;
		if (!StartTransientInventory(StartupError))
		{
			if (StartupError.IsEmpty()) StartupError = TEXT("Unable to prepare the testing inventory.");
			UE_LOG(LogTemp, Error, TEXT("Testing voxel inventory: %s"), *StartupError);
		}
		OnCatalogReady.Broadcast();
	}
	if (bCameraInitialized || !IsInventoryReady()) return;
	const UGameInstance* Instance = GetWorld()->GetGameInstance();
	const TArray<ULocalPlayer*> Players = Instance->GetLocalPlayers();
	if (Players.Num() != 1) return;
	APlayerController* Controller = Players[0]->GetPlayerController(GetWorld());
	ACameraManagerBase* Camera = Controller ? Cast<ACameraManagerBase>(Controller->PlayerCameraManager) : nullptr;
	if (!Camera || !GetPlacementAgent(Controller)) return;
	if (!Controller->GetPawn())
	{
		const FRotator Rotation(-45.f, 0.f, 0.f);
		Camera->SetInitialView(Rotation, 0.f);
		Camera->SetDesiredPivot(FVector(0., 0., 12. * Voxel->BlockSize()), Rotation);
	}
	bCameraInitialized = true;
}

bool UVoxelPrefabTestingModule::StartTransientInventory(FString& OutError)
{
	UVoxelModule* Voxel = UVoxelModule::GetPtr(GetWorld());
	UInventoryModule* Inventory = UInventoryModule::GetPtr(GetWorld());
	if (GetWorld()->GetNetMode() != NM_Standalone || !Inventory || !TransientInventoryProfile)
	{
		OutError = TEXT("A standalone world, inventory module and inventory profile are required.");
		return false;
	}
	TArray<FToolCatalogEntry> CatalogEntries;
	GetConfiguredCatalogEntries(CatalogEntries);
	TArray<UVoxelSceneObjectData*> SceneObjects;
	for (const FToolCatalogEntry& Entry : CatalogEntries)
	{
		if (UVoxelSceneObjectData* Object = Cast<UVoxelSceneObjectData>(Entry.Asset.LoadSynchronous())) SceneObjects.AddUnique(Object);
	}
	if (!Voxel->ConfigureSceneObjectDefinitions(SceneObjects, OutError)) return false;
	TArray<FInventoryItemDefinition> Items;
	TSet<FPrimaryAssetId> ItemIds;
	for (const UVoxelSceneObjectData* Grid : SceneObjects)
	{
		if (ItemIds.Contains(Grid->GetPrimaryAssetId())) continue;
		Items.Add(Grid->BuildInventoryDefinition());
		ItemIds.Add(Grid->GetPrimaryAssetId());
	}
	for (const FToolCatalogEntry& Entry : CatalogEntries)
	{
		if (Cast<USceneObjectData>(Entry.Asset.LoadSynchronous()) || !Entry.ItemId.IsValid() || ItemIds.Contains(Entry.ItemId)) continue;
		const FSoftObjectPath ItemPath = UAssetManager::Get().GetPrimaryAssetPath(Entry.ItemId);
		const UAbilityItemDataBase* Data = Cast<UAbilityItemDataBase>(ItemPath.TryLoad());
		if (!Data || Data->GetPrimaryAssetId() != Entry.ItemId)
		{
			OutError = FString::Printf(TEXT("Unable to load testing catalog item %s."), *Entry.ItemId.ToString());
			return false;
		}
		Items.Add(Data->BuildInventoryDefinition());
		ItemIds.Add(Entry.ItemId);
	}
	for (const UVoxelData* Data : UAssetModuleStatics::LoadPrimaryAssets<UVoxelData>(FName(TEXT("Voxel"))))
	{
		if (!Data || !Data->bRegisterBlock) continue;
		FInventoryItemDefinition Item = Data->BuildInventoryDefinition();
		if (Item.ItemId.IsValid() && Item.PlaceableVoxelId.IsValid() && !ItemIds.Contains(Item.ItemId))
		{
			ItemIds.Add(Item.ItemId);
			Items.Add(MoveTemp(Item));
		}
	}
	FInventoryProfileDefinition Profile;
	if (!TransientInventoryProfile->Compile(Profile, OutError) ||
		!Inventory->Configure(Items, MakeArrayView(&Profile, 1), MakeShared<FInventoryRuleProvider>(), OutError)) return false;
	InventoryBridge = MakeShared<FVoxelInventoryDomainBridge>(*Inventory);
	SceneBridge = MakeShared<FSceneObjectInventoryDomainBridge>(*Inventory);
	USceneModule* Scene = USceneModule::GetPtr(GetWorld());
	if (!Scene || !SceneBridge->Register(OutError) || !InventoryBridge->Register(OutError) ||
		!Inventory->OpenTransientStore(Voxel->GetManifest().WorldId, FGuid::NewGuid(), OutError) ||
		!SceneBridge->AttachScene(*Scene, OutError) || !InventoryBridge->AttachRuntime(*Voxel->GetRuntime(), OutError)) return false;
	if (!Voxel->BindInventoryBridge(InventoryBridge.ToSharedRef(),
		[](FVoxelInteractionPlan&, AActor*, FInventoryBatchRequest&, FString&) { return true; }))
	{
		InventoryBridge->DetachRuntime();
		OutError = TEXT("The voxel inventory bridge is already bound.");
		return false;
	}
	return true;
}

void UVoxelPrefabTestingModule::OnTermination(EPhase InPhase)
{
	if (PHASEC(InPhase, EPhase::Primary))
	{
		if (PlacementAgent) PlacementAgent->DestroyComponent();
		PlacementAgent = nullptr;
		if (VoxelSceneAI) VoxelSceneAI->Terminate();
		VoxelSceneAI = nullptr;
		bInventoryStarted = false;
		bCameraInitialized = false;
		StartupError.Reset();
	}
	if (PHASEC(InPhase, EPhase::Primary) && InventoryBridge)
	{
		if (UVoxelModule* Voxel = UVoxelModule::GetPtr(GetWorld())) Voxel->UnbindInventoryBridge(InventoryBridge);
		InventoryBridge->DetachRuntime();
		InventoryBridge.Reset();
		if (SceneBridge && SceneBridge->IsAttached()) { FString Error; SceneBridge->DetachScene(Error); }
		SceneBridge.Reset();
	}
	Super::OnTermination(InPhase);
}
