#pragma once

#include "Blueprint/UserWidget.h"
#include "Widget/WidgetModuleTypes.h"
#include "WidgetInventorySplit.generated.h"

class UCommonRange;

USTRUCT(BlueprintType)
struct WHFRAMEWORKUTILITIES_API FWidgetInventorySplitParameter : public FWidgetOpenParameter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StackCount = 0;
};

/** 弹窗中的物品拆分数量输入 */
UCLASS(BlueprintType, Blueprintable)
class WHFRAMEWORKUTILITIES_API UWidgetInventorySplit : public UUserWidget
{
	GENERATED_BODY()

public:
	UWidgetInventorySplit(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	void Configure(const FParameter& InParam);

	UFUNCTION(BlueprintPure)
	int32 GetQuantity() const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	UPROPERTY(Transient)
	TObjectPtr<UCommonRange> SplitCount;
};
