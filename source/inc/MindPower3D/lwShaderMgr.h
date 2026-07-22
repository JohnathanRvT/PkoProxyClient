//
#pragma once

#include "lwHeader.h"
#include "lwDirectX.h"
#include "lwFrontAPI.h"
#include "lwShaderTypes.h"
#include "lwInterfaceExt.h"

LW_BEGIN

// ===============================================
// directX8 shader manager
struct lwShaderInfo {
	BYTE* data;
	BYTE* decl;
	DWORD size;
	DWORD handle;
};

class lwShaderMgr : public lwIShaderMgr {
	LW_STD_DECLARATION()

private:
	lwIDeviceObject* _dev_obj{nullptr};
#if (defined LW_USE_DX8)
	lwShaderInfo* _vs_seq{nullptr};
#elif (defined LW_USE_DX9)
	lwVertexShaderInfo* _vs_seq{nullptr};
	lwVertDeclInfo9* _decl_seq{nullptr};
#endif
	DWORD _vs_size{0};
	DWORD _vs_num{0};
#if (defined LW_USE_DX9)
	DWORD _decl_size{0};
	DWORD _decl_num{0};
#endif

	lwIShaderDeclMgr* _decl_mgr{nullptr};

public:
	lwShaderMgr(lwIDeviceObject* dev_obj);
	~lwShaderMgr();

	LW_RESULT LoseDevice() override;
	LW_RESULT ResetDevice() override;
	lwIShaderDeclMgr* GetShaderDeclMgr() override { return _decl_mgr; }
	std::tuple<DWORD*, DWORD> LoadFileShader(std::string file, DWORD binary_flag) override;

#if (defined LW_USE_DX8)
	LW_RESULT Init(DWORD vs_buf_size, DWORD ps_buf_size) override;
	LW_RESULT RegisterVertexShader(DWORD type, DWORD* code, DWORD size, DWORD usage, DWORD* decl, DWORD decl_size) override;
	LW_RESULT RegisterVertexShader(DWORD type, std::string file, DWORD usage, DWORD* decl, DWORD decl_size, DWORD file_flag) override;
	LW_RESULT QueryVertexShader(DWORD* ret_obj, DWORD type) override;
	LW_RESULT UnregisterVertexShader(DWORD type) override;
#elif (defined LW_USE_DX9)
	LW_RESULT Init(DWORD vs_buf_size, DWORD decl_buf_size, DWORD ps_buf_size);
	LW_RESULT RegisterVertexShader(DWORD type, DWORD* data, DWORD size);
	LW_RESULT RegisterVertexShader(DWORD type, std::string file, DWORD binary_flag);

	LW_RESULT RegisterVertexDeclaration(DWORD type, D3DVERTEXELEMENT9* data);
	LW_RESULT QueryVertexShader(IDirect3DVertexShaderX** ret_obj, DWORD type);
	LW_RESULT QueryVertexDeclaration(IDirect3DVertexDeclarationX** ret_obj, DWORD type);
	lwVertexShaderInfo* GetVertexShaderInfo(DWORD type) { return &_vs_seq[type]; }
#endif
};

LW_END