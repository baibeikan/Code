#ifndef __VOLUME_H__
#define __VOLUME_H__

#define VOLUME_API 

// 获得声音 0-100
VOLUME_API int GetVolume();

// 设置声音 0-100
VOLUME_API void SetVolume(int nVolume);

// 设置静音
VOLUME_API void SetMute(bool bMute);

VOLUME_API bool GetMute();

#endif

