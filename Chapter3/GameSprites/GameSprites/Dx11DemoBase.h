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

		// 更新
        virtual void Update( float dt ) = 0;
		// 渲染
        virtual void Render( ) = 0;

    protected:
		// 实例句柄
        HINSTANCE hInstance_;
		// 窗口句柄
        HWND hwnd_;

		// 驱动类型
        D3D_DRIVER_TYPE driverType_;
        D3D_FEATURE_LEVEL featureLevel_;

		// 3d设备
        ID3D11Device* d3dDevice_;
		// 3d设备内容
        ID3D11DeviceContext* d3dContext_;
		// 交换链
        IDXGISwapChain* swapChain_;
		// 缓冲背景渲染backbuffer
        ID3D11RenderTargetView* backBufferTarget_;
};

#endif