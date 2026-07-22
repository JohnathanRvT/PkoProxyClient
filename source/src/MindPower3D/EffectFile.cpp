// DXEffectFile.cpp: implementation of the CMPEffectFile class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
//#include "../../../proj/EffectEditer.h"
//#include <mindpower.h>
#include "GlobalInc.h"

#include "EffectFile.h"
//#include "DXUtil.h"
#include "MPRender.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMPEffectFile::CMPEffectFile() {
	m_pEffect = nullptr;
	m_pDev = nullptr;
	_vecTechniques.clear();
	_iTechNum = 0;
	_dwVShader = 0;
}
#ifdef USE_RENDER
CMPEffectFile::CMPEffectFile(MPRender* pDev)
#else
CMPEffectFile::CMPEffectFile(LPDIRECT3DDEVICEX pDev)
#endif
{
	m_pDev = pDev;
	m_pEffect = nullptr;
	_vecTechniques.clear();
	_iTechNum = 0;
	_dwVShader = 0;
}
CMPEffectFile::~CMPEffectFile() {
	free();
}
#ifdef USE_RENDER
void CMPEffectFile::InitDev(MPRender* pDev)
#else
void CMPEffectFile::InitDev(LPDIRECT3DDEVICEX pDev)
#endif
{
	m_pDev = pDev;
}

BOOL CMPEffectFile::LoadEffectFromFile(LPCSTR pszfile) {
	HRESULT hr;
#if (defined LW_USE_DX9)
#ifdef USE_RENDER
	if (FAILED(hr = D3DXCreateEffectFromFile(m_pDev->GetDevice(), pszfile, nullptr, nullptr, 0, nullptr, &m_pEffect, nullptr)))
#else
	if (FAILED(hr = D3DXCreateEffectFromFile(m_pDev, pszfile, nullptr, nullptr, 0, nullptr, &m_pEffect, nullptr)))
#endif
#elif (defined LW_USE_DX8)
#ifdef USE_RENDER
	if (FAILED(hr = D3DXCreateEffectFromFile(m_pDev->GetDevice(), pszfile, &m_pEffect, nullptr)))
#else
	if (FAILED(hr = D3DXCreateEffectFromFile(m_pDev, pszfile, &m_pEffect, nullptr)))
#endif
#endif

	{
		return FALSE;
	}

	D3DXTECHNIQUE_DESCX technique;

	//if (FAILED(m_pEffect->FindNextValidTechnique(NULL, &technique)))
	//{
	//	return FALSE;
	//}
	//_vecTechniques.push_back(technique);
	//_iTechNum++;

	//_DbgOut( " technique.Name", _iTechNum, S_OK,  (char*)technique.Name );

	char t_psz[4];
	strncpy_s(t_psz, "t0", _TRUNCATE);
	while (SUCCEEDED(m_pEffect->FindNextValidTechnique(t_psz, &technique))) {
		_vecTechniques.push_back(technique);
		_iTechNum++;
		//_DbgOut( " technique.Name", _iTechNum, S_OK,  (char*)technique.Name );

		_snprintf_s(t_psz, _TRUNCATE, "t%d", _iTechNum);
	}
	//_DbgOut( " technique.Name", _iTechNum, S_OK,  (char*)technique.Name );

	return TRUE;
}

//BOOL CMPEffectFile::LoadEffectFromResource(char*  pszsrc)
//{
//    HRESULT hr;
//    HMODULE hModule = NULL;
//    HRSRC rsrc;
//    HGLOBAL hgData;
//    LPVOID pvData;
//    DWORD cbData;
//	D3DXTECHNIQUE_DESC   technique;
//
//    rsrc = FindResource( hModule, pszsrc, "EFFECT" );
//    if( rsrc != NULL )
//    {
//        cbData = SizeofResource( hModule, rsrc );
//        if( cbData > 0 )
//        {
//            hgData = LoadResource( hModule, rsrc );
//            if( hgData != NULL )
//            {
//                pvData = LockResource( hgData );
//                if( pvData != NULL )
//                {
//                    if( FAILED( hr = D3DXCreateEffect(
//                        m_pDev, pvData, cbData,
//						&m_pEffect,NULL) ) )
//                    {
//                        return FALSE;
//                    }
//					else
//						return TRUE;
//                }
//            }
//        }
//    }
//	if (FAILED(m_pEffect->FindNextValidTechnique(NULL, &technique)))
//	{
//		return FALSE;
//	}
//	_vecTechniques.push_back(technique);
//	_iTechNum++;
//
//	while(SUCCEEDED(m_pEffect->FindNextValidTechnique(technique.Name, &technique)))
//	{
//		_vecTechniques.push_back(technique);
//		_iTechNum++;
//	}
//
//	return FALSE;
//}

void CMPEffectFile::free() {
	SAFE_RELEASE(m_pEffect);
	_vecTechniques.clear();
	_iTechNum = 0;
	_dwVShader = 0;
	//	if(_dwVShader)
	//	{
	//		m_pDev->DeleteVertexShader(_dwVShader);
	//		_dwVShader = 0;
	//	}
}

BOOL CMPEffectFile::OnLostDevice() {
	if (m_pEffect) {
		if (FAILED(m_pEffect->OnLostDevice()))
			return FALSE;
	}
	return TRUE;
}

BOOL CMPEffectFile::OnResetDevice() {
	if (m_pEffect) {
		if (FAILED(m_pEffect->OnResetDevice()))
			return FALSE;
	}
	return TRUE;
}
#ifdef USE_RENDER
MPRender* CMPEffectFile::GetDev()
#else
LPDIRECT3DDEVICEX CMPEffectFile::GetDev()
#endif
{
	return m_pDev;
}
