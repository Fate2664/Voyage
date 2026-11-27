#pragma once
#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
namespace OceanDebug
{
	extern VOYAGEOCEAN_API TAutoConsoleVariable<int32> CVarShowStats;
	extern VOYAGEOCEAN_API TAutoConsoleVariable<int32> CVarDrawSurface;
	extern VOYAGEOCEAN_API TAutoConsoleVariable<float> CVarDrawExtent;
	extern VOYAGEOCEAN_API TAutoConsoleVariable<float> CVarDrawSpacing;
	extern VOYAGEOCEAN_API TAutoConsoleVariable<float> CVarTimeScale;
	extern VOYAGEOCEAN_API TAutoConsoleVariable<int32> CVarPaused;
	
}