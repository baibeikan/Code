#include "StdAfx.h"
#include "Volume0.h"

bool GetVolumeControl(HMIXER hmixer ,long componentType,long ctrlType,MIXERCONTROL* mxc)
{
	MIXERLINECONTROLS mxlc;
	MIXERLINE mxl;
	mxl.cbStruct = sizeof(mxl);
	mxl.dwComponentType = componentType;
	if(!mixerGetLineInfo((HMIXEROBJ)hmixer, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE))
	{
		mxlc.cbStruct = sizeof(mxlc);
		mxlc.dwLineID = mxl.dwLineID;
		mxlc.dwControlType = ctrlType;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(MIXERCONTROL);
		mxlc.pamxctrl = mxc;
		if(mixerGetLineControls((HMIXEROBJ)hmixer,&mxlc,MIXER_GETLINECONTROLSF_ONEBYTYPE))
			return 0;
		else
			return 1;
	}
	return 0;
}

unsigned GetVolumeValue(HMIXER hmixer ,MIXERCONTROL *mxc)
{
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED vol; vol.dwValue=0;
	mxcd.hwndOwner = 0;
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = mxc->dwControlID;
	mxcd.cbDetails = sizeof(vol);
	mxcd.paDetails = &vol;
	mxcd.cChannels = 1;
	if(mixerGetControlDetails((HMIXEROBJ)hmixer, &mxcd, MIXER_OBJECTF_HMIXER|MIXER_GETCONTROLDETAILSF_VALUE))
		return -1;
	return vol.dwValue;
}

bool SetVolumeValue(HMIXER hmixer ,MIXERCONTROL *mxc, long volume)
{
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED vol;vol.dwValue = volume;
	mxcd.hwndOwner = 0;
	mxcd.dwControlID = mxc->dwControlID;
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.cbDetails = sizeof(vol);
	mxcd.paDetails = &vol;
	mxcd.cChannels = 1;
	if(mixerSetControlDetails((HMIXEROBJ)hmixer, &mxcd, MIXER_OBJECTF_HMIXER|MIXER_SETCONTROLDETAILSF_VALUE))
		return 0;
	return 1;
}

BOOL Volume::SetMute(BOOL bMute)
{
	HMIXER hMixer;
	MIXERCONTROL mxc;
	MIXERLINE mxl;
	MIXERLINECONTROLS mxlc;
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_SIGNED volStruct;
	MMRESULT mmr;

	//Sound Setting
	mmr = mixerOpen(&hMixer, 0, 0, 0, 0);
	if (mmr != MMSYSERR_NOERROR) return FALSE;

	// 初始化MIXERLINE结构体.
	ZeroMemory(&mxl, sizeof(MIXERLINE));
	mxl.cbStruct = sizeof(MIXERLINE);

	// 指出需要获取的通道，扬声器用MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
	mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

	mmr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);
	if (mmr != MMSYSERR_NOERROR) return FALSE;

	// 取得控制器.
	ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(mxc);
	mxlc.pamxctrl = &mxc;
	//mxc.cMultipleItems=0;

	ZeroMemory(&mxc, sizeof(MIXERCONTROL));
	mxc.cbStruct = sizeof(MIXERCONTROL);
	mmr = mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);
	if (mmr != MMSYSERR_NOERROR) return FALSE;

	// 初始化MIXERCONTROLDETAILS结构体
	ZeroMemory(&mxcd, sizeof(MIXERCONTROLDETAILS));
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);
	mxcd.dwControlID = mxc.dwControlID;
	mxcd.paDetails = &volStruct;
	mxcd.cChannels = 1;

	// 获得音量值
	volStruct.lValue = bMute;
	mmr = mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

	mixerClose(hMixer);
	return TRUE;
}

//设置音量值
bool WINAPI Volume::SetVolume(LONG vol)
{
	bool Result=false;  //用于判断函数是否执行成功
	MIXERCONTROL volCtrl;
	HMIXER hmixer;
	if(mixerOpen(&hmixer, 0, 0, 0, 0)) return 0;
	if(GetVolumeControl(hmixer,MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,MIXERCONTROL_CONTROLTYPE_VOLUME,&volCtrl))
	{  
		int ChangeValue=volCtrl.Bounds.lMaximum/INTERVAL;
		int TotalValue=ChangeValue*vol;
		if(SetVolumeValue(hmixer,&volCtrl,TotalValue))
			Result=true;
	}   
	mixerClose(hmixer);
	return Result;
}

//返回音量值
unsigned WINAPI Volume::GetVolume()
{
	unsigned rt=0; 
	MIXERCONTROL volCtrl;
	HMIXER hmixer; 
	if(mixerOpen(&hmixer, 0, 0, 0, 0)) return 0;
	if(!GetVolumeControl(hmixer,MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,MIXERCONTROL_CONTROLTYPE_VOLUME,&volCtrl))
		return 0;
	int ChangeValue=volCtrl.Bounds.lMaximum/INTERVAL; //每一次调用函数改变的音量值
	rt=GetVolumeValue(hmixer,&volCtrl)/ChangeValue;  //GetVolumeValue得到的是具体值
	mixerClose(hmixer);
	return rt;
}
