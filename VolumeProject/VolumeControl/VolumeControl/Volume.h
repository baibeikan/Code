#ifndef __VOLUME_H__
#define __VOLUME_H__

#define VOLUME_API 

// ������� 0-100
VOLUME_API int GetVolume();

// �������� 0-100
VOLUME_API void SetVolume(int nVolume);

// ���þ���
VOLUME_API void SetMute(bool bMute);

VOLUME_API bool GetMute();

#endif

