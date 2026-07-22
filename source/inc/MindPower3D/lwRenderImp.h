#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwClassDecl.h"
#include "lwErrorCode.h"
#include "lwDirectX.h"
#include "lwITypes.h"
#include "lwInterfaceExt.h"
#include "lwShaderMgr.h"

LW_BEGIN

// remarks
// 1。静态shader定义和声明
//    物件初始化的时候对decl和shader赋值，中间不作光照，动画的切换
// 2。动态shader定义和声明
//    物件初始化的时候对decl赋值，渲染时候根据动画类型，光照类型在
//    lwShaderDeclMgr中做动态匹配查找（查找算法对效率有要求）
// 3。自定义Shader定义和声明（user-defined shader）

class lwRenderCtrlAgent : public lwIRenderCtrlAgent {
	LW_STD_DECLARATION()

private:
	lwMatrix44 _mat_local;
	lwMatrix44 _mat_parent;
	lwMatrix44 _mat_global;

	lwIResourceMgr* _res_mgr;
	lwIMeshAgent* _mesh_agent;
	lwIMtlTexAgent* _mtltex_agent;
	lwIAnimCtrlAgent* _anim_agent;
	lwIRenderCtrlVS* _render_ctrl;

	DWORD _decl_type;
	DWORD _vs_type;
	DWORD _ps_type;

public:
	void setPixelShader(const std::string& filename) override {
		_render_ctrl->setPixelShader(filename);
	}

public:
	lwRenderCtrlAgent(lwIResourceMgr* res_mgr);
	~lwRenderCtrlAgent();

	lwIResourceMgr* GetResourceMgr() override {
		return _res_mgr;
	}

	lwIMeshAgent* GetMeshAgent() override {
		return _mesh_agent;
	}

	lwIMtlTexAgent* GetMtlTexAgent() override {
		return _mtltex_agent;
	}

	lwIAnimCtrlAgent* GetAnimCtrlAgent() override {
		return _anim_agent;
	}

	lwIRenderCtrlVS* GetRenderCtrlVS() override {
		return _render_ctrl;
	}

	void SetMatrix(const lwMatrix44* mat) override {
		_mat_global = *mat;
	}

	void SetLocalMatrix(const lwMatrix44* mat_local) override;
	void SetParentMatrix(const lwMatrix44* mat_parent) override;
	lwMatrix44* GetLocalMatrix() override {
		return &_mat_local;
	}

	lwMatrix44* GetParentMatrix() override {
		return &_mat_parent;
	}

	lwMatrix44* GetGlobalMatrix() override {
		return &_mat_global;
	}

	void BindMeshAgent(lwIMeshAgent* agent) override {
		_mesh_agent = agent;
	}

	void BindMtlTexAgent(lwIMtlTexAgent* agent) override {
		_mtltex_agent = agent;
	}

	void BindAnimCtrlAgent(lwIAnimCtrlAgent* agent) override {
		_anim_agent = agent;
	}

	LW_RESULT SetRenderCtrl(DWORD ctrl_type) override;
	void SetRenderCtrl(lwIRenderCtrlVS* ctrl) {
		_render_ctrl = ctrl;
	}

	void SetVertexShader(DWORD type) override {
		_vs_type = type;
	}

	void SetVertexDeclaration(DWORD type) override {
		_decl_type = type;
	}

	DWORD GetVertexShader() const override {
		return _vs_type;
	}
	DWORD GetVertexDeclaration() const override {
		return _decl_type;
	}

	virtual LW_RESULT Clone(lwIRenderCtrlAgent** ret_obj) override;
	virtual LW_RESULT BeginSet() override;
	virtual LW_RESULT EndSet() override;
	virtual LW_RESULT BeginSetSubset(DWORD subset) override;
	virtual LW_RESULT EndSetSubset(DWORD subset) override;
	virtual LW_RESULT DrawSubset(DWORD subset) override;
};

LW_END