//
#pragma once

#include "lwHeader.h"
#include "lwMath.h"
#include "lwDirectX.h"
#include "lwITypes.h"
#include "lwViewFrustum.h"
#include "lwInterfaceExt.h"
LW_BEGIN

// remarks: by lsh
// 1。具有透明属性的物件只有在显式调用加入scene manger后才做排序
struct lwSortPriProcInfo {
	lwTranspPrimitiveProc proc;
	void* param;
};
struct lwSortPriInfo {
	float d;
	lwIPrimitive* obj;
	lwSortPriProcInfo sppi;
};

class lwSceneMgr : public lwISceneMgr {
	LW_STD_DECLARATION()

private:
	lwISysGraphics* _sys_graphics;
	lwIResourceMgr* _res_mgr;
	lwIViewFrustum* _vf;
	lwSortPriInfo* _sort_obj_seq;
	lwSortPriProcInfo _sppi;
	DWORD _sort_obj_seq_size;
	DWORD _sort_obj_num;
	DWORD _max_sort_num;

	lwCullPriInfo _cull_pri_info;

	lwSceneFrameInfo _frame_info;

public:
	lwSceneMgr(lwISysGraphics* sys_graphics);
	~lwSceneMgr();

	LW_RESULT Update() override;
	LW_RESULT Render() override;
	LW_RESULT BeginRender() override;
	LW_RESULT EndRender() override;
	LW_RESULT RenderTransparentPrimitive() override;
	LW_RESULT AddTransparentPrimitive(lwIPrimitive* obj) override;
	LW_RESULT SortTransparentPrimitive() override;
	void SetTranspPrimitiveProc(lwTranspPrimitiveProc proc, void* param) override {
		_sppi.proc = proc;
		_sppi.param = param;
	}
	LW_RESULT CullPrimitive(lwIPrimitive* obj) override;
	lwIViewFrustum* GetViewFrustum() override { return _vf; }
	DWORD GetMaxSortNum() const override { return _max_sort_num; }
	void GetCullPrimitiveInfo(lwCullPriInfo* info) override { *info = _cull_pri_info; }
	lwSceneFrameInfo* GetSceneFrameInfo() override { return &_frame_info; }
};

LW_END