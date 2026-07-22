#pragma once
#include "Scene.h"
#include "UICursor.h"
#include "scenelight.h"
#include "MPFont.h"
#include "mousedown.h"

class xShipMgr;
class xShipMgrCShipBuilder;
class CEvent;

class CWorldScene : public CGameScene {
protected:
	virtual bool _Init() override;
	virtual bool _Clear() override;
	virtual void _Render() override;
	virtual void _FrameMove(DWORD dwTimeParam) override;

	virtual bool _HandleSuperKey() override;
	virtual void _KeyDownEvent(int key) override;
	virtual bool _MouseButtonDown(int nButton) override;
	virtual bool _MouseButtonUp(int nButton) override;
	virtual bool _MouseMove(int nOffsetX, int nOffsetY) override;
	virtual bool _MouseButtonDB(int nButton) override;

public:
	virtual void SetMainCha(int nChaID) override;
	virtual void SetScreen(int w, int h, bool IsFull) override;
	static void SetAttackChaColor(BYTE r, BYTE g, BYTE b);

	virtual void LoadingCall() override; // 在装载loading后,刷新

public:
	CWorldScene(stSceneInitParam& param);
	~CWorldScene();

	AnimCtrlLight* GetAnimLight(DWORD id) { return (id >= _dwAnimLightNum) ? nullptr : &_pAnimLightSeq[id]; }
	int SwitchShipBuilder();

	xShipMgr* GetShipMgr() { return _pShipMgr.get(); }

	CCharacter* HitSelectCharacter(int nScrX, int nScrY, int nSelect = 0); // nSelect为选择范围,参看CCharacter.h-eSelectCha

	CMouseDown& GetMouseDown() { return _cMouseDown; }

	int PickItem();

protected:
	BOOL _LoadAnimLight(const char* file);
	void _SceneCursor();
	BOOL _InitUI();
	bool _IsBlock(CCharacter* pCha, int x, int y);

private:
	std::unique_ptr<xShipMgr> _pShipMgr{nullptr};
	AnimCtrlLight* _pAnimLightSeq{nullptr};
	DWORD _dwAnimLightNum{0};
	CMPFont _cFont;

private:
	static BYTE _bAttackRed, _bAttackGreen, _bAttackBlue;

	int _nOldMainChaInArea{-1};
	CMouseDown _cMouseDown;
	bool _IsShowSideLife{false};
	static bool _IsShowPing;
	static bool _IsAutoPick;
	static bool _IsShowCameraInfo;

public:
	static bool _IsThrowItemHint;
	static int nSelectChaType;	// ±£´æÑ¡ÖÐ½ÇÉ«µÄÐÅÏ¢
	static int nSelectChaPart[5]; // ÉíÌåµÄ5¸ö²¿Î»
};
