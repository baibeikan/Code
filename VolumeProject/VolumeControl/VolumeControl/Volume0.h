#pragma once
#include <windows.h>
#include <mmsystem.h>

#pragma comment( lib, "winmm.lib" )

#define INTERVAL 100  //������������������20��ֵ

class Volume
{
public:
	static BOOL     WINAPI SetMute(BOOL bMute);
	static bool     WINAPI SetVolume(LONG vol);
	static unsigned WINAPI GetVolume();
};
