#include "OceanStateSubsystem.h"

#include "IOceanWaveQuery.h"
#include "OceanDebug.h"
#include "OceanSettings.h"
#include "VoyageOcean.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GerstnerWaveSet.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

UOceanStateSubsystem* UOceanStateSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UOceanStateSubsystem>() : nullptr;
}

void UOceanStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	OceanTime = 0.0;
	RefreshFromSettings();
#if WITH_EDITOR
	SettingsChangedHandle = GetMutableDefault<UOceanSettings>()->OnSettingChanged().AddUObject(
		this, &UOceanStateSubsystem::HandleSettingsChanged);
#endif
}

void UOceanStateSubsystem::Deinitialize()
{
#if WITH_EDITOR
	GetMutableDefault<UOceanSettings>()->OnSettingChanged().Remove(SettingsChangedHandle);
#endif
	ActiveQuery.Reset();
	GerstnerWaves.Reset();
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
	DrawDebugSurface();
#endif
}

void UOceanStateSubsystem::SetWaveQuery(TSharedPtr<IOceanWaveQuery> NewQuery)
{
	ActiveQuery = NewQuery.IsValid() ? NewQuery : FlatQuery;
}

void UOceanStateSubsystem::RefreshFromSettings()
{
	const UOceanSettings* Settings = GetDefault<UOceanSettings>();
	Seed = Settings->Seed;
	SeaLevel = Settings->SeaLevel;
	SeaState = Settings->InitialSeaState;
	FlatQuery = MakeShared<FFlatOceanWaveQuery>(SeaLevel);
	GerstnerWaves.Reset();
	if (Settings->bUseTestWaves)
	{
		const TArray<FGerstnerWave> Source = Settings->TestWaves.Num() > 0
			                                     ? Settings->TestWaves
			                                     : FGerstnerWaveSet::MakeTestWaves();
		GerstnerWaves = MakeShared<FGerstnerWaveSet>(Source, SeaLevel, Settings->HeightQueryIterations,
		                                             Settings->MaxSteepnessSum);
	}
	SetWaveQuery(GerstnerWaves);
	UE_LOG(LogOcean, Log, TEXT("Ocean refreshed: seed=%d sealevel=%.1f waves=%d steepness=%.2f"),
	       Seed, SeaLevel,
	       GerstnerWaves ? GerstnerWaves->Num() : 0,
	       GerstnerWaves ? GerstnerWaves->GetSteepnessSum() : 0.0);
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
	GEngine->AddOnScreenDebugMessage(StatsKey + 1, 0.f, FColor::Cyan,
	FString::Printf(TEXT("Waves=%d  sum(QkA)=%.2f  (%s)"),
		GerstnerWaves ? GerstnerWaves->Num() : 0,
		GerstnerWaves ? GerstnerWaves->GetSteepnessSum() : 0.0,
		GerstnerWaves ? TEXT("Gerstner") : TEXT("Flat")));

}

void UOceanStateSubsystem::DrawDebugSurface() const
{
#if ENABLE_DRAW_DEBUG
	const int32 Mode = OceanDebug::CVarDrawSurface.GetValueOnGameThread();
	UWorld* World = GetWorld();
	if (Mode <= 0 || !World)
	{
		return;
	}
	const double Spacing = FMath::Max(10.0, static_cast<double>(OceanDebug::CVarDrawSpacing.GetValueOnGameThread()));
	const int32 HalfCount = FMath::Clamp(FMath::FloorToInt(OceanDebug::CVarDrawExtent.GetValueOnGameThread() / Spacing), 1, 100);
	const int32 Count = HalfCount * 2 + 1;
	FVector Centre = FVector::ZeroVector;
	if (const APlayerController* PC = World->GetFirstPlayerController())
	{
		if (PC->PlayerCameraManager)
		{
			Centre = PC->PlayerCameraManager->GetCameraLocation();
		}
	}
	const FVector2D Origin(FMath::GridSnap(Centre.X, Spacing) - HalfCount * Spacing,
		FMath::GridSnap(Centre.Y, Spacing) - HalfCount * Spacing);
	const IOceanWaveQuery& Query = *ActiveQuery;
	
	TArray<FVector> Points;
	Points.SetNumUninitialized(Count * Count);
	for (int32 Y = 0; Y < Count; Y++)
	{
		for (int32 X = 0; X < Count; X++)
		{
			const FVector2D Rest = Origin + FVector2D(X * Spacing, Y * Spacing);
			Points[Y * Count + X] = FVector(Rest, SeaLevel) + Query.GetDisplacement(Rest, OceanTime);
		}
	}
	const FColor MeshColor(0, 140, 255);
	for (int32 Y = 0; Y < Count; Y++)
	{
		for (int32 X = 0; X < Count; X++)
		{
			const FVector& P = Points[Y * Count + X];
			if (X + 1 < Count) DrawDebugLine(World, P, Points[Y * Count + X + 1], MeshColor, false, -1.f, 0, 1.5f);
			if (Y + 1 < Count) DrawDebugLine(World, P, Points[(Y + 1) * Count + X], MeshColor, false, -1.f, 0, 1.5f);
		}
	}
	if (Mode >= 2)
	{
		constexpr int32 ProbeStep = 4;
		for (int32 Y = 0; Y < Count; Y += ProbeStep)
		{
			for (int32 X = 0; X < Count; X += ProbeStep)
			{
				const FVector2D XY = Origin + FVector2D((X + 0.5) * Spacing, (Y + 0.5) * Spacing);
				const FVector P(XY, Query.GetHeight(XY, OceanTime));
				const FVector N = Query.GetNormal(XY, OceanTime);
				DrawDebugPoint(World, P, 8.f, FColor::Yellow, false, -1.f);
				DrawDebugLine(World, P, P + N * Spacing * 0.7f, FColor::Green, false, -1.f, 0, 1.f);
			}
		}
	}
#endif
	
	
}

#if WITH_EDITOR
void UOceanStateSubsystem::HandleSettingsChanged(UObject*, FPropertyChangedEvent&)
{
	RefreshFromSettings();
}
#endif


TStatId UOceanStateSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UOceanStateSubsystem, STATGROUP_Tickables);
}

bool UOceanStateSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE || WorldType == EWorldType::Editor;
}
