//----------------------------------------------------------------------
// 名称:快捷命令控件
// 作者:lh 2004-11-02
// 用途:用于容纳一个快捷命令，仅只有一个指向外部实体的快捷方式
// 最后修改日期:
//----------------------------------------------------------------------

#pragma once
#include "UICommandCompent.h"

namespace GUI {
using GuiComEvent = void (*)(CGuiData* pSender, CCommandObj* pItem, bool& isAccept);

class CCommandObj;
class CFastCommand : public CCommandCompent {
public:
	CFastCommand(CForm& frmOwn);
	CFastCommand(const CFastCommand& rhs);
	CFastCommand& operator=(const CFastCommand& rhs);
	~CFastCommand();
	GUI_CLONE(CFastCommand)

	void Render() override;
	bool MouseRun(int x, int y, MouseClickState key) override;
	eAccept SetCommand(CCommandObj* p, int x, int y) override;
	void DragRender() override;
	CCompent* GetHintCompent(int x, int y) override;
	void RenderHint(int x, int y) override;

	static void DelCommand(CCommandObj* p);

	static CFastCommand* FindFastCommand(const CCommandObj* p); // 查找对应的快捷控件

	CCommandObj* GetCommand() { return _pCommand; }
	void DelCommand() {
		if (_pCommand)
			DelCommand(_pCommand);
	}
	void AddCommand(CCommandObj* p);

	void SetKeybind(unsigned short keybind);
	unsigned short GetKeybind();

public:
	GuiComEvent evtChange{nullptr}; // 快捷栏发生变化

protected:
	void _SetSelf();
	static void _DragEnd(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CFastCommand*)(pSender))->_DragEnd(x, y, key);
	}
	void _DragEnd(int x, int y, MouseClickState key);

protected:
	CCommandObj* _pCommand{nullptr};
	unsigned short _pKeybind{0};

	using fasts = std::vector<CFastCommand*>;
	static fasts _fast;
};

// 用于容纳一个命令实体
class COneCommand : public CCommandCompent {
public:
	enum eShowStyle {
		enumSmall, // 正常显示图标方式
		enumSale,  // 显示售出
	};
	COneCommand(CForm& frmOwn);
	COneCommand(const COneCommand& rhs);
	COneCommand& operator=(const COneCommand& rhs);
	~COneCommand();
	GUI_CLONE(COneCommand)

	void Render() override;
	bool MouseRun(int x, int y, MouseClickState key) override;
	void DragRender() override;
	eAccept SetCommand(CCommandObj* p, int x, int y) override;
	CCompent* GetHintCompent(int x, int y) override;
	void RenderHint(int x, int y) override;

	CCommandObj* GetCommand() const { return _pCommand; }
	void DelCommand();

	void AddCommand(CCommandObj* p);

	UseComandEvent GetUseCommantEvent() override { return evtUseCommand; }

	void SetActivePic(CGuiPic* p) { _pActive = p; }
	void SetIsShowActive(bool v) { _IsShowActive = v; }
	void SetShowStyle(eShowStyle v) { _eShowStyle = v; }

public:
	GuiComEvent evtBeforeAccept{nullptr};
	GuiComEvent evtThrowItem{nullptr};
	UseComandEvent evtUseCommand{nullptr};

protected:
	void _SetSelf();
	void _Copy(const COneCommand& rhs);
	static void _DragEnd(CGuiData* pSender, int x, int y, MouseClickState key) {
		((COneCommand*)(pSender))->_DragEnd(x, y, key);
	}
	void _DragEnd(int x, int y, MouseClickState key);

protected:
	CCommandObj* _pCommand{nullptr};
	eShowStyle _eShowStyle{enumSmall};
	bool _IsShowActive{false};

	CGuiPic* _pActive{nullptr}; // 激活图象
};

// 内联函数

} // namespace GUI
