//
#pragma once

#include "lwHeader.h"
#include "lwMath.h"
#include "lwInterfaceExt.h"
#include "lwClassDecl.h"
#include "lwITypes.h"
#include "lwPrimitive.h"
#include "lwLinkCtrl.h"

LW_BEGIN

class lwItem : public lwIItem {
private:
	lwIResourceMgr* _res_mgr;
	lwISceneMgr* _scene_mgr;

	lwLinkCtrl* _link_ctrl;
	DWORD _link_parent_id;
	DWORD _link_item_id;

	lwStateCtrl _state_ctrl;
	char _file_name[LW_MAX_NAME];
	lwIPrimitive* _obj;
	lwMatrix44 _mat_base;
	DWORD _id;
	DWORD _item_type;
	float _opacity;

	LW_STD_DECLARATION()

public:
	lwItem(lwIResourceMgr* res_mgr);
	~lwItem();

	void SetFileName(const char* file) { _tcsncpy_s(_file_name, file, _TRUNCATE); }
	char* GetFileName() { return _file_name; }

	virtual lwIPrimitive* GetPrimitive() override { return _obj; }

	LW_RESULT Copy(lwIItem* src_obj) override;
	LW_RESULT Clone(lwIItem** ret_obj) override;

	lwMatrix44* GetMatrix() override { return &_mat_base; }
	void SetMatrix(const lwMatrix44* mat) override { _mat_base = *mat; }
	void SetOpacity(float opacity) override;

	LW_RESULT Load(const char* file, int arbitrary_flag = 0) override;
	LW_RESULT Load(lwIGeomObjInfo* info) override;
	LW_RESULT Update() override;
	LW_RESULT Render() override;
	LW_RESULT Destroy() override;

	LW_RESULT HitTestPrimitive(lwPickInfo* info, const lwVector3* org, const lwVector3* ray) override;

	void SetMaterial(const lwMaterial* mtl) override;

	void ShowBoundingObject(int show) override;

	const lwMatrix44* GetObjDummyMatrix(DWORD id) override;
	const lwMatrix44* GetObjBoneDummyMatrix(DWORD id);

	LW_RESULT PlayDefaultAnimation() override;

	// 这里我们定义如果道具具有骨骼动画，则dummy只有绑定在骨骼上才是有效的
	LW_RESULT GetDummyMatrix(lwMatrix44* mat, DWORD id) override;
	LW_RESULT GetObjDummyRunTimeMatrix(lwMatrix44* mat, DWORD id) override;

	LW_RESULT SetLinkCtrl(lwLinkCtrl* ctrl, DWORD link_parent_id, DWORD link_item_id) override;
	LW_RESULT ClearLinkCtrl() override;

	void SetObjState(DWORD state, BYTE value) override { return _state_ctrl.SetState(state, value); }
	DWORD GetObjState(DWORD state) const override { return _state_ctrl.GetState(state); }
	LW_RESULT RegisterSceneMgr(lwISceneMgr* scene_mgr) override {
		_scene_mgr = scene_mgr;
		return LW_RET_OK;
	}

	LW_RESULT SetTextureLOD(DWORD level) override;
	float GetOpacity() override { return _opacity; }
};

LW_END