#include "Voxel/VoxelSpawnPrewarm.h"

#include "Voxel/Chunks/VoxelSectionKey.h"
#include "Voxel/VoxelModule.h"

bool FVoxelSpawnPrewarm::CalculateCollisionRadius(const UVoxelModule& InModule, const FIntVector& InCenter,
	const FVector& InDesired, const float InRadius, const float InHalfHeight, const float InSearchDistance,
	int32& OutRadius, FString& OutError)
{
	OutError.Reset();
	if (!InModule.IsReady() || !VoxelCoord::IsValid(InCenter) || InDesired.ContainsNaN() ||
		!FMath::IsFinite(InRadius) || !FMath::IsFinite(InHalfHeight) || !FMath::IsFinite(InSearchDistance) ||
		InRadius <= 0.f || InHalfHeight < InRadius || InSearchDistance < 0.f || InSearchDistance > 4096.f)
	{
		OutError = TEXT("体素出生位置或胶囊尺寸无效。");
		return false;
	}
	const FVector Extent(InRadius + 2.f, InRadius + 2.f, InHalfHeight + InSearchDistance + 2.f);
	FIntVector MinimumCell;
	FIntVector MaximumCell;
	if (!VoxelCoord::FromWorld(InDesired - Extent, InModule.BlockSize(), MinimumCell) ||
		!VoxelCoord::FromWorld(InDesired + Extent, InModule.BlockSize(), MaximumCell))
	{
		OutError = TEXT("体素出生碰撞查询范围超出世界坐标。");
		return false;
	}
	const FVoxelSectionKey Minimum = VoxelCoord::Section(MinimumCell);
	const FVoxelSectionKey Maximum = VoxelCoord::Section(MaximumCell);
	const int64 CountX = int64(Maximum.X) - Minimum.X + 1;
	const int64 CountY = int64(Maximum.Y) - Minimum.Y + 1;
	const int64 CountZ = int64(Maximum.Z) - Minimum.Z + 1;
	if (CountX <= 0 || CountY <= 0 || CountZ <= 0 || CountX > 512 ||
		CountY > 512 / CountX || CountZ > 512 / (CountX * CountY))
	{
		OutError = TEXT("体素出生碰撞查询超过支持的 section 数量。");
		return false;
	}
	const FVoxelGenerationSettings& Settings = InModule.GetManifest().Settings;
	const FIntVector First(Minimum.X, Minimum.Y,
		FMath::Max(Minimum.Z, VoxelCoord::FloorDiv(Settings.MinZ, VoxelBlock::Size)));
	const FIntVector Last(Maximum.X, Maximum.Y,
		FMath::Min(Maximum.Z, VoxelCoord::FloorDiv(Settings.MaxZ - 1, VoxelBlock::Size)));
	if (First.Z > Last.Z)
	{
		OutError = TEXT("体素出生碰撞查询不在世界高度范围内。");
		return false;
	}
	int64 AxisRadius = 0;
	double MaxCenterDistanceSquared = 0.0;
	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		const int64 Center = InCenter[Axis];
		const int64 FirstIndex = First[Axis];
		const int64 LastIndex = Last[Axis];
		AxisRadius = FMath::Max(AxisRadius, Center - ((FirstIndex + 1) * VoxelBlock::Size - 1));
		AxisRadius = FMath::Max(AxisRadius, LastIndex * VoxelBlock::Size - Center);
		const int64 FirstDistance = FMath::Abs(FirstIndex * VoxelBlock::Size + VoxelBlock::Size / 2 - Center);
		const int64 LastDistance = FMath::Abs(LastIndex * VoxelBlock::Size + VoxelBlock::Size / 2 - Center);
		const double Distance = static_cast<double>(FMath::Max(FirstDistance, LastDistance));
		MaxCenterDistanceSquared += Distance * Distance;
	}
	const double SectionHalfDiagonal = FMath::Sqrt(3.0) * (VoxelBlock::Size / 2.0);
	int64 Required = FMath::Max<int64>(16, FMath::Max(AxisRadius,
		FMath::CeilToInt64(FMath::Max(0.0, FMath::Sqrt(MaxCenterDistanceSquared) - SectionHalfDiagonal))));
	while (FMath::Square(static_cast<double>(Required) + SectionHalfDiagonal) < MaxCenterDistanceSquared)
	{
		++Required;
	}
	if (Required > MAX_int32 - 32)
	{
		OutError = TEXT("体素出生碰撞预热半径超出支持范围。");
		return false;
	}
	OutRadius = static_cast<int32>(Required);
	return true;
}
