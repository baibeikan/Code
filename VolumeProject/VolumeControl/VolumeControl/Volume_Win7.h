#pragma once
#include "mmdeviceapi.h"
#include "Endpointvolume.h"

#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)  \
{ (punk)->Release(); (punk) = NULL; }

class Volume_Win7
{
public:
	/* nVolume:0.0 - 1.0, ∂‘”¶ system volume:0 - 100*/
	static bool    ChangeVolume(double nVolume, bool bScalar = true);
	static HRESULT MuteDevice(bool bMute);
	static float   GetVolume();
};


