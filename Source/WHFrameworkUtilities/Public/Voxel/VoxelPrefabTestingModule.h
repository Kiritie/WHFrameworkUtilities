#pragma once

#include "Testing/TestingModule.h"
#include "VoxelPrefabTestingModule.generated.h"

class FVoxelInventoryDomainBridge;
class UInventoryProfile;
class UVoxelSceneAI;
class UVoxelAgentComponent;

/** Testing session for a standalone voxel prefab authoring world. */
UCLASS()
class WHFRAMEWORKUTILITIES_API UVoxelPrefabTestingModule : public UTestingModule
{
	GENERATED_BODY()

public:
	UVoxelPrefabTestingModule();
	virtual void OnRefresh(float InDeltaSeconds, bool bInEditor) override;
	virtual void OnTermination(EPhase InPhase) override;
	virtual bool UsesPlacementList() const override { return true; }
	virtual bool SelectPlacement(APlayerController* InController, FPrimaryAssetId InItemId) override;
	bool CreatePlacement(APlayerController* InController, FPrimaryAssetId InItemId);
	UVoxelAgentComponent* GetPlacementAgent(APlayerController* InController);
	void ToggleAIPanel(APlayerController* InController);
	UVoxelSceneAI* GetVoxelSceneAI() const { return VoxelSceneAI; }
	UVoxelSceneAI* InitializeVoxelSceneAI();
	bool IsInventoryReady() const { return bInventoryStarted && StartupError.IsEmpty(); }
	const FString& GetStartupError() const { return StartupError; }

protected:
	UPROPERTY(EditAnywhere, Category = "Testing|Voxel")
	TObjectPtr<UInventoryProfile> TransientInventoryProfile;

	UPROPERTY(EditAnywhere, Category = "Testing|UI")
	TSubclassOf<UUserWidgetBase> AIPanelClass;

private:
	bool StartTransientInventory(FString& OutError);
	bool bInventoryStarted = false;
	bool bCameraInitialized = false;
	FString StartupError;

	UPROPERTY(Transient)
	TObjectPtr<UVoxelSceneAI> VoxelSceneAI;

	UPROPERTY(Transient)
	TObjectPtr<UVoxelAgentComponent> PlacementAgent;
	TSharedPtr<FVoxelInventoryDomainBridge> InventoryBridge;
	TSharedPtr<class FSceneObjectInventoryDomainBridge> SceneBridge;
};
