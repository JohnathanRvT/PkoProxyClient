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

class lwModel : public lwIModel, public lwLinkCtrl {
private:
	lwIResourceMgr* _res_mgr;
	lwISceneMgr* _scene_mgr;

	lwStateCtrl _state_ctrl;
	char _file_name[LW_MAX_NAME];
	lwIPrimitive* _obj_seq[LW_MAX_MODEL_GEOM_OBJ_NUM];
	DWORD _obj_num;
	lwMatrix44 _mat_base;
	DWORD _id;
	DWORD _model_id;

	lwIHelperObject* _helper_object;
	float _opacity;

	LW_STD_DECLARATION()

public:
	lwModel(lwIResourceMgr* res_mgr);
	~lwModel();

	DWORD GetModelID() const { return _model_id; }
	void SetFileName(const char* file) { _tcsncpy_s(_file_name, file, _TRUNCATE); }
	char* GetFileName() { return _file_name; }

	LW_RESULT Clone(lwIModel** ret_obj);
	LW_RESULT Copy(lwIModel* src_obj);

	lwMatrix44* GetMatrix() override { return &_mat_base; }
	void SetMatrix(const lwMatrix44* mat) override { _mat_base = *mat; }

	LW_RESULT Load(lwIModelObjInfo* info) override;
	LW_RESULT Load(const char* file, DWORD model_id = LW_INVALID_INDEX) override;
	LW_RESULT Update() override;
	LW_RESULT Render() override;
	LW_RESULT RenderPrimitive(DWORD id) override;
	LW_RESULT RenderHelperObject() override;
	LW_RESULT Destroy() override;

	void SetMaterial(const lwMaterial* mtl) override;
	void SetOpacity(float opacity) override;

	LW_RESULT HitTestPrimitive(lwPickInfo* info, const lwVector3* org, const lwVector3* ray) override;
	LW_RESULT HitTestPrimitiveHelperMesh(lwPickInfo* info, const lwVector3* org, const lwVector3* ray, const char* type_name) override;
	LW_RESULT HitTestPrimitiveHelperBox(lwPickInfo* info, const lwVector3* org, const lwVector3* ray, const char* type_name) override;
	LW_RESULT HitTest(lwPickInfo* info, const lwVector3* org, const lwVector3* ray) override;
	LW_RESULT HitTestHelperMesh(lwPickInfo* info, const lwVector3* org, const lwVector3* ray, const char* type_name) override;
	LW_RESULT HitTestHelperBox(lwPickInfo* info, const lwVector3* org, const lwVector3* ray, const char* type_name) override;

	LW_RESULT PlayDefaultAnimation() override;

	LW_RESULT SortPrimitiveObj();

	void ShowHelperObject(int show) override;
	void ShowHelperMesh(int show) override;
	void ShowHelperBox(int show) override;
	void ShowBoundingObject(int show) override;
	lwIHelperObject* GetHelperObject() override { return _helper_object; }

	DWORD GetPrimitiveNum() const override { return _obj_num; }
	lwIPrimitive* GetPrimitive(DWORD id) override { return _obj_seq[id]; }

	void SetObjState(DWORD state, BYTE value) override { return _state_ctrl.SetState(state, value); }
	DWORD GetObjState(DWORD state) const override { return _state_ctrl.GetState(state); }

	LW_RESULT RegisterSceneMgr(lwISceneMgr* scene_mgr) override {
		_scene_mgr = scene_mgr;
		return LW_RET_OK;
	}

	LW_RESULT SetItemLink(const lwItemLinkInfo* info) override;
	LW_RESULT ClearItemLink(lwIItem* obj) override;

	// link ctrl method
	virtual LW_RESULT GetLinkCtrlMatrix(lwMatrix44* mat, DWORD link_id) override;

	LW_RESULT SetTextureLOD(DWORD level) override;
	float GetOpacity() override { return _opacity; }

	LW_RESULT CullPrimitive() override;
	DWORD GetCullingPrimitiveNum() override;
	LW_RESULT ExtractModelInfo(lwIModelObjInfo* out_info) override;
};

LW_END