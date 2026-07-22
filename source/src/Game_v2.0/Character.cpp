#include "Stdafx.h"
#include "Character.h"
//add by ALLEN 2007-10-16
#include "state_reading.h"

#include "UIHeadSay.h"
#include "UICommand.h"
#include "uistartform.h"
#include "uiboatform.h"
#include "event.h"
#include "SceneHeight.h"
#include "UIBoothForm.h"
#include "stmove.h"
#include "UIMiniMapForm.h"
#ifdef CALUA_COMPAT
#include "caLua.h" //CHANGED: Removed CaLua dependency
#else
#include "lua_platform.h" //CHANGED: Make game use Luajit instead
#endif

extern std::unique_ptr<CGameApp> g_pGameApp;
extern bool g_IsShowShop;

CSkillRecord* CCharacter::_pDefaultSkillInfo = nullptr;
CSkillRecord* CCharacter::_pReadySkillInfo = nullptr;
bool CCharacter::_IsShowShadow = true;

// 特殊表现
enum eSpecial {
	enumAlphaIn = 1,	// 淡入
	enumAlphaOut = 2,   // 淡出
	enumDrop = 3,		// 下沉
	enumFastRaise = 4,  // 快速上升
	enumFastDrop = 5,   // 快速下沉
	enumHelixPos = 6,   // 公转
	enumHelixYaw = 7,   // 自转
	enumHelixOut = 8,   // 向外变大螺旋
	enumHelixIn = 9,	// 向内缩小螺旋
	enumDisappear = 10, // 立即消失
	enumCyclone = 11,   // 飓风
};

#ifdef _LOG_NAME_
bool CCharacter::IsShowLogName = false;
#endif

inline int GetWeaponBackDummy(int nWeaponType, bool isLeftHand) {
	int nBackDummy = -1;
	if (isLeftHand) {
		switch (nWeaponType) {
		case 2:
		case 5:
		case 9:
		case 10:
			nBackDummy = 21;
			break;
		case 1:
		case 3:
		case 11:
			nBackDummy = 22;
			break;
		case 4:
		case 7:
		case 8:
			nBackDummy = 23;
			break;
		}
	} else {
		switch (nWeaponType) {
		case 1:
		case 2:
		case 5:
		case 9:
		case 10:
			nBackDummy = 21;
			break;
		case 3:
		case 11:
		case 18:
		case 19:
			nBackDummy = 22;
			break;
		case 4:
		case 7:
		case 8:
			nBackDummy = 23;
			break;
		}
	}
	return nBackDummy;
}

// Add by rock.wang 2009.04.28 通过能量值从脚本中获取项链称号
// Begin
void SetPreNameByEnergy(int nItem, short* itemEnergy, char szName[16], DWORD& dwColor, int len = 0, short sex = 0) {
	if (!nItem) {
		szName[0] = '\0';
		return;
	}
#ifdef CALUA_COMPAT
	typedef struct RetPreName {
		char* pColor;
		char* preName;
	} RetPreName;

	RetPreName* pRet = (RetPreName*)CLU_CallScriptFunction("GetPreviouName", "char*,char*", "int,ushort,ushort,ushort", nItem, itemEnergy[0], itemEnergy[1], sex); //最后参数0为正常性别,1为特殊性别

	if (pRet) {
		strncpy_s(szName, len, pRet->preName, _TRUNCATE);

		std::string color = pRet->pColor;
		std::string _r, _g, _b;

		size_t dotFirst = color.find(",");
		size_t dotLast = color.find(",", dotFirst + 1);

		_r = color.substr(0, dotFirst);
		_g = color.substr(dotFirst + 1, dotLast - dotFirst - 1);
		_b = color.substr(dotLast + 1, color.length() - dotLast);

		if (_r.length() <= 0 || _g.length() <= 0 || _b.length() <= 0) {
			dwColor = D3DCOLOR_ARGB(255, 255, 255, 255);
			szName[0] = '0';
			return;
		} else {
			dwColor = D3DCOLOR_ARGB(255, atoi(_r.c_str()), atoi(_g.c_str()), atoi(_b.c_str()));
			return;
		}
	}
#else
	typedef struct RetPreName {
		DWORD dwColor;
		char* preName;
	} RetPreName;

	lua_getglobal(L, "GetPreviouName");
	lua_pushnumber(L, nItem);
	lua_pushnumber(L, itemEnergy[0]);
	lua_pushnumber(L, itemEnergy[1]);
	lua_pushnumber(L, sex);
	lua_handled_pcall(L, 4, 2, 0);
	RetPreName pRet;
	pRet.dwColor = (DWORD)lua_tonumber(L, 1);
	pRet.preName = (char*)lua_tostring(L, 2);
	lua_settop(L, 0);

	strncpy_s(szName, len, pRet.preName, _TRUNCATE);
	dwColor = pRet.dwColor;
#endif
}
// End

static void __keyframe_proc(DWORD type, DWORD pose_id, DWORD key_id, DWORD key_frame, void* param) {
	CCharacter* pCha = ((CCharacter*)param);
	switch (type) {
	case KEYFRAME_TYPE_BEGIN: {
		pCha->GetActor()->ActionBegin(pCha->GetCurPoseType());

		if (pCha->GetCurPoseType() > 6 && pCha->GetCurPoseType() < 15) {
			CEffectObj* pEffect;
			CSceneItem* item = pCha->GetLinkItem(LINK_ID_RIGHTHAND);
			if (item && item->GetItemDrapID() > 0) {
				pEffect = pCha->GetScene()->GetFirstInvalidEffObj();
				if (pEffect) {
					if (pEffect->Create(item->GetItemDrapID())) {
						pEffect->setFollowObj((CSceneNode*)item, NODE_ITEM);
						pEffect->Emission(-1, &pCha->GetPos(), nullptr);
						pEffect->SetValid(TRUE);
					}
				}
			}
			item = pCha->GetLinkItem(LINK_ID_LEFTHAND);
			if (item && item->GetItemDrapID() > 0) {
				pEffect = pCha->GetScene()->GetFirstInvalidEffObj();
				if (pEffect) {
					if (pEffect->Create(item->GetItemDrapID())) {
						pEffect->setFollowObj((CSceneNode*)item, NODE_ITEM);

						pEffect->Emission(-1, &pCha->GetPos(), nullptr);
						pEffect->SetValid(TRUE);
					}
				}
			}
		}
		break;
	}
	case KEYFRAME_TYPE_KEY:
		pCha->ActionKeyFrame(key_id);
		break;
	case KEYFRAME_TYPE_END:
		pCha->GetActor()->ActionEnd(pCha->GetCurPoseType());
		break;
	}
}

// ========================
CCharacter::CCharacter() : _dwGuildColor(COLOR_BLACK) {
	_Attr.set(ATTR_MXHP, 20);
	_Attr.set(ATTR_HP, 20);
	_Attr.set(ATTR_MSPD, 400);

	memset(&_stChaPart, 0, sizeof(_stChaPart));

	_ChaState.AllTrue();
}

CCharacter::~CCharacter() {}

int CCharacter::LoadCha(const LoadChaInfo* info) {
	if (_Create(info->cha_id, 0) == 0)
		return FALSE;

	if (info->bone[0] != 0) {
		if (FAILED(LoadBone(info->bone))) {
			LG("error", RES_STRING(CL_LANGUAGE_MATCH_16));
			return FALSE;
		}
	}

	for (DWORD i = 0; i < 5; i++) {
		if (info->part[i][0] == 0)
			continue;

		if (FAILED(LoadPart(i, info->part[i]))) {
			LG("error", RES_STRING(CL_LANGUAGE_MATCH_16));
			return FALSE;
		}
	}

	for (DWORD i = 0; i < 4; i++) {
		if (strlen(info->prop[i]) == 0)
			continue;

		CSceneItem* p = this->GetScene()->AddSceneItem(info->prop[i]);

		if (p == nullptr) {
			LG("error", "msgLoad Ship Power System error!\n");
			return FALSE;
		}
		p->PlayDefaultAnimation();

		lwSceneItemLinkInfo ili;
		ili.data = 99;
		ili.id = 0;
		ili.obj = p;
		ili.link_item_id = 0;
		ili.link_parent_id = i;

		this->SetSubObjItemLink(2, &ili);
	}

	return TRUE;
}

BOOL CCharacter::_Create(int nCharTypeID, int nType) {
	return TRUE;
}

bool CCharacter::LoadBoat(stNetChangeChaPart& stPart) {
	memcpy(&_stChaPart, &stPart, sizeof(_stChaPart));
	DWORD dwBuf[8] = {0};
	if (_pShipInfo = ConvertPartTo8DWORD(stPart, dwBuf)) {
		//if(_pShipInfo->sNumHeader||_pShipInfo->sNumEngine||_pShipInfo->sNumCannon||_pShipInfo->sNumEquipment)
		//	_bUpdate = true;
		if (stPart.sBoatID > 10)
			_bUpdate = true;
		else
			_bUpdate = false;

		return LoadShip(stPart.sPosID, dwBuf) ? true : false;
	}

	return false;
}

void CCharacter::_UpdateValid(BOOL bValid) {
	if (bValid == FALSE) {
		setSideID(0);
		if (GetDrunkState())
			CCharacterModel::SetDrunkState(FALSE);
		_pChaState->ChaDestroy();

		DieTime();
		if (_pEvent) {
			_pEvent->SetIsValid(false);
			_pEvent = nullptr;
		}
		_DetachAllItem();
		for (int i = 0; i < enumEQUIP_NUM; i++) {
			if (_pHandItem[i]) {
				_pHandItem[i]->SetValid(FALSE);
				_pHandItem[i] = nullptr;
			}
		}

		for (int i = 0; i < ITEM_FACE_MAX; i++) {
			if (_pItemFaceEff[i]) {
				_pItemFaceEff[i]->SetValid(FALSE);
				_pItemFaceEff[i] = nullptr;
			}
		}
		memset(_ItemFace, 0, sizeof(_ItemFace));

		GetScene()->HandleSceneMsg(SCENEMSG_CHA_DESTROY, getID());
	} else {
		SetPoseKeyFrameProc(__keyframe_proc, (void*)this);
	}
}

void CCharacter::FrameMove(DWORD dwTimeParam) {
	CSceneNode::FrameMove(dwTimeParam);

	if (!_isArrive) {
		if (_IsMoveTimeType) {
			static float dis;
			static D3DXVECTOR2 vTmp;

			dis = (float)(CGameApp::GetCurTick() - _dwStartTime) * _fMoveSpeed;
			if (dis > _fMoveLen) {
				setPos((int)_vMoveEnd.x, (int)_vMoveEnd.y);

				_isArrive = true;
			} else {
				vTmp = _vMoveStart + _vMoveDir * dis;
				setPos((int)vTmp.x, (int)vTmp.y);
			}
		} else if (GetChaState()->IsTrue(enumChaStateMove)) {
			// 重新计算新的ProgressRate(决定当前的位置和高度)------------------------------------------------------------------------------
			if (_nTurnCnt > 1)
				_fStepProgressRate = _fStepProgressRate + _fStep * CWaitMoveState::GetMoveRate() / (float)_nTurnCnt;
			else
				_fStepProgressRate = _fStepProgressRate + _fStep * CWaitMoveState::GetMoveRate();

			_CalPos(_fStepProgressRate);
			if (_fStepProgressRate >= 1.0f) {
				_isArrive = true;
			} else if (_isStopMove) {
				_isArrive = true;
			}
		}
	} else {
		// 每300ms刷新一次高度
		if (_pSceneHeight->FrameMove(dwTimeParam)) {
			_UpdateHeight();

			if (_pNpcStateItem) {
				//_pNpcStateItem->setHeightOff( (int)(GetDefaultChaInfo()->fHeight * 100.0f) );
				_pNpcStateItem->setHeightOff((int)(GetDefaultChaInfo()->fHeight * (GetIsFly() ? 130.0f : 100.0f)));
			}
		}
	}

	if (_nTurnCnt) {
		// 更新方向
		_nYaw += _nAngleStep;
		SetYaw(Angle2Radian((float)_nYaw));
		UpdateYawPitchRoll();
		_nTurnCnt--;
	}

	_pActor->FrameMove(dwTimeParam);

	if (_Special.IsAny()) {
		static long x, y;
		static float f = 0.0f;

		if (_Special.IsTrue(enumDrop)) {
			_nHeightOff--;
			_fMapHeight = 0.0f;
		}
		if (_Special.IsTrue(enumFastRaise)) {
			_nHeightOff += 25;
			_fMapHeight = 0.0f;
		}
		if (_Special.IsTrue(enumFastDrop)) {
			_nHeightOff -= 25;
			if (_nHeightOff < 0) {
				_nHeightOff = 0;
				_Special.SetFalse(enumFastDrop);
			}
			_fMapHeight = 0.0f;
		}
		if (_Special.IsTrue(enumAlphaOut)) {
			f = GetOpacity() - 0.03f;
			if (f < 0.0f) {
				f = 0.0f;
				_Special.SetFalse(enumAlphaOut);
			}
			SetOpacity(f);
		}
		if (_Special.IsTrue(enumAlphaIn)) {
			// 淡入完成时，所有特效都取消
			f = GetOpacity() + 0.03f;
			if (f > _fMaxOpacity) {
				f = _fMaxOpacity;
				_Special.SetFalse(enumAlphaIn);
			}
			SetOpacity(f);
		}
		if (_Special.IsTrue(enumHelixOut)) {
			_nHelixAngle += 13;
			_nHelixRadii += 20;
			::eddy(_nHelixCenterX, _nHelixCenterY, _nHelixAngle, _nHelixRadii, x, y);
			_nCurX = x;
			_nCurY = y;
		}
		if (_Special.IsTrue(enumHelixIn)) {
			_nHelixAngle += 13;
			_nHelixRadii -= 20;
			if (_nHelixRadii > 0) {
				::eddy(_nHelixCenterX, _nHelixCenterY, _nHelixAngle, _nHelixRadii, x, y);
				_nCurX = x;
				_nCurY = y;
			} else {
				_nCurX = _nHelixCenterX;
				_nCurY = _nHelixCenterY;
				_Special.SetFalse(enumHelixIn);
			}
		}
		if (_Special.IsTrue(enumHelixYaw)) {
			setYaw((getYaw() + 18) % 360);
		}
		if (_Special.IsTrue(enumHelixPos)) {
			_nHelixAngle += 13;
			::eddy(_nHelixCenterX, _nHelixCenterY, _nHelixAngle, 200, x, y);
			_nCurX = x;
			_nCurY = y;
		}
		if (_Special.IsTrue(enumCyclone)) {
			_nHelixAngle += 13;
			::eddy(_nHelixCenterX, _nHelixCenterY, _nHelixAngle, _nHelixRadii, x, y);
			_nCurX = x;
			_nCurY = y;

			if (_nHeightOff <= 500)
				_nHeightOff += 25;
		}
		_UpdateHeight();
	}

	_computeLinkedMatrix();

	// by lsh
	// 注意，这里的FrameMove必需放在最后调用，否则会有位置误差
	CCharacterModel::FrameMove();
}

void CCharacter::Render() {
	if (!_pScene->IsPointVisible((float)_nCurX / 100.0f, (float)_nCurY / 100.0f))
		return;

	g_Render.EnableZBuffer(TRUE);

	CSceneNode::Render();

	CCharacterModel::Render();
}

void CCharacter::RefreshLevel(int nMainLevel) {
	if (IsMonster()) {
		int nDifLevel = (int)(getGameAttr()->get(ATTR_LV) - nMainLevel);
		if (nDifLevel >= 7) {
			_pHeadSay->SetNameColor(D3DCOLOR_ARGB(255, 241, 14, 240));
		} else if (nDifLevel >= 5) {
			_pHeadSay->SetNameColor(D3DCOLOR_ARGB(255, 255, 0, 0));
		} else if (nDifLevel >= 3) {
			_pHeadSay->SetNameColor(D3DCOLOR_ARGB(255, 255, 207, 0));
		} else if (nDifLevel >= -2) {
			_pHeadSay->SetNameColor(D3DCOLOR_ARGB(255, 255, 255, 255));
		} else if (nDifLevel >= -4) {
			_pHeadSay->SetNameColor(D3DCOLOR_ARGB(255, 217, 255, 119));
		} else if (nDifLevel >= -6) {
			_pHeadSay->SetNameColor(D3DCOLOR_ARGB(255, 0, 255, 12));
		} else {
			_pHeadSay->SetNameColor(D3DCOLOR_ARGB(255, 192, 192, 192));
		}
	} else {
		_pHeadSay->SetNameColor(COLOR_WHITE);
	}
}

void CCharacter::RefreshUI(int nParam) {
	_pHeadSay->SetLifeNum((int)_Attr.get(ATTR_HP), (int)_Attr.get(ATTR_MXHP));

	if (g_stUIBoat.GetHuman() == this) {
		g_stUIStart.RefreshMainLifeNum((long)_Attr.get(ATTR_HP), (long)_Attr.get(ATTR_MXHP));
		g_stUIStart.RefreshMainSP((long)_Attr.get(ATTR_SP), (long)_Attr.get(ATTR_MXSP));

		g_stUIStart.RefreshMainExperience(_Attr.get(ATTR_CEXP), _Attr.get(ATTR_CLEXP), _Attr.get(ATTR_NLEXP));
		g_stUIStart.RefreshLv((long)_Attr.get(ATTR_LV));
	}
}

BOOL CCharacter::WieldItem(const lwSceneItemLinkInfo* info) {
	if (FAILED(MPCharacter::SetItemLink(info)))
		return FALSE;

	CSceneItem* item = (CSceneItem*)info->obj;
	item->setAttachedCharacterID(getID());

	return TRUE;
}

void CCharacter::DestroyLinkItem() {
	for (DWORD i = 0; i < GetLinkItemNum(); i++) {
		CSceneItem* obj = (CSceneItem*)GetLinkItem(i);
		obj->SetValid(0);
	}
}

int CCharacter::_GetTargetAngle(int nTargetX, int nTargetY, BOOL bBack) {
	D3DXVECTOR2 dPosition = D3DXVECTOR2((float)nTargetX, (float)nTargetY) - D3DXVECTOR2((float)_nCurX, (float)_nCurY);
	if (bBack)
		dPosition *= -1;

	int nAngle = (int)(180.0f - Radian2Angle(atan2f(dPosition.x, dPosition.y)));

	return FixAngle(nAngle);
}

void CCharacter::setPos(int nX, int nY) {
#ifdef _STATE_DEBUG
	if (IsMainCha()) {
		static DWORD tick = 0;
		int dis = GetDistance(nX, nY, _nCurX, _nCurY);
		if (dis > 300.0f)
			LG("main_cha_pos", "\t\tpos:%d, %d, target:%d, %d, dis:%d, Tick:%d, FPS: %d\n", _nCurX, _nCurY, nX, nY, dis, GetTickCount() - tick, g_Render.GetFPS());
		else
			LG("main_cha_pos", "pos:%d, %d, target:%d, %d, dis:%d, Tick:%d, FPS: %d\n", _nCurX, _nCurY, nX, nY, dis, GetTickCount() - tick, g_Render.GetFPS());
		tick = GetTickCount();
	}
#endif

	if (_Special.IsAny()) {
		_nHelixCenterX = nX;
		_nHelixCenterY = nY;
	}

	_nCurX = nX;
	_nCurY = nY;

	if (_pNpcStateItem) {
		_pNpcStateItem->setPos(nX, nY);
	}

	_UpdateHeight();
}

int CCharacter::FaceTo(int nAngle) {
	if (!_pDefaultChaInfo->IsFace())
		return getYaw();

	_nYaw = FixAngle(_nYaw);

	nAngle = FixAngle(nAngle);

	if (IsBoat()) {
		_nAngleStep = (int)(300.0f / (float)CGameApp::GetFrameFPS());
	} else {
		_nAngleStep = (int)(600.0f / (float)CGameApp::GetFrameFPS());
	}

	if (!_isArrive && IsMainCha() && !IsBoat()) {
		_nAngleStep = (int)((float)_nAngleStep * 0.75f);
		if (_nAngleStep <= 0)
			_nAngleStep = 1;
	}

	int nDis = GetAngleDistance(_nYaw, nAngle);

	_nTurnCnt = (abs)(nDis) / _nAngleStep;

	int nLeft = nDis % _nAngleStep;

	if (nDis < 0) {
		_nAngleStep *= -1;
	}

	if (nLeft != 0) {
		_nYaw += nLeft;
		SetYaw(Angle2Radian((float)_nYaw));
		UpdateYawPitchRoll();
	}
	return nAngle;
}

bool CCharacter::ItemEffect(int nEffectID, int nItemDummy, int nAngle) {
	if (nEffectID == 0 || nItemDummy == -1)
		return false;

	CSceneItem* item = GetAttackItem();
	if (!item)
		return false;

	CEffectObj* pEffect = GetScene()->GetFirstInvalidEffObj();
	if (!pEffect)
		return false;

	if (!pEffect->Create(nEffectID))
		return false;

	static MPMatrix44 mat;
	if (item->GetObjDummyRunTimeMatrix(&mat, nItemDummy) >= 0) {
		pEffect->setFollowObj((CSceneNode*)item, NODE_ITEM, nItemDummy);
		if (nAngle != 999)
			pEffect->SetEffectDir(nAngle);
		pEffect->Emission(-1, (D3DXVECTOR3*)&mat._41, nullptr);
	} else {
		pEffect->setFollowObj((CSceneNode*)item, NODE_ITEM);
		if (nAngle != 999)
			pEffect->SetEffectDir(nAngle);
		pEffect->Emission(-1, &GetPos(), nullptr);
	}

	pEffect->SetValid(TRUE);
	return true;
}

CEffectObj* CCharacter::SelfEffect(int nEffectID, int nDummy, bool isLoop, int nSize, int nAngle) {
	if (nEffectID <= 0)
		return nullptr;

	// added by Philip.Wu  2008-01-25 乱斗白银城里不播放这些特效
	if (g_stUIMap.IsPKSilver() && this->IsPlayer() && this->GetMainType() != enumMainPlayer) {
		if (574 <= nEffectID && nEffectID <= 577)
			return nullptr;
	}
	CEffectObj* pEffect = GetScene()->GetFirstInvalidEffObj();
	if (!pEffect)
		return nullptr;

	if (!pEffect->Create(nEffectID))
		return nullptr;

	if (isLoop)
		pEffect->setLoop(isLoop);

	if (nSize > 0 && pEffect->GetBaseSize() > 0.0f) // 如果小于零,不支持缩放
	{
		SkillCtrl ctrl;
		ctrl.fSize = (float)nSize / 100.0f / pEffect->GetBaseSize();
		pEffect->SetSkillCtrl(&ctrl);
	}
	static MPMatrix44 mat;

	if (IsBoat()) {
		//xShipInfo* pInfo = ::GetShipInfo( GetPart().sBoatID );
		if (!_bUpdate) {
			if (nDummy >= 0 && GetObjDummyRunTimeMatrix(&mat, nDummy, 0) >= 0) {
				if (nAngle != 999)
					pEffect->SetEffectDir(nAngle);
				pEffect->setFollowObj((CSceneNode*)this, NODE_CHA, nDummy);
				pEffect->Emission(-1, (D3DXVECTOR3*)&mat._41, nullptr);
			} else {
				if (nAngle != 999)
					pEffect->SetEffectDir(nAngle);
				pEffect->setFollowObj((CSceneNode*)this, NODE_CHA);
				pEffect->Emission(-1, &GetPos(), nullptr);
			}
		} else
			goto __ret;
	} else {
	__ret:
		if (nDummy >= 0 && GetObjDummyRunTimeMatrix(&mat, nDummy) >= 0) {
			if (nAngle != 999)
				pEffect->SetEffectDir(nAngle);
			pEffect->setFollowObj((CSceneNode*)this, NODE_CHA, nDummy);
			pEffect->Emission(-1, (D3DXVECTOR3*)&mat._41, nullptr);
		} else {
			if (nAngle != 999)
				pEffect->SetEffectDir(nAngle);
			pEffect->setFollowObj((CSceneNode*)this, NODE_CHA);
			pEffect->Emission(-1, &GetPos(), nullptr);
		}
	}

	pEffect->SetValid(TRUE);
	return pEffect;
}

CEffectObj* CCharacter::SkyEffect(int nEffectID, int nBeginDummy, int nItemDummy, int nSpeed, D3DXVECTOR3* pTarget, int nTargetChaID, CSkillRecord* pSkill) {
	if (nEffectID <= 0)
		return nullptr;

	CEffectObj* pEffect = GetScene()->GetFirstInvalidEffObj();
	if (!pEffect)
		return nullptr;

	if (!pEffect->Create(nEffectID))
		return nullptr;

	static D3DXVECTOR3 pos;
	static MPMatrix44 mat;
	if (nBeginDummy >= 0 && GetObjDummyRunTimeMatrix(&mat, nBeginDummy) >= 0) {
		pos = *(D3DXVECTOR3*)&mat._41;
	} else if (nItemDummy >= 0) {
		CSceneItem* item = GetAttackItem();
		if (item && item->GetObjDummyRunTimeMatrix(&mat, nItemDummy) >= 0) {
			pos = *(D3DXVECTOR3*)&mat._41;
		} else {
			pos = GetPos();
		}
	} else {
		pos = GetPos();
	}

	if (pSkill) {
		switch (pSkill->GetShape()) {
		case enumRANGE_TYPE_NONE:
		case enumRANGE_TYPE_SQUARE: // 柱形（长度，宽度）
		case enumRANGE_TYPE_CIRCLE: // 圆形（半径）
			break;
		case enumRANGE_TYPE_FAN: // 扇形（半径，角度）
		{
			pEffect->SetFanAngle(pSkill->GetParam()[1]);
			pEffect->SetMagicDist((float)pSkill->GetParam()[0] / 100.0f);
		}
		case enumRANGE_TYPE_STICK: // 柱形（长度，宽度）
		{
			// 取方向，为解决特效从手上道具飞出时，鼠标却点在道具与主角之间时，特效飞的方向不对
			int x, y;
			GetDistancePos(GetCurX(), GetCurY(), int(pTarget->x * 100.0f), int(pTarget->y * 100.0f), 500, x, y);
			pTarget->x = (float)x / 100;
			pTarget->y = (float)y / 100;
		} break;
		};
	}

	pEffect->setOwerID(getID());
	pEffect->SetVel((float)nSpeed);
	pEffect->Emission(nTargetChaID, &pos, pTarget);
	pEffect->SetValid(TRUE);
	return pEffect;
}

bool CCharacter::ChangeReadySkill(int nSkillID) {
	CSkillRecord* p = GetSkillRecordInfo(nSkillID);
	if (!p) {
		// 没技能
		g_pGameApp->AddTipText(RES_STRING(CL_LANGUAGE_MATCH_17), p->szName);
		return false;
	}

	if (_pReadySkillInfo && _pReadySkillInfo->sID == nSkillID)
		return true;

	if (!p->GetIsValid()) {
		g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_18), p->szName);
		return false;
	}

	g_pGameApp->AddTipText(RES_STRING(CL_LANGUAGE_MATCH_19), p->szName);
	_SetReadySkill(p);
	return true;
}

void CCharacter::_UpdateHeight() {
	_vPos.x = (float)_nCurX / 100.0f;
	_vPos.y = (float)_nCurY / 100.0f;

	if (_nPoseHeightOff == ERROR_POSE_HEIGHT) {
		switch (_pDefaultChaInfo->chTerritory) {
		case 1:
			_fMapHeight = (float)_pDefaultChaInfo->sSeaHeight / 100.0f;
			break;
		default:
			_fMapHeight = _pScene->GetGridHeight(_vPos.x, _vPos.y);
		}

		_vPos.z = _fMapHeight + (float)(_nHeightOff) / 100.0f;
	} else {
		_vPos.z = (float)(_nPoseHeightOff) / 100.0f;
	}

	SetPos((float*)&_vPos); // 子类对设置高度的实现, _vPos.x与_vPos.y都不变
	if (GetDrunkState()) {
		UpdateChaMatrix();
	}
}
//void	CCharacter::SetHieght(float fhei)
//{
//	if( _nPoseHeightOff==ERROR_POSE_HEIGHT )
//	{
//		switch( _pDefaultChaInfo->chTerritory )
//		{
//		case 1:
//			//_fMapHeight = (float)_pDefaultChaInfo->sSeaHeight / 100.0f;
//			_fMapHeight = fhei +_pScene->GetGridHeight(_vPos.x, _vPos.y);
//			break;
//		default:
//			_fMapHeight = _pScene->GetGridHeight(_vPos.x, _vPos.y);
//		}
//
//		_vPos.z = _fMapHeight + (float)( _nHeightOff ) / 100.0f;
//	}
//	else
//	{
//		_vPos.z = (float)( _nPoseHeightOff ) / 100.0f;
//	}
//
//	SetPos((float*)&_vPos); // 子类对设置高度的实现, _vPos.x与_vPos.y都不变
//	if( GetDrunkState() )
//	{
//		UpdateChaMatrix();
//	}
//}

void CCharacter::OperatorEffect(char oper, int x, int y) {
	//switch( oper )
	//{
	//case 3:// 上岸
	//	if( IsBoat() )
	//	{
	//		D3DXVECTOR3 pos;
	//		pos.x = (float)x / 100.0f;
	//		pos.y = (float)y / 100.0f;
	//		pos.z = _pScene->GetGridHeight( pos.x, pos.y );

	//		CEffectObj* pEffect = SkyEffect( 1021, 2, -1, 2500, &pos );
	//           if( pEffect )
	//           {
	//               pEffect->GetEffDelay()->SetPosEffect( 157, pos );
	//           }
	//	}
	//	break;
	//case 1:// 下海
	//	if( IsPlayer() )
	//	{
	//		D3DXVECTOR3 pos;
	//		pos.x = (float)x / 100.0f;
	//		pos.y = (float)y / 100.0f;
	//		pos.z = _pScene->GetGridHeight( pos.x, pos.y );

	//		CEffectObj* pEffect = SkyEffect( 1021, 2, -1, 2500, &pos );
	//           if( pEffect )
	//           {
	//               pEffect->GetEffDelay()->SetPosEffect( 158, pos );
	//           }
	//	}
	//	break;
	//}
}

void CCharacter::setName(const char* pszName) {
	strncpy_s(_szName, sizeof(_szName), pszName, _TRUNCATE);
	_pHeadSay->SetName(_szName);
}

void CCharacter::UpdateTileColor() {
	BYTE r = 255;
	BYTE g = 255;
	BYTE b = 255;

	MPTerrain* t = _pScene->GetTerrain();
	if (t) {
		D3DXVECTOR3 v = this->GetPos();
		MPTile* tile = t->GetTile((int)v.x, (int)v.y);
		if (tile) {
			D3DXCOLOR c = tile->dwXColor;

			//if(c.r < 0.4f)
			//    c.r = 0.4f;
			//if(c.g < 0.4f)
			//    c.g = 0.4f;
			//if(c.b < 0.4f)
			//    c.b = 0.4f;

			r = (BYTE)((float)r * ((1.0f + c.r) / 2));
			g = (BYTE)((float)g * ((1.0f + c.g) / 2));
			b = (BYTE)((float)b * ((1.0f + c.b) / 2));

			//const BYTE lmt = 160;
			//if(r < lmt)
			//    r = lmt;
			//if(g  < lmt)
			//    g = lmt;
			//if(b < lmt)
			//    b = lmt;
		}
	}

	if (GetEmiColorFlag() == 0) {
		CCharacterModel::SetAmbColor(r, g, b);
	}
}

void CCharacter::PlayAni(char* pAni, int nMax) {
	_Special.AllFalse();
	_nHeightOff = 0;

	_nHelixCenterX = _nCurX;
	_nHelixCenterY = _nCurY;
	for (int i = 0; i < nMax; i++) {
		if (pAni[i] == 0)
			break;

		switch (pAni[i]) {
		case enumAlphaIn:
			_Special.SetTrue(pAni[i]);
			SetOpacity(0.0f);
			break;
		case enumHelixOut:
			_Special.SetTrue(pAni[i]);
			_nHelixRadii = 0;
			_nHelixAngle = getYaw();
			break;
		case enumHelixIn:
			_Special.SetTrue(pAni[i]);
			_nHelixRadii = 400;
			_nHelixAngle = getYaw();
			break;
		case enumHelixPos:
			_Special.SetTrue(pAni[i]);
			_nHelixRadii = 200;
			_nHelixAngle = getYaw();
			break;
		case enumFastDrop:
			_Special.SetTrue(pAni[i]);
			// _nHeightOff = 200;
			break;
		case enumDisappear:
			_Special.SetTrue(pAni[i]);
			SetHide(TRUE);
			break;
		case enumCyclone:
			if (_pDefaultChaInfo->IsCyclone()) {
				_Special.SetTrue(pAni[i]);
				_nHelixAngle = getYaw();
				_nHelixRadii = 200;
				_nHeightOff = 0;
			}
			break;
		default:
			_Special.SetTrue(pAni[i]);
		}
	}
}

void CCharacter::InitState() {
	GetActor()->InitState();

	_isArrive = true;

	_Special.AllFalse();

	SetOpacity(_fMaxOpacity);

	_nHeightOff = 0;
	_fMapHeight = 0.0f;
	_nPoseHeightOff = ERROR_POSE_HEIGHT;
	_InFight = false;

	SetHide(FALSE);

	_dwStartTime = 0;

	_ChaState.AllTrue();
	_hits.clear();

	_pHeadSay->Reset();

	_pSceneHeight->Reset();

	_szShopName[0] = 0;
	_PK.AllFalse();
	setSideID(0);
}

void CCharacter::setSideID(long v) {
	_lSideID = v;

	if (_pSideShade) {
		_pSideShade->SetValid(FALSE);
		_pSideShade = nullptr;
	}

	if (_lSideID != 0 && IsPlayer()) {
		LG(getLogName(), "Add SideID:%d\n", _lSideID);
		_pSideShade = _pScene->GetFirstInvalidShadeObj();
		if (_pSideShade) {
			LG(getLogName(), "Add SideID:%d Successed\n", _lSideID);
			_pSideShade->Create(IsBoat() ? 7 : 2);
			_pSideShade->setColor(_lSideID == 1 ? 0xff00ff00 : 0xffff0000);
			_pSideShade->setChaID(getID());
			_pSideShade->setUpSea(true);
			_pSideShade->Emission(0, &GetPos(), nullptr);
			_pSideShade->SetValid(TRUE);
		}
	}
}

void CCharacter::ActionKeyFrame(DWORD key_id) {
	if (_pDefaultChaInfo) {
		if (_pDefaultChaInfo->sEffectActionID[0] == key_id) {
			SelfEffect(_pDefaultChaInfo->sEffectActionID[1], _pDefaultChaInfo->sEffectActionID[2]);
		}
	}

	switch (GetCurPoseType()) {
	case POSE_RUN:
	case POSE_RUN2:
		if (_pszFootMusic && key_id == 0) {
			CGameScene::PlayEnvSound(_pszFootMusic, GetCurX(), GetCurY());
		}
		break;
	case POSE_WAITING:
	case POSE_WAITING2:
		if (_pszWhoopMusic) {
			CGameScene::PlayEnvSound(_pszWhoopMusic, GetCurX(), GetCurY());
		}
		break;
	}

	GetActor()->ActionKeyFrame(GetCurPoseType(), key_id);

	if (_IsShowShadow && _eChaCtrlType == enumCHACTRL_PLAYER && !IsBoat() && GetScene()->GetTileTexAttr((int)GetPos().x, (int)GetPos().y) == 1) {
		GetScene()->GetPugMgr()->NewPug(&GetPos(), (float)getYaw() * 0.01745329f + D3DX_PI);
	}
}

void CCharacter::SetDefaultChaInfo(CChaRecord* pInfo) {
	_pDefaultChaInfo = pInfo;

	_pszFootMusic = nullptr;
	if (_pDefaultChaInfo->sFootfall != -1) {
		CMusicInfo* pMusicInfo = GetMusicInfo(_pDefaultChaInfo->sFootfall);
		if (pMusicInfo) {
			_pszFootMusic = pMusicInfo->szDataName;
		}
	}

	_pszWhoopMusic = nullptr;
	if (_pDefaultChaInfo->sWhoop != -1) {
		CMusicInfo* pMusicInfo = GetMusicInfo(_pDefaultChaInfo->sWhoop);
		if (pMusicInfo) {
			_pszWhoopMusic = pMusicInfo->szDataName;
		}
	}

	_fMaxOpacity = (float)_pDefaultChaInfo->chDiaphaneity / 100.0f;
}

bool CCharacter::PlayPose(DWORD pose, DWORD type, int time, int fps, bool isBlend) {
	fps = CGameApp::GetFrameFPS();
	LG(getLogName(), "Pose:%d, type:%d, time:%d\n", pose, type, time, fps);

	bool rv = GetCurPoseType() == pose;

#if 1
	// 飞行用（测试代码）
	if (GetIsFly()) {
		if (pose == POSE_WAITING || pose == POSE_WAITING2)
			pose = POSE_FLY_WAITING; // 站立 -> 飞行
		if (pose == POSE_RUN || pose == POSE_RUN2)
			pose = POSE_FLY_RUN; // 走动 -> 飘走
		if (pose == POSE_SHOW)
			pose = POSE_FLY_SHOW; // 摆酷 -> 空中摆酷
		if (pose == POSE_SEAT)
			pose = POSE_FLY_SEAT; // 坐下 -> 空中悬空坐
	}
#endif

	if ((GetCurPoseType() == POSE_RUN2) || (GetCurPoseType() == POSE_RUN) || GetCurPoseType() == POSE_FLY_RUN) {
		if ((pose != POSE_RUN2) && (pose != POSE_RUN) && (pose != POSE_FLY_RUN)) {
			GetScene()->HandleSceneMsg(SCENEMSG_CHA_ENDMOVE, getTypeID(), getID());
		}
	} else {
		if ((pose == POSE_RUN2) || (pose == POSE_RUN) || (pose == POSE_FLY_RUN)) {
			GetScene()->HandleSceneMsg(SCENEMSG_CHA_BEGINMOVE, getTypeID(), getID());
		}
	}

	if (isBlend) {
		CCharacterModel::PlayPose(pose, type, time, fps, 1);
	} else {
		if (enumMODAL_MAIN_CHA == _eChaModalType && pose > POSE_WAITING && pose < POSE_ATTACK) {
			CCharacterModel::PlayPose(pose, type, time, fps, 1);
		} else {
			CCharacterModel::PlayPose(pose, type, time, fps, 0);
		}
	}

	if (rv)
		return false;

	switch (pose) {
	case POSE_WAITING2:
		CGameScene::PlayEnvSound(_pDefaultChaInfo->sWhoop, GetCurX(), GetCurY());
		break;
	case POSE_DIE:
		CGameScene::PlayEnvSound(_pDefaultChaInfo->sDirge, GetCurX(), GetCurY());
		break;
	}
	return true;
}

void CCharacter::_FightSwitch(bool isFight) {
	_IsFightPose = isFight;
	if (isFight) {
		DetachItem(21);
		DetachItem(22);
		DetachItem(23);

		int dummy = -1;
		if (_pHandItem[enumEQUIP_LHAND]) {
			AttachItem(enumEQUIP_LHAND, _pHandItem[enumEQUIP_LHAND], -1);

			dummy = GetWeaponBackDummy(_pHandItem[enumEQUIP_LHAND]->GetItemInfo()->sType, true);
			if (dummy > 0) {
				_pHandItem[enumEQUIP_LHAND]->setAlpla();
			}
		}

		if (_pHandItem[enumEQUIP_RHAND]) {
			AttachItem(enumEQUIP_RHAND, _pHandItem[enumEQUIP_RHAND], -1);

			dummy = GetWeaponBackDummy(_pHandItem[enumEQUIP_RHAND]->GetItemInfo()->sType, false);
			if (dummy > 0) {
				_pHandItem[enumEQUIP_RHAND]->setAlpla();
			}
		}
	} else {
		int dummy = -1;
		if (_pHandItem[enumEQUIP_LHAND]) {
			dummy = GetWeaponBackDummy(_pHandItem[enumEQUIP_LHAND]->GetItemInfo()->sType, true);
			if (dummy > 0) {
				DetachItem(enumEQUIP_LHAND);

				AttachItem(dummy, _pHandItem[enumEQUIP_LHAND], -1, 3);
				_pHandItem[enumEQUIP_LHAND]->setAlpla();
			} else {
				AttachItem(enumEQUIP_LHAND, _pHandItem[enumEQUIP_LHAND], -1);
			}
		}
		if (_pHandItem[enumEQUIP_RHAND]) {
			dummy = GetWeaponBackDummy(_pHandItem[enumEQUIP_RHAND]->GetItemInfo()->sType, false);
			if (dummy > 0) {
				DetachItem(enumEQUIP_RHAND);

				AttachItem(dummy, _pHandItem[enumEQUIP_RHAND], -1, 3);
				_pHandItem[enumEQUIP_RHAND]->setAlpla();
			} else {
				AttachItem(enumEQUIP_RHAND, _pHandItem[enumEQUIP_RHAND], -1);
			}
		}
	}
}

void CCharacter::UpdataFace(stNetChangeChaPart& stPart) {
	int nCharID = getTypeID();
	int itemID = stPart.SLink[enumEQUIP_NECK].sID;
	short* itemEnergy = stPart.SLink[enumEQUIP_NECK].sEnergy;

	if (!IsBoat()) {
		SetPreNameByEnergy(itemID, itemEnergy, _szPreName, _szPreColor, 16, nCharID);
	}

	memcpy(&_stChaPart, &stPart, sizeof(_stChaPart));

	_DetachAllItem();
	int nHairID = stPart.SLink[enumEQUIP_HEAD].sID != 0 ? stPart.SLink[enumEQUIP_HEAD].sID : stPart.sHairID;
	if (!UpdataItem(nHairID, 0) && nHairID != stPart.sHairID) {
		UpdataItem(stPart.sHairID, 0);
	}

	for (int i = 1; i < enumEQUIP_NUM; i++) {
		UpdataItem(stPart.SLink[i].sID, i);
	}

	if (nCharID >= 1 || nCharID <= 4) {
		if (_pHandItem[enumEQUIP_RHAND])
			_pHandItem[enumEQUIP_RHAND]->SetForgeEffect(stPart.SLink[enumEQUIP_RHAND].lDBParam[0], nCharID);

		if (_pHandItem[enumEQUIP_LHAND])
			_pHandItem[enumEQUIP_LHAND]->SetForgeEffect(stPart.SLink[enumEQUIP_LHAND].lDBParam[0], nCharID);
	}

	// 最后才确定武器摆放方式
	RefreshItem(true);
}

xShipInfo* CCharacter::ConvertPartTo8DWORD(stNetChangeChaPart& stPart, DWORD* dwBuf) {
	xShipInfo* pInfo = ::GetShipInfo(stPart.sBoatID);
	if (!pInfo) {
		LG("boat_error", RES_STRING(CL_LANGUAGE_MATCH_20), stPart.sBoatID);
		return nullptr;
	}

	// 船只是否可以更新组件
	xShipPartInfo* pData = GetShipPartInfo(pInfo->sBody);
	if (pData == nullptr) {
		LG("boat_error", RES_STRING(CL_LANGUAGE_MATCH_21), pInfo->sBody);
		return nullptr;
	}

	dwBuf[0] = pData->dwModel;
	if (pInfo->byIsUpdate) {
		pData = GetShipPartInfo(stPart.sHeader);
		if (pData == nullptr) {
			LG("boat_error", RES_STRING(CL_LANGUAGE_MATCH_22), stPart.sHeader);
			return nullptr;
		}
		dwBuf[1] = pData->dwModel;

		pData = GetShipPartInfo(stPart.sEngine);
		if (pData == nullptr) {
			LG("boat_error", RES_STRING(CL_LANGUAGE_MATCH_23), stPart.sEngine);
			return nullptr;
		}
		dwBuf[2] = pData->dwModel;

		dwBuf[3] = 0;
		for (int i = 0; i < BOAT_MAXNUM_MOTOR; i++) {
			xShipPartInfo* pMotorData = GetShipPartInfo(pData->sMotor[i]);
			if (pMotorData) {
				dwBuf[i + 4] = pMotorData->dwModel;
			} else {
				dwBuf[i + 4] = 0;
			}
		}
	}
	return pInfo;
}

bool CCharacter::UpdataItem(int nItem, DWORD nLink) {
	if (enumMODAL_MAIN_CHA != _eChaModalType)
		return false;

	switch (nLink) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		if (nItem == 0)
			nItem = _pDefaultChaInfo->sSkinInfo[nLink];

		if (CCharacterModel::ChangePart(nLink, nItem) == 0) {
			LG(getLogName(), "msgChangePart Error, Item[%d], Link[%d]\n\n", nItem, nLink);
			return false;
		}
		return true;
	case enumEQUIP_LHAND:
	case enumEQUIP_RHAND:
		if (nItem > 0 && nItem != enumEQUIP_BOTH_HAND) {
			if (!_pHandItem[nLink] || _pHandItem[nLink]->GetItemInfo()->lID != nItem) {
				if (_pHandItem[nLink]) {
					_pHandItem[nLink]->SetValid(FALSE);
					_pHandItem[nLink] = nullptr;
				}

				CSceneItem* item = _pScene->AddSceneItem(nItem, getTypeID());
				if (item) {
					_pHandItem[nLink] = item;
				} else {
					LG(getLogName(), "msgChangePart AddSceneItem Error, Item[%d], Link[%d]\n\n", nItem, nLink);
					return false;
				}
			}
		} else {
			if (_pHandItem[nLink]) {
				DetachItem(_pHandItem[nLink]);

				_pHandItem[nLink]->SetValid(FALSE);
				_pHandItem[nLink] = nullptr;
			}
		}
		return false;
	case enumEQUIP_NECK:
	case enumEQUIP_HAND1:
	case enumEQUIP_HAND2:
	case enumEQUIP_Jewelry1:
	case enumEQUIP_Jewelry2:
	case enumEQUIP_Jewelry3:
	case enumEQUIP_Jewelry4:
		return true;
	}
	return false;
}

void CCharacter::setNpcState(DWORD dwState) {
	if (_pNpcStateItem) {
		_pNpcStateItem->SetValid(FALSE);
		_pNpcStateItem = nullptr;
	}

	if (_chGMLv) {
		if (_chGMLv >= 21 && _chGMLv <= 30) {
			_pNpcStateItem = _pScene->AddSceneItem("hd.lgo");
		} else {
			_pNpcStateItem = _pScene->AddSceneItem("gm.lgo");
		}
	} else if (ROLE_MIS_DELIVERY & dwState) {
		_pNpcStateItem = _pScene->AddSceneItem("sighyellow.lgo");
	} else if (ROLE_MIS_ACCEPT & dwState) {
		_pNpcStateItem = _pScene->AddSceneItem("sighgreen.lgo");
	} else if (ROLE_MIS_PENDING & dwState) {
		_pNpcStateItem = _pScene->AddSceneItem("sighgray.lgo");
	}

	if (_pNpcStateItem) {
		_pNpcStateItem->setIsSystem(true);
		_pNpcStateItem->setHeightOff((int)(GetDefaultChaInfo()->fHeight * 100.0f));
		_pNpcStateItem->setPos(GetCurX(), GetCurY());
		_pNpcStateItem->PlayDefaultAnimation();
		_pNpcStateItem->setYaw(0);
	}
}

void CCharacter::DieTime() {
	int size = (int)GetEffectNum();
	int id;
	CEffectObj* pEff;
	for (int n = 0; n < size; n++) {
		id = GetEffectID(n);
		if (id < 0)
			continue;
		pEff = GetScene()->GetEffect(id);
		if (pEff && pEff->IsValid()) {
			pEff->SetValid(FALSE);
		}
	}
	RemoveEffect();

	if (GetDrunkState())
		CCharacterModel::SetDrunkState(FALSE);

	if (_pBoatFog) {
		_pBoatFog->SetValid(FALSE);
		_pBoatFog = nullptr;
	}
	if (_pShopItem) {
		_pShopItem->SetValid(FALSE);
		_pShopItem = nullptr;
	}
	if (_pNpcStateItem) {
		_pNpcStateItem->SetValid(FALSE);
		_pNpcStateItem = nullptr;
	}

	for (int i = 0; i < ITEM_FACE_MAX; i++)
		SetItemFace(i, 0);
	// setSideID( 0 );
}

void CCharacter::SynchroSkillState(stSkillState* pState, int nCount) {
	static CBoolSet set;
	set = _pChaState->Synchro(pState, nCount);

	if (set.IsTrue(enumChaStateNoShop) != _ChaState.IsTrue(enumChaStateNoShop)) {
		if (_pShopItem) {
			_pShopItem->SetValid(FALSE);
			_pShopItem = nullptr;
		}

		if (set.IsFalse(enumChaStateNoShop)) {
			if (g_IsShowShop) {
				switch (CChaStateMgr::GetLastShopLevel()) {
				case 1:
					// PlayPose( POSE_WAITING, PLAY_LOOP_SMOOTH );
					_pShopItem = _pScene->AddSceneItem("baitan1.lgo");
					break;
				default:
					// PlayPose( POSE_SEAT, PLAY_LOOP_SMOOTH );
					_pShopItem = _pScene->AddSceneItem("baitan2.lgo");
					break;
				}
				if (_pShopItem) {
					_pShopItem->setIsSystem(true);
					_pShopItem->setYaw(getYaw());
					_pShopItem->setPos(GetCurX(), GetCurY());
				}
			}
		} else {
			g_stUIBooth.CloseBoothByOther(getAttachID());
		}
	}

	if (set.IsTrue(enumChaStateNoHide) != _ChaState.IsTrue(enumChaStateNoHide)) {
		if (set.IsTrue(enumChaStateNoHide))
			SetOpacity(_fMaxOpacity);
		else
			SetOpacity(_fMaxOpacity / 2.0f);
	}

	if (set.IsTrue(enumChaStateNoAni) != _ChaState.IsTrue(enumChaStateNoAni)) {
		if (set.IsTrue(enumChaStateNoAni)) {
			if (IsMainCha()) {
				g_pGameApp->EnableCameraFollow(true);
			}

			StopAni();
		} else {
			if (IsMainCha()) {
				g_pGameApp->EnableCameraFollow(false);
			}

			PlayAni(CChaStateMgr::GetLastActInfo()->nActBehave, CChaStateMgr::GetLastActInfo()->GetActNum());
		}
	}

	//if( set.IsTrue(enumChaStateMove) != _ChaState.IsTrue(enumChaStateMove) )
	//{
	//	if( set.IsTrue( enumChaStateMove ) ) PlayPoseContinue();
	//	else PlayPosePause();
	//}

	if (set.IsTrue(enumChaStateNoDizzy) != _ChaState.IsTrue(enumChaStateNoDizzy)) {
		if (_pDefaultChaInfo->IsCyclone()) {
			if (set.IsTrue(enumChaStateNoDizzy)) {
				CCharacterModel::SetDrunkState(FALSE);

				//CDizzyState* pState = dynamic_cast<CDizzyState*>(GetActor()->GetCurState());
				//if( pState ) pState->Stop();
			} else {
				CCharacterModel::SetDrunkState(TRUE);
				//GetActor()->OverAllState();
				//CDizzyState* pState = new CDizzyState(GetActor());
				//GetActor()->SwitchState( pState );

				//set.SetFalse( enumChaStateMove );
				//set.SetFalse( enumChaStateAttack );
				//set.SetFalse( enumChaStateUseSkill );
			}
		}
	}

	_ChaState = set;
}

void CCharacter::RefreshShopShop() {
	if (_pShopItem) {
		_pShopItem->SetValid(FALSE);
		_pShopItem = nullptr;
	}

	if (g_IsShowShop && _ChaState.IsFalse(enumChaStateNoShop)) {
		switch (CChaStateMgr::GetLastShopLevel()) {
		case 1:
			// PlayPose( POSE_WAITING, PLAY_LOOP_SMOOTH );
			_pShopItem = _pScene->AddSceneItem("baitan1.lgo");
			break;
		default:
			// PlayPose( POSE_SEAT, PLAY_LOOP_SMOOTH );
			_pShopItem = _pScene->AddSceneItem("baitan2.lgo");
			break;
		}
		if (_pShopItem) {
			_pShopItem->setIsSystem(true);
			_pShopItem->setYaw(getYaw());
			_pShopItem->setPos(GetCurX(), GetCurY());
		}
	}
}

bool CCharacter::GetRunTimeMatrix(MPMatrix44* mat, DWORD dummy_id) {
	return GetObjDummyRunTimeMatrix(mat, dummy_id) == 0;
}

void CCharacter::LoadingCall() {
	RefreshItem(true);
}

void CCharacter::RefreshItem(bool isFirst) {
	if (isFirst) {
		CheckIsFightArea();

		_FightSwitch(_InFight);

		PlayPose(GetPose(GetCurPoseType()), PLAY_LOOP_SMOOTH);
	} else {
		if (_InFight != _IsFightPose) {
			_FightSwitch(_InFight);
		}
	}
}

void CCharacter::SwitchFightPose() {
	if (!_IsFightPose) {
		_FightSwitch(true);
	}
}

void CCharacter::ForceMove(int nTargetX, int nTargetY) {
	if (_nCurX == nTargetX && _nCurY == nTargetY) {
		_isArrive = true;
		return;
	}

	_nTargetX = nTargetX;
	_nTargetY = nTargetY;

	_IsMoveTimeType = !IsMainCha();
	if (_IsMoveTimeType) {
		_fMoveSpeed = (float)getMoveSpeed() / 950.0f;
		_fMoveLen = (int)GetDistance(_nCurX, _nCurY, nTargetX, nTargetY) + 1.0f;
		if (_fMoveLen > 0.0f) {
			_dwStartTime = CGameApp::GetCurTick();

			_vMoveStart.x = (float)_nCurX;
			_vMoveStart.y = (float)_nCurY;
			_vMoveEnd.x = (float)nTargetX;
			_vMoveEnd.y = (float)nTargetY;
			_vMoveDir = _vMoveEnd - _vMoveStart;
			D3DXVec2Normalize(&_vMoveDir, &_vMoveDir);
			_isArrive = false;
			_isStopMove = false;
		} else {
			_isArrive = true;
		}
	} else {
		float fDistance = (float)GetDistance(_nCurX, _nCurY, nTargetX, nTargetY);
		CGameApp::GetFrame()->RefreshFPS();
		_fStep = 1.0f / (fDistance / (float)getMoveSpeed() * (float)(CGameApp::GetFrameFPS() + 2));

		if (_fStep > 1.0) {
			_isArrive = true;
			return;
		}

		_nLastPosX = _nCurX;
		_nLastPosY = _nCurY;

		_fStepProgressRate = 0.0;
		_isArrive = false;

		_isStopMove = false;
	}
}

void CCharacter::ResetReadySkill() {
	CCommandObj::Cancel();
	_SetReadySkill(nullptr);
}

void CCharacter::SetHide(BOOL bHide) {
	if (_bHide == bHide)
		return;

	_bHide = bHide;
	RefreshShadow();

	int i;
	for (i = 0; i < enumEQUIP_NUM; i++)
		if (_pHandItem[i])
			_pHandItem[i]->SetHide(bHide);

	if (_pNpcStateItem)
		_pNpcStateItem->SetHide(bHide);

	if (_pShopItem)
		_pShopItem->SetHide(bHide);

	if (_pSideShade)
		_pSideShade->SetHide(bHide);

	for (i = 0; i < GetEffectNum(); i++) {
		GetScene()->GetEffect(GetEffectID(i))->SetHide(bHide);
	}
	for (i = 0; i < GetShadeNum(); i++) {
		GetScene()->GetShadeObj(GetShadeID(i))->SetHide(bHide);
	}
	if (_pBoatFog) {
		_pBoatFog->SetHide(bHide);
	}

	for (int i = 0; i < ITEM_FACE_MAX; i++) {
		if (_pItemFaceEff[i]) {
			_pItemFaceEff[i]->SetHide(bHide);
		}
	}
}

void CCharacter::RefreshFog() {
	if (!_pDefaultChaInfo->HaveEffectFog())
		return;

	if (_bHide)
		return;

	static float hp[] = {0.2f, 0.5f, 0.8f};

	int nIDX = 0;
	if (_pBoatFog) {
		nIDX = _pBoatFog->getIdxID();
	}

	int nEffectID = 0;
	float fHPRate = (float)getGameAttr()->get(ATTR_HP) / (float)getGameAttr()->get(ATTR_MXHP);
	if (fHPRate < hp[0]) {
		nEffectID = _pDefaultChaInfo->GetEffectFog(0);
	} else if (fHPRate < hp[1]) {
		nEffectID = _pDefaultChaInfo->GetEffectFog(1);
	} else if (fHPRate < hp[2]) {
		nEffectID = _pDefaultChaInfo->GetEffectFog(2);
	}

	if (nEffectID != nIDX) {
		if (_pBoatFog) {
			_pBoatFog->SetValid(FALSE);
		}
		_pBoatFog = SelfEffect(nEffectID, 4, true);
	}
}

void CCharacter::SetItemFace(unsigned int nIndex, int nItem) {
	if (nIndex >= ITEM_FACE_MAX)
		return;

	if (nItem == _ItemFace[nIndex])
		return;

	_ItemFace[nIndex] = nItem;

	if (_pItemFaceEff[nIndex]) {
		_pItemFaceEff[nIndex]->SetValid(FALSE);
		_pItemFaceEff[nIndex] = nullptr;
	}

	CItemRecord* pInfo = GetItemRecordInfo(nItem);
	if (!pInfo || pInfo->sItemEffect[0] == 0)
		return;

	switch (nIndex) {
	case 0: // 第一栏放置翅膀
		if (pInfo->sType != EItemType::Wing) {
			return;
		} else {
			DWORD pose = GetCurPoseType();
			if (GetIsFly() && (pose == POSE_WAITING || pose == POSE_WAITING2))
				PlayPose(POSE_FLY_WAITING); // 站立 -> 飞行

			CCharacter* pCha = this;
			if (g_stUIMap.IsPKSilver() && pCha->IsPlayer() && pCha->GetMainType() != enumMainPlayer)
				return;

			if (pInfo->sNeedLv > this->_Attr.get(ATTR_LV))
				return;

			if (pInfo->chBody[0] != -1) {
				bool bFlag = false;
				for (int i = 0; i < defITEM_BODY; i++) {
					if (pInfo->chBody[i] != -1 && pInfo->chBody[i] == GetDefaultChaInfo()->lID) {
						bFlag = true;
						break;
					}
				}
				if (!bFlag)
					return;
			}
		}
		break;
	case 1:
		if (pInfo->sType != EItemType::Pet) {
			return; // 第二栏放宠物精灵
		} else {
			CCharacter* pCha = this;
			if (g_stUIMap.IsPKSilver() && pCha->IsPlayer() && pCha->GetMainType() != enumMainPlayer)
				return;

			if (pInfo->sNeedLv > this->_Attr.get(ATTR_LV))
				return;

			if (pInfo->chBody[0] != -1) {
				bool bFlag = false;
				for (int i = 0; i < defITEM_BODY; i++) {
					if (pInfo->chBody[i] != -1 && pInfo->chBody[i] == GetDefaultChaInfo()->lID) {
						bFlag = true;
						break;
					}
				}
				if (!bFlag)
					return;
			}
		}
		break;
	default:
		return;
	}

	CEffectObj* pEffect = _pScene->GetFirstInvalidEffObj();
	if (!pEffect)
		return;

	if (!pEffect->Create(pInfo->sItemEffect[0]))
		return;

	pEffect->setLoop(true);
	pEffect->setFollowObj((CSceneNode*)this, NODE_CHA, pInfo->sItemEffect[1]);
	pEffect->Emission(-1, nullptr, nullptr);
	pEffect->SetValid(TRUE);

	_pItemFaceEff[nIndex] = pEffect;
}

bool CCharacter::GetIsPet() {
	long nID = GetDefaultChaInfo()->lID;

	switch (nID) {
	case 929: // 圣诞小老人
	case 930: // 剃光毛的火鸡
	case 931: // 麋鹿宝宝
	case 932: // 雪人宝宝
		return true;
	}

	return false;
}

bool CCharacter::GetIsFly() {
	int nID = GetItemFace(0);
	return (128 <= nID && nID <= 140 && nID != 135) ? true : false;

	//CItemRecord* pItem = GetItemRecordInfo(GetItemFace(0));
	//return (pItem && pItem->sType == 44 && _Attr.get(ATTR_SAILLV) > 0) ? true : false;
}

void CCharacter::_computeLinkedMatrix() {
	if (mParentNode) {
		MPMatrix44 mat;
		mParentNode->GetRunTimeMatrix(&mat, mParentBoneID);

		// extract translation information from parent bone.
		D3DXVECTOR3 pos;
		pos.x = mat._41;
		pos.y = mat._42;
		pos.z = mat._43;

		// extract scaling information from parent bone.
		D3DXVECTOR3 scale;
		scale.x = sqrt(mat._11 * mat._11 + mat._21 * mat._21 + mat._31 * mat._31);
		scale.y = sqrt(mat._12 * mat._12 + mat._22 * mat._22 + mat._32 * mat._32);
		scale.z = sqrt(mat._13 * mat._13 + mat._23 * mat._23 + mat._33 * mat._33);

		// extract rotation information from parent bone.
		float rotZ = D3DX_PI - atan2(mat._21, mat._11);
		float rotY = -asin(-mat._31 / scale.x); //
		float rotX = -atan2(mat._32 / scale.y, mat._33 / scale.z);

		float yaw = Radian2Angle(rotZ);
		float pitch = Radian2Angle(rotY);
		float roll = Radian2Angle(rotX);

		//SetPos((float*)(&pos));
		setPos(int(pos.x * 100.f), int(pos.y * 100.f));
		SetScale(scale);
		setYaw(int(yaw));
		setPitch(int(pitch));
		setRoll(int(roll));
	}
}

//add by ALLEN 2007-10-16
bool CCharacter::IsReadingBook() {
	CReadingState* pState = dynamic_cast<CReadingState*>(GetActor()->GetCurState());
	return (nullptr != pState);
}
