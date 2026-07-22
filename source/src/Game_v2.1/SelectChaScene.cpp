#include "stdafx.h"

#include "SelectChaScene.h"

#include "GameApp.h"
#include "Character.h"
#include "SceneObj.h"
#include "UiFormMgr.h"
#include "UiTextButton.h"
#include "CharacterAction.h"
#include "SceneItem.h"
#include "ItemRecord.h"
#include "PacketCmd.h"
#include "GameConfig.h"

#include "Character.h"
#ifdef CALUA_COMPAT
#include "caLua.h" //CHANGED: Removed CaLua dependency
#include "lualib.h"
#include "lauxlib.h"
#else
#include <lua.hpp>
#endif
#include "UIRender.h"
#include "UIEdit.h"
#include "UILabel.h"
#include "uiformmgr.h"
#include "uitextbutton.h"
#include "uilabel.h"
#include "uiprogressbar.h"
#include "uiscroll.h"
#include "uilist.h"
#include "uicombo.h"
#include "uiimage.h"
#include "UICheckBox.h"
#include "uiimeinput.h"
#include "uigrid.h"
#include "uilistview.h"
#include "uipage.h"
#include "uitreeview.h"
#include "uiimage.h"
#include "UILabel.h"
#include "RenderStateMgr.h"
#include "uiDoublePwdForm.h"

#include "UIMemo.h"
#ifdef CALUA_COMPAT
#include "caLua.h" //CHANGED: Removed CaLua dependency
#endif
#include "cameractrl.h"

#include "Connection.h"
#include "ServerSet.h"
#include "GameAppMsg.h"

#include "UI3DCompent.h"
#include "UIForm.h"
#include "UITemplete.h"
#include "commfunc.h"
#include "uiboxform.h"
#include "CreateChaScene.h"
#include "loginscene.h"
#include "UIItemCommand.h"
#include "GuildData.h"
#include "UIChat.h"
#include "NetGuild.h"

//~ Constructors ==============================================================
CSelectChaScene::CSelectChaScene(stSceneInitParam& param)
	: CGameScene(param), /*m_nCurChaIndex(-1),*/ m_isInit(false), m_isCreateCha(false),
	  frmSelectCha(NULL), btnDel(NULL), btnYes(NULL), btnCreate(NULL), btnExit(NULL) {
	LG("scene memory", "CSelectChaScene Create\n");
	selectableCharacters = {
		new SelectableCharacter(2164, 1657, 140),
		new SelectableCharacter(2410, 1389, 180),
		new SelectableCharacter(2614, 1579, 220),
	};
}

//~ Destructors ===============================================================
CSelectChaScene::~CSelectChaScene() {
	LG("scene memory", "CSelectChaScene Destroy\n");

	for (auto& it : selectableCharacters) {
		delete it;
	}
}

//~ 场景相关的 ===============================================================

//-----------------------------------------------------------------------
bool CSelectChaScene::_Init() {
	if (!CGameScene::_Init()) {
		//父类调用_Init()出错,简单地返回false.
		return false;
	}

	{ // save loading res mt flag, and resume these flags in _Clear() before this scene destoried.
		lwIByteSet* res_bs = g_Render.GetInterfaceMgr()->res_mgr->GetByteSet();
		_loadtex_flag = res_bs->GetValue(OPT_RESMGR_LOADTEXTURE_MT);
		_loadmesh_flag = res_bs->GetValue(OPT_RESMGR_LOADMESH_MT);
		res_bs->SetValue(OPT_RESMGR_LOADTEXTURE_MT, 0);
		res_bs->SetValue(OPT_RESMGR_LOADMESH_MT, 0);
	}

	_bEnableCamDrag = TRUE;
#ifdef __FPS_DEBUG__
	MPTimer tInit;
	tInit.Begin();
#endif
	// Init Onece
	if (!m_isInit) {
		m_isInit = true;

		// 添加背景物件
		int constexpr LOGIN_SCENE_ID = 512;
		pObj = AddSceneObj(LOGIN_SCENE_ID);

		if (pObj) {
			pObj->SetCullingFlag(0);
			pObj->setPos(0, 0);
			pObj->setYaw(180);

			DWORD const num = pObj->GetPrimitiveNum();
			for (DWORD i = 0; i < num; i++) {
				pObj->GetPrimitive(i)->SetState(STATE_TRANSPARENT, 0);
				pObj->GetPrimitive(i)->SetState(STATE_UPDATETRANSPSTATE, 0);
			}

			{
				pObj->PlayDefaultAnimation();

				constexpr DWORD p_id_num = 6;
				constexpr DWORD p_id[p_id_num][5] = {
					{0, -1, -1, ANIM_CTRL_TYPE_BONE, PLAY_ONCE},
					{1, -1, -1, ANIM_CTRL_TYPE_BONE, PLAY_ONCE},
					{2, -1, -1, ANIM_CTRL_TYPE_BONE, PLAY_ONCE},
					{3, -1, -1, ANIM_CTRL_TYPE_BONE, PLAY_ONCE},
					{4, -1, -1, ANIM_CTRL_TYPE_BONE, PLAY_ONCE},
					{26, 0, 0, ANIM_CTRL_TYPE_TEXIMG, PLAY_LOOP},
				};

				lwPlayPoseInfo ppi = {};
				ppi.bit_mask = PPI_MASK_DEFAULT;
				ppi.pose = 0;
				ppi.frame = 0.0f;
				ppi.type = PLAY_ONCE;
				ppi.velocity = 0.5f;

				lwAnimCtrlObjTypeInfo type_info;
				for (DWORD i = 0; i < num; i++) {
					lwIPrimitive* p = pObj->GetPrimitive(i);

					for (DWORD j = 0; j < p_id_num; j++) {
						if (p->GetID() == p_id[j][0]) {
							lwIAnimCtrlAgent* anim_agent = p->GetAnimAgent();
							if (!anim_agent) {
								continue;
							}

							type_info.type = p_id[j][3];
							type_info.data[0] = p_id[j][1];
							type_info.data[1] = p_id[j][2];
							lwIAnimCtrlObj* ctrl_obj = anim_agent->GetAnimCtrlObj(&type_info);
							if (!ctrl_obj) {
								continue;
							}

							ppi.type = p_id[j][4];
							ctrl_obj->PlayPose(&ppi);
						}
					}
				}
			}

			{
				const DWORD id_num = 3;
				const DWORD id_buf[id_num] = {46, 47, 48};

				lwIPrimitive* pri;
				DWORD pri_num = pObj->GetPrimitiveNum();

				for (DWORD j = 0; j < pri_num; j++) {
					pri = pObj->GetPrimitive(j);

					for (DWORD i = 0; i < id_num; i++) {
						if (pri->GetID() == id_buf[i]) {
							pAure[i] = pri;
							pAure[i]->SetState(STATE_VISIBLE, 0);
						}
					}
				}
			}
		}
	}

	g_Render.SetClip(g_Config.fnear, g_Config.ffar);

	CCameraCtrl* pCam = g_pGameApp->GetMainCam();
	if (pCam) {
		g_pGameApp->EnableCameraFollow(TRUE);
		pCam->m_EyePos.x = 23.749f;
		pCam->m_EyePos.y = 20.923f;
		pCam->m_EyePos.z = 1.982f;

		pCam->m_RefPos.x = 20.034f;
		pCam->m_RefPos.y = -194.137f;
		pCam->m_RefPos.z = 0.868f;
	}
	g_Render.SetWorldViewFOV(Angle2Radian(70.0f));
	g_Render.SetWorldViewAspect(1.33f);
	g_Render.SetClip(1.0f, 2000.0f);

	// ?????NULl?
	if (pCam) {
		g_Render.LookAt(pCam->m_EyePos, pCam->m_RefPos);
	}
	g_Render.SetCurrentView(MPRender::VIEW_WORLD);
	MPIDeviceObject* dev_obj = g_Render.GetInterfaceMgr()->dev_obj;

	SetupVertexFog(dev_obj, 0, 0, D3DCOLOR_XRGB(28, 221, 246), D3DFOG_EXP2, 1, 0.0006f);

	g_Render.SetRenderStateForced(D3DRS_LIGHTING, 0);
	g_Render.SetRenderState(D3DRS_AMBIENT, 0xffffffff);

	//初始化UI
	if (!_InitUI()) {
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------
bool CSelectChaScene::_Clear() {
	if (frmSelectCha) {
		frmSelectCha->SetIsShow(false);
	}
	if (!CGameScene::_Clear()) {
		//父类Clear失败,简单返回false.
		return false;
	}

	{ // reset loading res mt flag
		if (_loadtex_flag != 9 && _loadmesh_flag != 9) {
			lwIByteSet* res_bs = g_Render.GetInterfaceMgr()->res_mgr->GetByteSet();
			res_bs->SetValue(OPT_RESMGR_LOADTEXTURE_MT, _loadtex_flag);
			res_bs->SetValue(OPT_RESMGR_LOADMESH_MT, _loadmesh_flag);
		}
	}
	g_Render.SetClip(1.0f, 1000.0f);

	return true;
}
//-----------------------------------------------------------------------
void CSelectChaScene::_Render() {
	//CGameScene::_Render();

#ifdef __FPS_DEBUG__
	MPTimer mpt;
	mpt.Begin();
#endif
	//CGameScene::_Render();
	MPIDeviceObject* dev_obj = g_Render.GetInterfaceMgr()->dev_obj;
	RenderStateMgr* rsm = g_pGameApp->GetRenderStateMgr();
	DWORD dwOldState;

	rsm->BeginScene();

	//渲染人物
	rsm->BeginCharacter();

	D3DLIGHTX env_light;
	D3DLIGHTX env_light_old;
	memset(&env_light, 0, sizeof(env_light));
	env_light.Type = D3DLIGHT_DIRECTIONAL;

	env_light.Direction.x = -1.0f;
	env_light.Direction.y = -1.0f;
	env_light.Direction.z = -0.5f;
	D3DXVec3Normalize((D3DXVECTOR3*)&env_light.Direction, (D3DXVECTOR3*)&env_light.Direction);

	MPDwordByte4 c;
	c.b[3] = 0xff;
	c.b[2] = 185;
	c.b[1] = 36;
	c.b[0] = 54;
	env_light.Diffuse.r = (float)(c.b[2] / 255.0f);
	env_light.Diffuse.g = (float)(c.b[1] / 255.0f);
	env_light.Diffuse.b = (float)(c.b[0] / 255.0f);

	g_Render.GetLight(0, &env_light_old);
	g_Render.SetLight(0, &env_light);

	dev_obj->GetRenderState(D3DRS_LIGHTING, &dwOldState);

	for (auto& it : pAure) {
		if (it) {
			it->SetState(STATE_VISIBLE, 0);
		}
	}

	for (auto& it : selectableCharacters) {
		if (!it->pCha) {
			continue;
		}

		dev_obj->SetRenderState(D3DRS_LIGHTING, 0);
		if (it == m_ptrCurCha) {
			dev_obj->SetRenderState(D3DRS_LIGHTING, 1);
			if (pAure[it->iPos]) {
				pAure[it->iPos]->SetState(STATE_VISIBLE, 1);
			}
			it->pCha->setYaw(m_ptrCurCha->yaw);
		}
		it->pCha->Render();
	}

	dev_obj->SetRenderState(D3DRS_LIGHTING, dwOldState);

	g_Render.SetLight(0, &env_light_old);

	rsm->EndCharacter();

	//渲染场景
	rsm->BeginSceneObject();

	if (pObj) {
		pObj->FrameMove(0);
	}
	dev_obj->GetRenderState(D3DRS_LIGHTING, &dwOldState);
	dev_obj->SetRenderState(D3DRS_LIGHTING, FALSE);

	SetupVertexFog(dev_obj, 0, 0, D3DCOLOR_XRGB(255, 104, 13), D3DFOG_EXP2, 1, 0.0025f);

	if (pObj) {
		pObj->Render();
	}

	rsm->EndSceneObject();

	rsm->BeginTranspObject();
	lwUpdateSceneTransparentObject();
	rsm->EndTranspObject();

	rsm->EndScene();

	dev_obj->SetRenderState(D3DRS_FOGENABLE, FALSE);
}

void CSelectChaScene::_RenderUI() //Renders player name, level and class on top of a character.
{
	for (auto selectableCharacter : selectableCharacters) {
		if (!selectableCharacter->pCha) {
			continue;
		}

		//渲染人物上面的字

		if ((selectableCharacter->iFontX == -1) || (selectableCharacter->iFontY == -1)) {
			lwMatrix44 mat;
			D3DXVECTOR3 pos = selectableCharacter->pCha->GetPos();
			selectableCharacter->pCha->GetRunTimeMatrix(&mat, 1);

			pos.x = mat._41;
			pos.y = mat._42;
			pos.z = mat._43;

			int nScreenX, nScreenY;
			if (!g_Render.WorldToScreen(pos.x, pos.y, pos.z, &nScreenX, &nScreenY)) { //FIXED: Player name plate didin't show if world didn't exist on first render.
				return;																  //Return because if this failed, no players are going to have real font coords, would they?
			}

			//字中心位置和人物位置的差
			int constexpr fontYOffset = -80;
			selectableCharacter->iFontX = nScreenX;
			selectableCharacter->iFontY = nScreenY + fontYOffset;
		}

		char szBuf[8];
		string sFirst = string(selectableCharacter->pCha->getName());
		_itoa_s(selectableCharacter->iLevel, szBuf, sizeof(szBuf), 10);
		string sSecond = string("Lv") + string(szBuf) + " " + selectableCharacter->sProfession;
		if (sFirst.length() < sSecond.length()) {
			int nBlankNum = (int)(sSecond.length() - sFirst.length()) / 2;

			for (int i(0); i < nBlankNum; i++) {
				sFirst = string(" ") + sFirst;
			}
		} else {
			int nBlankNum = (int)(sFirst.length() - sSecond.length()) / 2;

			for (int i(0); i < nBlankNum; i++) {
				sSecond = string(" ") + sSecond;
			}
		}
		CGuiFont::s_Font.TipRender((sFirst + "\n" + sSecond).c_str(),
								   selectableCharacter->iFontX, selectableCharacter->iFontY);
	}
}

//-----------------------------------------------------------------------
void CSelectChaScene::_FrameMove(DWORD dwTimeParam) {
	CGameScene::_FrameMove(dwTimeParam);
}

//-----------------------------------------------------------------------
bool CSelectChaScene::_MouseButtonDown(int nButton) {
	// 创建二次密码窗体显示时，不允许点击人物  add by Philip.Wu  2006-07-20
	if (g_stUIDoublePwd.GetIsShowCreateForm() ||
		g_stUIDoublePwd.GetIsShowAlterForm() ||
		g_stUIDoublePwd.GetIsShowDoublePwdForm()) {
		return false;
	}

	//判断鼠标是否点中人物
	CCharacter* pHitCha = this->HitTestCharacter(
		g_pGameApp->GetMouseX(), g_pGameApp->GetMouseY());
	if (!pHitCha) {
		return false;
	}

	//确定点击人物的位置
	auto it = std::find_if(selectableCharacters.begin(), selectableCharacters.end(),
						   [pHitCha](SelectableCharacter const* cha) { return cha->pCha == pHitCha; });
	if (it == selectableCharacters.cend()) {
		return false;
	}
	auto selectedChar = *it;

	if (selectedChar != m_ptrCurCha && m_ptrCurCha) {
		m_ptrCurCha->pCha->PlayPose(1, PLAY_LOOP, -1,
									CGameApp::GetFrameFPS(), true);
		SetChaDark(m_ptrCurCha->pCha);
	} else {
		m_ptrCurCha = selectedChar;
		m_ptrCurCha->pCha->PlayPose(2, PLAY_LOOP, -1,
									CGameApp::GetFrameFPS(), true);
	}
	m_ptrCurCha = selectedChar;
	m_ptrCurCha->pCha->SetColor(
		m_ptrCurCha->chaColor[0],
		m_ptrCurCha->chaColor[1],
		m_ptrCurCha->chaColor[2]);

	//处理表单上的按钮是否可用
	UpdateButton();

	return true;
}

//-----------------------------------------------------------------------
bool CSelectChaScene::_MouseButtonDB(int nButton) {
	if (!_MouseButtonDown(nButton)) {
		return false;
	}

	SendBeginPlayToServer();
	g_pGameApp->Waiting();
	return true;
}

//-----------------------------------------------------------------------
void CSelectChaScene::_KeyDownEvent(int key) {
	if (m_ptrCurCha) {   /*有角色被选中的情况下*/
		int iRotate = 0; // left:-1	right:1
		if (VK_LEFT == key) {
			iRotate = -1;
		} else if (VK_RIGHT == key) {
			iRotate = 1;
		}

		m_ptrCurCha->yaw += -(iRotate)*15;
		m_ptrCurCha->yaw = (m_ptrCurCha->yaw + 360) % 360;
	}
}

//-----------------------------------------------------------------------
void CSelectChaScene::LoadingCall() // 在装载loading后,刷新
{
	CGameScene::LoadingCall();

	// 每次进游戏都会经过选人界面，清理掉道具技能的COOLDOWN信息
	CItemCommand::ClearCoolDown();

	// 清空公会显示（如果玩家没公会则不会通知客户端，会遗留下前一次的公会名）
	//CGuildData::SetGuildMasterID(NULL);
	//CGuildData::SetGuildName("");
	//if(g_stUIChat.GetGuildNode()) g_stUIChat.GetGuildNode()->Clear();

	//NetPC_GUILD_START_BEGIN(0, 0, 0);
	//NetPC_GUILD_START_END();

	static bool bLoadRes2 = false;
	if (!bLoadRes2) {
		bLoadRes2 = true;
		//g_pGameApp->LoadRes2();
		//g_pGameApp->LoadRes3();
		//g_pGameApp->LoadRes4();
	}

	if (!g_Config.m_IsDoublePwd) {
		// 显示创建二次密码窗体
		g_stUIDoublePwd.ShowCreateForm();

		//CBoxMgr::ShowSelectBox(_evtCreateDoublePwdEvent, RES_STRING(CL_LANGUAGE_MATCH_800), true);//"当前帐号未创建二次密码\n\n是否现在创建?"
	} else if (GetChaCount() == 0 && frmWelcomeNotice) {
		// 当前无角色，显示新手提示
		frmWelcomeNotice->ShowModal();
	} else if (m_isCreateCha) {
		m_isCreateCha = false;

		if (GetChaCount() == 1 && frmCreateOKNotice) {
			// 刚创建第一个角色
			frmCreateOKNotice->ShowModal();
		}
	}

	g_pGameApp->PlayMusic(1);
}

//-----------------------------------------------------------------------
void CSelectChaScene::SetMainCha(int nChaID) {
	CGameScene::SetMainCha(nChaID);
}

//~ UI相关的函数 =============================================================

//-----------------------------------------------------------------------
bool CSelectChaScene::_InitUI() {
	//选人界面的表单
	frmSelectCha = CFormMgr::s_Mgr.Find("frmSelect", GetInitParam()->nUITemplete);
	if (!frmSelectCha) {
		return false;
	}

	btnDel = dynamic_cast<CTextButton*>(frmSelectCha->Find("btnDel"));
	if (!btnDel) {
		return false;
	}
	btnYes = dynamic_cast<CTextButton*>(frmSelectCha->Find("btnYes"));
	if (!btnYes) {
		return false;
	}
	btnCreate = dynamic_cast<CTextButton*>(frmSelectCha->Find("btnCreate"));
	if (!btnCreate) {
		return false;
	}
	btnExit = dynamic_cast<CTextButton*>(frmSelectCha->Find("btnNo"));
	if (!btnExit) {
		return false;
	}
	btnAlter = dynamic_cast<CTextButton*>(frmSelectCha->Find("btnAlter"));
	if (!btnAlter) {
		return false;
	}

	// 设置创建按钮闪烁
	btnCreate->SetFlashCycle();

	frmSelectCha->SetPos(
		(g_pGameApp->GetWindowWidth() - frmSelectCha->GetWidth()) / 2,
		g_pGameApp->GetWindowHeight() - frmSelectCha->GetHeight() - 20);
	frmSelectCha->Refresh();
	frmSelectCha->Show();

	frmSelectCha->evtEntrustMouseEvent = _SelChaFrmMouseEvent;

	// 创建欢迎界面   该界面仅在当前帐号内无角色时出现
	frmWelcomeNotice = CFormMgr::s_Mgr.Find("frmWelcomeNotice");
	if (!frmWelcomeNotice) {
		return false;
	}
	frmWelcomeNotice->evtEntrustMouseEvent = _evtWelcomeNoticeEvent;

	// 定义首次创建角色成功提示界面   该界面仅在该帐号走完第一个角色的创建流程后显示
	frmCreateOKNotice = CFormMgr::s_Mgr.Find("frmCreateOKNotice");
	if (!frmCreateOKNotice) {
		return false;
	}
	frmCreateOKNotice->evtEntrustMouseEvent = _evtCreateOKNoticeEvent;

	// 处理按钮是否可以用
	UpdateButton();

	frmChaNameAlter = CFormMgr::s_Mgr.Find("frmChaNameAlter");
	if (!frmChaNameAlter) {
		return false;
	}
	frmChaNameAlter->evtEntrustMouseEvent = _evtChaNameAlterMouseEvent;

	return true;
}

//-----------------------------------------------------------------------
void CSelectChaScene::_SelChaFrmMouseEvent(CCompent* pSender, int nMsgType,
										   int x, int y, DWORD dwKey) {
	string strName = pSender->GetName();

	if (_stricmp("frmSelect", pSender->GetForm()->GetName()) != 0) {
		return;
	}

	if (strName == "btnCreate") {
		//切换到创建人物场景
		stSceneInitParam param;
		param.nTypeID = enumCreateChaScene;
		param.strName = "";
		param.strMapFile = "";
		param.nUITemplete = enumCreateChaForm;
		param.nMaxCha = 20;
		param.nMaxObj = 20;
		param.nMaxItem = 20;
		param.nMaxEff = 20;

		CCreateChaScene* pkScene =
			dynamic_cast<CCreateChaScene*>(g_pGameApp->CreateScene(&param));
		if (!pkScene)
			return;
		CSelectChaScene& rkSelectChaScene = CSelectChaScene::GetCurrScene();

		g_pGameApp->GotoScene(pkScene, false);
		pkScene->setLastScene(&rkSelectChaScene);
	} else if (strName == "btnYes") {
		//进入游戏
		//向服务器发送开始游戏的消息
		GetCurrScene().SendBeginPlayToServer();
		CGameApp::Waiting();
	} else if (strName == "btnDel") {
		if (g_Config.m_IsDoublePwd) {
			// 删除角色需要二次密码  modify by Philip.Wu  2006-07-19
			g_stUIDoublePwd.SetType(CDoublePwdMgr::DELETE_CHARACTOR);
			g_stUIDoublePwd.ShowDoublePwdForm();
		}
		//else
		//{
		// 删除帐号
		//CBoxMgr::ShowSelectBox(_CheckFrmMouseEvent, RES_STRING(CL_LANGUAGE_MATCH_384), true);
		//}
	} else if (strName == "btnNo") {
		if (g_TomServer.bEnable) {
			g_pGameApp->SetIsRun(false);
			return;
		}

		// 退出选人场景
		CS_Logout();
		CS_Disconnect(DS_DISCONN);
		g_pGameApp->LoadScriptScene(enumLoginScene);
	} else if (strName == "btnAlter") {
		// 更新二次密码
		g_stUIDoublePwd.ShowAlterForm();
	}

	return;
}

//-----------------------------------------------------------------------
// 此函数已作废
//void CSelectChaScene::_CheckFrmMouseEvent(CCompent *pSender, int nMsgType,
//                                          int x, int y, DWORD dwKey)
//{
//    if( nMsgType == CForm::mrYes )
//    {
//        //向服务器发送删除角色的消息
//        GetCurrScene().SendDelChaToServer();
//        CGameApp::Waiting();
//        return;
//    }
//    return;
//}

// 询问是否要创建二次密码  add by Philip.Wu  2006-07-20
void CSelectChaScene::_evtCreateDoublePwdEvent(CCompent* pSender, int nMsgType, int x, int y, DWORD dwKey) {
	if (nMsgType == CForm::mrYes) {
		// 显示创建二次密码窗体
		g_stUIDoublePwd.ShowCreateForm();
	} else {
		// 玩家取消创建二次密码，退出
		if (g_TomServer.bEnable) {
			g_pGameApp->SetIsRun(false);
			return;
		}

		// 退出选人场景
		CS_Logout();
		CS_Disconnect(DS_DISCONN);
		g_pGameApp->LoadScriptScene(enumLoginScene);
	}
}

//~ 逻辑相关的函数 ==========================================================

//-----------------------------------------------------------------------
void CSelectChaScene::DelCurrentSelCha() {
	m_ptrCurCha->pCha->SetValid(false);	//在场景中删除该角色
	m_ptrCurCha->pCha = nullptr;		   //位置置空
	m_ptrCurCha->occupiedPosition = false; //表示该位置为空
	m_ptrCurCha = nullptr;

	//处理相关UI界面
	UpdateButton();
	return;
}

//-----------------------------------------------------------------------
bool CSelectChaScene::CreateCha(const string& sName, int nChaIndex, stNetChangeChaPart* part) {
	if (m_ptrCurCha) {
		SetChaDark(m_ptrCurCha->pCha);
	}

	CCharacter* pCha = this->AddCharacter(part->sTypeID);
	if (!pCha) {
		return false;
	}
	pCha->setName(sName.c_str());
	pCha->UpdataFace(*part);

	//搜索第一个可用的位置
	auto it = std::find_if(selectableCharacters.begin(), selectableCharacters.end(),
						   [](SelectableCharacter const* cha) { return cha->occupiedPosition ? false : true; });
	if (it == selectableCharacters.cend()) {
		return false;
	}
	auto freeCha = (*it);
	pCha->setPos(freeCha->x, freeCha->y);
	pCha->setYaw(freeCha->yaw);
	freeCha->occupiedPosition = true;
	pCha->GetColor(freeCha->chaColor);
	m_ptrCurCha = freeCha;
	//SetChaDark(pCha);

	freeCha->pCha = pCha;
	freeCha->iLevel = 1;
	freeCha->sProfession = RES_STRING(CL_LANGUAGE_MATCH_385);
	freeCha->iPos = std::distance(selectableCharacters.begin(), it);
	freeCha->iFontX = -1;
	freeCha->iFontY = -1;

	m_isCreateCha = true;

	UpdateButton();
	return true;
}

//-----------------------------------------------------------------------
void CSelectChaScene::SendDelChaToServer(const char szPassword2[]) {
	//if (m_nCurChaIndex >= 0 && m_nCurChaIndex <= 2)
	if (m_ptrCurCha) {
		//通知服务器删除角色
		CS_DelCha(m_ptrCurCha->pCha->getName(), szPassword2);
	}
}

//-----------------------------------------------------------------------
void CSelectChaScene::SendBeginPlayToServer() {
	if (!m_ptrCurCha && !m_ptrCurCha->pCha) {
		return;
	}

	CS_BeginPlay(m_ptrCurCha->pCha->getName());

	CCharacter* pCha = m_ptrCurCha->pCha;

	//测试代码
	LG("select", "Client Send:%s,%d,%d,%d,%d,%d\n",
	   pCha->getName(), pCha->GetPartID(0), pCha->GetPartID(1),
	   pCha->GetPartID(2), pCha->GetPartID(3), pCha->GetPartID(4));
}

bool CSelectChaScene::SelectCharacters(NetChaBehave* chabehave, int num) {
	int max = selectableCharacters.size();
	num = min(3, num);
	for (int i = 0; i < num; i++) {
		stNetChangeChaPart* part = (stNetChangeChaPart*)chabehave[i].sLook;

		CCharacter* pCha = this->AddCharacter(part->sTypeID);
		if (!pCha) {
			return false;
		}

		pCha->setName(chabehave[i].sCharName);
		pCha->UpdataFace(*part);
		if (!selectableCharacters[i]->occupiedPosition) {
			selectableCharacters[i]->occupiedPosition = true;
			pCha->setPos(selectableCharacters[i]->x, selectableCharacters[i]->y);
			pCha->setYaw(selectableCharacters[i]->yaw);
		}
		pCha->GetColor(selectableCharacters[i]->chaColor);
		if (i != 0) {
			SetChaDark(pCha);
		}

		selectableCharacters[i]->pCha = pCha;
		selectableCharacters[i]->iLevel = (int)(chabehave[i].iDegree);
		selectableCharacters[i]->sProfession = chabehave[i].sJob;
		selectableCharacters[i]->iPos = i;
		selectableCharacters[i]->iFontX = -1;
		selectableCharacters[i]->iFontY = -1;
	}

	UpdateButton();

	return true;
}

//-----------------------------------------------------------------------
CSelectChaScene& CSelectChaScene::GetCurrScene() {
	CSelectChaScene* pScene =
		dynamic_cast<CSelectChaScene*>(g_pGameApp->GetCurScene());

	if (!pScene)
		NULL;

	return *pScene;
}

void CSelectChaScene::SelectChaError(int error_no, const char* error_info) {
	g_pGameApp->MsgBox("%s", g_GetServerError(error_no));
	LG("error", "%s Error, Code:%d, Info: %s", error_info, error_no, g_GetServerError(error_no));
	CGameApp::Waiting(false);
}
void CSelectChaScene::SetChaDark(CCharacter* pCha) {
	pCha->SetColor(129, 121, 114);
}

void CSelectChaScene::UpdateButton() {

	auto it = std::find_if(selectableCharacters.begin(), selectableCharacters.end(),
						   [](SelectableCharacter const* cha) { return !cha->occupiedPosition; });
	it != selectableCharacters.cend() ? btnCreate->SetIsEnabled(true) : btnCreate->SetIsEnabled(false);

	if (m_ptrCurCha) {
		btnDel->SetIsEnabled(true);
		btnYes->SetIsEnabled(true);
	} else {
		btnDel->SetIsEnabled(false);
		btnYes->SetIsEnabled(false);
	}

	if (!g_Config.m_IsDoublePwd) {
		btnCreate->SetIsEnabled(false);
		btnAlter->SetIsEnabled(false);
	} else {
		btnAlter->SetIsEnabled(true);
	}
}

// 获得角色个数
int CSelectChaScene::GetChaCount() const {
	return std::count_if(selectableCharacters.cbegin(), selectableCharacters.cend(),
						 [](SelectableCharacter const* cha) { return cha->occupiedPosition ? true : false; });
}

void CSelectChaScene::ShowWelcomeNotice(bool bShow) {
	if (frmWelcomeNotice) {
		frmWelcomeNotice->ShowModal();
	}
}

// 欢迎界面 事件处理
void CSelectChaScene::_evtWelcomeNoticeEvent(CCompent* pSender, int nMsgType, int x, int y, DWORD dwKey) {
	string strName = pSender->GetName();
	CSelectChaScene* pSelectChaScene = dynamic_cast<CSelectChaScene*>(g_pGameApp->GetCurScene());

	if (pSelectChaScene) {
		if (strName == "btnYes") {
			pSelectChaScene->frmWelcomeNotice->Close();
		}
	}
}

// 首次创建角色成功提示界面 事件处理
void CSelectChaScene::_evtCreateOKNoticeEvent(CCompent* pSender, int nMsgType, int x, int y, DWORD dwKey) {
	string strName = pSender->GetName();
	CSelectChaScene* pSelectChaScene = dynamic_cast<CSelectChaScene*>(g_pGameApp->GetCurScene());

	if (pSelectChaScene) {
		if (strName == "btnYes") {
			pSelectChaScene->frmCreateOKNotice->Close();
		}
	}
}

// 首次创建角色成功提示界面 事件处理
void CSelectChaScene::_evtChaNameAlterMouseEvent(CCompent* pSender, int nMsgType, int x, int y, DWORD dwKey) {
	string strName = pSender->GetName();
	CSelectChaScene* pSelectChaScene = dynamic_cast<CSelectChaScene*>(g_pGameApp->GetCurScene());

	if (pSelectChaScene) {
		if (strName == "btnYes") {
			pSelectChaScene->frmCreateOKNotice->Close();
		}
	}
}
