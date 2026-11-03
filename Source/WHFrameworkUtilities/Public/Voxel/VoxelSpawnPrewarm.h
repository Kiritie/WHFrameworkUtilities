#pragma once

#include "CoreMinimal.h"

class UVoxelModule;

class WHFRAMEWORKUTILITIES_API FVoxelSpawnPrewarm
{
public:
	static bool CalculateCollisionRadius(const UVoxelModule& InModule, const FIntVector& InCenter,
		const FVector& InDesired, float InRadius, float InHalfHeight, float InSearchDistance,
		int32& OutRadius, FString& OutError);
};
