//--------------------------------------------------------------
// 名称:表单类
// 作者:lh 2004-07-08
// 设计思想:控件容器
// 用途:热键显示隐藏,拖动
// 最后修改日期:2004-10-09
//--------------------------------------------------------------
#pragma once
#include "uiguidata.h"
#include "Mouse.h"

namespace GUI {

using GuiFormCloseEvent = void (*)(CForm* pForm, bool& IsClose);
using GuiFormBeforeShowEvent = void (*)(CForm* pForm, bool& IsShow);
using GuiFormEscCloseEvent = void (*)(CForm* pForm);

// 用于在表单中的按钮控件事件回调,
// pSender为按钮,nMsgType为eModalResult
using FormMouseEvent = void (*)(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);

class CTextButton;
class CMenu;
class CForm : public CGuiData {
	friend class CCompent;
	friend class CContainer;
	friend class CFormMgr;

public:
	enum eModalResult {
		mrNone,
		mrClose,
		mrYes,
		mrNo,
		mrOk,
		mrCancel,
		mrEnd,
	};
	enum eFormStyle //表单的风格: 普通,全居中,左右居中,上下居中,居左, 居右,居上,居下
	{
		fsNone,
		fsAllCenter,
		fsXCenter,
		fsYCenter,
		fsLeft,
		fsRight,
		fsTop,
		fsBottom,
		fsLeftTop,
		fsRightTop,
		fsLeftBottom,
		fsRightBottom,

	};

	CForm();
	CForm(const CForm& rhs);
	CForm& operator=(const CForm& rhs);
	~CForm();
	GUI_CLONE(CForm)

	void Init() override;
	void Render() override;
	void Refresh() override;

	bool MouseRun(int x, int y, MouseClickState key) override;
	void SetIsShow(bool v) override {
		if (v)
			Show();
		else
			Close();
	}
	void SetParent(CGuiData* p) override;
	void Reset() override;

	bool MenuMouseRun(int x, int y, MouseClickState key);

public:
	void ForEach(CompentFun pFun); // Form中所有Compent都执行一次pFun函数
	void FrameMove(int nCount);

public:
	void Show();
	void ShowModal();
	void Hide();
	void Close();
	CCompent* GetHitCommand(int x, int y);
	CGuiData* GetHintGui(int x, int y);
	void SetStyle(eFormStyle index, int offWidth = 0, int offHeight = 0, bool bRedraw = false);
	int GetFormStyle() { return _formStyle; }

	void SetIsEscClose(bool v) { _IsEscClose = v; }
	bool GetIsEscClose() { return _IsEscClose; }

	CCompent* Find(const char*) override;
	void SetAlpha(BYTE alpha) override;
	void SetIsDrag(bool v) override;

	bool GetIsModal() { return _isModal; }
	eModalResult GetModalResult() { return _modalResult; }
	void SetModalResult(eModalResult v) { _modalResult = v; }

	static CForm* GetActive() { return _pActive; }
	CFramePic* GetFrameImage() { return _pImage; }

	void SetHotKey(char key) { _cHotKey = key; }
	char GetHotKey() { return _cHotKey; }
	void SetHotKeyHandler(GuiHotKeyEvent handler) { evtHotkeyHandler = handler; }

	bool SetNextActiveCompent(bool isNext = true); // 激活表单里下一个控件,isNext=false为上一个控件

	void SetActiveCompent(CCompent* pActive);

	// 得到当前菜单
	void PopMenu(CMenu* pMenu, int x = 0, int y = 0);

	void SetPointer(void* v) { _pPointer = v; }
	void* GetPointer() { return _pPointer; }

	void SetEnterButton(CTextButton* v) { _pEnterButton = v; }
	// Add by lark.li 20090220 begin
	void SetInMainForm(bool isIn = false) { _isInMainForm = isIn; }
	// End

public:
	bool OnChar(char key);
	bool OnKeyDown(int key);
	void OnSetScreen();

	CCompent* FindActiveCompent();

	int ClearChild(); // 解除所有与子窗口的关系,返回解除个数

public:									 // 事件
	GuiEvent evtShow{nullptr};			 // 显示时
	GuiEvent evtHide{nullptr};			 // 隐藏时
	GuiEvent evtActive{nullptr};		 // 得到焦点时
	GuiEvent evtLost{nullptr};			 // 失去焦点时
	GuiMouseEvent evtMouseDown{nullptr}; // 鼠标有键在窗口上按下时
	GuiKeyDownEvent evtKeyDown{nullptr};
	GuiKeyCharEvent evtKeyChar{nullptr};
	GuiEvent evtOnSetScreen{nullptr};
	GuiMouseEvent evtMouseDragEnd{nullptr};   // 拖动结束
	GuiHotKeyEvent evtHotkeyHandler{nullptr}; //热键处理 返回true表示已经处理，false表示未处理

	GuiFormEscCloseEvent evtEscClose{nullptr};
	GuiFormBeforeShowEvent evtBeforeShow{nullptr};
	GuiFormCloseEvent evtClose{nullptr}; // 关闭时调用

public: // 用于控件的回调事件
	FormMouseEvent evtEntrustMouseEvent{nullptr};

protected:
	void _OnActive();
	bool _AddCompent(CCompent* c, bool isCheck);
	bool _AddFrameMove(CCompent* c, bool isCheck);

	void _TempleteInit();  // 进入模板前的初始化
	void _TempleteClear(); // 模板退出后的清除

protected:
	void* _pPointer{nullptr};

	using vcs = std::vector<CCompent*>;
	vcs _compents;	// 表单内部的控件,也是显示控件时的顺序
	vcs _mouse;		  // 处理控件的鼠标
	vcs _actives;	 // 可激活的列表
	vcs _allCompents; // 所有控件
	vcs _frames;

	using vfrm = std::list<CForm*>;
	vfrm _childs; // 子窗体

	int _nActiveCompentID{0}; // 当前激活的Compent索引

	bool _isModal{false}; // 显示时是否按模态显示

	static CForm* _pActive;

	eModalResult _modalResult{mrNone}; // 窗口模态
	eFormStyle _formStyle{fsNone};

	bool _IsEscClose{true}; // 为true,则按esc时关闭表单

	CFramePic* _pImage{new CFramePic(this)}; // 背景图,共二帧，一帧为普通状态，一帧为激活状态
	CTextButton* _pEnterButton{nullptr};

protected:
	char _cHotKey{0};		   // 激活的热键
	CMenu* _pPopMenu{nullptr}; // 弹出菜单

private:
	void _Copy(const CForm* rhs);
	void _CopyCompent(const CForm* rhs);
	void _ActiveForm(MouseClickState key);

private:
	static void _DragMouseEvent(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _DragMouseEventBegin(CGuiData* pSender, int x, int y, MouseClickState key);
	static int _nDragOffX, _nDragOffY;
	int _nOffWidth{0};  //-Added by Arcol
	int _nOffHeight{0}; //-Added by Arcol
	// Add by lark.li 20090220 begin
	bool _isInMainForm{false};
	// End
};

inline void CForm::_ActiveForm(MouseClickState key) {
	if (((key & MouseClickState::Down) != MouseClickState()) && !GetIsModal() && _pActive != this) {
		_OnActive();
	}
}

inline void CForm::Close() {
	_modalResult = mrClose;
	Hide();
}

} // namespace GUI
