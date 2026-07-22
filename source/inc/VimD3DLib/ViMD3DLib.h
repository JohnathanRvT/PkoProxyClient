//ViMD3DLib.h
//-------------------------------------------------------------------
/*///////////////////////////////////////////////////////////////////

DESCRIPTION:	VisuMotion D3D Library

CREATED BY:		Jens Meichsner

HISTORY:		Created 02/01/2006

*/
///////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------

#ifndef _VIM_D3DLIB_
#define _VIM_D3DLIB_

//-------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////

#if (defined LW_USE_DX9)
#include <d3d9.h>
using IDirect3DDeviceX = IDirect3DDevice9;
using IDirect3DTextureX = IDirect3DTexture9;
#else
#include <d3d8.h>
using IDirect3DDeviceX = IDirect3DDevice8;
using IDirect3DTextureX = IDirect3DTexture8;
#endif

//-------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////

//VisuMotion D3D object
typedef void* VIMD3D;

//Version of library
#define VIMD3DLIB_VERSION 1000

//-------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////
// import / export

// Dynamic linking against ViMD3DLib.dll
#ifndef VIM_D3D_API
#define VIM_D3D_API extern

//-------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////

// First call to the library
BOOL ViMD3D_InitLib(char* szError, UINT version = VIMD3DLIB_VERSION);

// Retrieve information about the current display. Returns FALSE if 3d display or VisuMotion Configurator is not present.
typedef BOOL (*FN_ViMD3D_GetInfo)(DWORD&, DWORD&, DWORD&, DWORD&, DWORD&, DWORD&, DWORD&);
VIM_D3D_API FN_ViMD3D_GetInfo ViMD3D_GetInfo;

// Initialize VIMD3D
typedef BOOL (*FN_ViMD3D_Create)(IDirect3DDeviceX*, VIMD3D*);
VIM_D3D_API FN_ViMD3D_Create ViMD3D_Create;

// Release VIMD3D
typedef BOOL (*FN_ViMD3D_Release)(VIMD3D);
VIM_D3D_API FN_ViMD3D_Release ViMD3D_Release;

// Raster all rendered views:
/*
This function uses texture stage 0 & 1 and changes the blending mode and the depth test.
If the device was created with D3DCREATE_PUREDEVICE, the fuction won't be able to restore
the original state. Also make sure, that all vertex/pixel shaders and effects are deactivated
before calling it.
*/

typedef BOOL (*FN_ViMD3D_RasterTextures)(VIMD3D, DWORD, DWORD, DWORD, DWORD, DWORD, IDirect3DTextureX**);
VIM_D3D_API FN_ViMD3D_RasterTextures ViMD3D_RasterTextures;
typedef BOOL (*FN_ViMD3D_RasterTexturesUV)(VIMD3D, DWORD, DWORD, DWORD, DWORD, DWORD, IDirect3DTextureX**, float*);
VIM_D3D_API FN_ViMD3D_RasterTexturesUV ViMD3D_RasterTexturesUV;

////////////
// Utilities

// Calculate 3d focalpoint and 3d translation from 3d nearplane and 3d farplane
typedef BOOL (*FN_ViMD3Dutil_GetCamFocalTranslation)(float&, float&, float, float, float, float, float, DWORD, DWORD);
VIM_D3D_API FN_ViMD3Dutil_GetCamFocalTranslation ViMD3Dutil_GetCamFocalTranslation;

// Create a left hand projection matrix
// fovy = Field of view in the y direction, in radians.
typedef BOOL (*FN_ViMD3Dutil_MatrixPerspectiveFovLH)(D3DMATRIX*, float, float, float, float, float, float, DWORD, DWORD);
VIM_D3D_API FN_ViMD3Dutil_MatrixPerspectiveFovLH ViMD3Dutil_MatrixPerspectiveFovLH;

// fovy = Field of view in the y direction, in radians.
typedef BOOL (*FN_ViMD3Dutil_MatrixPerspectiveFovRH)(D3DMATRIX*, float, float, float, float, float, float, DWORD, DWORD);
VIM_D3D_API FN_ViMD3Dutil_MatrixPerspectiveFovRH ViMD3Dutil_MatrixPerspectiveFovRH;

// Applies a stereo modification to a given matrix. This function is less accurate then MatrixPerspectiveFovLH/MatrixPerspectiveFovRH.
typedef BOOL (*FN_ViMD3Dutil_MatrixModify)(D3DMATRIX*, float, float, float, float, DWORD, DWORD);
VIM_D3D_API FN_ViMD3Dutil_MatrixModify ViMD3Dutil_MatrixModify;

#endif

//-------------------------------------------------------------------

#endif _VIM_D3DLIB_

//-------------------------------------------------------------------
//EOF