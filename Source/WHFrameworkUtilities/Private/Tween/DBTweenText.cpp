// Copyright 2021-2022, DearBing. All Rights Reserved.


#include "Tween/DBTweenText.h"

UDBTweenText* UDBTweenText::DOText(FString TweenName,float DurTime, FString Text, EaseType easeType, bool IsLoop)
{
	UDBTweenText* result = NewObject<UDBTweenText>();
	result->AllocaKey(TweenName);
	result->Init(DurTime);
	result->TextValue = Text;
	result->CurrentLen = 0;
	result->SetLooping(IsLoop);
	result->CurEasyType = easeType;
	return result;
}

UDBTweenText* UDBTweenText::DOTextByCurve(FString TweenName,float DurTime /*= 1*/, FString Text /*= TEXT("")*/, UCurveFloat* Curve, bool IsLoop /*= false*/)
{
	UDBTweenText* result = NewObject<UDBTweenText>();
	result->AllocaKey(TweenName);
	result->Init(DurTime);
	result->TextValue = Text;
	result->CurrentLen = 0;
	result->SetLooping(IsLoop);
	if (Curve == nullptr)
	{
		result->CurEasyType = EaseType::Linear;
	}
	else
	{
		result->FloatCurve = Curve;
	}
	return result;
}

void UDBTweenText::UpdateFunc(float deltaTime)
{
	if (Paused)return;
	Super::UpdateFunc(deltaTime);
	if (Finished)
	{
		CurrentLen = TextValue.Len();
		OnUpdate.Broadcast(TextValue.Mid(0, CurrentLen));
		OnComplete.Broadcast(TextValue.Mid(0, CurrentLen));
		return;
	}
	if (!isInit)
	{
		isInit = true;
		OnFirst.Broadcast(TextValue.Mid(0, CurrentLen));
	}
	CurrentLen = Evaluate() * TextValue.Len();
	OnUpdate.Broadcast(TextValue.Mid(0, CurrentLen));
}
