#include "GerstnerWaveSet.h"
#include "GerstnerWaveSet.h"

#include "VoyageOcean.h"

namespace
{
	constexpr double GravityCm = 980.665;
	constexpr double MaxPhysicalKA = 0.45;
}

FGerstnerWaveSet::FGerstnerWaveSet(TConstArrayView<FGerstnerWave> InWaves, double InSeaLevel, int32 InHeightIterations,
                                   double InMaxSteepnessSum) : SeaLevel(InSeaLevel),
                                                            HeightIterations(FMath::Max(InHeightIterations, 1))
{
	Waves.Reserve(InWaves.Num());
	for (const FGerstnerWave& Wave : InWaves)
	{
		if (Wave.WaveLength < 10.f || Wave.Amplitude <= 0.f)
		{
			continue;
		}

		const double DirRad = FMath::DegreesToRadians(static_cast<double>(Wave.DirectionDegrees));
		FWaveRuntime R;
		R.Dir = FVector2D(FMath::Cos(DirRad), FMath::Sin(DirRad));
		R.K = UE_DOUBLE_TWO_PI / Wave.WaveLength;
		R.Omega = FMath::Sqrt(GravityCm * R.K);
		R.Amplitude = Wave.Amplitude;
		R.QA = FMath::Clamp(static_cast<double>(Wave.Steepness), 0.0, 1.0) * Wave.Amplitude;
		R.Phase = FMath::DegreesToRadians(static_cast<double>(Wave.PhaseDegrees));

		if (R.K * R.Amplitude > MaxPhysicalKA)
		{
			UE_LOG(LogOcean, Warning,
			       TEXT("Gerstner wave L=%.0fcm A=%.0fcm is steeper than a real wave can be (kA=%.2f > %.2f)."),
			       Wave.WaveLength, Wave.Amplitude, R.K * R.Amplitude, MaxPhysicalKA);
		}
		SteepnessSum += R.QA * R.K;
		Waves.Add(R);
	}

	if (SteepnessSum > InMaxSteepnessSum && SteepnessSum > 0.0)
	{
		const double Scale = InMaxSteepnessSum / SteepnessSum;
		for (FWaveRuntime& R : Waves)
		{
			R.QA *= Scale;
		}
		UE_LOG(LogOcean, Warning, TEXT("Gerstner steepness sum %.2f exceeds %.2f; horizontal motion scaled by %.2f."),
		       SteepnessSum, InMaxSteepnessSum, Scale);
		SteepnessSum = InMaxSteepnessSum;
	}
}

TArray<FGerstnerWave> FGerstnerWaveSet::MakeTestWaves()
{
	auto Make = [](float Wavelength, float Amplitude, float Direction, float Steepness, float Phase)
	{
		FGerstnerWave W;
		W.WaveLength = Wavelength;
		W.Amplitude = Amplitude;
		W.DirectionDegrees = Direction;
		W.Steepness = Steepness;
		W.PhaseDegrees = Phase;
		return W;
	};
	
	return {
		Make(12000.f, 200.f,   0.f, 0.6f,   0.f),
		Make( 6000.f, 100.f,  25.f, 0.6f,  70.f),
		Make( 3000.f,  50.f, -20.f, 0.5f, 140.f),
		Make( 1500.f,  25.f,  50.f, 0.5f, 210.f),
		Make(  700.f,  12.f, -45.f, 0.4f, 280.f),
		Make(  350.f,   6.f,  70.f, 0.4f, 330.f),
	};
	
}

FVector FGerstnerWaveSet::GetDisplacement(const FVector2D& RestXY, double Time) const
{
	FVector D;
	Evaluate(RestXY, Time, D, nullptr);
	return D;
}

double FGerstnerWaveSet::GetHeight(const FVector2D& WorldXY, double Time) const
{
	FVector D;
	Evaluate(FindRestPosition(WorldXY, Time), Time, D, nullptr);
	return SeaLevel + D.Z;
}

FVector FGerstnerWaveSet::GetNormal(const FVector2D& WorldXY, double Time) const
{
	FVector D, N;
	Evaluate(FindRestPosition(WorldXY, Time), Time, D, &N);
	return N;
}

void FGerstnerWaveSet::SampleSurface(TConstArrayView<FVector2D> WorldXY, double Time,
	TArrayView<FOceanSurfaceSample> OutSamples) const
{
	check(WorldXY.Num() == OutSamples.Num());
	for (int32 i = 0; i < WorldXY.Num(); ++i)
	{
		FVector D, N;
		Evaluate(FindRestPosition(WorldXY[i], Time), Time, D, &N);
		OutSamples[i].Height = SeaLevel + D.Z;
		OutSamples[i].Normal = N;
	}
}

FVector2D FGerstnerWaveSet::FindRestPosition(const FVector2D& WorldXY, double Time) const
{
	FVector2D Rest = WorldXY;
	for (int32 i = 0; i < HeightIterations; ++i)
	{
		FVector D;
		Evaluate(Rest, Time, D, nullptr);
		Rest = WorldXY - FVector2D(D.X, D.Y);
	}
	return Rest;
}

void FGerstnerWaveSet::Evaluate(const FVector2D& RestXY, double Time, FVector& OutDisplacement,
                                FVector* OutNormal) const
{
	OutDisplacement = FVector::ZeroVector;
	
	FVector dPdx(1.0, 0.0, 0.0);
	FVector dPdy(0.0, 1.0, 0.0);
	for (const FWaveRuntime& W: Waves)
	{
		const double TimeTerm = FMath::Fmod(W.Omega * Time, UE_DOUBLE_TWO_PI);
		const double Theta = W.K * FVector2D::DotProduct(W.Dir, RestXY) - TimeTerm + W.Phase;
		const double S = FMath::Sin(Theta);
		const double C = FMath::Cos(Theta);
		
		OutDisplacement.X += W.QA * W.Dir.X * C;
		OutDisplacement.Y += W.QA * W.Dir.Y * C;
		OutDisplacement.Z += W.Amplitude * S;
		if (OutNormal)
		{
			const double QAKS = W.QA * W.K * S;
			const double AKC = W.Amplitude * W.K * C;
			dPdx.X -= QAKS * W.Dir.X * W.Dir.X;
			dPdx.Y -= QAKS * W.Dir.X * W.Dir.Y;
			dPdx.Z += AKC  * W.Dir.X;
			dPdy.X -= QAKS * W.Dir.X * W.Dir.Y;
			dPdy.Y -= QAKS * W.Dir.Y * W.Dir.Y;
			dPdy.Z += AKC  * W.Dir.Y;
		}
	}
	if (OutNormal)
	{
		*OutNormal = FVector::CrossProduct(dPdx, dPdy).GetSafeNormal(UE_SMALL_NUMBER, FVector::UpVector);
	}
}
