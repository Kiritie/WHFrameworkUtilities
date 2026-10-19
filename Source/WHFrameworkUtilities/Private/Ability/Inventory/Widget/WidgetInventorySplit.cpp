#include "Ability/Inventory/Widget/WidgetInventorySplit.h"

#include "Widget/Common/CommonRange.h"
#include "Blueprint/WidgetTree.h"

UWidgetInventorySplit::UWidgetInventorySplit(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

TSharedRef<SWidget> UWidgetInventorySplit::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	if (!WidgetTree->RootWidget)
	{
		SplitCount = WidgetTree->ConstructWidget<UCommonRange>(UCommonRange::StaticClass(), TEXT("SplitCount"));
		WidgetTree->RootWidget = SplitCount;
	}
	return Super::RebuildWidget();
}

void UWidgetInventorySplit::Configure(const FParameter& InParam)
{
	const FWidgetInventorySplitParameter& Parameter = InParam.GetRef<FWidgetInventorySplitParameter>();
	check(Parameter.StackCount > 1);
	FCommonRangeConfig Config;
	Config.DecimalPlaces = 0;
	Config.bHasMin = true;
	Config.bHasMax = true;
	Config.Min = 1;
	Config.Max = Parameter.StackCount - 1;
	Config.Step = 1;
	Config.bShowStepButtons = true;
	SplitCount->Configure(Config);
	SplitCount->SetValue(FParameter(Parameter.StackCount / 2));
	SplitCount->SetInputFocus();
}

int32 UWidgetInventorySplit::GetQuantity() const
{
	return SplitCount->GetValue().Get<int32>();
}
