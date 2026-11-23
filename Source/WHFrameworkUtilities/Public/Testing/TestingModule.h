#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Main/Base/ModuleBase.h"
#include "Main/MainModuleAccess.h"
#include "Testing/TestingCatalogTypes.h"
#include "TestingModule.generated.h"

class UTestingCatalogData;
class UUserWidgetBase;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FToolCatalogReady);

/** Shared state for tool and project testing sessions. */
UCLASS()
class WHFRAMEWORKUTILITIES_API UTestingModule : public UModuleBase
{
	GENERATED_BODY()
	GENERATED_MODULE(UTestingModule)

public:
	UTestingModule();
	virtual void OnTermination(EPhase InPhase) override;
	virtual bool IsPanelAvailable() const { return true; }
	virtual bool UsesStandaloneKeyHints() const { return true; }
	bool IsPanelKey(APlayerController* InController, FKey InKey) const;
	const TArray<FTestingInputHint>& GetInputHints() const { return InputHints; }
	void TogglePanel(APlayerController* InController);
	virtual bool UsesPlacementList() const { return false; }
	virtual bool SelectPlacement(APlayerController* InController, FPrimaryAssetId InItemId) { return false; }
	bool AddPlacement(APlayerController* InController, const FToolCatalogEntry& InEntry);
	bool CyclePlacement(APlayerController* InController, int32 InDirection);

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void SetCatalogSelection(const FToolCatalogEntry& InEntry);

	UFUNCTION(BlueprintPure, Category = "Testing")
	bool GetCatalogSelection(FToolCatalogEntry& OutEntry) const;

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void ClearCatalogSelection();

	UFUNCTION(BlueprintPure, Category = "Testing")
	void GetConfiguredCatalogEntries(TArray<FToolCatalogEntry>& OutEntries) const;

	UPROPERTY(BlueprintAssignable, Category = "Testing")
	FToolCatalogSelectionChanged OnCatalogSelectionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Testing")
	FToolCatalogReady OnCatalogReady;

protected:
	UPROPERTY(EditAnywhere, Category = "Testing")
	TArray<TObjectPtr<UTestingCatalogData>> Catalogs;

	UPROPERTY(EditAnywhere, Category = "Testing|UI")
	TSubclassOf<UUserWidgetBase> PanelClass;

	UPROPERTY(EditAnywhere, Category = "Testing|UI")
	TArray<FTestingInputHint> InputHints;

private:
	UPROPERTY(Transient)
	FToolCatalogEntry CatalogSelection;

	UPROPERTY(Transient)
	TArray<FToolCatalogEntry> PlacementEntries;

	int32 PlacementIndex = INDEX_NONE;

	bool bHasCatalogSelection = false;
};
