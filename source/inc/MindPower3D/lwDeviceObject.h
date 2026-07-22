//
#pragma once

#include "lwHeader.h"
#include "lwDirectX.h"
#include "lwD3DSettings.h"
#include "lwStreamObj.h"
#include "lwMath.h"
#include "lwClassDecl.h"
#include "lwInterfaceExt.h"
#include "lwPreDefinition.h"

LW_BEGIN

class lwVertexBuffer;
class lwIndexBuffer;

class lwDeviceObject : public lwIDeviceObject {
	friend class lwVertexBuffer;
	friend class lwIndexBuffer;

private:
	lwSysGraphics* _sys_graphics;

	DWORD _rs_value[LW_MAX_RENDERSTATE_NUM];
	DWORD _tss_value[LW_MAX_TEXTURESTAGE_NUM][LW_MAX_TEXTURESTAGESTATE_NUM];
	DWORD _ss_value[LW_MAX_SAMPLESTAGE_NUM][LW_MAX_SAMPLESTATE_NUM];
	IDirect3DBaseTextureX* _tex_seq[LW_MAX_TEXTURESTAGE_NUM];

#if (defined LW_USE_DX9)
	DWORD _fvf_value{D3DFMT_UNKNOWN};
	IDirect3DVertexShaderX* _shader_value{nullptr};
	IDirect3DVertexDeclarationX* _decl_value;
#elif (defined LW_USE_DX8)
	IDirect3DVertexShaderX _shader_value{D3DFMT_UNKNOWN};
#endif

	IDirect3DVertexBufferX* _vb_value[LW_MAX_STREAM_NUM];
	IDirect3DIndexBufferX* _ib_value;

	BOOL _light_enable[LW_MAX_LIGHT_NUM];
	D3DLIGHTX _light_seq[LW_MAX_LIGHT_NUM];

	IDirect3DX* _d3d{nullptr};
	IDirect3DDeviceX* _dev{nullptr};
	lwD3DCreateParam _d3d_create_param;
	D3DDISPLAYMODE _display_mode;
	D3DCAPSX _dev_caps;
	D3DVIEWPORTX _viewport;
	lwBackBufferFormatsInfo _bbf_caps;

	RECT _rc_window;
	RECT _rc_client;

	lwVertexBuffer* _svb_head;
	lwIndexBuffer* _sib_head;

	//DWORD _
	lwMatrix44 _mat_view;
	lwMatrix44 _mat_proj;
	lwMatrix44 _mat_viewproj;
	lwMatrix44 _mat_world;

	lwWatchDevVideoMemInfo _watch_vm_info;

	// begin mark
	DWORD _mark_vertex_num;
	DWORD _mark_polygon_num;
	// end mark

	LW_STD_DECLARATION()

private:
	void _ReleaseStreamBuffer(lwVertexBuffer* buf);
	void _ReleaseStreamBuffer(lwIndexBuffer* buf);
	void _ClearStreamBuffer();

public:
	lwDeviceObject(lwSysGraphics* sys_graphics);
	~lwDeviceObject();

	LW_RESULT CreateDirect3D() override;
	LW_RESULT CreateDevice(lwD3DCreateParam* param) override;
	LW_RESULT ResetDevice(D3DPRESENT_PARAMETERS* d3dpp) override;
	LW_RESULT ResetDeviceStateCache() override;
	LW_RESULT ResetDeviceTransformMatrix() override;

	LW_RESULT SetDirect3D(IDirect3DX* d3d) override;
	LW_RESULT SetDevice(IDirect3DDeviceX* dev) override;
	IDirect3DX* GetDirect3D() override { return _d3d; }
	IDirect3DDeviceX* GetDevice() override { return _dev; }
	lwD3DCreateParam* GetD3DCreateParam() override { return &_d3d_create_param; }
	D3DDISPLAYMODE* GetAdapterDisplayMode() override { return &_display_mode; }
	D3DCAPSX* GetDeviceCaps() override { return &_dev_caps; }
	lwBackBufferFormatsInfo* GetBackBufferFormatsCaps() override { return &_bbf_caps; }

	LW_RESULT CreateVertexBuffer(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBufferX** vb, HANDLE* handle) override;
	LW_RESULT CreateIndexBuffer(UINT length, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DIndexBufferX** ib, HANDLE* handle) override;
	LW_RESULT CreateTexture(IDirect3DTextureX** out_tex, const lwTexDataInfo* info, DWORD level, DWORD usage, DWORD format, D3DPOOL pool) override;
	LW_RESULT CreateTexture(IDirect3DTextureX** out_tex, UINT width, UINT height, UINT level, DWORD usage, D3DFORMAT format, D3DPOOL pool) override;
	LW_RESULT CreateTextureFromFileInMemory(IDirect3DTextureX** out_tex, void* data, UINT data_size, UINT width, UINT height, UINT mip_level, DWORD usage, D3DFORMAT format, D3DPOOL pool, DWORD filter, DWORD mip_filter, D3DCOLOR colorkey, D3DXIMAGE_INFO* src_info, PALETTEENTRY* palette) override;
	LW_RESULT CreateVertexBuffer(lwIVertexBuffer** out_obj) override;
	LW_RESULT CreateIndexBuffer(lwIIndexBuffer** out_obj) override;
	LW_RESULT CreateRenderTarget(IDirect3DSurfaceX** o_surface, UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE multi_sample, DWORD multi_sample_quality, BOOL lockable, HANDLE* handle) override;
	LW_RESULT CreateDepthStencilSurface(IDirect3DSurfaceX** o_surface, UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE multi_sample, DWORD multi_sample_quality, BOOL discard, HANDLE* handle) override;
	LW_RESULT CreateCubeTexture(IDirect3DCubeTextureX** o_tex, UINT edge_length, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, HANDLE* handle) override;
	LW_RESULT CreateOffscreenPlainSurface(IDirect3DSurfaceX** surface, UINT width, UINT height, D3DFORMAT format, D3DPOOLX pool, HANDLE* handle) override;

	LW_RESULT ReleaseTex(IDirect3DTextureX* tex) override;
	LW_RESULT ReleaseVertexBuffer(IDirect3DVertexBufferX* vb) override;
	LW_RESULT ReleaseIndexBuffer(IDirect3DIndexBufferX* ib) override;

#if (defined LW_USE_DX9)
	LW_RESULT SetFVFX(DWORD fvf);
	LW_RESULT SetVertexShader(IDirect3DVertexShaderX* shader);
	LW_RESULT SetVertexShaderForced(IDirect3DVertexShaderX* shader);
	LW_RESULT SetVertexDeclaration(IDirect3DVertexDeclarationX* decl);
	LW_RESULT SetVertexDeclarationForced(IDirect3DVertexDeclarationX* decl);
	LW_RESULT SetVertexShaderConstantF(UINT reg_id, const float* data, UINT v_num);
#elif (defined LW_USE_DX8)
	LW_RESULT SetVertexShader(IDirect3DVertexShaderX shader) override;
	LW_RESULT SetVertexShaderConstant(UINT reg_id, const void* data, UINT v_num) override;
#endif

	LW_RESULT SetStreamSource(UINT stream_num, IDirect3DVertexBufferX* stream_data, UINT offset_byte, UINT stride) override;
	LW_RESULT SetIndices(IDirect3DIndexBufferX* index_data, UINT base_vert_index) override;
	LW_RESULT DrawPrimitive(D3DPRIMITIVETYPE pt_type, UINT start_vertex, UINT count) override;
	LW_RESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE pt_type, INT base_vert_index, UINT min_index, UINT vert_num, UINT start_index, UINT count) override;
	LW_RESULT DrawPrimitiveUP(D3DPRIMITIVETYPE pt_type, UINT count, const void* data, UINT stride) override;
	LW_RESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE pt_type, UINT min_vert_index, UINT vert_num, UINT count, const void* index_data, D3DFORMAT index_data_fmt, const void* vert_data, UINT vert_stride) override;

	LW_RESULT SetTransform(D3DTRANSFORMSTATETYPE state, const lwMatrix44* mat) override;
	LW_RESULT SetTransformView(const lwMatrix44* mat) override;
	LW_RESULT SetTransformProj(const lwMatrix44* mat) override;
	LW_RESULT SetTransformWorld(const lwMatrix44* mat) override;

#if 1
	const lwMatrix44* GetMatProj() override {
		return &_mat_proj;
	}
	const lwMatrix44* GetMatView() override { return &_mat_view; }
	const lwMatrix44* GetMatViewProj() override { return &_mat_viewproj; }
#else
	const lwMatrix44* GetMatProj() {
		static lwMatrix44 mat;
		_dev->GetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&mat);
		return &mat;
	}
	const lwMatrix44* GetMatView() {
		static lwMatrix44 mat;
		_dev->GetTransform(D3DTS_VIEW, (D3DMATRIX*)&mat);
		return &mat;
	}
	const lwMatrix44* GetMatViewProj() {
		static lwMatrix44 mat_view;
		static lwMatrix44 mat_proj;
		static lwMatrix44 mat_viewproj;
		_dev->GetTransform(D3DTS_VIEW, (D3DMATRIX*)&mat_view);
		_dev->GetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&mat_proj);
		lwMatrix44Multiply(&mat_viewproj, &mat_view, &mat_proj);
		return &mat_viewproj;
	}
#endif

	LW_RESULT SetViewPort(const D3DVIEWPORTX* vp);
	LW_RESULT SetMaterial(lwMaterial* mtl) override { return _dev->SetMaterial((D3DMATERIALX*)mtl); }

	LW_RESULT SetTexture(DWORD stage, IDirect3DBaseTextureX* tex) override;
	LW_RESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value) override;
	LW_RESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) override;
	LW_RESULT SetTextureForced(DWORD stage, IDirect3DTextureX* tex) override;
	LW_RESULT SetRenderStateForced(D3DRENDERSTATETYPE state, DWORD value) override;
	LW_RESULT SetTextureStageStateForced(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) override;
#if (defined LW_USE_DX9)
	LW_RESULT SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);
	LW_RESULT SetSamplerStateForced(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);
#endif

	LW_RESULT GetViewPort(D3DVIEWPORTX* vp);
	LW_RESULT GetRenderState(DWORD state, DWORD* value) override;
	LW_RESULT GetTextureStageState(DWORD stage, DWORD state, DWORD* value) override;
#if (defined LW_USE_DX9)
	LW_RESULT GetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE state, DWORD* value);
#endif
	LW_RESULT GetTexture(DWORD stage, IDirect3DBaseTextureX** tex) override;

	LW_RESULT SetLight(DWORD id, const D3DLIGHTX* light) override;
	LW_RESULT GetLight(DWORD id, D3DLIGHTX* light) override;
	LW_RESULT LightEnable(DWORD id, BOOL flag) override;
	LW_RESULT LightEnableForced(DWORD id, BOOL flag) override;
	LW_RESULT GetLightEnable(DWORD id, BOOL* flag) override;

#if (defined LW_USE_DX9)
	BOOL GetSoftwareVertexProcessing() override;
#endif

	LW_RESULT InitStateCache() override;
	LW_RESULT InitCapsInfo() override;

	LW_RESULT UpdateWindowRect() override;
	LW_RESULT GetWindowRect(RECT* rc_wnd, RECT* rc_client) override;
	// begin mark
	void BeginBenchMark() override;
	void EndBenchMark() override;
	DWORD GetMarkVertexNum() const { return _mark_vertex_num; }
	DWORD GetMarkPolygonNum() const override { return _mark_polygon_num; }
	lwWatchDevVideoMemInfo* GetWatchVideoMemInfo() override { return &_watch_vm_info; }
	// end mark

	LW_RESULT CheckCurrentDeviceFormat(DWORD type, D3DFORMAT check_fmt) override;

	LW_RESULT ScreenToWorld(lwVector3* org, lwVector3* ray, int x, int y) override;
	LW_RESULT WorldToScreen(int* x, int* y, float* z, const lwVector3* v) override;

	LW_RESULT DumpRenderState(const char* file) override;
};

LW_END