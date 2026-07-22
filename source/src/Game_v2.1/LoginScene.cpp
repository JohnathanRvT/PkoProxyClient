#include "stdafx.h"
#include "LoginScene.h"

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
#include "cameractrl.h"
#include "UIListView.h"

#include "UIMemo.h"
#ifdef CALUA_COMPAT
#include "caLua.h" //CHANGED: Removed CaLua dependency
#endif

#include "Connection.h"
#include "ServerSet.h"
#include "GameAppMsg.h"

#include "UI3DCompent.h"
#include "UIForm.h"
#include "UITemplete.h"
#include "commfunc.h"
#include "uiboxform.h"
#include "bill.h"

#include <shellapi.h>

//#define USE_STATUS		// 此协议与大波的网络有冲突，关闭 lh by 2006-3-22

#ifdef USE_STATUS
#include "UdpClient.h"

using namespace client_udp;
#endif

#pragma comment(lib, "shell32.lib")

#include "xmlwriter.h"

//#ifdef KOP_TOM.
TOM_SERVER g_TomServer;
//#endif
Cooperate g_cooperate; //  add by jampe

CForm* CLoginScene::frmArea = NULL;
CForm* CLoginScene::frmAccount = NULL;
CForm* CLoginScene::frmKeyboard = NULL; // add by Philip.Wu  软键盘界面  2006-06-05

std::array<CList*, MAX_REGION_LIST> CLoginScene::lstRegion = {};
std::array<CLabelEx*, MAX_REGION_LIST> regionNames = {};

CEdit* CLoginScene::edtID = NULL;
CEdit* CLoginScene::edtPassword = NULL;
CEdit* CLoginScene::edtFocus = NULL;	 // add by Philip.Wu  光标激活的编辑框  2006-06-06
CCheckBox* CLoginScene::chkShift = NULL; // add by Philip.Wu  软键盘上的 Shift  2006-06-09
CCheckBox* CLoginScene::chkID = NULL;

CImage* CLoginScene::imgLogo1 = NULL;
CImage* CLoginScene::imgLogo2 = NULL;

static void _GoBack(CCompent* pSender, int nMsgType, int x, int y, DWORD dwKey) {
	CLoginScene* pLogin = dynamic_cast<CLoginScene*>(CGameApp::GetCurScene());
	if (pLogin) {
		if (g_cooperate.code) {
			pLogin->InitRegionServerLists();
			pLogin->frmArea->SetIsShow(true);
		} else {
			//CHANGED: Make login show if connection fails instead of server selection.
			pLogin->ShowLoginForm();
		}
	}
}

CLoginScene::CLoginScene(stSceneInitParam& param) : CGameScene(param),
													_eState(eLoginState::Init),
													m_bPasswordError(false),
													m_sPassport("nobill"),
													m_sUsername(""),
													m_sPassword("") {
	LG("scene memory", "CLoginScene Create\n");
	_loadtex_flag = 9;
	_loadmesh_flag = 9;

	m_iCurSelRegionIndex = 1;
	m_iCurSelServerIndex = 0;
}

CLoginScene::~CLoginScene() {
	LG("scene memory", "CLoginScene Destroy\n");
}

bool CLoginScene::_Init() {
	_IsUseSound = false;

	_eState = eLoginState::Init;

	{ // save loading res mt flag
		lwIByteSet* res_bs = g_Render.GetInterfaceMgr()->res_mgr->GetByteSet();
		_loadtex_flag = res_bs->GetValue(OPT_RESMGR_LOADTEXTURE_MT);
		_loadmesh_flag = res_bs->GetValue(OPT_RESMGR_LOADMESH_MT);
		res_bs->SetValue(OPT_RESMGR_LOADTEXTURE_MT, 0);
		res_bs->SetValue(OPT_RESMGR_LOADMESH_MT, 0);
	}

	CGameScene::_Init();

	SetFocus(g_pGameApp->GetHWND());

	if (!_InitUI()) {
		LG("login_ini", RES_STRING(CL_LANGUAGE_MATCH_168));

		return false;
	}

	CFormMgr::s_Mgr.SetEnabled(TRUE);

	// ???????????
	char const szUpdateFileName[] = "_Update.exe";
	SetFileAttributes(szUpdateFileName, FILE_ATTRIBUTE_NORMAL);
	DeleteFile(szUpdateFileName);

	m_sUsername = "player";
	m_sPassword = "";
	static bool isFirst = true;
	if (g_TomServer.bEnable && isFirst) {
		isFirst = false;
		m_sUsername = g_TomServer.szUser;
		m_sPassword = g_TomServer.szPassword;
		m_sPassport = g_TomServer.szPassport;
		_Connect();
		return true;
	}

	return true;
}

bool CLoginScene::_Clear() {
	CGameScene::_Clear();

	//    g_Render.SetRenderState(D3DRS_FOGENABLE, 0);
	{ // reset loading res mt flag
		if (_loadtex_flag != 9 && _loadmesh_flag != 9) {
			lwIByteSet* res_bs = g_Render.GetInterfaceMgr()->res_mgr->GetByteSet();
			res_bs->SetValue(OPT_RESMGR_LOADTEXTURE_MT, _loadtex_flag);
			res_bs->SetValue(OPT_RESMGR_LOADMESH_MT, _loadmesh_flag);
		}
	}

	g_Render.SetClip(1.F, 1000.0f);

	return true;
}

void CLoginScene::ShowLoginForm() {
	chkID->SetIsChecked(m_bSaveAccount);
	edtID->SetCaption(m_sSaveAccount.c_str());
	edtPassword->SetCaption("");
	frmAccount->Show();
	frmArea->Hide();

	// add by Philip.Wu  2006-07-03  显示输入框时同时显示软键盘
	frmKeyboard->SetIsShow(false); //CHANGED: Made keyboard hide by default
	imgLogo1->SetIsShow(true);	 //CHANGED: Made logo appear by default
	imgLogo2->SetIsShow(true);	 //CHANGED: Made logo appear by default

	if (m_sSaveAccount == "") {
		edtID->SetActive(edtID);
	} else {
		edtPassword->SetActive(edtPassword);
	}
}

void CLoginScene::_FrameMove(DWORD dwTimeParam) {

	int x = g_pGameApp->GetMouseX();
	int y = g_pGameApp->GetMouseY();
	GetRender().ScreenConvert(x, y);

	if (frmArea->GetIsShow()) {
		for (auto& list : lstRegion) {
			if (!list->InRect(x, y)) {
				list->GetItems()->GetSelect()->SetNoSelect();
			}
		}
	}

	if (_eState == eLoginState::Connect) {
		switch (g_NetIF->GetConnStat()) {
		case Connection::CNST_CONNECTING: {
		} break;
		case Connection::CNST_INVALID:
		case Connection::CNST_FAILURE: {
			if (g_TomServer.bEnable) {
				MessageBox(0, RES_STRING(CL_LANGUAGE_MATCH_169), "Info", 0);
				g_pGameApp->SetIsRun(false);
				return;
			}

			// ·µ»Øµ½Ñ¡Ôñ·þÎñÆ÷³¡¾°
			_eState = eLoginState::Init;
			CGameApp::Waiting(false);

			ShowKeyboard(false);
			frmArea->SetIsShow(false);
			frmAccount->SetIsShow(false);
			g_stUIBox.ShowMsgBox(_GoBack, RES_STRING(CL_LANGUAGE_MATCH_169));
		} break;
		case Connection::CNST_CONNECTED: {
			CGameApp::Waiting(false);

			//连接成功,发送登陆消息
			_Login();
		} break;
		case Connection::CNST_TIMEOUT:
			_eState = eLoginState::Init;
			g_pGameApp->SendMessage(APP_NET_DISCONNECT, 1000);
			return;
		}
		return;
	}
}

void CLoginScene::_Render() {
	static bool IsLoad = false;
	static CGuiPic LoginPic;
	if (!IsLoad) {
		LoginPic.LoadImage("texture/ui/new_login.jpg", 1024, 768, 0, 0, 0, 1.F, 1.F);
		IsLoad = true;
	}
	LoginPic.SetScale(0, GetRender().GetScreenWidth(), GetRender().GetScreenHeight());
	LoginPic.Render(0, 0);
}

void CLoginScene::LoadingCall() {
	g_pGameApp->PlayMusic(1);
}

//-----------------
// 界面相关Routines
//-----------------
void CLoginScene::CallbackUIEvent_LoginScene(CCompent* pSender, int state, int x, int y, DWORD key) {
	CLoginScene* pScene = dynamic_cast<CLoginScene*>(CGameApp::GetCurScene());
	if (!pScene) {
		return;
	}

	string const strName = pSender->GetName();
	if (_stricmp("frmAccount", pSender->GetForm()->GetName()) == 0) {
		if (strName == "btnYes") {
			pScene->LoginFlow();
		} else if (strName == "btnNo") {
			if (g_NetIF->IsConnected()) {
				CS_Disconnect(DS_DISCONN);
			}
			pSender->GetForm()->Hide();
			return;
		}
	}
}

void CLoginScene::_evtRegionFrm(CCompent* pSender, int state, int x, int y, DWORD key) {
	CLoginScene* pkScene = dynamic_cast<CLoginScene*>(CGameApp::GetCurScene());
	if (!pkScene) {
		return;
	}

	string const strName = (const char*)pSender->GetName();
	if (strName == "btnNo") {
		//关闭服务器列表表单,显示通告表单
		pSender->GetForm()->SetIsShow(false);

		g_pGameApp->SetIsRun(false);
		return;
	}
}

void CLoginScene::_evtLoginFrm(CCompent* pSender, int state, int x, int y, DWORD key) {
	CLoginScene* pkScene = dynamic_cast<CLoginScene*>(CGameApp::GetCurScene());
	if (!pkScene) {
		return;
	}

	string const strName = (const char*)pSender->GetName();
	if (strName == "btnYes") {
		// 先关闭软键盘
		if (frmKeyboard->GetIsShow()) {
			frmKeyboard->SetIsShow(false);
			imgLogo1->SetIsShow(true);
			imgLogo2->SetIsShow(true);
		}

		// 连接服务器
		pkScene->LoginFlow();
	} else if (strName == "btnNo") {
		// 先关闭软键盘
		if (frmKeyboard->GetIsShow()) {
			frmKeyboard->SetIsShow(false);
			imgLogo1->SetIsShow(true);
			imgLogo2->SetIsShow(true);
		}

		//关闭连接
		if (g_NetIF->IsConnected()) {
			CS_Disconnect(DS_DISCONN);
		}
		//关闭登陆表单,显示上一次显示的服务器列表表单
		pSender->GetForm()->SetIsShow(false);
		pkScene->ShowRegionList();
	} else if (strName == "btnKeyboard") {
		// 软键盘显示与关闭  add by Philip  2006-06-20
		// 如果打开软键盘。则不能显示 LOGO 图片（LOGO 是由两张拼接而成的）
		// 不然会有焦点冲突，导致软键盘在 LOGO 后面而无法点击
		bool bShow = frmKeyboard->GetIsShow();

		ShowKeyboard(!bShow);
		//imgLogo1->SetIsShow(bShow);
		//imgLogo2->SetIsShow(bShow);
		//frmKeyboard->SetIsShow(! bShow);
	}
}

void CLoginScene::_evtRegionLDBDown(CGuiData* pSender, int x, int y, DWORD key) {
	CLoginScene* pkScene = dynamic_cast<CLoginScene*>(CGameApp::GetCurScene());
	if (!pkScene) {
		return;
	}

	CList* activeRegion = dynamic_cast<CList*>(pSender);
	if (!activeRegion) {
		return;
	}

	for (auto region : lstRegion) {
		if (region != activeRegion) {
			continue;
		};
		const char* selectedServerName = activeRegion->GetItems()->GetSelect()->GetItem()->GetBegin()->GetString();
		const CServerGroupInfo* info = GetServerGroupInfo(selectedServerName);
		if (!info) {
			return;
		}

		int RegionIndex = GetReginIndex(info->szRegion);
		pkScene->SetCurSelRegionIndex(RegionIndex);

		int ServerGroupCnt = GetCurServerGroupCnt(RegionIndex);
		if (ServerGroupCnt == 0) {
			break;
		}

		int ServerIndex = activeRegion->GetItems()->GetSelect()->GetIndex();
		pkScene->SetCurSelServerIndex(ServerIndex);

		pkScene->ShowLoginForm();
		break;
	}
}

void CLoginScene::_evtEnter(CGuiData* pSender) // added by billy
{
	CLoginScene* pScene = dynamic_cast<CLoginScene*>(CGameApp::GetCurScene());
	if (!pScene) {
		return;
	}
	pScene->LoginFlow();
}

void CLoginScene::InitRegionServerLists() {
	for (auto& list : lstRegion) {
		list->GetItems()->Clear();
	}

	ReginListMap& ListMap = GetReginListMap();

	// Init Region names
	for (int Index = 1; Index <= MAX_REGION_LIST; ++Index) {
		auto iter = ListMap.find(Index);
		if (iter != ListMap.end()) {
			ReginList& regionList = iter->second;
			for (auto it : regionList) {
				regionNames[Index - 1]->SetCaption(it.c_str());
			}
		}
	}

	// Init server names
	for (int RegionIndex = 0; RegionIndex < GetRegionCnt(); ++RegionIndex) {
		const int ServerCount = GetCurServerGroupCnt(RegionIndex);
		for (int ServerIndex = 0; ServerIndex < ServerCount; ++ServerIndex) {
			const char* ServerName = GetCurServerGroupName(RegionIndex, ServerIndex);
			lstRegion[RegionIndex]->Add(ServerName);
		}
	}

	// Hide region header when region is empty
	for (int i = 0; i < MAX_REGION_LIST; ++i) {
		if (lstRegion[i]->GetItems()->GetCount() < 1) {
			regionNames[i]->SetCaption("");
		}
	}

	//选择默认的第一个选项
	SetCurSelRegionIndex(0);
	CListItems* items = lstRegion[0]->GetItems();
	if (!items)
		return;
	items->Select(GetCurSelRegionIndex());
}

BOOL CLoginScene::_InitUI() {

	// Initialize Server Selection UI
	{
		frmArea = CFormMgr::s_Mgr.Find("frmArea");
		if (!frmArea) {
			return false;
		}
		frmArea->evtEntrustMouseEvent = _evtRegionFrm;

		const char* luaRegionNames[] = {
			"region0Name",
			"region1Name",
			"region2Name",
		};
		const char* luaRegionLists[] = {
			"lstRegion0",
			"lstRegion1",
			"lstRegion2",
		};

		for (int index = 0; index < MAX_REGION_LIST; ++index) {
			regionNames[index] = static_cast<CLabelEx*>(frmArea->Find(luaRegionNames[index]));
			if (!regionNames[index]) {
				return false;
			}
		}

		for (int index = 0; index < MAX_REGION_LIST; index++) {
			lstRegion[index] = static_cast<CList*>(frmArea->Find(luaRegionLists[index]));
			if (lstRegion[index]) {
				lstRegion[index]->evtListMouseDown = _evtRegionLDBDown;
			}
		}

		InitRegionServerLists();
		if (!g_TomServer.bEnable) {
			frmArea->SetIsShow(true);
		}
	}

	{ // Initialize login UI
		frmAccount = CFormMgr::s_Mgr.Find("frmAccount");
		if (!frmAccount) {
			return false;
		}
		frmAccount->evtEntrustMouseEvent = _evtLoginFrm;

		chkID = static_cast<CCheckBox*>(frmAccount->Find("chkID"));
		if (!chkID) {
			return false;
		}
		m_bSaveAccount = false;
		//检测上次时候选中了账号
		char szChkID[128] = {0};
		string strChkID;
		ifstream inCheck("user\\checkid.txt");
		if (inCheck.is_open()) {
			while (!inCheck.eof()) {
				inCheck.getline(szChkID, 128);
				strChkID = szChkID;
				int nCheck = Str2Int(strChkID);
				m_bSaveAccount = (nCheck == 1) ? true : false;
				chkID->SetIsChecked(m_bSaveAccount);
			}
		} else {
			m_bSaveAccount = true;
			chkID->SetIsChecked(m_bSaveAccount);
		}

		edtID = dynamic_cast<CEdit*>(frmAccount->Find("edtID"));

		if (!edtID)
			return false;
		m_sSaveAccount = "";
		//读入数据, 在上次chkID按下时候 ，把帐号写入到了username.txt文件， 现在读入该帐号
		char szName[128] = {0};
		ifstream in("user\\username.txt");

		_bAutoInputAct = FALSE;
		if (in.is_open()) {
			while (!in.eof()) {
				in.getline(szName, 128);
			}
			_bAutoInputAct = TRUE;
		}
		m_sSaveAccount = string(szName);
		edtID->SetCaption(m_sSaveAccount.c_str());

		if (edtID) {
			edtID->evtEnter = _evtEnter;
			edtID->SetIsWrap(true);
		}

		edtPassword = dynamic_cast<CEdit*>(frmAccount->Find("edtPassword"));
		if (edtPassword) {
			edtPassword->SetCaption("");
			edtPassword->SetIsPassWord(true);
			edtPassword->SetIsWrap(true);
			edtPassword->evtEnter = _evtEnter;
		}

		// 添加软键盘界面
		frmKeyboard = CFormMgr::s_Mgr.Find("frmKeyboard");
		if (!frmKeyboard) {
			return false;
		}

		chkShift = (CCheckBox*)frmKeyboard->Find("chkShift");
		if (!chkShift) {
			return false;
		}

		// 设置软键盘界面鼠标点击事件处理
		frmKeyboard->evtEntrustMouseEvent = _evtKeyboardFromMouseEvent;

		if (edtID) {
			edtID->evtActive = _evtAccountFocus;
		}

		if (edtPassword) {
			edtPassword->evtActive = _evtAccountFocus;
		}
	}

	{ // Logo images seen in the background
		imgLogo1 = static_cast<CImage*>(frmAccount->Find("imgLogo1"));
		if (!imgLogo1) {
			return false;
		}

		imgLogo2 = static_cast<CImage*>(frmAccount->Find("imgLogo2"));
		if (!imgLogo2) {
			return false;
		}
	}

	return TRUE;
}

bool CLoginScene::IsValidCheckChaName(const char* name) {
	if (!::IsValidName(name, (unsigned short)strlen(name))) {
		g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_51));
		return false;
	}
	return true;

	const char* s = name;
	int len = (int)strlen(s);
	bool bOk = true;

	for (int i = 0; i < len; i++) {
		if (s[i] & 0x80) {
			if (!(s[i] == -93)) //用于处理是否是双字节的字母
			{
				i++;
			} else {
				bOk = false;
				i++;
				break;
			}
		} else {
			if (!(isdigit(s[i]) || isalpha(s[i]))) {
				bOk = false;
				break;
			}
		}
	}

	if (!bOk)
		g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_52));

	return bOk;
}

bool CLoginScene::_CheckAccount() {
	// 本地检查用户名和密码
	if (strlen(edtID->GetCaption()) == 0) {
		g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_174));
		return false;
	}
	if (!IsValidCheckChaName(edtID->GetCaption()))
		return false;

	if (strlen(edtPassword->GetCaption()) <= 4) {
		g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_175));
		return false;
	}

	// 保存用户名
	SaveUserName(*chkID, *edtID);

	m_sUsername = edtID->GetCaption();
	m_sPassword = edtPassword->GetCaption();

	return true;
}

bool CLoginScene::_Bill() {

	m_sPassport = "nobill";
	return true;
}

void CLoginScene::_Connect() {
	CGameApp::Waiting(true);

	_eState = eLoginState::Connect;

	if (g_TomServer.bEnable) {
		CS_Connect(g_TomServer.szServerIP.c_str(), g_TomServer.nPort, g_Config.m_nConnectTimeOut);
		return;
	}

	//如果没有得到随机选择Gate的Ip
	const char* selectGateIP = SelectGroupIP(m_iCurSelRegionIndex, m_iCurSelServerIndex);
	if (!selectGateIP) {
		LG("connect", RES_STRING(CL_LANGUAGE_MATCH_180), m_iCurSelRegionIndex, m_iCurSelServerIndex);
	} else {
		CS_Connect(selectGateIP, 1973, g_Config.m_nConnectTimeOut);
	}
	//#endif
}

void CLoginScene::LoginFlow() {
	////////////////////////////////////////
	//
	//  By Jampe
	//  合作商用户名密码处理
	//  2006/5/19
	//
	switch (g_cooperate.code) {
	case COP_OURGAME:
	case COP_SINA:
	case COP_CGA: {
		m_sUsername = g_cooperate.uid;
		m_sPassword = g_cooperate.pwd;
	} break;
	case 0:
	default: {
		if (!_CheckAccount())
			return;
	} break;
	}
	//  end
	if (!_Bill())
		return;
	_Connect();
}

void CLoginScene::_Login() {
	_eState = eLoginState::Account;

	if (!m_sUsername.empty() && !m_sPassword.empty()) {
		//连接服务器

		//确定后发送消息
		CS_Login(m_sUsername.c_str(), m_sPassword.c_str(), m_sPassport.c_str());

		CGameApp::Waiting();
	}
}

void CLoginScene::SaveUserName(CCheckBox& chkID, CEdit& edtID) {
	//创建新文件夹
	if (!CreateDirectory("user", NULL)) {
	}

	m_bSaveAccount = chkID.GetIsChecked();
	m_sSaveAccount = string(edtID.GetCaption());

	//写文件
	FILE* fchk;
	fopen_s(&fchk, "user\\checkid.txt", "wb");
	if (fchk) {
		fwrite(m_bSaveAccount ? "1" : "0", strlen("1"), 1, fchk);
		fclose(fchk);
	}

	FILE* fp;
	fopen_s(&fp, "user\\username.txt", "wb");
	if (fp) {
		if (m_bSaveAccount)
			fwrite(m_sSaveAccount.c_str(), m_sSaveAccount.size(), 1, fp);
		else
			fwrite("", 0, 1, fp);

		fclose(fp);
	}
}

void CLoginScene::_evtVerErrorFrm(CCompent* pSender, int nMsgType, int x, int y, DWORD key) {
	g_pGameApp->SetIsRun(false);

	if (nMsgType != CForm::mrYes) {
		// 弹开一个网页
		if (strlen(g_Config.m_szVerErrorHTTP) == 0) {
			return;
		}

		::ShellExecute(NULL, "open",
					   g_Config.m_szVerErrorHTTP,
					   NULL, NULL, SW_SHOW);
		return;
	}

	// 自动更新
	extern bool g_IsAutoUpdate;
	g_IsAutoUpdate = true;
}

void CLoginScene::Error(int error_no, const char* error_info) {
	CGameApp::Waiting(false);
	LG("error", "%s Error, Code:%d, Info: %s", error_info, error_no, g_GetServerError(error_no));

	if (ERR_MC_VER_ERROR == error_no && !g_TomServer.bEnable) {
		CBoxMgr::ShowSelectBox(_evtVerErrorFrm, RES_STRING(CL_LANGUAGE_MATCH_181), true);
		return;
	}

	g_pGameApp->MsgBox("%s", g_GetServerError(error_no));
}

void CLoginScene::ShowRegionList() {
	CS_Disconnect(DS_DISCONN);

	if (frmKeyboard) {
		ShowKeyboard(false);
	}

	if (frmAccount) {
		frmAccount->SetIsShow(false);
	}

	InitRegionServerLists();
	if (frmArea) {
		frmArea->SetIsShow(true);
	}
}

// add by Philip.Wu  2006-06-05
// 软键盘的鼠标点击按钮事件
void CLoginScene::_evtKeyboardFromMouseEvent(CCompent* pSender, int nMsgType, int x, int y, DWORD dwKey) {
	if (!edtFocus) {
		return;
	}

	CLoginScene* pLoginScene = dynamic_cast<CLoginScene*>(CGameApp::GetCurScene());
	if (!pLoginScene) {
		return;
	}

	string strText = edtFocus->GetCaption();
	string strName = pSender->GetName();
	if (strName.empty()) {
		return;
	}

	// 按键消息的处理
	if (strName == "btnClose" || strName == "btnYes") // 关闭软键盘
	{
		if (frmKeyboard->GetIsShow()) {
			ShowKeyboard(false);
		}
	} else if (strName == "btnDel") // 删除最后一个字符
	{
		if (strText.size() > 0) {
			strText.resize(strText.size() - 1);
			edtFocus->SetCaption(strText.c_str());
		}
	} else if (strName == "chkShift") // 大小转换
	{
	} else if (strName == "btnOther101") // 第一排特别字符键
	{
		strText += '~';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther102") {
		strText += '!';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther103") {
		strText += '@';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther104") {
		strText += '#';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther105") {
		strText += '$';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther106") {
		strText += '%';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther107") {
		strText += '^';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther108") {
		strText += '&';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther109") {
		strText += '*';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther110") {
		strText += '(';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther111") {
		strText += ')';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther112") {
		strText += '_';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther113") {
		strText += '+';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther114") {
		strText += '|';
		edtFocus->SetCaption(strText.c_str());
	}

	else if (strName == "btnOther201") // 第二排特别字符键
	{
		strText += '`';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther202") {
		strText += '-';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther203") {
		strText += '=';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther204") {
		strText += '\\';
		edtFocus->SetCaption(strText.c_str());
	}

	else if (strName == "btnOther301") // 第三排特别字符键
	{
		strText += '{';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther302") {
		strText += '}';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther303") {
		strText += '[';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther304") {
		strText += ']';
		edtFocus->SetCaption(strText.c_str());
	}

	else if (strName == "btnOther401") // 第四排特别字符键
	{
		strText += ':';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther402") {
		strText += '\"';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther403") {
		strText += ';';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther404") {
		strText += '\'';
		edtFocus->SetCaption(strText.c_str());
	}

	else if (strName == "btnOther501") // 第五排特别字符键
	{
		strText += '<';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther502") {
		strText += '>';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther503") {
		strText += '?';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther504") {
		strText += ',';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther505") {
		strText += '.';
		edtFocus->SetCaption(strText.c_str());
	} else if (strName == "btnOther506") {
		strText += '/';
		edtFocus->SetCaption(strText.c_str());
	}

	else // 功能键全部排除后，剩下的是输入字符键按下
	{
		char cAdd = strName.at(strName.size() - 1);

		// 判断是否是数字或字母（暂时偷懒方式处理）
		if (('0' <= cAdd && cAdd <= '9')) {
			strText += cAdd;
			edtFocus->SetCaption(strText.c_str());
		} else if ('A' <= cAdd && cAdd <= 'Z') {
			if (chkShift->GetIsChecked()) {
				// 大写
				strText += cAdd;
			} else {
				// 小写
				strText += cAdd + 32;
			}
			edtFocus->SetCaption(strText.c_str());
		}
	}
}

// add by Philip.Wu  2006-06-07
// 编辑框激活事件（保存下激活的编辑框）
void CLoginScene::_evtAccountFocus(CGuiData* pSender) {
	CEdit* edtTemp = dynamic_cast<CEdit*>(pSender);
	if (edtTemp) {
		edtFocus = edtTemp;
	}
}

void CLoginScene::ShowKeyboard(bool bShow) {
	frmKeyboard->SetIsShow(bShow);

	//imgLogo1->SetIsShow(! bShow);
	//imgLogo2->SetIsShow(! bShow);
}
