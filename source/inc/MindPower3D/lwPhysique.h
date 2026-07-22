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

struct lwPhysiqueBoneInfo {
	lwPhysique* p;
	std::string str;
	lwIAnimCtrlObjBone* bc;
	lwIAnimCtrlBone* cb;
	lwIAnimDataBone* data;
	lwAnimCtrlObjTypeInfo tp;
	lwResFileAnimData res;
};

struct lwPhysiquePriInfo {
	lwPhysique* p;
	DWORD part_id;
	std::string str;
};

class lwGeomManager {
public:
	lwGeomManager();
	~lwGeomManager();

	lwGeomObjInfo* GetGeomObjInfo(const char file[]);
	bool LoadGeomobj(const char file[]);

	lwIAnimDataBone* GetBoneData(const char file[]);
	bool LoadBoneData(const char file[]);

private:
	using GEOMOBJ_MAP = std::map<std::string, lwGeomObjInfo*>;
	GEOMOBJ_MAP m_GeomobjMap;

	using ANIMDATA_MAP = std::map<std::string, lwIAnimDataBone*>;
	ANIMDATA_MAP m_AnimDataMap;
};

extern lwGeomManager g_GeomManager;

class lwPhysique : public lwIPhysique, public lwLinkCtrl {
private:
	DWORD _id;
	lwIResourceMgr* _res_mgr;
	lwISceneMgr* _scene_mgr;
	lwIPrimitive* _obj_seq[LW_MAX_SUBSKIN_NUM];
	lwIAnimCtrlAgent* _anim_agent;

	lwStateCtrl _state_ctrl;
	char _file_name[LW_MAX_NAME];
	lwMatrix44 _mat_base;
	float _opacity;
	int volatile _count;
	bool _start;
	bool _end;

	LW_STD_DECLARATION();

public:
	lwPhysique(lwIResourceMgr* res_mgr);
	lwPhysique();
	~lwPhysique();

	// link ctrl method
	virtual LW_RESULT GetLinkCtrlMatrix(lwMatrix44* mat, DWORD link_id) override;

	lwIResourceMgr* GetResourceMgr() override { return _res_mgr; }

	LW_RESULT Destroy() override;
	LW_RESULT LoadBoneCatch(lwPhysiqueBoneInfo& info);
	LW_RESULT LoadBone(const char* file) override;
	LW_RESULT LoadPrimitive(DWORD part_id, lwIGeomObjInfo* geom_info) override;
	LW_RESULT LoadPriCatch(const lwPhysiquePriInfo& info);
	LW_RESULT LoadPrimitive(DWORD part_id, const char* file) override;
	LW_RESULT DestroyPrimitive(DWORD part_id) override;
	LW_RESULT CheckPrimitive(DWORD part_id) override { return _obj_seq[part_id] ? LW_RET_OK : LW_RET_FAILED; }

	LW_RESULT Update() override;
	LW_RESULT Render() override;

	lwMatrix44* GetMatrix() override { return &_mat_base; }
	void SetMatrix(const lwMatrix44* mat) override { _mat_base = *mat; }

	void SetOpacity(float opacity) override;

	LW_RESULT PlayPose(const lwPlayPoseInfo* info);
	LW_RESULT PlayObjImpPose(const lwPlayPoseInfo* info, DWORD obj_id, DWORD ctrl_type);

	lwIPoseCtrl* GetObjImpPoseCtrl(DWORD skin_id, DWORD ctrl_type);

	LW_RESULT HitTestPrimitive(lwPickInfo* info, const lwVector3* org, const lwVector3* ray) override;
	;
	LW_RESULT HitTestPhysique(lwPickInfo* info, const lwVector3* org, const lwVector3* ray) override;

	void ShowHelperObject(int show) override;
	void ShowBoundingObjectPhysique(int show) override;

	LW_RESULT SetItemLink(const lwItemLinkInfo* info) override;
	LW_RESULT ClearItemLink(lwIItem* obj) override;

	void SetObjState(DWORD state, BYTE value) override { return _state_ctrl.SetState(state, value); }
	DWORD GetObjState(DWORD state) const override { return _state_ctrl.GetState(state); }

	lwIPrimitive* GetPrimitive(DWORD id) override { return _obj_seq[id]; }
	lwIAnimCtrlAgent* GetAnimCtrlAgent() override { return _anim_agent; }

	LW_RESULT RegisterSceneMgr(lwISceneMgr* scene_mgr) override {
		_scene_mgr = scene_mgr;
		return LW_RET_OK;
	}
	LW_RESULT SetTextureLOD(DWORD level) override;
	float GetOpacity() override { return _opacity; }

	virtual void Start() override { _start = true; }
	virtual void End() override { _end = true; }
	virtual bool isLoaded() override { return _end && _count == 0; }
	void incCount() { _count++; }
	void decCount() { _count--; }

	void setComponentColour(size_t index, D3DCOLOR colour, const std::string& filterTextureName) override {
		mIndexColourFilterList[index] = ColourFilterPair(colour, filterTextureName);
	}

	void setTextureOperation(size_t index, D3DTEXTUREOP operation) override {
		mIndexTextureOPList[index] = operation;
	}

	const char* getTextureOperationDescription(size_t operation) override {
		return _res_mgr->getTextureOperationDescription(operation);
	}

	void setPixelShader(size_t index, const std::string& filename) override {
		if (_obj_seq) {
			if (_obj_seq[index]) {
				_obj_seq[index]->setPixelShader(filename);
			}
		}
	}

private:
	using ColourFilterPair = std::pair<D3DCOLOR, std::string>;
	using IndexColourFilterPairList = std::map<size_t, ColourFilterPair>;
	IndexColourFilterPairList mIndexColourFilterList;

	using IndexTextureOPPairList = std::map<size_t, D3DTEXTUREOP>;
	IndexTextureOPPairList mIndexTextureOPList;
};

LW_END