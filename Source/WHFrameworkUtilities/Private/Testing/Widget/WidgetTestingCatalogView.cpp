#include "Testing/Widget/WidgetTestingCatalogView.h"

#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "Testing/TestingModule.h"
#include "Widget/Common/CommonButton.h"
#include "Widget/Common/CommonInput.h"
#include "Widget/Common/CommonTextBlockN.h"

UWidgetTestingCatalogView::UWidgetTestingCatalogView(const FObjectInitializer& InInitializer)
	: Super(InInitializer)
	, EntryButtonClass(UCommonButton::StaticClass())
{
	EntryButtonStyle = FGameplayTag::RequestGameplayTag(TEXT("UI.Style.Button.Secondary.List.Row"));
	CategoryButtonStyle = FGameplayTag::RequestGameplayTag(TEXT("UI.Style.Button.Borderless.Category"));
	EntryTextStyle = FGameplayTag::RequestGameplayTag(TEXT("UI.Style.Text.Label.Field"));
}

void UWidgetTestingCatalogView::NativeConstruct()
{
	Super::NativeConstruct();
	Input_Search->OnValueChanged.AddUniqueDynamic(this, &ThisClass::OnSearchChanged);
	if (Btn_AddToInventory)
	{
		Btn_AddToInventory->SetIsEnabledN(bAddToInventoryEnabled);
		Btn_AddToInventory->OnClicked().AddWeakLambda(this, [this]()
		{
			FToolCatalogEntry Entry;
			if (bAddToInventoryEnabled && GetSelection(Entry)) OnAddToInventory.Broadcast(Entry);
		});
	}
	if (Btn_Create)
	{
		Btn_Create->SetIsEnabledN(bCreateEnabled);
		Btn_Create->OnClicked().AddWeakLambda(this, [this]()
		{
			FToolCatalogEntry Entry;
			if (bCreateEnabled && GetSelection(Entry)) OnCreateRequested.Broadcast(Entry);
		});
	}
	RefreshEntries();
}

void UWidgetTestingCatalogView::NativeDestruct()
{
	Input_Search->OnValueChanged.RemoveDynamic(this, &ThisClass::OnSearchChanged);
	if (Btn_AddToInventory) Btn_AddToInventory->OnClicked().RemoveAll(this);
	if (Btn_Create) Btn_Create->OnClicked().RemoveAll(this);
	Super::NativeDestruct();
}

void UWidgetTestingCatalogView::SetEntries(const TArray<FToolCatalogEntry>& InEntries)
{
	const FPrimaryAssetId PreviousId = Entries.IsValidIndex(SelectedIndex) ? Entries[SelectedIndex].GetAssetId() : FPrimaryAssetId();
	const FSoftObjectPath PreviousAsset = Entries.IsValidIndex(SelectedIndex) ? Entries[SelectedIndex].Asset.ToSoftObjectPath() : FSoftObjectPath();
	const FName PreviousCategory = Entries.IsValidIndex(SelectedIndex) ? Entries[SelectedIndex].Category : NAME_None;
	Entries = InEntries;
	SelectedIndex = !PreviousId.IsValid() && PreviousAsset.IsNull() ? INDEX_NONE : Entries.IndexOfByPredicate([PreviousId, PreviousAsset, PreviousCategory](const FToolCatalogEntry& Entry)
	{
		return Entry.Category == PreviousCategory && Entry.bAvailable &&
			(PreviousId.IsValid() ? Entry.GetAssetId() == PreviousId : Entry.Asset.ToSoftObjectPath() == PreviousAsset);
	});
	SelectedButton.Reset();
	if (Text_Selected) Text_Selected->SetText(Entries.IsValidIndex(SelectedIndex) ? Entries[SelectedIndex].DisplayName : FText::GetEmpty());
	if (Text_Description) Text_Description->SetText(Entries.IsValidIndex(SelectedIndex) ? Entries[SelectedIndex].Description : FText::GetEmpty());
	ApplyIcon(Image_Selected, Entries.IsValidIndex(SelectedIndex) ? Entries[SelectedIndex].Icon : TSoftObjectPtr<UObject>());
	if (Entries.IsValidIndex(SelectedIndex))
	{
		OnSelectionChanged.Broadcast(Entries[SelectedIndex]);
	}
	else
	{
		SetCreateEnabled(false);
		SetAddToInventoryEnabled(false);
		if (PreviousId.IsValid() || !PreviousAsset.IsNull())
		{
			FToolCatalogEntry Empty;
			Empty.bAvailable = false;
			OnSelectionChanged.Broadcast(Empty);
		}
	}
	RefreshEntries();
}

void UWidgetTestingCatalogView::SetCategory(FName InCategory)
{
	if (Category != InCategory && SelectedIndex != INDEX_NONE)
	{
		ClearSelection();
		FToolCatalogEntry Empty;
		Empty.bAvailable = false;
		OnSelectionChanged.Broadcast(Empty);
	}
	Category = InCategory;
	for (const auto& Pair : CategoryButtons)
	{
		if (UCommonButton* Button = Pair.Value.Get())
		{
			if (Pair.Key == Category) Button->SetIsSelected(true, false);
			else Button->ClearSelection();
		}
	}
	RefreshEntries();
}

void UWidgetTestingCatalogView::SetCategories(const TArray<FToolCatalogCategory>& InCategories)
{
	if (!Categories || !GetOwningPlayer()) return;
	Categories->ClearChildren();
	CategoryButtons.Reset();
	for (const FToolCatalogCategory& Item : InCategories)
	{
		if (Item.Name.IsNone() || CategoryButtons.Contains(Item.Name)) continue;
		UCommonButton* Button = CreateWidget<UCommonButton>(GetOwningPlayer(), EntryButtonClass);
		if (!Button) continue;
		Button->SetTitle(Item.DisplayName);
		if (CategoryButtonStyle.IsValid()) Button->SetStyleTag(CategoryButtonStyle);
		if (EntryTextStyle.IsValid()) Button->SetTitleTextStyleTag(EntryTextStyle);
		Button->SetIsSelectable(true);
		Button->SetTriggerClickedAfterSelection(true);
		Categories->AddChild(Button);
		CategoryButtons.Add(Item.Name, Button);
		const FName Name = Item.Name;
		Button->OnClicked().AddWeakLambda(this, [this, Name]()
		{
			SetCategory(Name);
			OnCategoryChanged.Broadcast(Name);
		});
	}
	SetCategory(Category);
}

void UWidgetTestingCatalogView::ClearSelection()
{
	SetAddToInventoryEnabled(false);
	SetCreateEnabled(false);
	SelectedIndex = INDEX_NONE;
	if (SelectedButton.IsValid()) SelectedButton->ClearSelection();
	SelectedButton.Reset();
	if (Text_Selected) Text_Selected->SetText(FText::GetEmpty());
	if (Text_Description) Text_Description->SetText(FText::GetEmpty());
	ApplyIcon(Image_Selected, TSoftObjectPtr<UObject>());
}

void UWidgetTestingCatalogView::SetAddToInventoryEnabled(bool bInEnabled)
{
	bAddToInventoryEnabled = bInEnabled;
	if (Btn_AddToInventory) Btn_AddToInventory->SetIsEnabledN(bAddToInventoryEnabled);
}

void UWidgetTestingCatalogView::SetCreateEnabled(bool bInEnabled)
{
	bCreateEnabled = bInEnabled;
	if (Btn_Create) Btn_Create->SetIsEnabledN(bCreateEnabled);
}

bool UWidgetTestingCatalogView::GetSelection(FToolCatalogEntry& OutEntry) const
{
	if (!Entries.IsValidIndex(SelectedIndex)) return false;
	OutEntry = Entries[SelectedIndex];
	return true;
}

void UWidgetTestingCatalogView::OnSearchChanged(UCommonInput* InInput, const FParameter& InValue)
{
	RefreshEntries();
}

void UWidgetTestingCatalogView::RefreshEntries()
{
	if (!Input_Search || !List_Entries || !GetOwningPlayer()) return;
	List_Entries->ClearChildren();
	SelectedButton.Reset();
	const FString Filter = Input_Search->GetText().ToString().TrimStartAndEnd();
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		const FToolCatalogEntry& Entry = Entries[Index];
		if ((!Category.IsNone() && Entry.Category != Category) ||
			(!Filter.IsEmpty() && !Entry.DisplayName.ToString().Contains(Filter, ESearchCase::IgnoreCase) &&
				!Entry.AssetId.ToString().Contains(Filter, ESearchCase::IgnoreCase) && !Entry.ItemId.ToString().Contains(Filter, ESearchCase::IgnoreCase))) continue;
		UCommonButton* Button = CreateWidget<UCommonButton>(GetOwningPlayer(), EntryButtonClass);
		if (!Button) continue;
		Button->SetTitle(Entry.DisplayName);
		FSlateBrush IconBrush;
		IconBrush.SetResourceObject(Entry.Icon.LoadSynchronous());
		IconBrush.ImageSize = FVector2D(32.f, 32.f);
		Button->SetIconBrush(IconBrush);
		Button->SetContentSpacing(8.f);
		if (EntryButtonStyle.IsValid()) Button->SetStyleTag(EntryButtonStyle);
		if (EntryTextStyle.IsValid()) Button->SetTitleTextStyleTag(EntryTextStyle);
		Button->SetIsSelectable(true);
		Button->SetTriggerClickedAfterSelection(true);
		Button->SetIsEnabledN(Entry.bAvailable);
		Button->SetToolTipText(Entry.bAvailable ? Entry.DisplayName : Entry.UnavailableReason);
		CastChecked<UScrollBoxSlot>(List_Entries->AddChild(Button))->SetPadding(FMargin(2.f, 3.f));
		const TWeakObjectPtr<UCommonButton> WeakButton(Button);
		Button->OnClicked().AddWeakLambda(this, [this, Index, WeakButton]() { SelectEntry(Index, WeakButton.Get()); });
		if (SelectedIndex == Index) { SelectedButton = Button; Button->SetIsSelected(true, false); }
	}
}

void UWidgetTestingCatalogView::SelectEntry(int32 InIndex, UCommonButton* InButton)
{
	if (!Entries.IsValidIndex(InIndex) || !Entries[InIndex].bAvailable) return;
	if (SelectedButton.IsValid() && SelectedButton.Get() != InButton) SelectedButton->ClearSelection();
	SelectedIndex = InIndex;
	SelectedButton = InButton;
	if (InButton) InButton->SetIsSelected(true, false);
	const FToolCatalogEntry& Entry = Entries[InIndex];
	if (Text_Selected) Text_Selected->SetText(Entry.DisplayName);
	if (Text_Description) Text_Description->SetText(Entry.Description);
	ApplyIcon(Image_Selected, Entry.Icon);
	OnSelectionChanged.Broadcast(Entry);
}

void UWidgetTestingCatalogView::ApplyIcon(UImage* InImage, const TSoftObjectPtr<UObject>& InIcon)
{
	if (!InImage) return;
	UObject* Icon = InIcon.LoadSynchronous();
	if (UTexture2D* Texture = Cast<UTexture2D>(Icon)) InImage->SetBrushFromTexture(Texture);
	else if (UMaterialInterface* Material = Cast<UMaterialInterface>(Icon)) InImage->SetBrushFromMaterial(Material);
	else InImage->SetBrushFromTexture(nullptr);
}
