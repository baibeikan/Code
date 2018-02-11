#include "MRVideo.h"


HRESULT WINAPI DXCreateRenderTexture(ID3D11Device* pd3dDevice,
	ID3D11Texture2D** ppTexture, ID3D11ShaderResourceView** ppSRV, ID3D11RenderTargetView** ppRTV,
	UINT width, UINT height, DXGI_FORMAT format)
{
	HRESULT hr = S_OK;
	// Create SB
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
	td.Width = width;
	td.Height = height;
	td.Format = format;
	td.ArraySize = 1;
	td.CPUAccessFlags = 0;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	td.MiscFlags = 0;
	td.MipLevels = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	V_RETURN(pd3dDevice->CreateTexture2D(&td, NULL, ppTexture));
	// Create RTV
	if (ppRTV)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = td.Format;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		V_RETURN(pd3dDevice->CreateRenderTargetView((ID3D11Resource*)*ppTexture, &rtvDesc, ppRTV));
	}
	// Create SRV
	if (ppSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		hr = pd3dDevice->CreateShaderResourceView((ID3D11Resource*)*ppTexture, &srvDesc, ppSRV);
	}
	return hr;
}


MRVideo::MRVideo(ID3D11Device* pD3D11Device, IMFDXGIDeviceManager* pDXGIManager) : m_pD3D11Device(pD3D11Device)
{
	HRESULT hr = S_OK;
	InitializeCriticalSectionEx(&m_critSec, 0, 0);

	IMFMediaEngineClassFactory* pFactory = NULL;
	CoCreateInstance(CLSID_MFMediaEngineClassFactory, NULL, 
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));

	IMFAttributes* pAttributes = NULL;
	hr = MFCreateAttributes(&pAttributes, 1);
	hr = pAttributes->SetUnknown(MF_MEDIA_ENGINE_DXGI_MANAGER, (IUnknown*)pDXGIManager);
	hr = pAttributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, (IUnknown*)this);
	hr = pAttributes->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, DXGI_FORMAT_R8G8B8A8_UNORM);

	const DWORD flags = MF_MEDIA_ENGINE_WAITFORSTABLE_STATE;
	hr = pFactory->CreateInstance(flags, pAttributes, &m_pMediaEngine);
	if (hr == S_OK)
	{
		hr = m_pMediaEngine->QueryInterface(__uuidof(IMFMediaEngine), (void**)&m_pMediaEngineEx);
	}

	SAFE_RELEASE(pFactory);
	SAFE_RELEASE(pAttributes);
}

MRVideo::~MRVideo()
{
	EnterCriticalSection(&m_critSec);
	m_fStopTimer = TRUE;
	m_fPlaying = FALSE;
	if (m_pMediaEngine)
		m_pMediaEngine->Shutdown();
	LeaveCriticalSection(&m_critSec);

	SAFE_RELEASE(m_pTexture2D);
	SAFE_RELEASE(m_pSRV2D);
	SAFE_RELEASE(m_pMediaEngineEx);
	SAFE_RELEASE(m_pMediaEngine);

	DeleteCriticalSection(&m_critSec);
}

ID3D11ShaderResourceView* MRVideo::loadVideo(const wchar_t* wcFile, BOOL loop)
{
	if (!m_pMediaEngine)
		return NULL;

	HRESULT hr = S_OK;
	BSTR strpath = SysAllocString(wcFile);
	hr = m_pMediaEngine->SetSource(strpath);
	if (FAILED(hr))
		return NULL;
	hr = m_pMediaEngine->SetLoop(loop);
	if (FAILED(hr))
		return NULL;
	
	LONGLONG pts = 0;
	while (m_pMediaEngine->OnVideoStreamTick(&pts) != S_OK);

	EnterCriticalSection(&m_critSec);
	SAFE_RELEASE(m_pTexture2D);
	SAFE_RELEASE(m_pSRV2D);
	DWORD cx = 0;
	DWORD cy = 0;
	m_pMediaEngine->GetNativeVideoSize(&cx, &cy);
	DXCreateRenderTexture(m_pD3D11Device, &m_pTexture2D, &m_pSRV2D, NULL, cx, cy, DXGI_FORMAT_R8G8B8A8_UNORM);
	LeaveCriticalSection(&m_critSec);

	return m_pSRV2D;
}

void MRVideo::updateVideo()
{
	if (!m_pMediaEngine)
		return;

	EnterCriticalSection(&m_critSec);
	LONGLONG pts = 0;
	if (m_pMediaEngine->OnVideoStreamTick(&pts) == S_OK)
	{
		D3D11_TEXTURE2D_DESC desc = { 0 };
		m_pTexture2D->GetDesc(&desc);
		MFARGB bkColor = { 0 };
		RECT rcDst = { 0, 0, desc.Width, desc.Height };
		m_pMediaEngineEx->TransferVideoFrame(m_pTexture2D, NULL, &rcDst, &bkColor);
	}
	LeaveCriticalSection(&m_critSec);
}

void MRVideo::Play()
{
	if (!m_pMediaEngine)
		return;

	if (m_pMediaEngine->HasVideo() && m_fStopTimer)
		m_fStopTimer = FALSE;

	if (m_fEOS)
	{
		m_pMediaEngine->SetCurrentTime(0.);
		m_fPlaying = TRUE;
	}
	else
	{
		m_pMediaEngine->Play();
	}
	m_fEOS = FALSE;
}

void MRVideo::Pause()
{
	if (!m_pMediaEngine)
		return;

	if (!m_fEOS)
		m_pMediaEngine->Pause();
}

void MRVideo::FrameStep()
{
	if (!m_pMediaEngineEx)
		return;

	m_pMediaEngineEx->FrameStep(TRUE);
}

void MRVideo::SetVolume(double fVol)
{
	if (!m_pMediaEngineEx)
		return;

	m_pMediaEngineEx->SetVolume(fVol);
}

void MRVideo::SetBalance(double fBal)
{
	if (!m_pMediaEngineEx)
		return;

	m_pMediaEngineEx->SetBalance(fBal);
}

void MRVideo::SetMute(BOOL mute)
{
	if (!m_pMediaEngineEx)
		return;

	m_pMediaEngineEx->SetMuted(mute);
}

SIZE MRVideo::GetSize()
{
	if (!m_pMediaEngine)
		return { 0, 0 };
	if (!m_pMediaEngine->HasVideo())
		return { 0, 0 };
	
	DWORD cx = 0;
	DWORD cy = 0;
	m_pMediaEngine->GetNativeVideoSize(&cx, &cy);
	return { cx, cy };
}

double MRVideo::GetDuration(BOOL* pbCanSeek)
{
	if (!m_pMediaEngine)
	{
		if (pbCanSeek)
			*pbCanSeek = FALSE;
		return 0.0;
	}

	double duration = m_pMediaEngine->GetDuration();
	// NOTE:
	// "duration != duration"
	// This tests if duration is NaN, because NaN != NaN
	if (duration != duration || duration == numeric_limits<float>::infinity())
	{
		duration = 0.0f;
		if (pbCanSeek)
			*pbCanSeek = FALSE;
	}
	else
	{
		DWORD caps = 0;
		m_pMediaEngineEx->GetResourceCharacteristics(&caps);
		if (pbCanSeek)
			*pbCanSeek = (caps & 0x00000002) == 0x00000002;
	}

	return duration;
}

double MRVideo::GetPosition()
{
	if (!m_pMediaEngine)
		return 0.0;
	if (!m_pMediaEngine->HasVideo())
		return 0.0;

	return m_pMediaEngine->GetCurrentTime();
}

void MRVideo::SetPosition(double pos)
{
	if (!m_pMediaEngine)
		return;
	if (!m_pMediaEngine->HasVideo())
		return;

	m_pMediaEngine->SetCurrentTime(pos);
}

double MRVideo::GetRate()
{
	if (!m_pMediaEngine)
		return 0.0;
	if (!m_pMediaEngine->HasVideo())
		return 0.0;

	return m_pMediaEngine->GetPlaybackRate();
}

void MRVideo::SetRate(double rate)
{
	if (!m_pMediaEngine)
		return;
	if (!m_pMediaEngine->HasVideo())
		return;

	m_pMediaEngine->SetPlaybackRate(rate);
}
