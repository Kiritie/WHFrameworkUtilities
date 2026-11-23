#include "Voxel/Widget/WidgetVoxelPrefabSceneAIPanel.h"

#include "AI/Generation/AIModelRequestOptions.h"
#include "AI/Voxel/VoxelSceneAI.h"
#include "GameFramework/PlayerController.h"
#include "Scene/Object/Voxel/VoxelSceneObjectRegistry.h"
#include "Voxel/VoxelPrefabTestingModule.h"
#include "Voxel/Chunks/VoxelSectionKey.h"
#include "Voxel/Runtime/VoxelRegistry.h"
#include "Voxel/VoxelModule.h"
#include "Widget/Common/CommonButton.h"
#include "Widget/Common/CommonInput.h"
#include "Widget/Common/CommonTextBlockN.h"

UWidgetVoxelPrefabSceneAIPanel::UWidgetVoxelPrefabSceneAIPanel(const FObjectInitializer& InInitializer)
    : Super(InInitializer)
{
    WidgetInputMode = EWidgetInputMode::UIOnly;
    bWidgetActivatable = true;
    bWidgetAutoFocus = true;
    SetIsFocusable(true);
}

void UWidgetVoxelPrefabSceneAIPanel::OnCreate(const FParameter& InParam)
{
    Super::OnCreate(InParam);
    Btn_AIGenerate->OnClicked().AddWeakLambda(this, [this]() { GenerateScene(Input_AIPrompt->GetText().ToString()); });
    Btn_AIConfirm->OnClicked().AddUObject(this, &ThisClass::ConfirmScene);
    Btn_AICancel->OnClicked().AddUObject(this, &ThisClass::CancelScene);
}

void UWidgetVoxelPrefabSceneAIPanel::OnOpen(const FParameter& InParam, bool bInstant)
{
    if (UVoxelPrefabTestingModule* Testing = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld())))
    {
        if (UVoxelSceneAI* SceneAI = Testing->InitializeVoxelSceneAI())
        {
            if (SceneAI->Settings) SetAIOptions(*SceneAI->Settings);
            SceneAI->OnStatusChanged.RemoveAll(this);
            SceneAI->OnStatusChanged.AddUObject(this, &ThisClass::UpdateAIStatus);
        }
    }
    Super::OnOpen(InParam, bInstant);
    UpdateAIStatus();
}

void UWidgetVoxelPrefabSceneAIPanel::OnClose(bool bInstant)
{
    if (UVoxelPrefabTestingModule* Testing = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld())))
    {
        if (UVoxelSceneAI* SceneAI = Testing->GetVoxelSceneAI()) SceneAI->OnStatusChanged.RemoveAll(this);
    }
    Super::OnClose(bInstant);
}

FReply UWidgetVoxelPrefabSceneAIPanel::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent)
{
    if (KeyEvent.GetKey() == EKeys::Escape)
    {
        Close();
        return FReply::Handled();
    }
    return Super::NativeOnPreviewKeyDown(Geometry, KeyEvent);
}

void UWidgetVoxelPrefabSceneAIPanel::SetAIStatus(const FString& InStatus)
{
	if (Text_AIStatus) Text_AIStatus->SetText(FText::FromString(InStatus));
}

void UWidgetVoxelPrefabSceneAIPanel::SetAIOptions(const UAIModelRequestOptions& InOptions)
{
	if (Input_AIProvider) Input_AIProvider->SetText(FText::FromName(InOptions.Provider));
	if (Input_AIEndpoint) Input_AIEndpoint->SetText(FText::FromString(InOptions.Endpoint));
	if (Input_AIModel) Input_AIModel->SetText(FText::FromString(InOptions.Model));
	if (Input_AIKeyVariable) Input_AIKeyVariable->SetText(FText::FromString(InOptions.KeyEnvironmentVariable));
	if (Input_AIMaxOutputTokens) Input_AIMaxOutputTokens->SetText(FText::FromString(LexToString(InOptions.MaxOutputTokens)));
	if (Input_AIMaxRequests) Input_AIMaxRequests->SetText(FText::FromString(LexToString(InOptions.MaxRequests)));
	if (Input_AITokenBudget) Input_AITokenBudget->SetText(FText::FromString(LexToString(InOptions.SessionOutputTokenBudget)));
}

void UWidgetVoxelPrefabSceneAIPanel::ReadAIOptions(UAIModelRequestOptions& OutOptions) const
{
	if (Input_AIProvider) OutOptions.Provider = FName(*Input_AIProvider->GetText().ToString().TrimStartAndEnd());
	if (Input_AIEndpoint) OutOptions.Endpoint = Input_AIEndpoint->GetText().ToString().TrimStartAndEnd();
	if (Input_AIModel) OutOptions.Model = Input_AIModel->GetText().ToString().TrimStartAndEnd();
	if (Input_AIKeyVariable) OutOptions.KeyEnvironmentVariable = Input_AIKeyVariable->GetText().ToString().TrimStartAndEnd();
	if (Input_AIMaxOutputTokens) OutOptions.MaxOutputTokens = FCString::Atoi(*Input_AIMaxOutputTokens->GetText().ToString());
	if (Input_AIMaxRequests) OutOptions.MaxRequests = FCString::Atoi(*Input_AIMaxRequests->GetText().ToString());
	if (Input_AITokenBudget) OutOptions.SessionOutputTokenBudget = FCString::Atoi(*Input_AITokenBudget->GetText().ToString());
}

FString UWidgetVoxelPrefabSceneAIPanel::GetAllowedBlockTypes() const
{
	return Input_AIBlockTypes ? Input_AIBlockTypes->GetText().ToString().TrimStartAndEnd() : FString();
}

void UWidgetVoxelPrefabSceneAIPanel::UpdateAIStatus()
{
	const UVoxelPrefabTestingModule* Testing = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld()));
	const UVoxelSceneAI* SceneAI = Testing ? Testing->GetVoxelSceneAI() : nullptr;
	if (!SceneAI) return;
	FString Status = AIActionError.IsEmpty() ? SceneAI->GetStatus() : AIActionError;
	if (!SceneAI->GetCandidateDetails().IsEmpty()) Status += TEXT("\n") + SceneAI->GetCandidateDetails();
	SetAIStatus(Status);
}

void UWidgetVoxelPrefabSceneAIPanel::GenerateScene(const FString& InPrompt)
{
	AIActionError.Reset();
	UVoxelPrefabTestingModule* Testing = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld()));
	UVoxelSceneAI* SceneAI = Testing ? Testing->GetVoxelSceneAI() : nullptr;
	UVoxelModule* Voxel = UVoxelModule::GetPtr(GetWorld());
	APlayerController* Controller = GetOwningPlayer();
	const auto Registry = Voxel ? Voxel->GetRegistry() : nullptr;
	if (!SceneAI || !SceneAI->Settings || !Controller || !Voxel || !Voxel->IsReady() || !Registry)
	{
		AIActionError = TEXT("A ready voxel world and local player are required.");
		UpdateAIStatus();
		return;
	}
	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
	if (!VoxelCoord::FromWorld(ViewLocation + ViewRotation.Vector() * AIRegionDistance, Voxel->BlockSize(), SceneAI->Origin))
	{
		AIActionError = TEXT("The player view is outside the supported voxel world.");
		UpdateAIStatus();
		return;
	}
	ReadAIOptions(*SceneAI->Settings);
	const int32 Limit = VoxelBlock::MaxAbsCoordinate;
	SceneAI->Min = FIntVector(-Limit - SceneAI->Origin.X, -Limit - SceneAI->Origin.Y, -SceneAI->Origin.Z);
	SceneAI->Max = FIntVector(Limit - SceneAI->Origin.X, Limit - SceneAI->Origin.Y, Limit - SceneAI->Origin.Z);
	SceneAI->MaxCells = AIMaxCells;
	SceneAI->AllowedTypes.Reset();
	TArray<FString> SelectedTypes;
	GetAllowedBlockTypes().ParseIntoArray(SelectedTypes, TEXT(","), true);
	for (FString& Type : SelectedTypes) Type.TrimStartAndEndInline();
	for (const FVoxelRuntimeDefinition& Definition : Registry->Definitions)
	{
		if (Definition.TypeId && Definition.AssetID.IsValid() && Definition.EntityKind == 0 &&
			(SelectedTypes.IsEmpty() || SelectedTypes.Contains(Definition.AssetID.ToString()))) SceneAI->AllowedTypes.Add(Definition.AssetID);
	}
	if (!SelectedTypes.IsEmpty())
	{
		TSet<FString> Requested;
		for (const FString& Type : SelectedTypes) Requested.Add(Type);
		if (Requested.Num() != SelectedTypes.Num() || Requested.Num() != SceneAI->AllowedTypes.Num())
		{
			AIActionError = TEXT("The allowed block list contains a duplicate, unknown, or unsupported type.");
			UpdateAIStatus();
			return;
		}
	}
	SceneAI->Generate(InPrompt, AIActionError);
	UpdateAIStatus();
}

void UWidgetVoxelPrefabSceneAIPanel::ConfirmScene()
{
	AIActionError.Reset();
	UVoxelPrefabTestingModule* Testing = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld()));
	if (UVoxelSceneAI* SceneAI = Testing ? Testing->GetVoxelSceneAI() : nullptr) SceneAI->Confirm(AIActionError);
	UpdateAIStatus();
}

void UWidgetVoxelPrefabSceneAIPanel::CancelScene()
{
	AIActionError.Reset();
	UVoxelPrefabTestingModule* Testing = Cast<UVoxelPrefabTestingModule>(UTestingModule::GetPtr(GetWorld()));
	if (UVoxelSceneAI* SceneAI = Testing ? Testing->GetVoxelSceneAI() : nullptr) SceneAI->Cancel();
	UpdateAIStatus();
}

