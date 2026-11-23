#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Testing/TestingCatalogTypes.h"
#include "TestingCatalogData.generated.h"

/** Optional reusable entries supplied by a tool or a project. */
UCLASS(BlueprintType)
class WHFRAMEWORKUTILITIES_API UTestingCatalogData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Testing")
	TArray<FToolCatalogEntry> Entries;
};
