// Copyright 2021-2022, DearBing. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DBTweenBase.h"
#include "DBTweenText.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHandleText, FString, curValue);
UCLASS()
class WHFRAMEWORKUTILITIES_API UDBTweenText : public UDBTweenBase
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"),Category = "DBTweenUtil | DBTweenText")
		static UDBTweenText* DOText(FString TweenName,float DurTime = 1, FString Text = TEXT(""), EaseType easeType = EaseType::Linear, bool IsLoop = false);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "DBTweenUtil | DBTweenText")
		static UDBTweenText* DOTextByCurve(FString TweenName,float DurTime = 1, FString Text = TEXT(""), UCurveFloat* Curve = nullptr,bool IsLoop = false);

	UPROPERTY(BlueprintAssignable)
		FHandleText OnFirst;

	UPROPERTY(BlueprintAssignable)
		FHandleText OnUpdate;

	UPROPERTY(BlueprintAssignable)
		FHandleText OnComplete;

	FString TextValue;
	int32 CurrentLen;
protected:
	virtual void UpdateFunc(float deltaTime) override;
};
