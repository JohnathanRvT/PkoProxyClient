//
#pragma once

#include "lwHeader.h"
#include "lwDirectX.h"
#include "lwFrontAPI.h"
#include "lwStreamObj.h"
#include "lwExpObj.h"
#include "lwITypes.h"
#include "lwInterfaceExt.h"
#include "lwDeviceObject.h"
#include "lwResourceMgr.h"
#include "lwPrimitiveHelper.h"

LW_BEGIN

class lwPrimitive : public lwIPrimitive {
	LW_STD_DECLARATION()

protected:
	DWORD _id;
	DWORD _parent_id;
	lwStateCtrl _state_ctrl;

	lwIResourceMgr* _res_mgr;
	lwIMeshAgent* _mesh_agent;
	lwIMtlTexAgent* _mtltex_agent_seq[LW_MAX_SUBSET_NUM];
	lwIAnimCtrlAgent* _anim_agent;
	lwIRenderCtrlAgent* _render_agent;
	lwIHelperObject* _helper_object;

private:
	LW_RESULT _UpdateTransparentState();

public:
	lwPrimitive(lwIResourceMgr* mgr);
	virtual ~lwPrimitive();

	virtual void SetMatrixLocal(const lwMatrix44* mat) override;
	virtual void SetMatrixParent(const lwMatrix44* mat) override;

	lwMatrix44* GetMatrixLocal() override;
	lwMatrix44* GetMatrixGlobal() override;
	void SetID(DWORD id) override { _id = id; }
	void SetParentID(DWORD id) override { _parent_id = id; }
	DWORD GetID() const override { return _id; }
	DWORD GetParentID() const override { return _parent_id; }

	LW_RESULT HitTest(lwPickInfo* info, const lwVector3* org, const lwVector3* ray) override;

	LW_RESULT DestroyRenderCtrlAgent() override;
	LW_RESULT DestroyMeshAgent() override;
	LW_RESULT DestroyMtlTexAgent(DWORD subset) override;
	LW_RESULT Destroy() override;

	LW_RESULT Load(lwIGeomObjInfo* info, const char* tex_path, const lwResFile* res) override;
	LW_RESULT LoadMesh(lwMeshInfo* info) override;
	LW_RESULT LoadMesh(const lwResFileMesh* info) override;
	LW_RESULT LoadMtlTex(DWORD mtl_id, lwMtlTexInfo* info, const char* tex_path) override;
	LW_RESULT LoadAnimData(lwIAnimDataInfo* info, const char* tex_path, const lwResFile* res) override;
	LW_RESULT LoadRenderCtrl(const lwRenderCtrlCreateInfo* rcci) override;

	LW_RESULT ExtractGeomObjInfo(lwIGeomObjInfo* info) override;

	virtual LW_RESULT Update() override;
	virtual LW_RESULT Render() override;
	virtual LW_RESULT RenderSubset(DWORD subset) override;

	LW_RESULT Clone(lwIPrimitive** ret_obj) override;

	lwIResourceMgr* GetResourceMgr() override { return _res_mgr; }
	lwIMtlTexAgent* GetMtlTexAgent(DWORD id) override { return _mtltex_agent_seq[id]; }
	lwIMeshAgent* GetMeshAgent() override { return _mesh_agent; }
	lwIAnimCtrlAgent* GetAnimAgent() override { return _anim_agent; }
	lwIRenderCtrlAgent* GetRenderCtrlAgent() override { return _render_agent; }
	lwIHelperObject* GetHelperObject() override { return _helper_object; }
	LW_RESULT GetSubsetNum(DWORD* subset_num) override;

	void SetMeshAgent(lwIMeshAgent* agent) override { _mesh_agent = agent; }
	void SetMtlTexAgent(DWORD subset, lwIMtlTexAgent* agent) override { _mtltex_agent_seq[subset] = agent; }
	void SetAnimCtrlAgent(lwIAnimCtrlAgent* agent) override { _anim_agent = agent; }
	void SetRenderCtrl(lwIRenderCtrlAgent* obj) override { _render_agent = obj; }
	void SetHelperObject(lwIHelperObject* obj) override { _helper_object = obj; }
	void SetMaterial(const lwMaterial* mtl) override;
	void SetOpacity(float opacity) override;

	// object state method
	void SetState(const lwStateCtrl* ctrl) override { _state_ctrl = *ctrl; }
	void SetState(DWORD state, BYTE value) override {
		_state_ctrl.SetState(state, value);
	}
	BYTE GetState(DWORD state) const override {
		return _state_ctrl.GetState(state);
	}

	LW_RESULT SetTextureLOD(DWORD level) override;
	LW_RESULT PlayDefaultAnimation() override;
	LW_RESULT ResetTexture(DWORD subset, DWORD stage, const char* file, const char* tex_path) override;

	void setPixelShader(const std::string& filename) override {
		_render_agent->setPixelShader(filename);
	}
};

LW_RESULT lwPrimitivePlayDefaultAnimation(lwIPrimitive* obj);

LW_END