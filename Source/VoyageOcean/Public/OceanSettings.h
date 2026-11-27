#pragma once
#include "OceanTypes.h"
#include "Runtime/DeveloperSettings/Public/Engine/DeveloperSettings.h"
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
};
