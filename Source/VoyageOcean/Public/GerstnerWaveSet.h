#pragma once
#include "CoreMinimal.h"
#include "IOceanWaveQuery.h"
#include "OceanTypes.h"

struct FGerstnerWave;

class VOYAGEOCEAN_API FGerstnerWaveSet final : public IOceanWaveQuery
{
public:
	FGerstnerWaveSet(TConstArrayView<FGerstnerWave> InWaves, double InSeaLevel, int32 InHeightIterations,
	                 double InMaxSteepnessSum);
	
	static TArray<FGerstnerWave> MakeTestWaves();
	
	virtual FVector GetDisplacement(const FVector2D& RestXY, double Time) const override;
	virtual double GetHeight(const FVector2D& WorldXY, double Time) const override;
	virtual FVector GetNormal(const FVector2D& WorldXY, double Time) const override;
	virtual void SampleSurface(TConstArrayView<FVector2D> WorldXY, double Time, TArrayView<FOceanSurfaceSample> OutSamples) const override;
	
	FVector2D FindRestPosition(const FVector2D& WorldXY, double Time) const;
	int32 Num() const { return Waves.Num(); }
	double GetSteepnessSum() const { return SteepnessSum; }
	
private:
	struct FWaveRuntime
	{
		FVector2D Dir;		//unit direction of travel
		double K;			//wavenumber
		double Omega;		//angular frequency
		double Amplitude;	//vertical
		double QA;			//horizontal amplitude = Q * A
		double Phase;
	};
	
	void Evaluate(const FVector2D& RestXY, double Time, FVector& OutDisplacement, FVector* OutNormal) const;
	TArray<FWaveRuntime> Waves;
	double SeaLevel;
	int32 HeightIterations = 4;
	double SteepnessSum = 0.0;
};
