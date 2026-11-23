#include "Testing/Widget/WidgetTestingPanel.h"

#include "InputCoreTypes.h"
#include "Testing/TestingModule.h"
#include "Widget/Common/CommonWindow.h"
#include "Widget/ScreenWidgetTypes.h"

UWidgetTestingPanel::UWidgetTestingPanel(const FObjectInitializer& InInitializer)
	: Super(InInitializer)
{
	WidgetInputMode = EWidgetInputMode::UIOnly;
	bWidgetActivatable = true;
	bWidgetAutoFocus = true;
	SetIsFocusable(true);
}

void UWidgetTestingPanel::NativeConstruct()
{
	Super::NativeConstruct();
	Window_Content->OnCloseRequested.AddUniqueDynamic(this, &ThisClass::ClosePanel);
	CatalogView->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::OnCatalogSelection);
	CatalogView->OnAddToInventory.AddUniqueDynamic(this, &ThisClass::OnAddToInventory);
	CatalogView->OnCreateRequested.AddUniqueDynamic(this, &ThisClass::OnCreateRequested);
	CatalogView->OnCategoryChanged.AddUniqueDynamic(this, &ThisClass::OnCategoryChanged);
	CatalogView->SetCategories(CatalogCategories);
}

void UWidgetTestingPanel::NativeDestruct()
{
	Window_Content->OnCloseRequested.RemoveDynamic(this, &ThisClass::ClosePanel);
	CatalogView->OnSelectionChanged.RemoveDynamic(this, &ThisClass::OnCatalogSelection);
	CatalogView->OnAddToInventory.RemoveDynamic(this, &ThisClass::OnAddToInventory);
	CatalogView->OnCreateRequested.RemoveDynamic(this, &ThisClass::OnCreateRequested);
	CatalogView->OnCategoryChanged.RemoveDynamic(this, &ThisClass::OnCategoryChanged);
	Super::NativeDestruct();
}

FReply UWidgetTestingPanel::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent)
{
	const UTestingModule* Testing = UTestingModule::GetPtr(GetWorld());
	if (Testing && Testing->IsPanelKey(GetOwningPlayer(), KeyEvent.GetKey()))
	{
		Close();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(Geometry, KeyEvent);
}

void UWidgetTestingPanel::OnCatalogSelection(const FToolCatalogEntry& InEntry)
{
	HandleCatalogSelection(InEntry);
}

void UWidgetTestingPanel::OnAddToInventory(const FToolCatalogEntry& InEntry)
{
	HandleAddToInventory(InEntry);
}

void UWidgetTestingPanel::OnCreateRequested(const FToolCatalogEntry& InEntry)
{
	HandleCreate(InEntry);
}

void UWidgetTestingPanel::OnCategoryChanged(FName InCategory)
{
	HandleCategoryChanged(InCategory);
}

void UWidgetTestingPanel::ClosePanel()
{
	Close();
}
