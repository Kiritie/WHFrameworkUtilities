#pragma once

#include "Input/Manager/InputBindingBase.h"
#include "TestingInputBinding.generated.h"

/** Binds the shared testing panel input for the owning local player. */
UCLASS()
class WHFRAMEWORKUTILITIES_API UTestingInputBinding : public UInputBindingBase
{
	GENERATED_BODY()

public:
	UTestingInputBinding();
	virtual void OnBindInput(UInputComponentBase* InInputComponent) override;
	virtual void OnUnbindInput() override;

private:
	void PreviousPlacement();
	void NextPlacement();
	void CyclePlacement(int32 InDirection);
};
