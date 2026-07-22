#pragma once
#pragma once
#pragma once
#pragma once

#include "Scene.h"
#include "uiguidata.h"
#include "NetProtocol.h"

#define MAX_SEL_CHA 4
#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 64

static size_t m_iCurSelRegionIndex{1};
static size_t m_iCurSelServerIndex{0};

namespace GUI {
class CForm;
class C3DCompent;
class CGuiData;
class CLabelEx;
class CEdit;
class CTextButton;
class CList;
class CMemo;
class CCheckBox;
class CListView;
class CImage;
} // namespace GUI

class CCharacter;

struct TOM_SERVER {
	TOM_SERVER()
		: szPassport("tom") {
		nPort = 0;
		bEnable = false;
	}

	std::string szServerIP;
	int nPort;
	std::string szUser;
	std::string szPassword;
	std::string szPassport;
	bool bEnable;
};

//  合作商编号
//  By Jampe
#define COP_OURGAME 1001 //  联众
#define COP_SINA 1002	//  新浪
#define COP_CGA 1003	 //  浩方

int constexpr MAX_REGION_LIST = 3;

typedef struct _Cooperate_ {
	long code;
	std::string serv;
	std::string uid;
	std::string pwd;
} Cooperate, *pCooperate;

extern TOM_SERVER g_TomServer;
extern Cooperate g_cooperate;
//#endif

class CLoginScene : public CGameScene {
public:
	CLoginScene(stSceneInitParam& param);

	~CLoginScene();

	void LoginFlow();
	static bool IsValidCheckChaName(const char* name);

	void Error(int error_no, const char* error_info);
	void ShowServerSelection();

	void SetCurSelRegionIndex(size_t iIndex) { m_iCurSelRegionIndex = iIndex; }
	size_t GetCurSelRegionIndex() const { return m_iCurSelRegionIndex; }
	void SetCurSelServerIndex(size_t iIndex) { m_iCurSelServerIndex = iIndex; }
	size_t GetCurSelServerIndex() const { return m_iCurSelServerIndex; }
	void ShowLoginForm();

	bool IsPasswordError() const { return m_bPasswordError; }
	void SetPasswordError(bool bFlag) { m_bPasswordError = bFlag; }
	void InitRegionServerLists();

private:
	void SaveUserName(CCheckBox& chkID, CEdit& edtID);

	static void ShowKeyboard(bool bShow);

protected:
	virtual bool _Init() override;
	virtual void _FrameMove(DWORD dwTimeParam) override;
	virtual void _Render() override;
	virtual void LoadingCall() override;

	BOOL _InitUI();

	bool _Clear() override;
	BOOL _bAutoInputAct;

	void _Login();
	bool _CheckAccount();
	bool _Bill();
	void _Connect();

private:
	static CForm* frmAccount;
	static CForm* frmKeyboard; // add by Philip.Wu  软键盘界面  2006-06-05
	static CForm* frmArea;

	static std::array<CList*, MAX_REGION_LIST> lstRegion;

	static CEdit* edtID;
	static CEdit* edtPassword;
	static CCheckBox* chkID;
	static CEdit* edtFocus;		// add by Philip.Wu  光标激活的编辑框  2006-06-07
	static CCheckBox* chkShift; // add by Philip.Wu  软键盘上的 Shift  2006-06-09

	bool m_bSaveAccount;
	std::string m_sSaveAccount;
	std::string m_sPassport{"nobill"};
	std::string m_sUsername;
	std::string m_sPassword;

private:
	static void _evtEnter(CGuiData* pSender);
	static void _evtRegionLDBDown(CGuiData* pSender, int x, int y, MouseClickState key);

	static void _evtRegionFrm(CCompent* pSender, int state, int x, int y, MouseClickState key);
	static void _evtLoginFrm(CCompent* pSender, int state, int x, int y, MouseClickState key);
	static void _evtVerErrorFrm(CCompent* pSender, int state, int x, int y, MouseClickState key);

	static void CallbackUIEvent_LoginScene(CCompent* pSender, int state, int x, int y, MouseClickState key);

	// add by Philip.Wu  2006-06-05
	// 软键盘界面的鼠标事件
	static void _evtKeyboardFromMouseEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);

	// 编辑框激活事件（保存下激活的编辑框）
	static void _evtAccountFocus(CGuiData* pSender);

	enum class eLoginState {
		Init,
		Connect,
		Account,
		Login,
		Select,
		Play
	};

	eLoginState _eState{eLoginState::Init};
	BYTE _loadtex_flag{9};
	BYTE _loadmesh_flag{9};

	bool m_bPasswordError{false}; //判断密码是否错误

	friend void _GoBack(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);
};
