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
