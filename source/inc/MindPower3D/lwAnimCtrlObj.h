//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwErrorCode.h"
#include "lwInterfaceExt.h"

//////////////
//
LW_BEGIN

//

class lwAnimCtrlObjMat : public lwIAnimCtrlObjMat {
	enum {
		THIS_TYPE = ANIM_CTRL_TYPE_MAT
	};

	LW_STD_DECLARATION();

private:
	lwIResourceMgr* _res_mgr;
	lwIAnimCtrlMatrix* _anim_ctrl;
	lwPlayPoseInfo _ppi;
	lwAnimCtrlObjTypeInfo _type_info;

	lwMatrix44 _rtm;

public:
	lwAnimCtrlObjMat(lwIResourceMgr* res_mgr);
	~lwAnimCtrlObjMat();

	LW_RESULT Clone(lwIAnimCtrlObjMat** ret_obj) override;
	lwIAnimCtrlMatrix* AttachAnimCtrl(lwIAnimCtrlMatrix* ctrl) override;
	lwIAnimCtrlMatrix* DetachAnimCtrl() override;
	lwIAnimCtrl* GetAnimCtrl() override { return _anim_ctrl; }
	lwPlayPoseInfo* GetPlayPoseInfo() override { return &_ppi; }
	LW_RESULT SetTypeInfo(lwAnimCtrlObjTypeInfo* info) override;
	LW_RESULT GetTypeInfo(lwAnimCtrlObjTypeInfo* info) override;
	LW_RESULT PlayPose(const lwPlayPoseInfo* info) override;
	BOOL IsPlaying() override { return !(_anim_ctrl == nullptr || _ppi.type == PLAY_INVALID); }

	LW_RESULT UpdateAnimCtrl() override;
	LW_RESULT UpdateObject() override;

	LW_RESULT GetRTM(lwMatrix44* mat) override;
};

class lwAnimCtrlObjBone : public lwIAnimCtrlObjBone {
	enum {
		THIS_TYPE = ANIM_CTRL_TYPE_BONE
	};

	LW_STD_DECLARATION();

private:
	lwIResourceMgr* _res_mgr;
	lwIAnimCtrlBone* _anim_ctrl;
	lwPlayPoseInfo _ppi;
	lwAnimCtrlObjTypeInfo _type_info;

	lwIndexMatrix44* _dummy_rtm_seq;
	DWORD _dummy_rtm_num;
	lwMatrix44* _bone_rtm_seq;
	DWORD _bone_rtm_num;

public:
	lwAnimCtrlObjBone(lwIResourceMgr* res_mgr);
	~lwAnimCtrlObjBone();

	// base method
	LW_RESULT Clone(lwIAnimCtrlObjBone** ret_obj) override;
	lwIAnimCtrlBone* AttachAnimCtrl(lwIAnimCtrlBone* ctrl) override;
	lwIAnimCtrlBone* DetachAnimCtrl() override;
	lwIAnimCtrl* GetAnimCtrl() override { return _anim_ctrl; }
	lwPlayPoseInfo* GetPlayPoseInfo() override { return &_ppi; }
	void SetPlayPoseInfo(const lwPlayPoseInfo* ppi) override { _ppi = *ppi; }
	LW_RESULT SetTypeInfo(lwAnimCtrlObjTypeInfo* info) override;
	LW_RESULT GetTypeInfo(lwAnimCtrlObjTypeInfo* info) override;
	LW_RESULT PlayPose(const lwPlayPoseInfo* info) override;
	BOOL IsPlaying() override { return !(_anim_ctrl == nullptr || _ppi.type == PLAY_INVALID); }

	LW_RESULT UpdateAnimCtrl() override;
	LW_RESULT UpdateObject(lwIAnimCtrlObjBone* ctrl_obj, lwIMesh* mesh_obj) override;
	LW_RESULT UpdateHelperObject(lwIHelperObject* helper_obj) override;

	lwMatrix44* GetBoneRTMSeq() override {
		return _bone_rtm_seq;
	}
	lwMatrix44* GetDummyRTM(DWORD id) override;
	lwIndexMatrix44* GetDummyRTMSeq() override { return _dummy_rtm_seq; }
	DWORD GetBoneRTTMNum() override { return _bone_rtm_num; }
	DWORD GetDummyRTMNum() override { return _dummy_rtm_num; }
};

class lwAnimCtrlObjTexUV : public lwIAnimCtrlObjTexUV {
	enum {
		THIS_TYPE = ANIM_CTRL_TYPE_TEXUV
	};

	LW_STD_DECLARATION();

private:
	lwIResourceMgr* _res_mgr;
	lwIAnimCtrlTexUV* _anim_ctrl;
	lwPlayPoseInfo _ppi;
	lwAnimCtrlObjTypeInfo _type_info;

	lwMatrix44 _rtm;

public:
	lwAnimCtrlObjTexUV(lwIResourceMgr* res_mgr);
	~lwAnimCtrlObjTexUV();

	LW_RESULT Clone(lwIAnimCtrlObjTexUV** ret_obj) override;
	lwIAnimCtrlTexUV* AttachAnimCtrl(lwIAnimCtrlTexUV* ctrl) override;
	lwIAnimCtrlTexUV* DetachAnimCtrl() override;
	lwIAnimCtrl* GetAnimCtrl() override { return _anim_ctrl; }
	lwPlayPoseInfo* GetPlayPoseInfo() override { return &_ppi; }
	LW_RESULT SetTypeInfo(lwAnimCtrlObjTypeInfo* info) override;
	LW_RESULT GetTypeInfo(lwAnimCtrlObjTypeInfo* info) override;
	LW_RESULT PlayPose(const lwPlayPoseInfo* info) override;
	BOOL IsPlaying() override { return !(_anim_ctrl == nullptr || _ppi.type == PLAY_INVALID); }

	LW_RESULT UpdateAnimCtrl() override;
	LW_RESULT UpdateObject() override;

	LW_RESULT GetRTM(lwMatrix44* mat) override;
};

class lwAnimCtrlObjTexImg : public lwIAnimCtrlObjTexImg {
	enum {
		THIS_TYPE = ANIM_CTRL_TYPE_TEXIMG
	};

	LW_STD_DECLARATION();

private:
	lwIResourceMgr* _res_mgr;
	lwIAnimCtrlTexImg* _anim_ctrl;
	lwPlayPoseInfo _ppi;
	lwAnimCtrlObjTypeInfo _type_info;

	lwITex* _rt_tex;

public:
	lwAnimCtrlObjTexImg(lwIResourceMgr* res_mgr);
	~lwAnimCtrlObjTexImg();

	LW_RESULT Clone(lwIAnimCtrlObjTexImg** ret_obj) override;
	lwIAnimCtrlTexImg* AttachAnimCtrl(lwIAnimCtrlTexImg* ctrl) override;
	lwIAnimCtrlTexImg* DetachAnimCtrl() override;
	lwIAnimCtrl* GetAnimCtrl() override { return _anim_ctrl; }
	lwPlayPoseInfo* GetPlayPoseInfo() override { return &_ppi; }
	LW_RESULT SetTypeInfo(lwAnimCtrlObjTypeInfo* info) override;
	LW_RESULT GetTypeInfo(lwAnimCtrlObjTypeInfo* info) override;
	LW_RESULT PlayPose(const lwPlayPoseInfo* info) override;
	BOOL IsPlaying() override { return !(_anim_ctrl == nullptr || _ppi.type == PLAY_INVALID); }

	LW_RESULT UpdateAnimCtrl() override;
	LW_RESULT UpdateObject() override;

	LW_RESULT GetRunTimeTex(lwITex** tex) override;
};

class lwAnimCtrlObjMtlOpacity : public lwIAnimCtrlObjMtlOpacity {
	enum {
		THIS_TYPE = ANIM_CTRL_TYPE_MTLOPACITY
	};

	LW_STD_DECLARATION();

private:
	lwIResourceMgr* _res_mgr;
	lwIAnimCtrlMtlOpacity* _anim_ctrl;
	lwPlayPoseInfo _ppi;
	lwAnimCtrlObjTypeInfo _type_info;

	float _rt_opacity;

public:
	lwAnimCtrlObjMtlOpacity(lwIResourceMgr* res_mgr);
	~lwAnimCtrlObjMtlOpacity();

	LW_RESULT Clone(lwIAnimCtrlObjMtlOpacity** ret_obj) override;
	lwIAnimCtrlMtlOpacity* AttachAnimCtrl(lwIAnimCtrlMtlOpacity* ctrl) override;
	lwIAnimCtrlMtlOpacity* DetachAnimCtrl() override;
	lwIAnimCtrl* GetAnimCtrl() override { return _anim_ctrl; }
	lwPlayPoseInfo* GetPlayPoseInfo() override { return &_ppi; }
	LW_RESULT SetTypeInfo(lwAnimCtrlObjTypeInfo* info) override;
	LW_RESULT GetTypeInfo(lwAnimCtrlObjTypeInfo* info) override;
	LW_RESULT PlayPose(const lwPlayPoseInfo* info) override;
	BOOL IsPlaying() override { return !(_anim_ctrl == nullptr || _ppi.type == PLAY_INVALID); }

	LW_RESULT UpdateAnimCtrl() override;
	LW_RESULT UpdateObject() override;

	LW_RESULT GetRunTimeOpacity(float* opacity) override;
};

// lwAnimCtrlAgent
class lwAnimCtrlAgent : public lwIAnimCtrlAgent {
	LW_STD_DECLARATION()

private:
	lwIResourceMgr* _res_mgr;
	lwIAnimCtrlObj** _obj_seq;
	DWORD _obj_num;

private:
public:
	lwAnimCtrlAgent(lwIResourceMgr* res_mgr);
	~lwAnimCtrlAgent();

	LW_RESULT AddAnimCtrlObj(lwIAnimCtrlObj* obj) override;
	lwIAnimCtrlObj* RemoveAnimCtrlObj(lwAnimCtrlObjTypeInfo* info) override;
	lwIAnimCtrlObj* GetAnimCtrlObj(lwAnimCtrlObjTypeInfo* info) override;
	lwIAnimCtrlObj* GetAnimCtrlObj(DWORD id) override { return _obj_seq[id]; }
	DWORD GetAnimCtrlObjNum() override { return _obj_num; }

	LW_RESULT Update() override;

	LW_RESULT Clone(lwIAnimCtrlAgent** ret_obj) override;
	LW_RESULT ExtractAnimData(lwIAnimDataInfo* data_info) override;
};

LW_END