#pragma once

#include "Scene.h"
#include "uiguidata.h"
#include "NetProtocol.h"

class CSceneObj;
namespace GUI {
class CForm;
class C3DCompent;
class CGuiData;
class CLabelEx;
class CEdit;
class CTextButton;
class CList;
class CMemo;
} // namespace GUI

/**
 * 摘要：本类主要处理登陆流程中的选择角色场景,包括场景动画和GUI表单.
 * 　　　本类继承自CGameScene类.
 *
 * @author: Michael Chen
 * @date: 2005-04-26
 */
class CSelectChaScene : public CGameScene {
public:
	CSelectChaScene(stSceneInitParam& param);
	~CSelectChaScene();

	virtual void LoadingCall(); // 在装载loading后,刷新
	virtual void SetMainCha(int nChaID);

	static CSelectChaScene& GetCurrScene();

	//删除当前选中的角色
	void DelCurrentSelCha();
	//显示现有角色
	bool SelectCharacters(NetChaBehave* chabehave, int num);
	//新增角色
	bool CreateCha(const string& sName, int nChaIndex, stNetChangeChaPart* part);

	void SelectChaError(int error_no, const char* error_info);

	//向服务器发送删除角色消息
	void SendDelChaToServer(const char szPassword2[]);

	// 更新按钮状态
	void UpdateButton();

	// 获得角色个数
	int GetChaCount() const;

	void ShowWelcomeNotice(bool bShow = true);
	//UI????
	bool _InitUI();

protected:
	//场景相关函数
	virtual void _FrameMove(DWORD dwTimeParam);
	virtual void _Render();

	virtual bool _MouseButtonDown(int nButton);
	virtual bool _MouseButtonDB(int nButton);
	virtual void _KeyDownEvent(int key);

	virtual bool _Init();
	virtual bool _Clear();
	virtual void _RenderUI();

	static void _SelChaFrmMouseEvent(CCompent* pSender, int nMsgType,
									 int x, int y, DWORD dwKey);

	//	static void         _CheckFrmMouseEvent(CCompent *pSender, int nMsgType,
	//                                           int x, int y, DWORD dwKey);

	static void _evtCreateDoublePwdEvent(CCompent* pSender, int nMsgType,
										 int x, int y, DWORD dwKey); // 询问是否要创建二次密码

	static void _evtWelcomeNoticeEvent(CCompent* pSender, int nMsgType,
									   int x, int y, DWORD dwKey); // 欢迎界面 事件处理

	static void _evtCreateOKNoticeEvent(CCompent* pSender, int nMsgType,
										int x, int y, DWORD dwKey); // 首次创建角色成功提示界面 事件处理

	static void _evtChaNameAlterMouseEvent(CCompent* pSender, int nMsgType,
										   int x, int y, DWORD dwKey); // 首次创建角色成功提示界面 事件处理

	//~ 逻辑函数: ============================================================

	//向服务器发送开始游戏消息
	void SendBeginPlayToServer();

	void SetChaDark(CCharacter* pCha);

private:
	struct SelectableCharacter {
		SelectableCharacter(int x, int y, int yaw) : x(x), y(y), yaw(yaw) {}
		string sProfession;
		CCharacter* pCha = nullptr;
		int iLevel = -1;
		int iPos = -1;
		int iFontX = -1, iFontY = -1;
		int x = -1, y = -1, yaw = -1;
		BYTE chaColor[3] = {255, 255, 255};
		bool occupiedPosition = false;
	};
	vector<SelectableCharacter*> selectableCharacters; //????֐?Ľǉ?(??(ז̥)
	SelectableCharacter* m_ptrCurCha = nullptr;

	std::array<lwIPrimitive*, 3> pAure = {}; // Light shine down on selected character effect

	CSceneObj* pObj; //场景对象

	//UI
	CForm* frmSelectCha;
	CTextButton* btnDel;
	CTextButton* btnYes;
	CTextButton* btnCreate;
	CTextButton* btnExit;
	CTextButton* btnAlter;

	CForm* frmCheck;

	CForm* frmWelcomeNotice;  // 定义欢迎界面  该界面仅在当前帐号内无角色时出现
	CForm* frmCreateOKNotice; // 定义首次创建角色成功提示界面  该界面仅在该帐号走完第一个角色的创建流程后显示
	CForm* frmChaNameAlter;   // 定义改名界面  该界面仅对台湾版本有效，请做好备份工作

	//场景渲染和动画相关的
	BYTE _loadtex_flag;  //保存场景渲染和动画的设置
	BYTE _loadmesh_flag; //保存场景渲染和动画的设置

	bool m_isInit;
	bool m_isCreateCha;
};
