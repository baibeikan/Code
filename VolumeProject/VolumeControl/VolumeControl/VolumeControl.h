
// VolumeControl.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CVolumeControlApp:
// �йش����ʵ�֣������ VolumeControl.cpp
//

class CVolumeControlApp : public CWinApp
{
public:
	CVolumeControlApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CVolumeControlApp theApp;