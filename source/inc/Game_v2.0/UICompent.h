//--------------------------------------------------------------
// 名称:控件接口类
// 作者:lh 2004-07-08
// 用途:决定所属表单,当前激活控件,键盘鼠标的支持,焦点事件
// 最后修改日期:2004-10-09
//--------------------------------------------------------------
#pragma once
#include "uiguidata.h"
#include "uiform.h"
#include "uicursor.h"

namespace GUI {
class CCommandObj;
using UseComandEvent = void (*)(CCommandObj* pItem, bool& isUse);

enum eAccept {
	enumAccept, // 接收
	enumRefuse, // 拒绝
	enumFast,   // 仅接收快捷方式
};

enum class eMouseAction {
	None,  // Ã»ÓÐµãÔÚ¿Ø¼þÉÏ
	Gui,   // Õý³£µã»÷µÄGUIÉÏ
	Skill, // ¿ÉÊ¹ÓÃ¼¼ÄÜ,µ«²»ÄÜµã´©
	Drill, // ¿Ø¼þ¿É±»µã´©
};

class CCommandObj;
class CContainer;
class CCompent : public CGuiData {
public:
	friend class CFormMgr;
	friend class CForm;
	friend class CContainer;

public:
	CCompent(CForm& frmOwn);
	CCompent(const CCompent& rhs);
	CCompent& operator=(const CCompent& rhs);
	virtual ~CCompent();
	GUI_CLONE(CCompent)

	bool MouseRun(int x, int y, MouseClickState key) override;
	void Refresh() override;
	virtual void FrameMove(DWORD dwTime) {}
	virtual bool IsFrameMove() { return false; }

	eCompentAlign GetAlign() { return _align; }
	void SetAlign(eCompentAlign v) override;
	void SetIsDrag(bool v) override;
	void SetIsShow(bool v) override;

	void SetParent(CGuiData* p) override;

	void SetForm(CForm* f);
	CForm* GetForm() override { return _frmOwn; }

	CCompent* Find(const char* str) override { return _frmOwn->Find(str); }

	virtual CCompent* GetHintCompent(int x, int y); // 返回有提示的控件

	virtual CCompent* GetHitCommand(int x, int y) { return nullptr; }				// 为了兼容是容器的情况下,返回最底层的Compent
	virtual UseComandEvent GetUseCommantEvent() { return nullptr; }					// 这个事件在item执行时，从item里回调
	virtual eAccept SetCommand(CCommandObj* p, int x, int y) { return enumRefuse; } // 是否可接受拖动来的command

public:
	virtual void OnActive(); // 当激活时
	virtual void OnLost() {
		if (evtLost)
			evtLost(this);
	} // 当失去焦点时

	virtual bool OnKeyDown(int key) { return false; }
	virtual bool OnChar(char c) { return false; }

	virtual bool IsHandleMouse() { return false; } // 是否响应鼠标

	static void SetActive(CCompent* v);
	static CCompent* GetActive() { return _pActive; }

	void SetIsFocus(bool v) { _IsFocus = v; }
	bool GetIsFocus() { return _IsFocus; }

	void AddForm();

	eMouseAction GetMouseAction() { return _eMouseAction; }
	void SetMouseAction(eMouseAction v) { _eMouseAction = v; }

	static CCompent* GetLastMouseCompent() { return _pLastMouseCompent; }

public:
	GuiEvent evtLost{nullptr};   // 丢失焦点的事件
	GuiEvent evtActive{nullptr}; // 得到焦点的事件

protected:
	void _AddForm() override; // 将自己加入到所属的Form中
	void _SetActive();

private:
	void _Copy(const CCompent& rhs);

protected:
	static CCompent* _pActive; // 所有控件中仅能有一个控件能被激活

protected:
	CForm* _frmOwn; // 所属的表单,由这个表单负责显示，处理事件

	bool _IsFocus{false}; // 是否响应键盘焦点

	eCompentAlign _align{caNone};
	bool _isChild{false};		// 当前控件是子控件
	eMouseAction _eMouseAction{eMouseAction::Gui}; // 鼠标行为

	static CCompent* _pLastMouseCompent;
};

// 容器类,用于同步显示其上的许多CCompent
class CContainer : public CCompent {
public:
	CContainer(CForm& frmOwn);
	CContainer(const CContainer& rhs);
	CContainer& operator=(const CContainer& rhs);
	virtual ~CContainer();
	GUI_CLONE(CContainer)

	void Init() override;
	void Render() override;
	void Refresh() override;
	bool MouseRun(int x, int y, MouseClickState key) override;
	void SetAlpha(BYTE alpha) override;
	CCompent* Find(const char* str) override;
	CCompent* GetHitCommand(int x, int y) override;
	CCompent* GetHintCompent(int x, int y) override;
	void FrameMove(DWORD dwTime) override;

	bool AddCompent(CCompent* p);
	CCompent* GetIndex(unsigned int index);

	int GetIndex(CCompent* p); // 根据p,得到它在容器中的索引号,失败返回-1
	int GetSize() { return (int)_items.size(); }

	void ForEach(CompentFun pFun);

	eAccept SetCommand(CCommandObj* p, int x, int y) override { return enumRefuse; }

private:
	void _SetSelf(const CContainer& rhs);

protected:
	using items = std::vector<CCompent*>;
	items _items;
	items _mouse; // 处理控件的鼠标
};

// 类内联函数
inline void CCompent::_SetActive() {
	if (!_isChild && _IsFocus && GetActive() != this)
		SetActive(this);
}

inline void CCompent::SetForm(CForm* f) {
	_frmOwn = f;
	_pParent = f;
	_isChild = false;
}

inline CCompent* CContainer::GetIndex(unsigned int index) {
	return index < _items.size() ? _items[index] : nullptr;
}

} // namespace GUI
