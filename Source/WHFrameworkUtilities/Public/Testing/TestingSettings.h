#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TestingSettings.generated.h"

class UTestingCatalogData;

/** Optional project-supplied entries for the shared testing tools. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Testing"))
class WHFRAMEWORKUTILITIES_API UTestingSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("WHFramework"); }

	UPROPERTY(Config, EditAnywhere, Category = "Catalog")
	TArray<TSoftObjectPtr<UTestingCatalogData>> Catalogs;
};
