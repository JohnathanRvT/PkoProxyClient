//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwMath.h"
#include "lwExpObj.h"
#include "lwITypes.h"
#include "lwMisc.h"
#include "lwInterfaceExt.h"

//////////////
//
LW_BEGIN

class lwNodeBase {
protected:
	// base
	DWORD _type;
	DWORD _id;
	char _descriptor[64];
	lwMatrix44 _mat_local;
	lwMatrix44 _mat_world;
	lwByteSet _state_set;

	// link state
	lwINode* _parent;
	DWORD _link_parent_id;
	DWORD _link_id;

protected:
	//LW_RESULT _UpdateMatrix();
	BOOL _CheckVisibleState() { return _state_set.GetValue(STATE_VISIBLE); }

public:
	lwNodeBase();
	//~lwNodeBase();
};

class lwNodePrimitive : public lwNodeBase, public lwINodePrimitive {
	LW_STD_DECLARATION()

private:
	lwIResourceMgr* _res_mgr;
	lwIMeshAgent* _mesh_agent;
	lwIMtlTexAgent** _mtltex_agent_seq;
	lwIAnimCtrlAgent* _anim_agent;
	lwIRenderCtrlAgent* _render_agent;
	lwIHelperObject* _helper_object;
	//lwINodeBoneCtrl* _vertex_blend_ctrl;
	lwIAnimCtrlObjBone* _ref_ctrl_obj_bone;

	DWORD _mtltex_agent_seqsize;

private:
	LW_RESULT _DestroyMtlTexAgent();
	LW_RESULT _UpdateBoundingObject(lwIAnimCtrlObjBone* ctrl_obj);
	LW_RESULT _UpdateTransparentState();

public:
	lwNodePrimitive(lwIResourceMgr* res_mgr);
	virtual ~lwNodePrimitive();

	// base method
	void SetID(DWORD id) override { _id = id; }
	void SetLinkID(DWORD id) override { _link_id = id; }
	void SetParentLinkID(DWORD id) override { _link_parent_id = id; }
	LW_RESULT SetParent(lwINode* parent) override;
	void SetLocalMatrix(const lwMatrix44* mat) override { _mat_local = *mat; }
	void SetDescriptor(const char* str64) override { _tcsncpy_s(_descriptor, str64, _TRUNCATE); }
	DWORD GetType() const override { return _type; }
	DWORD GetID() const override { return _id; }
	lwMatrix44* GetLocalMatrix() override { return &_mat_local; }
	lwMatrix44* GetWorldMatrix() override { return &_mat_world; }
	char* GetDescriptor() override { return _descriptor; }
	lwINode* GetParent() const override { return _parent; }
	DWORD GetLinkID() const override { return _link_id; }
	DWORD GetParentLinkID() const override { return _link_parent_id; }
	lwIByteSet* GetStateSet() override { return &_state_set; }
	LW_RESULT GetLinkMatrix(lwMatrix44* mat, DWORD link_id) override;

	// primitive method
	void SetMeshAgent(lwIMeshAgent* agent) override { _mesh_agent = agent; }
	void SetMtlTexAgent(DWORD subset, lwIMtlTexAgent* agent) override { _mtltex_agent_seq[subset] = agent; }
	void SetAnimCtrlAgent(lwIAnimCtrlAgent* agent) override { _anim_agent = agent; }
	void SetRenderCtrl(lwIRenderCtrlAgent* obj) override { _render_agent = obj; }
	void SetHelperObject(lwIHelperObject* obj) override { _helper_object = obj; }
	void SetMaterial(const lwMaterial* mtl) override;
	void SetOpacity(float opacity) override;
	//LW_RESULT SetVertexBlendCtrl(lwINodeBoneCtrl* ctrl);

	lwIResourceMgr* GetResourceMgr() override { return _res_mgr; }
	lwIMtlTexAgent* GetMtlTexAgent(DWORD id) override { return _mtltex_agent_seq[id]; }
	lwIMeshAgent* GetMeshAgent() override { return _mesh_agent; }
	lwIAnimCtrlAgent* GetAnimCtrlAgent() override { return _anim_agent; }
	lwIRenderCtrlAgent* GetRenderCtrlAgent() override { return _render_agent; }
	lwIHelperObject* GetHelperObject() override { return _helper_object; }
	//lwINodeBoneCtrl* GetVertexBlendCtrl() { return _vertex_blend_ctrl; }
	LW_RESULT AllocateMtlTexAgentSeq(DWORD num) override;
	DWORD GetMtlTexAgentSeqSize() const override { return _mtltex_agent_seqsize; }
	LW_RESULT GetSubsetNum(DWORD* subset_num) override;

	LW_RESULT Destroy() override;
	LW_RESULT Load(lwIGeomObjInfo* info, const char* tex_path, const lwResFile* res) override;
	LW_RESULT LoadMesh(lwMeshInfo* info) override;
	LW_RESULT LoadMtlTex(DWORD mtl_id, lwMtlTexInfo* info, const char* tex_path) override;
	LW_RESULT LoadAnimData(lwIAnimDataInfo* info, const char* tex_path, const lwResFile* res) override;
	LW_RESULT LoadRenderCtrl(const lwRenderCtrlCreateInfo* rcci) override;
	LW_RESULT Copy(lwINodePrimitive* src_obj) override;

	LW_RESULT Update() override;
	LW_RESULT Render() override;
	LW_RESULT RenderSubset(DWORD subset) override;
	LW_RESULT RenderHelperObject() override;

	LW_RESULT HitTest(lwPickInfo* info, const lwVector3* org, const lwVector3* ray) override;
};

class lwNodeBoneCtrl : public lwNodeBase, public lwINodeBoneCtrl {
	LW_STD_DECLARATION()
private:
	lwIResourceMgr* _res_mgr;
	lwIAnimCtrlObjBone* _ctrl_obj;

public:
	lwNodeBoneCtrl(lwIResourceMgr* res_mgr);
	~lwNodeBoneCtrl();

	// base method
	void SetID(DWORD id) override { _id = id; }
	void SetLinkID(DWORD id) override { _link_id = id; }
	void SetParentLinkID(DWORD id) override { _link_parent_id = id; }
	LW_RESULT SetParent(lwINode* parent) override {
		_parent = parent;
		return LW_RET_OK;
	}
	void SetLocalMatrix(const lwMatrix44* mat) override { _mat_local = *mat; }
	void SetDescriptor(const char* str64) override { _tcsncpy_s(_descriptor, str64, _TRUNCATE); }
	void SetAnimCtrlObj(lwIAnimCtrlObjBone* ctrl_obj) override { _ctrl_obj = ctrl_obj; }
	DWORD GetType() const override { return _type; }
	DWORD GetID() const override { return _id; }
	lwMatrix44* GetLocalMatrix() override { return &_mat_local; }
	lwMatrix44* GetWorldMatrix() override { return &_mat_world; }
	char* GetDescriptor() override { return _descriptor; }
	lwINode* GetParent() const override { return _parent; }
	DWORD GetLinkID() const override { return _link_id; }
	DWORD GetParentLinkID() const override { return _link_parent_id; }
	lwIByteSet* GetStateSet() override { return &_state_set; }
	lwIAnimCtrlObjBone* GetAnimCtrlObj() override { return _ctrl_obj; }
	LW_RESULT GetLinkMatrix(lwMatrix44* mat, DWORD link_id) override;
	LW_RESULT Update() override;
	LW_RESULT Render() override { return LW_RET_OK; }

	LW_RESULT Load(lwIAnimDataBone* data) override;

	LW_RESULT PlayPose(const lwPlayPoseInfo* info) override;
	lwIPoseCtrl* GetPoseCtrl() override;
	lwPlayPoseInfo* GetPlayPoseInfo() override;
};

class lwNodeDummy : public lwNodeBase, public lwINodeDummy {
	LW_STD_DECLARATION()
private:
	lwIResourceMgr* _res_mgr;
	lwIAnimCtrlObjMat* _ctrl_obj;
	lwINodePrimitive* _ass_obj;

public:
	lwNodeDummy(lwIResourceMgr* res_mgr);
	~lwNodeDummy();

	// base method
	void SetID(DWORD id) override { _id = id; }
	void SetLinkID(DWORD id) override { _link_id = id; }
	void SetParentLinkID(DWORD id) override { _link_parent_id = id; }
	LW_RESULT SetParent(lwINode* parent) override {
		_parent = parent;
		return LW_RET_OK;
	}
	void SetLocalMatrix(const lwMatrix44* mat) override { _mat_local = *mat; }
	void SetDescriptor(const char* str64) override { _tcsncpy_s(_descriptor, str64, _TRUNCATE); }
	void SetAnimCtrlObj(lwIAnimCtrlObjMat* ctrl_obj) override { _ctrl_obj = ctrl_obj; }
	DWORD GetType() const override { return _type; }
	DWORD GetID() const override { return _id; }
	lwMatrix44* GetLocalMatrix() override { return &_mat_local; }
	lwMatrix44* GetWorldMatrix() override { return &_mat_world; }
	char* GetDescriptor() override { return _descriptor; }
	lwINode* GetParent() const override { return _parent; }
	DWORD GetLinkID() const override { return _link_id; }
	DWORD GetParentLinkID() const override { return _link_parent_id; }
	lwIByteSet* GetStateSet() override { return &_state_set; }
	LW_RESULT GetLinkMatrix(lwMatrix44* mat, DWORD link_id) override;
	lwIAnimCtrlObjMat* GetAnimCtrlObj() override { return _ctrl_obj; }

	LW_RESULT Update() override;
	LW_RESULT Render() override;

	LW_RESULT Load(lwIHelperDummyObjInfo* data) override;
	LW_RESULT CreateAssistantObject(const lwVector3* size, DWORD color) override;
	lwINodePrimitive* GetAssistantObject() override { return _ass_obj; }
};

class lwNodeHelper : public lwNodeBase, public lwINodeHelper {
	LW_STD_DECLARATION()

private:
	lwIResourceMgr* _res_mgr;

	lwIHelperDummy* _obj_dummy;
	lwIHelperBox* _obj_box;
	lwIHelperMesh* _obj_mesh;

public:
	lwNodeHelper(lwIResourceMgr* mgr);
	~lwNodeHelper();

	// base method
	void SetID(DWORD id) override { _id = id; }
	void SetLinkID(DWORD id) override { _link_id = id; }
	void SetParentLinkID(DWORD id) override { _link_parent_id = id; }
	LW_RESULT SetParent(lwINode* parent) override {
		_parent = parent;
		return LW_RET_OK;
	}
	void SetLocalMatrix(const lwMatrix44* mat) override { _mat_local = *mat; }
	void SetDescriptor(const char* str64) override { _tcsncpy_s(_descriptor, str64, _TRUNCATE); }
	DWORD GetType() const override { return _type; }
	DWORD GetID() const override { return _id; }
	lwMatrix44* GetLocalMatrix() override { return &_mat_local; }
	lwMatrix44* GetWorldMatrix() override { return &_mat_world; }
	char* GetDescriptor() override { return _descriptor; }
	lwINode* GetParent() const override { return _parent; }
	DWORD GetLinkID() const override { return _link_id; }
	DWORD GetParentLinkID() const override { return _link_parent_id; }
	lwIByteSet* GetStateSet() override { return &_state_set; }
	LW_RESULT GetLinkMatrix(lwMatrix44* mat, DWORD link_id) override;
	LW_RESULT Update() override;
	LW_RESULT Render() override;

	// helper object method
	LW_RESULT LoadHelperInfo(const lwHelperInfo* info, int create_instance_flag);
	LW_RESULT Copy(const lwINodeHelper* src);
	LW_RESULT Destroy();

	lwIHelperDummy* GetHelperDummy() { return _obj_dummy; }
	lwIHelperBox* GetHelperBox() { return _obj_box; }
	lwIHelperMesh* GetHelperMesh() { return _obj_mesh; }
};

class lwNodeObject : public lwINodeObject {
	LW_STD_DECLARATION()

private:
	lwIResourceMgr* _res_mgr;
	lwITreeNode* _obj_root;

	char _name[LW_MAX_NAME];

public:
	lwNodeObject(lwIResourceMgr* res_mgr);
	virtual ~lwNodeObject();

	LW_RESULT Update() override;
	LW_RESULT Render() override;
	LW_RESULT IgnoreNodesRender(const IgnoreStruct* is) override;
	LW_RESULT Destroy() override;
	LW_RESULT CullPrimitive() override;
	void SetName(const char* name) override { _tcsncpy_s(_name, name, _TRUNCATE); }
	void SetMatrix(const lwMatrix44* mat) override;
	char* GetName() override { return _name; }
	lwMatrix44* GetMatrix() override;
	lwIByteSet* GetStateSet() override;
	lwITreeNode* GetTreeNodeRoot() override { return _obj_root; }
	LW_RESULT QueryTreeNode(lwModelNodeQueryInfo* info) override;
	LW_RESULT InsertTreeNode(lwITreeNode* parent_node, lwITreeNode* prev_node, lwITreeNode* node) override;
	LW_RESULT RemoveTreeNode(lwITreeNode* node) override;
	LW_RESULT Load(const char* file, DWORD flag, lwITreeNode* parent_node) override;
};

// assistant method
LW_RESULT lwDestroyNodeObject(lwITreeNode* node);

// if there is not available animation ctrl then the return value is LW_RET_OK_1
LW_RESULT lwNodePrimitive_PlayPose(lwINodePrimitive* obj, const lwPlayPoseInfo* info, DWORD ctrl_type, DWORD subset, DWORD stage);
LW_RESULT lwNodePrimitive_PlayPoseAll(lwINodePrimitive* obj, const lwPlayPoseInfo* info);
LW_RESULT lwNodeBoneCtrl_PlayPose(lwINodeBoneCtrl* obj, const lwPlayPoseInfo* info);
LW_RESULT lwNodeDummy_PlayPose(lwINodeDummy* obj, const lwPlayPoseInfo* info);
LW_RESULT lwNodeObject_PlayDefaultPose(lwINodeObject* obj);
LW_RESULT lwNode_ShowBoundingObject(lwINode* obj, DWORD flag);
LW_RESULT lwNodeObject_ShowBoundingObject(lwINodeObject* obj, DWORD flag);
LW_RESULT lwNodeObject_DumpObjectTree(lwINodeObject* obj, const char* file);
LW_RESULT lwNodeObject_GetPrimitiveCullingNum(lwINodeObject* obj, DWORD* num);

LW_END