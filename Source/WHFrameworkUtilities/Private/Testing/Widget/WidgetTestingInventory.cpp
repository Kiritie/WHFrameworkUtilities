#include "Testing/Widget/WidgetTestingInventory.h"

#include "Components/Image.h"
#include "Testing/TestingModule.h"
#include "Widget/Common/CommonTextBlockN.h"

void UWidgetTestingInventory::OnOpen(const FParameter& InParam, bool bInstant)
{
	UTestingModule* Testing = UTestingModule::GetPtr(GetWorld());
	TestingModule = Testing;
	if (Testing) Testing->OnCatalogSelectionChanged.AddUniqueDynamic(this, &ThisClass::SetSelection);
	Super::OnOpen(InParam, bInstant);
	FToolCatalogEntry Entry;
	if (Testing && Testing->GetCatalogSelection(Entry)) SetSelection(Entry);
	else SetVisibility(ESlateVisibility::Collapsed);
}

void UWidgetTestingInventory::OnClose(bool bInstant)
{
	if (UTestingModule* Testing = TestingModule.Get()) Testing->OnCatalogSelectionChanged.RemoveDynamic(this, &ThisClass::SetSelection);
	TestingModule.Reset();
	Super::OnClose(bInstant);
}

void UWidgetTestingInventory::SetSelection(const FToolCatalogEntry& InEntry)
{
	SetVisibility(InEntry.ItemId.IsValid() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (Text_Name) Text_Name->SetText(InEntry.DisplayName);
	UWidgetTestingCatalogView::ApplyIcon(Image_Icon, InEntry.Icon);
}
