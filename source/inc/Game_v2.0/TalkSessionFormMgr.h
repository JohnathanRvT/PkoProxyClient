#pragma once

//#define MAX_SESSION_FORM 4

#include "UIMemo.h"
#include <memory>

enum class eTalkSessionStyle {
	Hidden,	// 隐藏样式
	Inform,	// 通知样式
	Normal,	// 正常模式
	Small,	 // 小窗口模式
	Minimized, // 最小模式
};

struct stNetSessCreate;

class CSessionMember {
public:
	CSessionMember(stNetSessCreate* pMember);
	~CSessionMember() = default;

	DWORD GetID() const { return _nID; }
	DWORD GetIconID() const { return _nIcon_id; }
	const char* GetName() const { return _strName.c_str(); }
	const char* GetMotto() const { return _strMotto.c_str(); }
	void SetMotto(const char* szMotto) { _strMotto = szMotto; }
	/* CHANGED(Snre3n) Implementation not in use
	CTeam*      GetTeam() { return _pTeam; }

	void        SetSessionFormPointer(void* p) { _pPointer = p; }
	void*       GetSessionFormPointer() { return _pPointer; }
*/

private:
	std::string _strName;
	std::string _strMotto;
	DWORD _nID{0};
	DWORD _nIcon_id{0};
	/* CHANGED(Snre3n) Implementation not in use
	//void*           _pPointer;
	//CTeam*          _pTeam;
	//CMemberData*    _pData;
*/
};

class CTalkSessionFormMgr;
class CTalkSessionForm {
	friend CTalkSessionFormMgr;

	struct sTalkMsg {
		std::string name;
		std::string motto;
		std::string text;
	};

public:
	CTalkSessionForm(DWORD sessionID, eTalkSessionStyle style);
	~CTalkSessionForm();

	CForm* GetForm();
	void RefreshForm();

	bool ChangeStyle(eTalkSessionStyle style);
	eTalkSessionStyle GetStyle() const { return m_Style; }

	void StopFlash();
	bool GetIsFlash() const { return m_bFlash; }
	void SetIsFlash(bool isFlash) {
		m_bFlash = isFlash;
		m_nFlashTimes = 0;
	}

	int GetTotalMembers() const;
	bool hasMember(DWORD chaID) const;
	bool hasMember(std::string name) const;
	CSessionMember* GetMemberByID(DWORD nID) const;
	CSessionMember* GetMemberByIndex(int nIndex) const;
	void AddMembers(stNetSessCreate* pArrMembers[], int memberNum = 1);
	bool DelMember(DWORD chaID);

	DWORD GetSessionID() const { return m_dwSessioinID; }
	void SetActiveSession(bool bActive, DWORD newID = 0) {
		m_bActive = bActive;
		m_dwSessioinID = newID;
	}
	bool IsActiveSession() const { return m_bActive; }

	void AddMemberToBuffer(std::string name) { m_strWaitForAdded += name + "\n"; }
	void ClearBuffer() {
		m_strWaitForSend.clear();
		m_strWaitForAdded.clear();
	}

private: // used internally or by CTalkSessionFormMgr through friendship
	void AddMessage(DWORD chaID, std::string talkMsg);
	void SendBufferMsg();
	void SetInformText(const char* name, const char* text);
	std::string GetMemberFromBuffer() {
		std::string str = m_strWaitForAdded;
		m_strWaitForAdded.clear();
		return str;
	}
	void SetData(LPVOID data) { m_pData = data; }
	LPVOID GetData() { return m_pData; }
	int GetInformShowChannel();
	void ReleaseInformShowChannel();
	void OnHotKeyShow();

	CForm* m_pMinimizeForm;

	bool m_bActive;
	bool m_bFlash;
	int m_nFlashTimes;
	CGuiTime* m_pTimer1;
	CGuiTime* m_pTimer2;
	std::string m_strWaitForSend;
	std::string m_strWaitForAdded;
	CForm* m_pForm;
	CForm* m_pNormalForm;
	CForm* m_pSmallForm;
	CForm* m_pInformForm;
	CEdit* m_pEdit;
	CEdit* m_pNormalEdit;
	CEdit* m_pSmallEdit;
	CRichMemo* m_pMemo;
	CRichMemo* m_pNormalMemo;
	//CRichMemo*		m_pSmallMemo;
	//CList*			m_pList;
	//CList*			m_pNormalList;
	//CList*			m_pSmallList;
	CList* m_pNameList;
	CList* m_pNormalNameList;
	CTextButton* m_pInformTextBtn;
	CLabelEx* m_pInformTitle;
	CGrid* m_pFaceGrid;
	static CMenu* m_playerMouseRight;
	DWORD m_dwSessioinID;
	std::string m_strTalkMsg;
	int m_nShowChannel;
	int m_nShowSx;
	int m_nShowSy;
	bool m_bQuitFlag;
	std::vector<std::unique_ptr<CSessionMember>> m_Members;
	eTalkSessionStyle m_Style;
	LPVOID m_pData;
	std::vector<sTalkMsg> m_TalkMessages;

	void ClearMessages();
	static void _OnMouseInformButton(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnMouseNormalButton(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnMouseSmallButton(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnMouseMinimizeButton(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnMouseQuitButton(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnMouseCloseButton(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnMouseSendButton(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnMouseSelectFace(CGuiData* pSender);
	static void _OnTimerInformShowing(CGuiTime* pSender);
	static void _OnMouseFaceListButton(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnMouseCleanText(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnMouseRightPlayerMenu(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnPlayerSelect(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _OnFaceLostFocus(CGuiData* pSender);
	static void _OnMouseSaveButton(CGuiData* pSender, int x, int y, MouseClickState key);
	static bool _OnHotKey(CForm* pSender, char key);
	static void GuiFormEscCloseEvent(CForm* pForm);
};

class CTalkSessionFormMgr {
public:
	struct sApplyMember {
		std::string name;
		static UCHAR numbers;
	};
	CTalkSessionFormMgr();
	~CTalkSessionFormMgr();

	static CTalkSessionForm* CreateSessionForm(DWORD sessionID, stNetSessCreate* pArrMembers[], int memberNum = 1);
	static CTalkSessionForm* CreateHiddenSessionForm(DWORD sessionID, stNetSessCreate* pArrMembers[], int memberNum = 1);
	static void ApplySession(CMember* pArrMember[], int memberNum = 1);
	static void ApplySession(sApplyMember pApplyMembers[]);
	static bool AddToSession(DWORD sessionID, stNetSessCreate* pMember);
	static void LeaveSession(DWORD sessionID, DWORD chaID);
	static void CloseSessionForm(DWORD sessionID);
	static void ClearAll();
	static CTalkSessionForm* GetSessionFormByIndex(int nIndex);
	static CTalkSessionForm* GetSessionFormByID(DWORD sessionID);
	static CTalkSessionForm* GetSessionFormByName(std::string name, int index = 0);
	static CTalkSessionForm* GetSessionFormByForm(CForm* form);
	static void SessionReceiveMsg(DWORD sessionID, DWORD chaID, std::string talkMsg);

	static int GetShowChannel();
	static void ReleaseShowChannel(int n);
	static POINT GetMiniFormPosition(DWORD sessionID);
	static POINT GetNormalFormPosition(DWORD sessionID);

	static void OnClickOpenSession(CItemObj* pItem);
	static void OnClickCloseSession(CTalkSessionForm* pSession);
	static void OnFlashSession();
	static bool OnHotKeyShow(char& key, int& control);

	static void RefreshSessionMember(DWORD chaID, const char* szMotto);

	static bool hasFlashSession();

	static void PrintTraceInfo();
	static DWORD GetSessionCount();

	//static void SetInformSize(RECT rectFrom,RECT rectTitle);
	//static void SetTablePosition(POINT tPosition);
	//static void TableInit();

	//static RECT TableToTitle(int tx,int ty);
	//static RECT TableToForm(int tx,int ty);

	//static void GetFormRect(DWORD sessionID,RECT &rect);
	//static void GetTitleRect(DWORD sessionID,RECT &rect);

protected:
	//static RECT INFORM_FORM;
	//static RECT INFORM_TITLE;
	//static int INFORM_TABLE_W;
	//static int INFORM_TABLE_H;
	//static POINT INFORM_TABLE_POSITION;

	//static CTalkSessionForm* CreateSessionForm(CMember *pMember,eTalkSessionStyle style=enumTalkSessionNormal);
	//static bool AddToSessionForm(CMember *pArrMembers,CTalkSessionForm *sessionForm);
	//static void LeaveSessionForm(CTalkSessionForm *sessionForm);

private:
	static int m_Channel[4];
	static DWORD m_SessionCounts;
	static std::vector<CTalkSessionForm*> m_SessionFormsLink;

	//static PS_Sessions* FindSession(CTalkSessionForm *sessionForm);
};
