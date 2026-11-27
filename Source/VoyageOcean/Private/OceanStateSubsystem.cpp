#include "OceanStateSubsystem.h"

#include "IOceanWaveQuery.h"
#include "OceanDebug.h"
#include "OceanSettings.h"
#include "VoyageOcean.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UOceanStateSubsystem* UOceanStateSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UOceanStateSubsystem>() : nullptr;
}

void UOceanStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const UOceanSettings* Settings = GetDefault<UOceanSettings>();
	Seed = Settings->Seed;
	SeaLevel = Settings->SeaLevel;
	SeaState = Settings->InitialSeaState;
	OceanTime = 0.0;
	FlatQuery = MakeShared<FFlatOceanWaveQuery>(SeaLevel);
	ActiveQuery = FlatQuery;
	UE_LOG(LogOcean, Log, TEXT("Ocean subsystem up: seed=%d sealevel=%.1f"), Seed, SeaLevel);
}

void UOceanStateSubsystem::Deinitialize()
{
	ActiveQuery.Reset();
	FlatQuery.Reset();
	Super::Deinitialize();
}

void UOceanStateSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (OceanDebug::CVarPaused.GetValueOnGameThread() == 0)
	{
		OceanTime += static_cast<double>(DeltaTime) * OceanDebug::CVarTimeScale.GetValueOnGameThread();
	}
#if !UE_BUILD_SHIPPING
	DrawDebugStats();
#endif

}

void UOceanStateSubsystem::SetWaveQuery(TSharedPtr<IOceanWaveQuery> NewQuery)
{
	ActiveQuery = NewQuery.IsValid() ? NewQuery : FlatQuery;
}

double UOceanStateSubsystem::GetWaterHeightAt(const FVector& WorldLocation) const
{
	return ActiveQuery->GetHeight(FVector2D(WorldLocation.X, WorldLocation.Y), OceanTime);
}

void UOceanStateSubsystem::DrawDebugStats() const
{
	if (!GEngine || OceanDebug::CVarShowStats.GetValueOnGameThread() == 0)
	{
		return;
	}
	constexpr int32 StatsKey = 0x0CEA0001; // fixed key: the line is replaced each frame instead of stacking
	const double TestHeight = ActiveQuery->GetHeight(FVector2D::ZeroVector, OceanTime);
	GEngine->AddOnScreenDebugMessage(StatsKey, 0.f, FColor::Cyan,
	                                 FString::Printf(
		                                 TEXT(
			                                 "Ocean  t=%.2fs  x%.2f%s  seed=%d  wind=%.1fm/s @ %.0fdeg  fetch=%.0fkm  H(0,0)=%.1fcm"),
		                                 OceanTime,
		                                 OceanDebug::CVarTimeScale.GetValueOnGameThread(),
		                                 OceanDebug::CVarPaused.GetValueOnGameThread() ? TEXT(" PAUSED") : TEXT(""),
		                                 Seed, SeaState.WindSpeed, SeaState.WindDirection, SeaState.Fetch, TestHeight));
}

TStatId UOceanStateSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UOceanStateSubsystem, STATGROUP_Tickables);
}

bool UOceanStateSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE || WorldType == EWorldType::Editor;
}
