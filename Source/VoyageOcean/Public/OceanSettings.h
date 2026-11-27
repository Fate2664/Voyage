#pragma once
#include "OceanTypes.h"
#include "Engine/DeveloperSettings.h"
#include "OceanSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Ocean"))
class VOYAGEOCEAN_API UOceanSettings : public UDeveloperSettings 
{
	GENERATED_BODY()
public:
	UPROPERTY(Config, EditAnywhere, Category = Ocean)
	int32 Seed = 1337;
	UPROPERTY(Config, EditAnywhere, Category = Ocean, meta = (Units = "Centimeters"))
	double SeaLevel = 0.0;
	UPROPERTY(Config, EditAnywhere, Category = Ocean)
	FOceanSeaState InitialSeaState;
	UPROPERTY(Config, EditAnywhere, Category = "Ocean|Waves")
	bool bUseTestWaves = true;
	UPROPERTY(Config, EditAnywhere, Category = "Ocean|Waves")
	TArray<FGerstnerWave> TestWaves;
	UPROPERTY(Config, EditAnywhere, Category = "Ocean|Waves", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MaxSteepnessSum = 0.9f;
	UPROPERTY(Config, EditAnywhere, Category = "Ocean|Waves", meta = (ClampMin = "1", ClampMax = "8"))
	int32 HeightQueryIterations = 4;
};
