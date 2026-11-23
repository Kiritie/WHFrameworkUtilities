#include "Testing/TestingModule.h"
#include "Input/InputTags.h"
#include "WHFrameworkCoreMacros.h"
#include "Testing/TestingCatalogData.h"
#include "Testing/TestingSettings.h"
#include "Engine/World.h"
#include "Common/LifecycleTypes.h"
#include "GameFramework/PlayerController.h"
#include "Input/InputModule.h"
#include "Widget/Screen/UserWidgetBase.h"
#include "Widget/WidgetModule.h"

IMPLEMENTATION_MODULE(UTestingModule)

UTestingModule::UTestingModule()
{
	ModuleName = TEXT("TestingModule");
	ModuleDisplayName = FText::FromString(TEXT("Testing Module"));
}

void UTestingModule::OnTermination(EPhase InPhase)
{
	if (PHASEC(InPhase, EPhase::Primary))
	{
		PlacementEntries.Reset();
		PlacementIndex = INDEX_NONE;
		ClearCatalogSelection();
	}
	Super::OnTermination(InPhase);
}

bool UTestingModule::AddPlacement(APlayerController* InController, const FToolCatalogEntry& InEntry)
{
	if (!InEntry.bAvailable || !InEntry.ItemId.IsValid()) return false;
	const int32 Existing = PlacementEntries.IndexOfByPredicate([&InEntry](const FToolCatalogEntry& Entry)
	{
		return Entry.ItemId == InEntry.ItemId;
	});
	if (Existing == INDEX_NONE) PlacementEntries.Add(InEntry);
	if (PlacementIndex != INDEX_NONE) return true;
	if (!SelectPlacement(InController, InEntry.ItemId))
	{
		if (Existing == INDEX_NONE) PlacementEntries.Pop();
		return false;
	}
	PlacementIndex = Existing == INDEX_NONE ? PlacementEntries.Num() - 1 : Existing;
	SetCatalogSelection(PlacementEntries[PlacementIndex]);
	return true;
}

bool UTestingModule::CyclePlacement(APlayerController* InController, int32 InDirection)
{
	if (PlacementEntries.IsEmpty() || PlacementIndex == INDEX_NONE || InDirection == 0) return false;
	const int32 Next = (PlacementIndex + (InDirection > 0 ? 1 : -1) + PlacementEntries.Num()) % PlacementEntries.Num();
	if (!SelectPlacement(InController, PlacementEntries[Next].ItemId)) return false;
	PlacementIndex = Next;
	SetCatalogSelection(PlacementEntries[PlacementIndex]);
	return true;
}

void UTestingModule::TogglePanel(APlayerController* InController)
{
	UWorld* World = GetWorld();
	UWidgetModule* Widgets = World ? UWidgetModule::GetPtr(World) : nullptr;
	if (!InController || InController->GetWorld() != World || !InController->IsLocalController() || !Widgets || !PanelClass || !IsPanelAvailable()) return;
	UUserWidgetBase* Panel = Widgets->GetUserWidget(InController, PanelClass);
	if (Panel && Panel->IsWidgetOpened()) Widgets->CloseUserWidget(InController, PanelClass);
	else Widgets->OpenUserWidget(InController, PanelClass);
}

bool UTestingModule::IsPanelKey(APlayerController* InController, FKey InKey) const
{
	const UInputModule* Input = UInputModule::GetPtr(GetWorld());
	if (!Input || !InController) return false;
	const FGameplayTag Action = GameplayTags::Input_SystemOperation;
	return Input->GetKeysByActionTag(Action, Input->FindPlayerRuntimeIndex(InController)).Contains(InKey);
}

void UTestingModule::SetCatalogSelection(const FToolCatalogEntry& InEntry)
{
	if (!InEntry.ItemId.IsValid() || !InEntry.bAvailable) return;
	CatalogSelection = InEntry;
	bHasCatalogSelection = true;
	OnCatalogSelectionChanged.Broadcast(CatalogSelection);
}

bool UTestingModule::GetCatalogSelection(FToolCatalogEntry& OutEntry) const
{
	if (!bHasCatalogSelection) return false;
	OutEntry = CatalogSelection;
	return true;
}

void UTestingModule::ClearCatalogSelection()
{
	CatalogSelection = FToolCatalogEntry();
	bHasCatalogSelection = false;
	OnCatalogSelectionChanged.Broadcast(CatalogSelection);
}

void UTestingModule::GetConfiguredCatalogEntries(TArray<FToolCatalogEntry>& OutEntries) const
{
	OutEntries.Reset();
	for (const UTestingCatalogData* Catalog : Catalogs)
	{
		if (Catalog) OutEntries.Append(Catalog->Entries);
	}
	for (const TSoftObjectPtr<UTestingCatalogData>& Reference : GetDefault<UTestingSettings>()->Catalogs)
	{
		if (const UTestingCatalogData* Catalog = Reference.LoadSynchronous()) OutEntries.Append(Catalog->Entries);
	}
}
