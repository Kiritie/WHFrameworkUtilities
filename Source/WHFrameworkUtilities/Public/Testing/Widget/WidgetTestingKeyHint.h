#pragma once

#include "Widget/Screen/UserWidgetBase.h"
#include "WidgetTestingKeyHint.generated.h"

class UWidgetInputKeyTips;
class UVerticalBox;

/** Screen wrapper for the configured testing panel key. */
UCLASS()
class WHFRAMEWORKUTILITIES_API UWidgetTestingKeyHint : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	UWidgetTestingKeyHint(const FObjectInitializer& InInitializer);
	virtual void OnOpen(const FParameter& InParam, bool bInstant) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetInputKeyTips> KeyTip;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> Hints;
};
