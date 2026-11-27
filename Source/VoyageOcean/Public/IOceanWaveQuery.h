#pragma once
#include "CoreMinimal.h"
#include "OceanTypes.h"

class VOYAGEOCEAN_API IOceanWaveQuery
{
public:
	virtual ~IOceanWaveQuery() = default;
	//Offset of surface point that started at RestXY
	//SurfacePoint = (RestXY, SeaLevel) + Displacement
	virtual FVector GetDisplacement(const FVector2D& RestXY, double Time) const = 0;
	
	virtual double GetHeight(const FVector2D& WorldXY, double Time) const = 0;
	virtual FVector GetNormal(const FVector2D& WorldXY, double Time) const = 0;
	
	//Batched query for buoyancy
	virtual void SampleSurface(TConstArrayView<FVector2D> WorldXY, double Time, TArrayView<FOceanSurfaceSample> OutSamples) const
	{
		check(WorldXY.Num() == OutSamples.Num());
		for (int32 i = 0; i < WorldXY.Num(); ++i)
		{
			OutSamples[i].Height = GetHeight(WorldXY[i], Time);
			OutSamples[i].Normal = GetNormal(WorldXY[i], Time);
		}
	}
};

class VOYAGEOCEAN_API FFlatOceanWaveQuery final : public IOceanWaveQuery
{
public: 
	explicit FFlatOceanWaveQuery(double InSeaLevel) : SeaLevel(InSeaLevel) {};
	virtual FVector GetDisplacement(const FVector2D& WorldXY, double) const override { return FVector::ZeroVector;}
	virtual double GetHeight(const FVector2D& WorldXY, double) const override { return SeaLevel;}
	virtual FVector GetNormal(const FVector2D& WorldXY, double) const override { return FVector::UpVector;}
	
private:
	double SeaLevel;
};
