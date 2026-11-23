#pragma once

#include "Widget/Screen/UserWidgetBase.h"
#include "WidgetVoxelPrefabSceneAIPanel.generated.h"

class UAIModelRequestOptions;
class UCommonButton;
class UCommonInput;
class UCommonTextBlockN;

/** Controls the AI-assisted editing session in a voxel world. */
UCLASS()
class WHFRAMEWORKUTILITIES_API UWidgetVoxelPrefabSceneAIPanel : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	UWidgetVoxelPrefabSceneAIPanel(const FObjectInitializer& InInitializer);
	virtual void OnCreate(const FParameter& InParam) override;
	virtual void OnOpen(const FParameter& InParam, bool bInstant) override;
	virtual void OnClose(bool bInstant = false) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|AI", meta = (ClampMin = "0"))
	float AIRegionDistance = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|AI", meta = (ClampMin = "1", ClampMax = "4096"))
	int32 AIMaxCells = 4096;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonInput> Input_AIPrompt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonInput> Input_AIProvider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonInput> Input_AIEndpoint;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonInput> Input_AIModel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonInput> Input_AIKeyVariable;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonInput> Input_AIBlockTypes;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonInput> Input_AIMaxOutputTokens;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonInput> Input_AIMaxRequests;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonInput> Input_AITokenBudget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlockN> Text_AIStatus;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButton> Btn_AIGenerate;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButton> Btn_AIConfirm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButton> Btn_AICancel;

private:
	void SetAIStatus(const FString& InStatus);
	void SetAIOptions(const UAIModelRequestOptions& InOptions);
	void ReadAIOptions(UAIModelRequestOptions& OutOptions) const;
	FString GetAllowedBlockTypes() const;
	void GenerateScene(const FString& InPrompt);
	void ConfirmScene();
	void CancelScene();
	void UpdateAIStatus();

	FString AIActionError;
};
