#pragma once
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mfmediaengine.h>
#include <mfcaptureengine.h>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfreadwrite.lib")

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <wincodec.h>
#include <vector>
using namespace std;

class MRVideo : public IMFMediaEngineNotify
{
	long m_cRef = 1;

public:
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		if (__uuidof(IMFMediaEngineNotify) == riid)
		{
			*ppv = static_cast<IMFMediaEngineNotify*>(this);
		}
		else
		{
			*ppv = nullptr;
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	STDMETHODIMP_(ULONG) Release()
	{
		LONG cRef = InterlockedDecrement(&m_cRef);
		if (cRef == 0)
			delete this;
		return cRef;
	}

	STDMETHODIMP EventNotify(DWORD meEvent, DWORD_PTR param1, DWORD param2)
	{
		if (meEvent == MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE)
		{
			SetEvent(reinterpret_cast<HANDLE>(param1));
		}
		else
		{
			switch (meEvent)
			{
			case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
				m_fEOS = FALSE;
				break;
			case MF_MEDIA_ENGINE_EVENT_CANPLAY:
				Play();
				break;
			case MF_MEDIA_ENGINE_EVENT_PLAY:
				m_fPlaying = TRUE;
				break;
			case MF_MEDIA_ENGINE_EVENT_PAUSE:
				m_fPlaying = FALSE;
				break;
			case MF_MEDIA_ENGINE_EVENT_ENDED:
				if (m_pMediaEngine->HasVideo())
				{
					m_fStopTimer = TRUE;
					m_fPlaying = FALSE;
				}
				m_fEOS = TRUE;
				break;
			case MF_MEDIA_ENGINE_EVENT_TIMEUPDATE:
				break;
			case MF_MEDIA_ENGINE_EVENT_ERROR:
				break;
			}
		}

		return S_OK;
	}

public:
	MRVideo(ID3D11Device* pD3D11Device, IMFDXGIDeviceManager* pDXGIManager);
	~MRVideo();

	BOOL isPlaying() const { return m_fPlaying; }
	BOOL isEos() const { return m_fEOS; }

public:
	ID3D11ShaderResourceView* loadVideo(const wchar_t* wcFile, BOOL loop = TRUE);
	void updateVideo();

	// state
	void Play();
	void Pause();
	void FrameStep();

	// Audio
	void SetVolume(double fVol);
	void SetBalance(double fBal);
	void SetMute(BOOL mute);

	// time
	SIZE GetSize();
	double GetDuration(BOOL* pbCanSeek);
	double GetPosition();
	void SetPosition(double pos);
	double GetRate();
	void SetRate(double rate);

protected:
	IMFMediaEngine*					m_pMediaEngine = NULL;
	IMFMediaEngineEx*				m_pMediaEngineEx = NULL;

	ID3D11Texture2D*				m_pTexture2D = NULL;
	ID3D11ShaderResourceView*		m_pSRV2D = NULL;

private:
	ID3D11Device*					m_pD3D11Device = NULL;
	CRITICAL_SECTION				m_critSec;

private:
	BOOL							m_fPlaying = FALSE;
	BOOL							m_fStopTimer = TRUE;
	BOOL							m_fEOS = FALSE;
};
