#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/PrimaryAssetId.h"
#include "TestingCatalogTypes.generated.h"

USTRUCT(BlueprintType)
struct WHFRAMEWORKUTILITIES_API FTestingInputHint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing", meta = (Categories = "Input"))
	TArray<FGameplayTag> Actions;
};

USTRUCT(BlueprintType)
struct WHFRAMEWORKUTILITIES_API FToolCatalogCategory
{
	GENERATED_BODY()

	FToolCatalogCategory() = default;
	FToolCatalogCategory(FName InName, FText InDisplayName) : Name(InName), DisplayName(MoveTemp(InDisplayName)) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	FText DisplayName;
};

USTRUCT(BlueprintType)
struct WHFRAMEWORKUTILITIES_API FToolCatalogEntry
{
	GENERATED_BODY()

	/** Identity of the displayed asset when it differs from the inventory item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	FPrimaryAssetId AssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	FPrimaryAssetId ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	bool bPlaceable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	int32 MaxLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	TSoftObjectPtr<UObject> Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	FName Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	TSoftObjectPtr<UObject> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	bool bAvailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catalog")
	FText UnavailableReason;

	FPrimaryAssetId GetAssetId() const { return AssetId.IsValid() ? AssetId : ItemId; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FToolCatalogSelectionChanged, const FToolCatalogEntry&, Entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FToolCatalogCategoryChanged, FName, Category);

