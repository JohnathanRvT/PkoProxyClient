//
#pragma once

#define USE_DX_VERSION 9
#define LW_USE_DX9
#if (!defined USE_DX_VERSION)
#define LW_USE_DX8
#endif

//#define DELAY_LOAD_DXDLL

//#ifdef LW_PRAGMA_LIB
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "amstrmid.lib")
#pragma comment(lib, "winmm.lib")
//#endif

#if (defined LW_USE_DX9)

//#define LW_SHADER_DEBUG_VS

#include <d3d9.h>
#include <d3dx9.h>
#include <d3d9types.h>
#include <d3dx9math.h>
#include <d3dx9mesh.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dinput8.lib")

using IDirect3DX = IDirect3D9;
using IDirect3DDeviceX = IDirect3DDevice9;
using IDirect3DTextureX = IDirect3DTexture9;
using IDirect3DVertexBufferX = IDirect3DVertexBuffer9;
using IDirect3DIndexBufferX = IDirect3DIndexBuffer9;
using IDirect3DSurfaceX = IDirect3DSurface9;
using IDirect3DVolumeX = IDirect3DVolume9;
using IDirect3DBaseTextureX = IDirect3DBaseTexture9;
using IDirect3DVolumeTextureX = IDirect3DVolumeTexture9;
using IDirect3DCubeTextureX = IDirect3DCubeTexture9;

using IDirect3DVertexShaderX = IDirect3DVertexShader9;
using IDirect3DVertexDeclarationX = IDirect3DVertexDeclaration9;
using SHADER_TYPE = IDirect3DVertexShaderX*;

using IDirect3DPixelShaderX = IDirect3DPixelShader9;
using D3DPOOLX = D3DPOOL;
using D3DXTECHNIQUE_DESCX = D3DXHANDLE;

using D3DLIGHTX = D3DLIGHT9;
using D3DMATERIALX = D3DMATERIAL9;
using D3DVERTEXELEMENTX = D3DVERTEXELEMENT9;
using D3DVIEWPORTX = D3DVIEWPORT9;
using D3DLOCK_TYPE = void;

using D3DCAPSX = D3DCAPS9;

using LPDIRECT3DDEVICEX = LPDIRECT3DDEVICE9;
using LPDIRECT3DTEXTUREX = LPDIRECT3DTEXTURE9;
using LPDIRECT3DSURFACEX = LPDIRECT3DSURFACE9;
using LPDIRECT3DVERTEXBUFFERX = LPDIRECT3DVERTEXBUFFER9;
using LPDIRECT3DINDEXBUFFERX = LPDIRECT3DINDEXBUFFER9;
using D3DTEXTURESTAGESTATETYPEX = D3DSAMPLERSTATETYPE;

//typedef IDirect3D

#define Direct3DCreateX Direct3DCreate9

//Used by "Draw" only
using VECTOR3X = D3DXVECTOR3;

#define CreateVertexBufferX(Length, Usage, FVF, Pool, ppVertexBuffer, pHandle) \
	CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pHandle)

#define CreateIndexBufferX(Length, Usage, Format, Pool, ppIndexBuffer, pHandle) \
	CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pHandle)

#define CreateTextureX(Width, Height, Level, Usage, Format, Pool, ppTexture, Handle) \
	CreateTexture(Width, Height, Level, Usage, Format, Pool, ppTexture, Handle)

#define CreateVolumeTextureX(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, Handle) \
	CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, Handle)

#define CreateCubeTextureX(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pHandle) \
	CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pHandle)

#define CreateRenderTargetX(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pHandle) \
	CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pHandle)

#define CreateDepthStencilSurfaceX(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pHandle) \
	CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pHandle)

#define SetStreamSourceX(StreamNum, StreamData, Offset, Stride) \
	SetStreamSource(StreamNum, StreamData, Offset, Stride)

#define SetIndicesX(pIndexData, BaseVertexIndex) \
	SetIndices(pIndexData)

#define DrawPrimitiveX(Type, StartVertex, PrimitiveCount) \
	DrawPrimitive(Type, StartVertex, PrimitiveCount)

#define DrawIndexedPrimitiveX(Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount) \
	DrawIndexedPrimitive(Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount)

//TODO: Add SetVertexShader(nullptr); before this on DirectX9. It's required!
#define SetFVFX(fvf) \
	SetFVF(fvf)

#define GetDisplayModeX(SwapChain, pMode) \
	GetDisplayMode(SwapChain, pMode)

#define CheckDeviceMultiSampleTypeX(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels) \
	CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels)

#define D3DXAssembleShaderFromFileX(pSrcFile, pDefines, pInclude, Flags, ppShader, ppErrorMsgs) \
    D3DXAssembleShaderFromFile(pSrcFile, pDefines, pInclude, Flags, ppShader, ppErrorMsgs)

#define CreateOffscreenPlainSurfaceX(Width, Height, Format, Pool, ppSurface, pSharedHandle) \
	CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle)

#define CreateVertexShaderX(pDeclaration, pFunction, ppShader, Usage) \
	CreateVertexShader(pFunction, ppShader)

#define CreatePixelShaderX(pFunction, ppShader) \
	CreatePixelShader(pFunction, ppShader)

#define D3DXCreateEffectFromFileX(pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors) \
	D3DXCreateEffectFromFile(pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors)

#define GetRenderTargetX(RenderTargetIndex, ppRenderTarget) \
	GetRenderTarget(RenderTargetIndex, ppRenderTarget)

#define SetRenderTargetX(RenderTargetIndex, ppRenderTarget, pNewZStencil) \
	SetRenderTarget(RenderTargetIndex, ppRenderTarget);

#define GetBackBufferX(iSwapChain, iBackBuffer, Type, ppBackBuffer) \
    GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer)

//3rd parameter is auto-type casted
#define LockX(OffsetToLock, SizeToLock, ppbData, Flags) \
    Lock(OffsetToLock, SizeToLock, (VOID**)ppbData, Flags)

#define UpdateSurfaceX(pSourceSurface, pSourceRect, cRects, pDestinationSurface, pDestPoint) \
	UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint)

#define CreateOffscreenPlainSurfaceX(Width, Height, Format, Pool, ppSurface, pSharedHandle) \
CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle)

//2nd parameter is auto-type casted
#define SetVertexShaderConstantX(StartRegister, pConstantData, Count) \
	SetVertexShaderConstantF(StartRegister, (float*)pConstantData, Count)

#define GetRenderTargetX(RenderTargetIndex, ppRenderTarget) \
	GetRenderTarget(RenderTargetIndex, ppRenderTarget)

#define D3DRS_ANTIALIASEDLINEENABLEX D3DRS_ANTIALIASEDLINEENABLE

#elif (defined LW_USE_DX8)

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#include <d3d8.h>
#include <d3dx8.h>
#include <d3d8types.h>
#include <d3dx8math.h>
#include <d3dx8mesh.h>
#include <dinput.h>

#pragma comment(lib, "d3d8.lib")
#pragma comment(lib, "d3dx8.lib")
#pragma comment(lib, "dinput8.lib")

using IDirect3DX = IDirect3D8;
using IDirect3DDeviceX = IDirect3DDevice8;
using IDirect3DTextureX = IDirect3DTexture8;
using IDirect3DVertexBufferX = IDirect3DVertexBuffer8;
using IDirect3DIndexBufferX = IDirect3DIndexBuffer8;
using IDirect3DSurfaceX = IDirect3DSurface8;
using IDirect3DVolumeX = IDirect3DVolume8;
using IDirect3DBaseTextureX = IDirect3DBaseTexture8;
using IDirect3DVolumeTextureX = IDirect3DVolumeTexture8;
using IDirect3DCubeTextureX = IDirect3DCubeTexture8;

using IDirect3DVertexShaderX = DWORD;
using IDirect3DVertexDeclarationX = DWORD;
using SHADER_TYPE = IDirect3DVertexShaderX;

using IDirect3DPixelShaderX = DWORD;
using D3DPOOLX = D3DPOOL;
using D3DXTECHNIQUE_DESCX = D3DXTECHNIQUE_DESC;

using D3DLIGHTX = D3DLIGHT8;
using D3DMATERIALX = D3DMATERIAL8;
using D3DVIEWPORTX = D3DVIEWPORT8;
using D3DLOCK_TYPE = BYTE;

using D3DCAPSX = D3DCAPS8;

using LPDIRECT3DDEVICEX = LPDIRECT3DDEVICE8;
using LPDIRECT3DTEXTUREX = LPDIRECT3DTEXTURE8;
using LPDIRECT3DSURFACEX = LPDIRECT3DSURFACE8;
using LPDIRECT3DVERTEXBUFFERX = LPDIRECT3DVERTEXBUFFER8;
using LPDIRECT3DINDEXBUFFERX = LPDIRECT3DINDEXBUFFER8;
using D3DTEXTURESTAGESTATETYPEX = D3DTEXTURESTAGESTATETYPE;

//Used by "Draw" only
using VECTOR3X = D3DXVECTOR2;

// begin dx8 not use this type
//typedef DWORD D3DSAMPLERSTATETYPE;
//
//#define SetSamplerState(sampler, type, value) \
//    SetTextureStageState(sampler, (D3DTEXTURESTAGESTATETYPE)type, value)
// end
//typedef IDirect3D

#define Direct3DCreateX Direct3DCreate8

#define CreateVertexBufferX(Length, Usage, FVF, Pool, ppVertexBuffer, pHandle) \
	CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer)

#define CreateIndexBufferX(Length, Usage, Format, Pool, ppIndexBuffer, pHandle) \
	CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer)

#define CreateTextureX(Width, Height, Level, Usage, Format, Pool, ppTexture, Handle) \
	CreateTexture(Width, Height, Level, Usage, Format, Pool, ppTexture)

#define CreateVolumeTextureX(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, Handle) \
	CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture)

#define CreateCubeTextureX(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pHandle) \
	CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture)

#define CreateRenderTargetX(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pHandle) \
	CreateRenderTarget(Width, Height, Format, MultiSample, Lockable, ppSurface)

#define CreateDepthStencilSurfaceX(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pHandle) \
	CreateDepthStencilSurface(Width, Height, Format, MultiSample, ppSurface)

#define SetVertexShaderConstantF(Register, Data, Count) \
	SetVertexShaderConstant(Register, (void*)Data, Count)

#define SetStreamSourceX(StreamNum, StreamData, Offset, Stride) \
	SetStreamSource(StreamNum, StreamData, Stride)

#define SetIndicesX(pIndexData, BaseVertexIndex) \
	SetIndices(pIndexData, BaseVertexIndex)

#define DrawPrimitiveX(Type, StartVertex, PrimitiveCount) \
	DrawPrimitive(Type, StartVertex, PrimitiveCount)

#define DrawIndexedPrimitiveX(Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount) \
	DrawIndexedPrimitive(Type, MinIndex, NumVertices, StartIndex, PrimitiveCount)

#define SetFVFX(fvf) \
	SetVertexShader(fvf)

#define GetDisplayModeX(SwapChain, pMode) \
	GetDisplayMode(pMode)

#define CheckDeviceMultiSampleTypeX(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels) \
	CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType)

#define D3DXAssembleShaderFromFileX(pSrcFile, pDefines, pInclude, Flags, ppShader, ppErrorMsgs) \
	D3DXAssembleShaderFromFile(pSrcFile, Flags, pDefines, ppShader, ppErrorMsgs)

#define CreateOffscreenPlainSurfaceX(Width, Height, Format, Pool, ppSurface, pSharedHandle) \
	CreateImageSurface(Width, Height, Format, ppSurface)

#define CreateVertexShaderX(pDeclaration, pFunction, ppShader, Usage) \
	CreateVertexShader(pDeclaration, pFunction, ppShader, Usage)

#define CreatePixelShaderX(pFunction, ppShader) \
	CreatePixelShader(pFunction, ppShader)

#define D3DXCreateEffectFromFileX(pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors) \
	D3DXCreateEffectFromFile(pDevice, pSrcFile, ppEffect, ppCompilationErrors)

#define GetRenderTargetX(RenderTargetIndex, ppRenderTarget) \
	GetRenderTarget(ppRenderTarget)

#define SetRenderTargetX(RenderTargetIndex, ppRenderTarget, pNewZStencil) \
	SetRenderTarget(ppRenderTarget, pNewZStencil)

#define GetBackBufferX(iSwapChain, iBackBuffer, Type, ppBackBuffer) \
	GetBackBuffer(iBackBuffer, Type, ppBackBuffer)

//3rd parameter is auto-type casted
#define LockX(OffsetToLock, SizeToLock, ppbData, Flags) \
	Lock(OffsetToLock, SizeToLock, (BYTE**)ppbData, Flags)

#define UpdateSurfaceX(pSourceSurface, pSourceRect, cRects, pDestinationSurface, pDestPoint) \
	CopyRects(pSourceSurface, pSourceRect, cRects, pDestinationSurface, pDestPoint)

#define CreateOffscreenPlainSurfaceX(Width, Height, Format, Pool, ppSurface, pSharedHandle) \
	CreateImageSurface(Width, Height, Format, ppSurface)

#define SetVertexShaderConstantX(StartRegister, pConstantData, Count) \
SetVertexShaderConstant(StartRegister, pConstantData, Count)

#define GetRenderTargetX(RenderTargetIndex, ppRenderTarget) \
GetRenderTarget(ppRenderTarget)

typedef struct _D3DVERTEXELEMENT9 {
	WORD Stream;
	WORD Offset;
	BYTE Type;
	BYTE Method;
	BYTE Usage;
	BYTE UsageIndex;
} D3DVERTEXELEMENT9;

using D3DVERTEXELEMENTX = D3DVERTEXELEMENT9;
#define D3DRS_ANTIALIASEDLINEENABLEX D3DRS_EDGEANTIALIAS

#endif
