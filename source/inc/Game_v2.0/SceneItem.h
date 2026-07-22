#pragma once

#include "SceneNode.h"
#include "MindPower.h"
#include "ItemRecord.h"

class CGameScene;
class CArcTrack;
class CEvent;
class CSceneHeight;
class lwItemLit;
class CEffectObj;
class CCharacterModel;

extern BOOL InitItemLit(const char* file);
BOOL ClearItemLit();

class CSceneItem : public MPSceneItem, public CSceneNode {
private:
	virtual BOOL _Create(int nObjTypeID, int nType) override;

public:
	CSceneItem();
	virtual ~CSceneItem();

	virtual void FrameMove(DWORD dwTimeParam) override;
	virtual void Render() override;

	void RenderUI();

	void setAttachedCharacterID(DWORD id) { _nCharacterID = id; }
	DWORD getAttachedCharacterID() const { return _nCharacterID; }

	int Load(const char* file);
	HRESULT LitResetTexture(DWORD item_id, DWORD lit_id);
	HRESULT LitUnresetTexture();

	void SetForgeEffect(DWORD value, int nCharID = 1); // 设置精炼效果表现,本函数在创建道具成功后调用

public:
	void setIsSystem(bool v) { _IsSystem = v; }
	bool IsHitText(int x, int y);
	void SetHide(BOOL bHide);

	CItemRecord* GetItemInfo() { return _pItemInfo; }
	int GetItemDrapID();

	void setIsShowName(bool v);
	bool getIsShowName() { return _IsShowName; }

	void setAlpla(float alpha = 0.0f);

	void setEvent(CEvent* pEvent) override { _pEvent = pEvent; }
	CEvent* getEvent() { return _pEvent; }

	bool IsPick();

	bool GetItemHeight(float* out_height);
	//lemon add@2005.1.5 for 爆料
	void PlayArcAni(D3DXVECTOR3 vStart, D3DXVECTOR3 vEnd, float fVel = 0.01f, float fHei = 3.0f);

protected:
	virtual void _UpdateYaw() override;
	virtual void _UpdatePitch() override;
	virtual void _UpdateRoll() override;
	virtual void _UpdatePos() override;
	virtual void _UpdateHeight() override;
	virtual void _UpdateValid(BOOL bValid) override;

	bool GetRunTimeMatrix(MPMatrix44* mat, DWORD dummy_id) override {
		return GetObjDummyRunTimeMatrix(mat, dummy_id) == 0;
	}

	float _fTerrainHeight{0.0f};

	DWORD _nCharacterID{-1UL};

protected:
	bool _IsSystem{false}; // 道具用于系统，不可操作
	bool _IsShowName{false};
	CItemRecord* _pItemInfo{nullptr};
	int _nDrapID;
	CEvent* _pEvent{nullptr};

	bool _IsAlpha{false};

	int _nNameW, _nNameH;

	std::unique_ptr<CArcTrack> _pArcTrack;
	std::unique_ptr<CSceneHeight> _pSceneHeight;

	DWORD _dwForgeValue{0};

	// added by clp
public:
	CEffectObj* bindEffect(int dummyID, int effectID, bool isLoop, int angle = -1);
	void unbindEffect();
	void setParentCharacter(CCharacterModel* character) {
		mParentCharacter = character;
	}
	CCharacterModel* getParentCharacter() {
		return mParentCharacter;
	}
	void setParentDummy(int dummyID) {
		mParentDummyID = dummyID;
	}
	int getParentDummy() {
		return mParentDummyID;
	}

private:
	CCharacterModel* mParentCharacter{nullptr};
	int mParentDummyID;
	CEffectObj* mEffect{nullptr};
};

inline void CSceneItem::_UpdateYaw() {
	SetYaw(Angle2Radian((float)_nYaw));
	UpdateYawPitchRoll();
}

inline void CSceneItem::_UpdatePitch() {
	SetPitch(Angle2Radian((float)_nPitch));
}

inline void CSceneItem::_UpdateRoll() {
	SetRoll(Angle2Radian((float)_nRoll));
}

inline int CSceneItem::GetItemDrapID() {
	return _pItemInfo->sDrap;
}

inline bool CSceneItem::IsPick() {
	return !_IsSystem && _pItemInfo->chIsPick && getAttachedCharacterID() == -1;
}
