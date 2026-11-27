#pragma once
#include "CoreMinimal.h"
#include "OceanTypes.generated.h"

USTRUCT(BlueprintType)
struct VOYAGEOCEAN_API FOceanSeaState
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean", meta = (ClampMin = "0", Units = "MetersPerSecond"))
	float WindSpeed = 8.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean", meta = (Units = "Degrees"))
	float WindDirection = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean", meta = (ClampMin = "1", Units = "Kilometers"))
	float Fetch = 100.f;
};

//A point on the ocean surface. Used for buoyancy calculations.
struct FOceanSurfaceSample
{
	double Height = 0.0;
	FVector Normal = FVector::UpVector;
};

USTRUCT(BlueprintType)
struct VOYAGEOCEAN_API FGerstnerWave
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wave, meta = (ClampMin = "10", Units = "Centimeters"))
	float WaveLength = 6000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wave, meta = (ClampMin = "10", Units = "Centimeters"))
	float Amplitude = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wave, meta = (Units = "Degrees"))
	float DirectionDegrees = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wave, meta = (ClampMin = "0", ClampMax = "1"))
	float Steepness = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wave, meta = (Units = "Degrees"))
	float PhaseDegrees = 0.f;
};
