#include "Volume.h"
#include <mmdeviceapi.h>
#include <Endpointvolume.h>


#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)  \
{ (punk)->Release(); (punk) = NULL; }


IAudioEndpointVolume* GetAudioEndpointVolume()
{
	HRESULT hr=NULL;

	IMMDeviceEnumerator *deviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, 
		__uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
	IMMDevice *defaultDevice = NULL;

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
	deviceEnumerator->Release();
	deviceEnumerator = NULL;

	IAudioEndpointVolume *endpointVolume = NULL;
	hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), 
		CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
	defaultDevice->Release();
	defaultDevice = NULL;
	return endpointVolume;
}

void ReleaseAudioEndpointVolume(IAudioEndpointVolume* pAudioEndpointVolume )
{
	if(pAudioEndpointVolume != nullptr)
		pAudioEndpointVolume->Release();

	pAudioEndpointVolume = nullptr;
}

VOLUME_API int GetVolume()
{
	HRESULT hr=NULL;

	CoInitialize(NULL);

	IAudioEndpointVolume *endpointVolume = GetAudioEndpointVolume();
	// -------------------------
	float currentVolume = 0;
	endpointVolume->GetMasterVolumeLevel(&currentVolume);
	//printf("Current volume in dB is: %f\n", currentVolume);
	hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
	ReleaseAudioEndpointVolume(endpointVolume);
	CoUninitialize();

	return int(currentVolume * 100);
}

VOLUME_API void SetVolume(int nVolume)
{
	HRESULT hr=NULL;
	float newVolume = nVolume / 100.0f;
	CoInitialize(NULL);
	IAudioEndpointVolume*endpointVolume = GetAudioEndpointVolume();
	hr = endpointVolume->SetMasterVolumeLevelScalar(newVolume, NULL);
	ReleaseAudioEndpointVolume(endpointVolume);
	CoUninitialize();
}

VOLUME_API void SetMute(bool bMute)
{
	CoInitialize(NULL);

	IAudioEndpointVolume* endpointVolume = GetAudioEndpointVolume();
	endpointVolume->SetMute(bMute, NULL);
	ReleaseAudioEndpointVolume(endpointVolume);

	CoUninitialize();
}

VOLUME_API bool GetMute()
{
	CoInitialize(NULL);

	BOOL bMute = FALSE;
	IAudioEndpointVolume* endpointVolume = GetAudioEndpointVolume();
	endpointVolume->GetMute(&bMute);
	ReleaseAudioEndpointVolume(endpointVolume);
	CoUninitialize();

	return bMute == TRUE;
}
