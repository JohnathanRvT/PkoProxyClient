#pragma once
#include "Scene.h"
#include "UICursor.h"
#include "scenelight.h"
#include "MPFont.h"
#include "mousedown.h"
#include "uiguidata.h"
#include "NetProtocol.h"

class xShipMgr;
class xShipMgrCShipBuilder;
class CEvent;

class CWorldScene : public CGameScene {
protected:
	virtual bool _Init();
	virtual bool _Clear();
	virtual void _Render();
	virtual void _FrameMove(DWORD dwTimeParam);

	virtual bool _HandleSuperKey();
	virtual void _KeyDownEvent(int key);
	virtual bool _MouseButtonDown(int nButton);
	virtual bool _MouseButtonUp(int nButton);
	virtual bool _MouseMove(int nOffsetX, int nOffsetY);
	virtual bool _MouseButtonDB(int nButton);

public:
	virtual void SetMainCha(int nChaID);
	virtual void SetScreen(int w, int h, bool IsFull);
	static void SetAttackChaColor(BYTE r, BYTE g, BYTE b);

	virtual void LoadingCall(); // ???loading?,??
public:
	CWorldScene(stSceneInitParam& param);
	~CWorldScene();

	AnimCtrlLight* GetAnimLight(DWORD id) { return (id >= _dwAnimLightNum) ? 0 : &_pAnimLightSeq[id]; }
	int SwitchShipBuilder();

	xShipMgr* GetShipMgr() { return _pShipMgr; }

	CCharacter* HitSelectCharacter(int nScrX, int nScrY, int nSelect = 0); // nSelect?????,??CCharacter.h-eSelectCha

	CMouseDown& GetMouseDown() { return _cMouseDown; }

	int PickItem();

protected:
	BOOL _LoadAnimLight(const char* file);
	void _SceneCursor();
	BOOL _InitUI();
	bool _IsBlock(CCharacter* pCha, int x, int y);

private:
	xShipMgr* _pShipMgr;
	AnimCtrlLight* _pAnimLightSeq;
	DWORD _dwAnimLightNum;
	CMPFont _cFont;

private:
	static BYTE _bAttackRed, _bAttackGreen, _bAttackBlue;

	int _nOldMainChaInArea;
	CMouseDown _cMouseDown;
	bool _IsShowSideLife;
	static bool _IsShowPing;
	static bool _IsAutoPick;
	static bool _IsShowCameraInfo;

public:
	static bool _IsThrowItemHint;
	static int nSelectChaType;	// 保存选中角色的信息
	static int nSelectChaPart[5]; // 身体的5个部位
};
