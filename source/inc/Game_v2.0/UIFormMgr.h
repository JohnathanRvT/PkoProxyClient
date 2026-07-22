//--------------------------------------------------------------
// 名称:表单管理类
// 作者:lh 2004-07-08
// 设计思想:管理所有表单
// 用途:显示,隐藏,查找表单,切换表单模板,提供全局钩子回调接口
// 最后修改日期:2004-10-09
//--------------------------------------------------------------
#pragma once
#include "uicompent.h"
#include "uiform.h"

namespace GUI {
class CFormMgr;
using FormMgrEvent = bool (*)(CFormMgr* pSender);

using KeyDownEvent = bool (*)(int& key);
using KeyCharEvent = bool (*)(char& key);
using MouseEvent = bool (*)(int& x, int& y, MouseClickState mouse);
using MouseScrollEvent = bool (*)(int& nScroll);
using HotKeyEvent = bool (*)(char& key, int& control); // 返回true中断

class CFormMgr {
	friend class CForm;

public:
	CFormMgr();
	~CFormMgr();

	static void SetDebugMode(bool v) { _IsDebugMode = v; }								   //设置是否启动调试模式（由控制台命令调用）-added by Arcol
	void ShowDebugInfo();																   //显示调试信息 -added by Arcol
	static void SetDrawFrameInDebugMode(bool v) { _IsDrawFrameInDebugMode = v; }		   //设置在调试模式下是否显示窗口边框 -added by Arcol
	static void SetDrawBackGroundInDebugMode(bool v) { _IsDrawBackgroundInDebugMode = v; } //置在调试模式下是否显示底色 -added by Arcol

	bool AddForm(CForm* form, int templete = 0);

	bool SetFormTempleteMax(int n);
	unsigned int GetFormTempleteMax() { return (unsigned int)_showforms.size(); }
	int GetFormTempletetNum() { return _nTempleteNo; }

	bool SwitchTemplete(int n); // 切换当前模板,如果为-1,则切换到默认模板

public:
	bool Init(HWND hWnd);
	void Clear();

	void FrameMove(int x, int y, MouseClickState dwMouseKey, DWORD dwTime);
	void Render();
	void RenderHint(int x, int y);
	bool HandleWindowMsg(DWORD dwMsg, DWORD dwParam1, DWORD dwParam2);

	bool OnKeyDown(int key);
	bool OnKeyChar(char key);
	bool OnHotKey(char key, int control);

	bool MouseScroll(int nScroll);
	void Refresh();

	void MouseReset();

	CForm* GetHitForm(int x, int y);

	CForm* Find(const char*);			  // 在当前模板中，查找Form
	CForm* Find(const char* str, int no); // 在no模板中，查找Form
	CForm* FindAll(const char*);		  // 在所有表单中，查找Form

	CForm* FindESCForm(); // 查找可用ESC关闭的Form
	int ModalFormNum() { return (int)_modal.size(); }

	using FormFun = void (*)(CForm* pSender);
	void ForEach(FormFun pFun); // 所有Form都执行一次pFun函数

public:
	void SetEnabled(bool v) { _bEnabled = v; }
	bool GetEnabled() { return _bEnabled; }
	void ResetAllForm();
	void SetScreen();

	void SetEnableHotKey(int flag, bool v);							// 西门文档修改
	bool GetEnableHotKey() { return _nEnableHotKey == 0xFFFFFFFF; } // 西门文档修改

	static bool IsMouseInGui() { return _eMouseAction == eMouseAction::Gui; }
	static eMouseAction GetMouseAction() { return _eMouseAction; }

public:										 // 全局钩子
	bool AddFormInit(FormMgrEvent pInitFun); // 读入脚本后,初始化时事件

	bool AddKeyDownEvent(KeyDownEvent event);
	bool DelKeyDownEvent(KeyDownEvent event);

	bool AddKeyCharEvent(KeyCharEvent event);
	bool DelKeyCharEvent(KeyCharEvent event);

	bool AddMouseEvent(MouseEvent event);
	bool DelMouseEvent(MouseEvent event);

	bool AddMouseScrollEvent(MouseScrollEvent event);
	bool DelMouseScrollEvent(MouseScrollEvent event);

	bool AddHotKeyEvent(HotKeyEvent event);
	bool DelHotKeyEvent(HotKeyEvent event);

public:
	static CFormMgr s_Mgr;

private:
	bool _AddMemory(CForm* form);
	bool _DelMemory(CForm* form);

	bool _MouseRun(int x, int y, MouseClickState mouse);

	void _ShowModal(CForm* form);
	void _ModalClose(CForm* form);

	void _SetNewActiveForm(); // 当前激活form丢失后，要寻找一个新的form激活

	void _InitFormID();

	void _DelShowForm(CForm* frm);
	void _AddShowForm(CForm* frm);
	void _UpdataShowForm(CForm* frm);

	void _ActiveCompent();

private:
	using vfrm = std::list<CForm*>;
	vfrm _allForms;
	vfrm* _forms; // "non-modal display form resource, the first one is the currently active form"
	vfrm _modal;  // "modal box being displayed"
	vfrm _show;   // "The non-modal box being displayed"

	using frmtemplete = std::vector<vfrm*>;
	frmtemplete _showforms;
	vfrm _defaulttemplete;

	bool _bEnabled{false};
	bool _bInit{false};
	int _nEnableHotKey{static_cast<int>(0xFFFFFFFF)}; // 西门文档修改

	using vkeydowns = std::vector<KeyDownEvent>;
	vkeydowns _OnKeyDown;

	using vkeychars = std::vector<KeyCharEvent>;
	vkeychars _OnKeyChar;

	using vmouses = std::vector<MouseEvent>;
	vmouses _OnMouseRun;

	using vscrolls = std::vector<MouseScrollEvent>;
	vscrolls _OnMouseScroll;

	using vinits = std::vector<FormMgrEvent>;
	vinits _vinits;

	using vhotkey = std::vector<HotKeyEvent>;
	vhotkey _vhotkey;

	int _nTempleteNo{-1}; // 当前的模板号

	int _nMouseHover{0}; // 鼠标在不做任何操作已经多少时间

	CGuiData* _pHintGui{nullptr};

	static eMouseAction _eMouseAction;
	static bool _IsDebugMode;				  // 是否在调试模式 -added by Arcol
	static bool _IsDrawFrameInDebugMode;	  // 仅用于调试模式 -added by Arcol
	static bool _IsDrawBackgroundInDebugMode; // 仅用于调试模式 -added by Arcol

private:
	void _DelForm(vfrm& list, CForm* frm);
};

inline void CFormMgr::MouseReset() {
	_nMouseHover = 0;
}

} // namespace GUI
