#include "OceanDebug.h"

namespace OceanDebug
{
	TAutoConsoleVariable<int32> CVarShowStats(
		TEXT("ocean.Debug.ShowStats"), 0,
		TEXT("Show ocean state on screen (time, seed, sea state, test height)."), ECVF_Cheat);
	TAutoConsoleVariable<int32> CVarDrawSurface(
		TEXT("ocean.Debug.DrawSurface"), 0,
		TEXT("Debug-draw CPU wave surface points around the camera (step 2)."), ECVF_Cheat);
	TAutoConsoleVariable<float> CVarDrawExtent(
		TEXT("ocean.Debug.DrawExtent"), 5000.f,
		TEXT("Half-size of the debug grid in cm."), ECVF_Cheat);
	TAutoConsoleVariable<float> CVarDrawSpacing(
		TEXT("ocean.Debug.DrawSpacing"), 250.f,
		TEXT("Spacing between debug grid points in cm."), ECVF_Cheat);
	TAutoConsoleVariable<float> CVarTimeScale(
		TEXT("ocean.TimeScale"), 1.f,
		TEXT("Multiplier on ocean clock speed (debug)."), ECVF_Cheat);
	TAutoConsoleVariable<int32> CVarPaused(
		TEXT("ocean.Paused"), 0,
		TEXT("Freeze the ocean clock (debug)."), ECVF_Cheat);
}
