/*
    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    DX11DemoBase Class - Base class for all demos to derive from.
*/


#ifndef _DEMO_BASE_H_
#define _DEMO_BASE_H_

#include<d3d11.h>
#include<d3dx11.h>
#include<DxErr.h>


class Dx11DemoBase
{
    public:
        Dx11DemoBase( );
        virtual ~Dx11DemoBase( );

        bool Initialize( HINSTANCE hInstance, HWND hwnd );
        void Shutdown( );

        bool CompileD3DShader( char* filePath, char* entry,
                               char* shaderModel, ID3DBlob** buffer );

        virtual bool LoadContent( );
        virtual void UnloadContent( );

		// ����
        virtual void Update( float dt ) = 0;
		// ��Ⱦ
        virtual void Render( ) = 0;

    protected:
		// ʵ�����
        HINSTANCE hInstance_;
		// ���ھ��
        HWND hwnd_;

		// ��������
        D3D_DRIVER_TYPE driverType_;
        D3D_FEATURE_LEVEL featureLevel_;

		// 3d�豸
        ID3D11Device* d3dDevice_;
		// 3d�豸����
        ID3D11DeviceContext* d3dContext_;
		// ������
        IDXGISwapChain* swapChain_;
		// ���屳����Ⱦbackbuffer
        ID3D11RenderTargetView* backBufferTarget_;
};

#endif