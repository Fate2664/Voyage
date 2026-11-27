#pragma once
#include "OceanTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "OceanStateSubsystem.generated.h"

class IOceanWaveQuery;
class FGerstnerWaveSet;

UCLASS()
class VOYAGEOCEAN_API UOceanStateSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	static UOceanStateSubsystem* Get(const UObject* WorldContext);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	
	const IOceanWaveQuery& GetWaveQuery() const { return *ActiveQuery; }
	void SetWaveQuery(TSharedPtr<IOceanWaveQuery> NewQuery);
	void RefreshFromSettings();
	
	UFUNCTION(BlueprintCallable, Category = Ocean)
	void SetOceanTime(double NewTime) { OceanTime = NewTime; }
	UFUNCTION(BlueprintPure, Category = Ocean)
	double GetOceanTime() const { return OceanTime; }
	UFUNCTION(BlueprintPure, Category = Ocean)
	int32 GetSeed() const { return Seed; }
	UFUNCTION(BlueprintPure, Category = Ocean)
	double GetSeaLevel() const { return SeaLevel; }
	UFUNCTION(BlueprintPure, Category = Ocean)
	const FOceanSeaState& GetSeaState() const { return SeaState; }
	UFUNCTION(BlueprintPure, Category = Ocean)
	double GetWaterHeightAt(const FVector& WorldLocation) const;
private:
	void DrawDebugStats() const;
	void DrawDebugSurface() const;
#if WITH_EDITOR
	void HandleSettingsChanged(UObject* Settings, struct FPropertyChangedEvent& Event);
	FDelegateHandle SettingsChangedHandle;
#endif
	
	TSharedPtr<FGerstnerWaveSet> GerstnerWaves;
	double OceanTime = 0.0;
	int32 Seed = 0;
	double SeaLevel = 0.0;
	UPROPERTY()
	FOceanSeaState SeaState;
	TSharedPtr<IOceanWaveQuery> FlatQuery;
	TSharedPtr<IOceanWaveQuery> ActiveQuery;
};
