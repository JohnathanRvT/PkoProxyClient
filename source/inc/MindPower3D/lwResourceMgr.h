#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwObjectPool.h"
#include "lwClassDecl.h"
#include "lwITypes.h"
#include "lwDirectX.h"
#include "lwExpObj.h"
#include "lwGraphicsUtil.h"
#include "lwInterfaceExt.h"
#include "lwShaderMgr.h"
#include "lwPreDefinition.h"
#include "lwMisc.h"
#include "lwPathInfo.h"
#include "lwSyncObj.h"

#include <list>
#include <vector>

LW_BEGIN

enum lwResStateEnum {
	RES_STATE_INVALID = 0x000,
	RES_STATE_INIT = 0x0001,
	RES_STATE_SYSTEMMEMORY = 0x0002,
	RES_STATE_VIDEOMEMORY = 0x0004,
	RES_STATE_LOADTEST = 0x0010,
	RES_STATE_LOADTEST_0 = 0x0020,
	RES_STATE_LOADING_VM = 0x1000,
};

#if 0
struct lwTexLogInfo
{
    char file[LW_MAX_PATH];
    DWORD width;
    DWORD height;
    D3DFORMAT fmt;
    DWORD devmem_size;

    void Save(FILE* fp)
    {
        char buf[512];
        _snprintf_s(buf, _TRUNCATE, "file:%s\nwidth: %d\theight: %d\tformat: %d\tmem_size:%d\n\n",
            file, width, height, fmt, devmem_size);

        fwrite(buf, strlen(buf), 1, fp);
    }
};

class lwTexLogMgr
{
    typedef list<lwTexLogInfo*> lwListTexLog;
private:
    lwListTexLog _list_texlog;

public:
    lwTexLogMgr()
    {};
    ~lwTexLogMgr()
    {
        lwListTexLog::iterator it = _list_texlog.begin();
        for(; it != _list_texlog.end(); ++it)
        {
            LW_DELETE((*it));
        }
    }

    void Add(const char* file, DWORD width, DWORD height, D3DFORMAT fmt, DWORD devmem_size)
    {
        lwTexLogInfo* info = LW_NEW(lwTexLogInfo);
        _tcsncpy_s(info->file, file, _TRUNCATE);
        info->width = width;
        info->height = height;
        info->fmt = fmt;
        info->devmem_size = devmem_size;

        _list_texlog.push_back(info);
    }

    void Save(const char* file)
    {
        lwListTexLog::iterator it;
        FILE* fp;
	fopen_s(&fp, file, "wt");
        if(fp == 0)
            goto __ret;

        it = _list_texlog.begin();
        for(; it != _list_texlog.end(); ++it)
        {
            (*it)->Save(fp);
        }

        DWORD total_size = 0;
        DWORD scene_size = 0;
        DWORD terrain_size = 0;
        DWORD ui_size = 0;
        DWORD mini_map = 0;

        it = _list_texlog.begin();
        for(; it != _list_texlog.end(); ++it)
        {
            total_size += (*it)->devmem_size;

            if(_tcsstr((*it)->file, "scene"))
            {
                scene_size += (*it)->devmem_size;
            }
            else if(_tcsstr((*it)->file, "terrain"))
            {
                terrain_size += (*it)->devmem_size;
            }
            else if(_tcsstr((*it)->file, "ui"))
            {
                ui_size += (*it)->devmem_size;
            }
            else if(_tcsstr((*it)->file, "minimap"))
            {
                mini_map += (*it)->devmem_size;
            }
        }

        char buf[256];
        _snprintf_s(buf, _TRUNCATE, "total: %d, scene: %d, terrain: %d, ui: %d, map: %d\n",
            total_size, scene_size, terrain_size, ui_size, mini_map);
        fwrite(buf, strlen(buf), 1, fp);

__ret:
        if(fp)
        {
            fclose(fp);
        }
    }
};
#else

struct lwTexLogFilterInfo {
	char filter_str[LW_MAX_PATH];
	DWORD size;
	DWORD num;

	BOOL Filter(DWORD op_type, const char* file, DWORD file_size) {
		if (_tcsstr(file, filter_str) == nullptr)
			return 0;

		if (op_type == 1) {
			size += file_size;
			num += 1;
		} else {
			size -= file_size;
			num -= 1;
		}

		return 1;
	}
};
class lwTexLogMgr {
	using lwTexLogFilterInfoSeq = std::vector<lwTexLogFilterInfo>;

private:
	FILE* _fp;
	DWORD _total_size;
	DWORD _total_num;
	lwTexLogFilterInfoSeq _v_filter_entity;

public:
	lwTexLogMgr() {
		_fp = nullptr;
		_total_size = 0;
		_total_num = 0;
		_v_filter_entity.reserve(20);
	};
	~lwTexLogMgr() {
		if (_fp) {
			char buf[256];

			for (auto& log : _v_filter_entity) {
				_snprintf_s(buf, _TRUNCATE, "type: %s, num: %d, size: %d\n", log.filter_str, log.num, log.size);
				fwrite(buf, strlen(buf), 1, _fp);
			}
			_snprintf_s(buf, _TRUNCATE, "Total num: %d, Total size: %d\n", _total_num, _total_size);
			fwrite(buf, strlen(buf), 1, _fp);

			fclose(_fp);
		}
	}

	LW_RESULT OpenLogFile(const char* file) {
		fopen_s(&_fp, file, "wt");
		return _fp ? LW_RET_OK : LW_RET_FAILED;
	}
	LW_RESULT AddTexType(const char* name) {
		lwTexLogFilterInfo info;
		info.num = 0;
		info.size = 0;
		_tcsncpy_s(info.filter_str, name, _TRUNCATE);

		_v_filter_entity.push_back(info);
		return LW_RET_OK;
	}

	LW_RESULT Log(DWORD op_type, const char* file, DWORD width, DWORD height, D3DFORMAT fmt, DWORD devmem_size) {
		if (_fp == nullptr)
			return LW_RET_FAILED;

		char op[2][64] =
			{
				"Load",
				"Release"};
		char* p;

		if (op_type == 1) {
			_total_size += devmem_size;
			_total_num += 1;
			p = op[0];
		} else {
			_total_size -= devmem_size;
			_total_num -= 1;
			p = op[1];
		}

		char buf[512];
		_snprintf_s(buf, _TRUNCATE, "%s file:%s\nwidth: %d\theight: %d\tformat: %d\tmem_size:%d\n",
					p, file, width, height, fmt, devmem_size);

		fwrite(buf, strlen(buf), 1, _fp);

		for (auto& log : _v_filter_entity) {
			if (log.Filter(op_type, file, devmem_size)) {
				//break;
			}
			_snprintf_s(buf, _TRUNCATE, "[%s]: num: %d, size: %d\t[%d]\n", log.filter_str, log.num, log.size, _total_size);
			fwrite(buf, strlen(buf), 1, _fp);
		}

		return LW_RET_OK;
	}
};

#endif

class lwTex : public lwITex {
	LW_STD_DECLARATION()

public:
	lwIResourceMgr* _res_mgr;
	int _ref;

	DWORD _reg_id;
	DWORD _state;	//...
	DWORD _tex_type; // file texture or user-defined texture

	DWORD _level;
	DWORD _usage;
	DWORD _format;
	D3DPOOL _pool;
	void* _data;
	DWORD _data_size;

	DWORD _stage;
	DWORD _byte_alignment_flag;
	DWORD _colorkey_type;
	lwColorValue4b _colorkey;
	char _file_name[LW_MAX_PATH];

	lwTexDataInfo _data_info;

	IDirect3DTextureX* _tex;

	//lwTextureStageStateSetTex2 _tss_set;
	//lwRenderStateSetTex2 _rs_set;
	lwRenderStateAtomSet _rsa_0;

	DWORD _load_type;
	DWORD _load_mask;
	DWORD _load_flag;
	DWORD _mt_flag;

public:
	lwTex(lwIResourceMgr* res_mgr);
	~lwTex();

	int AddRef(int i) override { return _ref += i; }
	int GetRef() override { return _ref; }

	LW_RESULT LoadTexInfo(const lwTexInfo* info, const char* tex_path) override;
	LW_RESULT LoadSystemMemory() override;
	LW_RESULT LoadVideoMemory() override;
	LW_RESULT LoadVideoMemoryMT() override;
	LW_RESULT LoadVideoMemoryEx() override;
	LW_RESULT LoadVideoMemoryDirect() override;
	LW_RESULT UnloadSystemMemory() override;
	LW_RESULT UnloadVideoMemory() override;
	LW_RESULT Unload() override;

	void SetRegisterID(DWORD id) override { _reg_id = id; }
	DWORD GetRegisterID() const override { return _reg_id; }

	char* GetFileName() override { return _file_name; }
	DWORD GetStage() const override { return _stage; }
	DWORD GetState() const override { return _state; }
	IDirect3DTextureX* GetTex() override { return _tex; }
	lwTexDataInfo* GetDataInfo() override { return &_data_info; }
	lwColorValue4b GetColorKey() const override { return _colorkey; }
	void GetTexInfo(lwTexInfo* info) override;

	void SetFileName(const char* file) { _tcsncpy_s(_file_name, file, _TRUNCATE); }
	void SetState(DWORD state) { _state = state; }
	void SetStage(DWORD stage) { _stage = stage; }
	void SetTexFormat(DWORD fmt) { _format = fmt; }
	void SetColorKeyType(DWORD type, const lwColorValue4b* c) {
		_colorkey_type = type;
		if (c) {
			_colorkey = *c;
		}
	}
	DWORD SetLOD(DWORD level) override;
	void SetLoadFlag(DWORD flag) override { _load_flag = flag; }

	void SetMTFlag(DWORD flag) override { _mt_flag = flag; }
	DWORD GetMTFlag() override { return _mt_flag; }
	BOOL IsLoadingOK() const override;

	LW_RESULT Register();
	LW_RESULT Unregister();

	LW_RESULT BeginPass() override;
	LW_RESULT EndPass() override;
	LW_RESULT BeginSet() override;
	LW_RESULT EndSet() override;
	LW_RESULT LoseDevice() override;
	LW_RESULT ResetDevice() override;

	void SetLoadResType(DWORD type) override { _load_type = type; }
	void SetLoadResMask(DWORD add_mask, DWORD remove_mask) override;
	DWORD GetLoadResMask() override { return _load_mask; }
	DWORD GetLoadResType() override { return _load_type; }
};

// remarks by lsh
// 有关lwMesh和lwMeshInfo之间的关系
// 1。lwMeshInfo作为纯粹的C类型结构体，再关联一组construct和destruct等的辅助函数。
//    lwMesh聚合该结构体
// 2。lwMeshInfo是一个class，同时拥有lwIMeshInfo的接口方式
//    lwMesh继承（或引用接口指针）该类，在效率上不能接受
//
class lwMesh : public lwIMesh {

	LW_STD_DECLARATION()

private:
public:
	lwIResourceMgr* _res_mgr;

	DWORD _reg_id;
	DWORD _state; //...

	DWORD _stream_type;
	DWORD _colorkey;
	LW_HANDLE _vb_id;
	LW_HANDLE _ib_id;
	lwIVertexBuffer* _svb;
	lwIIndexBuffer* _sib;

	lwResFileMesh _res_file;

	lwMeshDataInfo _data_info;
	lwMeshInfo _mesh_info;
	lwMeshInfo* _mesh_info_ptr;

	lwRenderStateAtomSet _rsa_0;

	DWORD _mt_flag;

public:
	lwMesh(lwIResourceMgr* res_mgr);
	~lwMesh();

	LW_RESULT LoadSystemMemory(const lwMeshInfo* info) override;
	LW_RESULT LoadSystemMemoryMT(const lwMeshInfo* info) override;
	LW_RESULT LoadSystemMemory() override;
	LW_RESULT LoadVideoMemory() override;
	LW_RESULT LoadVideoMemoryMT() override;
	LW_RESULT LoadVideoMemoryEx() override;
	LW_RESULT UnloadSystemMemory() override;
	LW_RESULT UnloadVideoMemory() override;
	LW_RESULT Unload() override;

	LW_RESULT ExtractMesh(lwMeshInfo* info) override;

	void SetStreamType(DWORD type) override { _stream_type = type; }
	DWORD GetStreamType() const override { return _stream_type; }

	DWORD GetRegisterID() const override { return _reg_id; }
	lwResFileMesh* GetResFileMesh() override { return &_res_file; }
	DWORD GetState() const override { return _state; }
	lwMeshInfo* GetMeshInfo() override { return &_mesh_info; }

	LW_HANDLE GetVBHandle() const override { return _vb_id; }
	LW_HANDLE GetIBHandle() const override { return _ib_id; }

	void SetRegisterID(DWORD id) override { _reg_id = id; }
	void SetState(DWORD state) { _state = state; }
	void SetColorkey(DWORD key) override { _colorkey = key; }
	LW_RESULT SetResFile(const lwResFileMesh* info) override;

	LW_RESULT Register();
	LW_RESULT Unregister();

	LW_RESULT AddStateToSet(DWORD state, DWORD value) override; //添加 RenderState 如果渲染状态表满了，则返回失败，最多8个
	LW_RESULT BeginSet() override;
	LW_RESULT EndSet() override;
	LW_RESULT DrawSubset(DWORD subset) override;
	LW_RESULT LoseDevice() override;
	LW_RESULT ResetDevice() override;

	lwILockableStreamVB* GetLockableStreamVB() override;
	lwILockableStreamIB* GetLockableStreamIB() override;

	BOOL IsLoadingOK() const override;
	void SetMTFlag(DWORD flag) override { _mt_flag = flag; }
	DWORD GetMTFlag() override { return _mt_flag; }
};

class lwMtlTexAgent : public lwIMtlTexAgent {
	LW_STD_DECLARATION()

	enum {
		RSA_SET_SIZE = 8
	};

private:
public:
	lwIResourceMgr* _res_mgr;

	float _opacity;
	DWORD _transp_type;
	lwMaterial _mtl;
	BOOL _render_flag;

	lwRenderStateAtomSet _rsa_0;

#ifdef MTLTEXAGENT_OPACITY_RSA_FLAG
	lwRenderStateAtomSet _rsa_opacity;
	//lwRenderStateAtom _opacity_rsa_seq[5];
#endif

	lwITex* _tex_seq[LW_MAX_MTL_TEX_NUM]; // multi-texture blending
	lwMatrix44* _uvmat[LW_MAX_MTL_TEX_NUM];
	lwITex* _tt_tex[LW_MAX_MTL_TEX_NUM];

	DWORD _opacity_reserve_rs[2]; // D3DRS_SRCBLEND, D3DRS_DESTBLEND

public:
	lwMtlTexAgent(lwIResourceMgr* mgr);
	~lwMtlTexAgent();

	void SetOpacity(float opacity) override { _opacity = opacity; }
	void SetTranspType(DWORD type) override { _transp_type = type; }
	void SetMaterial(const lwMaterial* mtl) override { _mtl = *mtl; }
	void SetRenderFlag(BOOL flag) override { _render_flag = flag; }
	lwIRenderStateAtomSet* GetMtlRenderStateSet() override { return (lwIRenderStateAtomSet*)&_rsa_0; }

	float GetOpacity() const override { return _opacity; }
	DWORD GetTransparencyType() const override { return _transp_type; }
	lwMaterial* GetMaterial() override { return &_mtl; }
	lwITex* GetTex(DWORD stage) override { return _tex_seq[stage]; }
	BOOL GetRenderFlag() override { return _render_flag; }

	LW_RESULT BeginPass() override;
	LW_RESULT EndPass() override;
	LW_RESULT BeginSet() override;
	LW_RESULT EndSet() override;

	LW_RESULT SetTex(DWORD stage, lwITex* obj, lwITex** ret_obj) override;
	LW_RESULT LoadMtlTex(lwMtlTexInfo* info, const char* tex_path) override;
	LW_RESULT LoadTextureStage(const lwTexInfo* info, const char* tex_path) override;

	LW_RESULT ExtractMtlTex(lwMtlTexInfo* info) override;

	LW_RESULT DestroyTextureStage(DWORD stage) override;
	LW_RESULT Destroy() override;
	LW_RESULT Clone(lwIMtlTexAgent** ret_obj) override;
	LW_RESULT SetTextureTransformMatrix(DWORD stage, const lwMatrix44* mat) override;
	LW_RESULT SetTextureTransformImage(DWORD stage, lwITex* tex) override;

	LW_RESULT SetTextureLOD(DWORD level) override;
	BOOL IsTextureLoadingOK() const override;
};

class lwMeshAgent : public lwIMeshAgent {
	LW_STD_DECLARATION()

private:
	lwIResourceMgr* _res_mgr;
	lwIMesh* _mesh_obj;
	lwRenderStateSetMesh2 _rs_set;
	DWORD _mt_flag;

public:
	lwMeshAgent(lwIResourceMgr* res_mgr);
	~lwMeshAgent();

	void SetRenderState(DWORD begin_end, DWORD state, DWORD value) override {
		lwSetRenderStateSet(&_rs_set, begin_end, state, value);
	}
	void SetRenderState(lwRenderStateSetMtl2* rs_set) override { _rs_set = *rs_set; }
	void SetMesh(lwIMesh* mesh) override { _mesh_obj = mesh; }
	lwIMesh* GetMesh() override { return _mesh_obj; }

	void SetMTFlag(DWORD flag) { _mt_flag = flag; }
	LW_RESULT BeginSet() override;
	LW_RESULT EndSet() override;
	LW_RESULT DrawSubset(DWORD subset) override;
	LW_RESULT LoadMesh(const lwMeshInfo* info) override;
	LW_RESULT LoadMesh(const lwResFileMesh* info) override;
	LW_RESULT DestroyMesh() override;
	LW_RESULT Destroy() override;
	LW_RESULT Clone(lwIMeshAgent** ret_obj) override;
};

class lwResBufMgr : public lwIResBufMgr {
	struct lwModelObjInfoMap {
		lwModelObjInfo info;
		char file[LW_MAX_PATH];
		DWORD size;
		DWORD hit_time;
	};

	using lwPoolSysMemTex = lwObjectPoolVoidPtr10240;
	using lwPoolModelObj = lwObjectPoolVoidPtr10240;

	LW_STD_DECLARATION()

private:
	lwIResourceMgr* _res_mgr;
	lwPoolSysMemTex _pool_sysmemtex;
	lwPoolModelObj _pool_modelobj;
	lwCriticalSection _lock_sysmemtex;

	DWORD _modelobj_data_size;
	DWORD _sysmemtex_data_size;

	DWORD _lmt_modelobj_data_size;
	DWORD _lmt_modelobj_data_time;

public:
	lwResBufMgr(lwIResourceMgr* res_mgr);
	~lwResBufMgr();

	LW_RESULT Destroy() override;
	LW_RESULT RegisterSysMemTex(LW_HANDLE* handle, const lwSysMemTexInfo* info) override;
	LW_RESULT QuerySysMemTex(lwSysMemTexInfo* info) override;
	LW_RESULT QuerySysMemTex(lwSysMemTexInfo** info, const char* file) override;
	LW_RESULT GetSysMemTex(lwSysMemTexInfo** info, LW_HANDLE handle) override;
	LW_RESULT UnregisterSysMemTex(LW_HANDLE handle) override;

	LW_RESULT RegisterModelObjInfo(LW_HANDLE handle, const char* file) override;
	LW_RESULT RegisterModelObjInfo(LW_HANDLE* handle, const char* file) override;
	LW_RESULT QueryModelObjInfo(lwIModelObjInfo** info, const char* file) override;
	LW_RESULT GetModelObjInfo(lwIModelObjInfo** info, LW_HANDLE handle) override;
	LW_RESULT UnregisterModelObjInfo(LW_HANDLE handle) override;

	void SetLimitModelObjInfo(DWORD lmt_size, DWORD lmt_time) override {
		_lmt_modelobj_data_size = lmt_size;
		_lmt_modelobj_data_time = lmt_time;
	}
	LW_RESULT FilterModelObjInfoSize();
};

class lwThreadPoolMgr : public lwIThreadPoolMgr {
	LW_STD_DECLARATION()

	enum {
		CRITICALSECTION_SEQ_SIZE = 1,
	};

private:
	lwIThreadPool* _pool_seq[THREAD_POOL_SIZE];
	CRITICAL_SECTION _cs_seq[CRITICALSECTION_SEQ_SIZE];

public:
	lwThreadPoolMgr();
	~lwThreadPoolMgr();

	LW_RESULT Create() override;
	LW_RESULT Destroy() override;

	lwIThreadPool* GetThreadPool(DWORD type) override { return _pool_seq[type]; }
	void LockCriticalSection(DWORD type) override { ::EnterCriticalSection(&_cs_seq[type]); }
	void UnlockCriticalSection(DWORD type) override { ::LeaveCriticalSection(&_cs_seq[type]); }
};

// lwResourceMgr
class lwResourceMgr : public lwIResourceMgr {
	using lwPoolMesh = lwObjectPoolVoidPtr10240;
	using lwPoolTex = lwObjectPoolVoidPtr40960;
	using lwPoolMeshRender = lwObjectPoolVoidPtr1024;
	using lwPoolTexRender = lwObjectPoolVoidPtr1024;
	using lwPoolAnimCtrl = lwObjectPoolVoidPtr1024;

	LW_STD_DECLARATION()

private:
	lwISysGraphics* _sys_graphics;
	lwIDeviceObject* _dev_obj;
	lwIStaticStreamMgr* _static_stream_mgr;
	lwIDynamicStreamMgr* _dynamic_stream_mgr;
	lwILockableStreamMgr* _lockable_stream_mgr;
	lwISurfaceStreamMgr* _surface_stream_mgr;
	lwIShaderMgr* _shader_mgr;
	lwIResBufMgr* _resbuf_mgr;
	lwIThreadPoolMgr* _thread_pool_mgr;

	lwPoolMesh _pool_mesh;
	lwPoolTex _pool_tex;
	lwPoolAnimCtrl _pool_animctrl;

	lwObjectPoolVoidPtr10240 _pool_model;
	lwObjectPoolVoidPtr1024 _pool_physique;
	lwObjectPoolVoidPtr1024 _pool_item;

	char _texture_path[LW_MAX_PATH];

	// render ctrl proc sequence
	lwRenderCtrlVSCreateProc _render_ctrl_proc_seq[LW_RENDER_CTRL_PROC_NUM];

	lwByteSet _byte_set;
	lwAssObjInfo _assobj_info;

	// begin debug information
	LW_DWORD _mesh_size_sm;
	LW_DWORD _mesh_size_vm;
	LW_DWORD _tex_size_sm;
	LW_DWORD _tex_size_vm;

public:
	lwTexLogMgr _texlog_mgr;
	// end

public:
	lwResourceMgr(lwISysGraphics* sys);
	~lwResourceMgr();

	lwISysGraphics* GetSysGraphics() override { return _sys_graphics; }
	lwIDeviceObject* GetDeviceObject() override { return _dev_obj; }
	lwIShaderMgr* GetShaderMgr() override { return _shader_mgr; }
	lwIStaticStreamMgr* GetStaticStreamMgr() override { return _static_stream_mgr; }
	lwIDynamicStreamMgr* GetDynamicStreamMgr() override { return _dynamic_stream_mgr; }
	lwILockableStreamMgr* GetLockableStreamMgr() override { return _lockable_stream_mgr; }
	lwISurfaceStreamMgr* GetSurfaceStreamMgr() override { return _surface_stream_mgr; }
	lwIResBufMgr* GetResBufMgr() override { return _resbuf_mgr; }
	lwIThreadPoolMgr* GetThreadPoolMgr() override { return _thread_pool_mgr; }
	lwIByteSet* GetByteSet() override { return &_byte_set; }

	LW_RESULT SetAssObjInfo(DWORD mask, const lwAssObjInfo* info) override;
	LW_RESULT GetAssObjInfo(lwAssObjInfo* info) override;

	LW_RESULT CreateMesh(lwIMesh** ret_obj) override;
	LW_RESULT CreateTex(lwITex** ret_obj) override;
	LW_RESULT CreateAnimCtrl(lwIAnimCtrl** ret_obj, DWORD type) override;
	LW_RESULT CreateAnimCtrlObj(lwIAnimCtrlObj** ret_obj, DWORD type) override;

	LW_RESULT CreateMeshAgent(lwIMeshAgent** ret_obj) override;
	LW_RESULT CreateMtlTexAgent(lwIMtlTexAgent** ret_obj) override;
	LW_RESULT CreateAnimCtrlAgent(lwIAnimCtrlAgent** ret_obj) override;
	LW_RESULT CreateRenderCtrlAgent(lwIRenderCtrlAgent** ret_obj) override;
	LW_RESULT CreateRenderCtrlVS(lwIRenderCtrlVS** ret_obj, DWORD type) override;

	LW_RESULT CreatePrimitive(lwIPrimitive** ret_obj) override;
	LW_RESULT CreateHelperObject(lwIHelperObject** ret_obj) override;

	LW_RESULT CreatePhysique(lwIPhysique** ret_obj) override;
	LW_RESULT CreateModel(lwIModel** ret_obj) override;
	LW_RESULT CreateItem(lwIItem** ret_obj) override;

	LW_RESULT CreateNode(lwINode** ret_obj, DWORD type) override;
	LW_RESULT CreateNodeObject(lwINodeObject** ret_obj) override;

	LW_RESULT CreateStaticStreamMgr(lwIStaticStreamMgr** mgr) override;
	LW_RESULT CreateDynamicStreamMgr(lwIDynamicStreamMgr** mgr) override;

	LW_RESULT GetMesh(lwIMesh** ret_obj, DWORD id) override;
	LW_RESULT GetTex(lwITex** ret_obj, DWORD id) override;
	LW_RESULT GetAnimCtrl(lwIAnimCtrl** ret_obj, DWORD id) override;

	// 直接注册，不通过文件方式查找，在这种情况下一旦内存数据被删除，显存数据将创建失败
	// 适用于一些帮助性的物体，如BoundingBox等
	LW_RESULT RegisterMesh(lwIMesh* obj) override;
	LW_RESULT RegisterTex(lwITex* obj) override;
	LW_RESULT RegisterAnimCtrl(lwIAnimCtrl* obj) override;
	LW_RESULT RegisterRenderCtrlProc(DWORD id, lwRenderCtrlVSCreateProc proc) override;

	LW_RESULT UnregisterMesh(lwIMesh* obj) override;
	LW_RESULT UnregisterTex(lwITex* obj) override;
	LW_RESULT UnregisterAnimCtrl(lwIAnimCtrl* obj) override;

	LW_RESULT ClearAllMesh();
	LW_RESULT ClearAllTex();
	LW_RESULT ClearAllAnimCtrl();

	LW_RESULT AddRefMesh(lwIMesh* obj, DWORD ref_cnt) override;
	LW_RESULT AddRefTex(lwITex* obj, DWORD ref_cnt) override;
	LW_RESULT AddRefAnimCtrl(lwIAnimCtrl* ret_obj, DWORD ref_cnt) override;

	LW_ULONG QueryTexRefCnt(lwITex* obj) override;

	LW_RESULT QueryMesh(DWORD* ret_id, const lwResFileMesh* rfm) override;
	LW_RESULT QueryTex(DWORD* ret_id, const char* file_name) override;
	// 目前只有RegisterAnimData时候使用RES_FILE_TYPE_GENERIC或RES_FILE_TYPE_GEOMETRY文件类型的资源才有效
	LW_RESULT QueryAnimCtrl(DWORD* ret_id, const lwResFileAnimData* info) override;

	LW_VOID ReleaseObject();
	LW_RESULT RegisterObject(DWORD* ret_id, void* obj, DWORD type) override;
	LW_RESULT UnregisterObject(void** ret_obj, DWORD id, DWORD type) override;
	LW_RESULT QueryObject(void** ret_obj, DWORD type, const char* file_name) override;
	LW_RESULT QueryModelObject(void** ret_obj, DWORD model_id) override;

	LW_RESULT LoseDevice() override;
	LW_RESULT ResetDevice() override;

	void SetTexturePath(const char* path) override { _tcsncpy_s(_texture_path, path, _TRUNCATE); }
	char* GetTexturePath() override { return _texture_path; }

public:
	IDirect3DTextureX* getMonochromaticTexture(D3DCOLOR colour, const std::string& filterTexture) override;

	const char* getTextureOperationDescription(size_t operation) override;

private:
	IDirect3DTextureX* _createMonochromaticTexture(
		D3DCOLOR colour,
		const std::string& filterTexture,
		size_t width, size_t height);

	using ColourFilterPair = std::pair<D3DCOLOR, std::string>;
	using ColorFilterPairTextureList = std::map<ColourFilterPair, IDirect3DTextureX*>;
	ColorFilterPairTextureList mColorFilterTextureList;
};

LW_END