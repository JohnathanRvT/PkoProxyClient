//--------------------------------------------------------------
// 名称:GUI接口
// 作者:lh 2004-07-15
// 设计思想:CGuiDat管理内存，提供控件接口
//		     CForm显示，处理事件，切换控件焦点，装载存储资源文件
// 注:所有实例只能在在堆中生成，由程序开始时静态分配
//    退出时释放资源
// 修改日期:2004-10-26 by lh
// 修改内容:将CItemObj的脚本功能分离为CUIScript,CItemRow支持多态
//--------------------------------------------------------------

#pragma once
#include <assert.h>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include <algorithm>
#include "UICursor.h"
#include "UIFont.h"
#include "UIPicture.h"
#include "Mouse.h"

// 以下宏用于支持Gui克隆
#define GUI_CLONE(TName)                                       \
public:                                                        \
	virtual CGuiData* Clone() { return new (##TName)(*this); } \
	virtual bool Clone(CGuiData* rhs) {                        \
		##TName* p = dynamic_cast<##TName*>(rhs);              \
		if (!p)                                                \
			return false;                                      \
		*this = *p;                                            \
		return true;                                           \
	}

namespace GUI {
// 用于函数MouseRun的参数key中,鼠标行为的集合


enum eCompentAlign {
	caNone,   // 不改变位置
	caLeft,   // 充满左边
	caLeftUp, // 放置在最左上角
	caUp,
	caRightUp,
	caRight,
	caRightBottom,
	caBottom,
	caLeftBottom,
	caClient,		// 充满全部
	caCenter,		// 完全居中
	caWidthCenter,  // 水平居中
	caHeightCenter, // 上下居中
	caEnd,			// 结束符，无意义
};

const BYTE DROP_ALPHA = 0xA0;

class CCompent;
class CGuiData;
class CForm;

using GuiMouseEvent = void (*)(CGuiData* pSender, int x, int y, MouseClickState key);
using GuiKeyDownEvent = bool (*)(CGuiData* pSender, int& key);
using GuiKeyCharEvent = bool (*)(CGuiData* pSender, char& key);
using GuiEvent = void (*)(CGuiData* pSender);
using GuiOnTimerEvent = void (*)(CGuiData* pSender, DWORD nEventID, PVOID lpData);
using CompentFun = void (*)(CCompent* pSender, unsigned int index);
using GuiHotKeyEvent = bool (*)(CForm* pSender, char key);
using GuiItemClickEvent = void (*)(std::string strItem);

// GUI对象拖动
class CDrag {
public:
	enum class eState {
		None = 0,
		Press,
		Drag,
		End,
	};

public:
	CDrag() = default;

	void SetYare(unsigned int n) {
		if (n < 30) {
			_nYareLen = n;
		}
	}

	bool MouseRun(int x, int y, MouseClickState key);
	CDrag::eState BeginMouseRun(CGuiData* gui, bool InRect, int x, int y, MouseClickState key); // 确定鼠标状态,返回0无动作

	static CDrag* GetDrag() { return _pDrag; }
	static CGuiData* GetParent() { return _pParent; }

	static bool IsDraging(CGuiData* p) { return _pParent == p; }
	static void SetSnapToGrid(DWORD dwWidth, DWORD dwHeight);

	static void SetDragX(int x) { _nDragX = x; }
	static void SetDragY(int y) { _nDragY = y; }

	static int GetDragX() { return _nDragX; }
	static int GetDragY() { return _nDragY; }

	static int GetStartX() { return _nStartX; }
	static int GetStartY() { return _nStartY; }

	static int GetX() { return _nStartX + _nDragX; }
	static int GetY() { return _nStartY + _nDragY; }

	void Reset();

	void SetIsMove(bool v) { _IsMove = v; }
	bool GetIsMove() { return _IsMove; }

	void SetIsUseGrid(bool v) { _IsUseGrid = v; }
	bool GetIsUseGrid() { return _IsUseGrid; }

	CDrag::eState GetState() { return _eState; }

	void SetDragInCursor(CCursor::eState v) { _crDragIn = v; }
	CCursor::eState GetDragInCursor() { return _crDragIn; }
	void SetDragCursor(CCursor::eState v) { _crDrag = v; }
	CCursor::eState GetDragCursor() { return _crDrag; }

public:
	GuiMouseEvent evtMouseDragBegin{nullptr}; // 开始拖动
	GuiMouseEvent evtMouseDragMove{nullptr};  // 正在拖动事件
	GuiMouseEvent evtMouseDragEnd{nullptr};   // 拖动结束

private:
	static CDrag* _pDrag;	  // 正在拖动中的对象
	static CGuiData* _pParent; // 正在拖动中的gui

	static int _nDragX, _nDragY;
	static int _nStartX, _nStartY;

	int _nYareLen{2};
	eState _eState{eState::None};

	bool _IsMove{true};	// 当移动结束后,直接将控件移动
	bool _IsUseGrid{true}; // 是否使用对齐表格

	CCursor::eState _crDragIn{CCursor::stEnd}; // 进入拖动区域内的光标
	CCursor::eState _crDrag{CCursor::stDrag};  // 拖动时的光标
	static CCursor::eState _crNormal;		   // 正常情况下的光标

	static DWORD _dwGridWidth, _dwGridHeight; // 对齐表格的宽高
	static DWORD _dwMouseUpTime;			  // 用于开始拖动后,已经没有MouseUP消息,但鼠标却已松开达到一定时间后取消拖动
};

// 以下宏用于支持Gui克隆
#define ITEM_CLONE(TName)                                   \
public:                                                     \
	virtual TName* Clone() { return new (##TName)(*this); } \
	virtual bool Clone(CItemObj* rhs) {                     \
		##TName* p = dynamic_cast<##TName*>(rhs);           \
		if (!p)                                             \
			return false;                                   \
		*this = *p;                                         \
		return true;                                        \
	}

// 显示在GUI对象中的小对象
class CItemObj {
public:
	virtual ~CItemObj();
	ITEM_CLONE(CItemObj)

	virtual void Render(int x, int y){};

	virtual int GetWidth() const { return 0; }
	virtual int GetHeight() const { return 0; }

	virtual bool HasHint() const { return false; }
	virtual void ReadyForHint(int x, int y, CCompent* pCompent){};
	virtual void RenderHint(int x, int y){};

	virtual const char* GetString() const { return ""; }
	virtual void SetString(const char* str) {}

	virtual void SetWidth(int v) {}
	virtual void SetColor(DWORD c) {}
};

// GUI祖先类
class CForm;
class CGuiData {

public:
	CGuiData();
	CGuiData(const CGuiData& rhs);
	CGuiData& operator=(const CGuiData& rhs);
	virtual ~CGuiData();
	GUI_CLONE(CGuiData)

	virtual void Init() {} // 用于外部初始化，如装载图片等

	virtual void Render() {} // 显示
	virtual void Refresh();  // 用于刷新显示用的绝对坐标

	// 键盘鼠标处理过程，已处理，返回true
	virtual bool MouseRun(int x, int y, MouseClickState key);
	virtual bool MouseScroll(int nScroll) { return _IsMouseIn; }

	virtual void SetParent(CGuiData* p) { _pParent = p; }
	virtual void SetAlign(eCompentAlign v) {}
	virtual void SetAlpha(BYTE alpha) {}
	virtual void SetIsEnabled(bool v) { _bEnabled = v; }
	virtual void SetMargin(int left, int top, int right, int bottom) {}

	virtual CGuiPic* GetImage() { return nullptr; }

	virtual CForm* GetForm() { return nullptr; }
	virtual void SetCaption(const char* str) {} // 设置显示的文本
	virtual void SetTextColor(DWORD color) {}
	virtual void SetIsDrag(bool v);

	virtual CCompent* Find(const char* str) { return nullptr; }

	virtual void DragRender() {}
	virtual void SetIsShow(bool v) { _bShow = v; }

	virtual void RenderHint(int x, int y);

	virtual bool IsAllowActive(); // 判断是否可以激活

	virtual void Reset(){}; // 用于外部调用,让滚动条清零等工作

public:
	static HWND GetHWND();				  // 得到windows handle
	static CGuiData* GetGui(DWORD dwID) { // 根据索引得到控件对象
		if (dwID == 0) {
			LG("gui", "NULL GUI\n");
			return nullptr;
		}
		if (dwID > _AllGui.size()) {
			LG("gui", "BIG[%d] GUI\n", dwID);
			return nullptr;
		}
		return _AllGui[dwID - 1];
	}
	static void ClearAllGui(); // 清除所有控件
	static void InitMemory();  // 在生成所有控件后调用

	static bool SetHintItem(CItemObj* pObj);
	static CItemObj* GetHintItem() { return _pHintItem; }

public:
	DWORD GetID() const { return _dwID; }

	const char* GetName() const { return _strName.c_str(); }
	void SetName(const char* name) { _strName = name; }

	void SetHint(const char* str) { _strHint = str; }

	void SetPos(int nLeft, int nTop) {
		_nLeft = nLeft;
		_nTop = nTop;
	}
	int GetLeft() const { return _nLeft; }
	int GetTop() const { return _nTop; }
	int GetRight() const { return _nLeft + _nWidth; }
	int GetBottom() const { return _nTop + _nHeight; }

	int GetX() const { return _nX1; }
	int GetY() const { return _nY1; }
	int GetX2() const { return _nX2; }
	int GetY2() const { return _nY2; }

	void SetSize(int w, int h) { _nWidth = w, _nHeight = h; }
	int GetWidth() const { return _nWidth; }
	int GetHeight() const { return _nHeight; }

	bool GetIsShow() const { return _bShow; }

	bool GetIsEnabled() const { return _bEnabled; }

	bool IsNormal() const { return _bShow && _bEnabled; }

	CGuiData* GetParent() { return _pParent; }

	bool GetIsDrag() const { return _pDrag != nullptr; }
	CDrag* GetDrag() { return _pDrag.get(); }

	void SetPointer(void* v) { _pVoid = v; }
	void* GetPointer() { return _pVoid; }

	void ScriptSetShow(bool v) { _bShow = v; } // 外部脚本用

	int nTag{0};

public:
	bool InRect(int x, int y) { return _IsMouseIn = (x >= _nX1 && x <= _nX2 && y >= _nY1 && y <= _nY2); }
	bool IsNoDrag(int x, int y, MouseClickState key); // 如果没有开始拖动,并且点在区域内,返回true

	static void SetScreen(float dx, float dy, float sx, float sy) {
		_fDrawX = dx, _fDrawY = dy, _fScreenX = sx, _fScreenY = sy;
	}
	void LineFrame();		   // 画一个边框
	void SelectChaLineFrame(); //画一个死人框

	void LineThinFrame(); // 画一个细框
	void FillFrame();	 // 填充

	static void SetMousePos(int x, int y) {
		_nMouseX = x;
		_nMouseY = y;
	}
	static int GetMouseX() { return _nMouseX; }
	static int GetMouseY() { return _nMouseY; }

	static void SetCursor(CCursor::eState v) { _eCursor = v; } // 仅用于MouseRun中
	static CCursor::eState GetCursor() { return _eCursor; }

protected:
	virtual void _AddForm() {} // 将自己加入到所属的Form中

protected:
	using vcs = std::vector<CGuiData*>;
	static vcs _AllGui; // 存放所有的控件资源

	DWORD _dwID{0}; // 注册控件的ID-即索引，外部不可改变

	CGuiData* _pParent{nullptr};

	std::string _strName;		 // 标识
	int _nWidth{0}, _nHeight{0}; // 宽高
	int _nLeft{0}, _nTop{0};	 // 相对坐标

	int _nX1{0}, _nX2{0}, _nY1{0}, _nY2{0}; // 显示用的绝对坐标

	bool _bShow{true};	// 是否显示
	bool _bEnabled{true}; // 是否可用
	std::unique_ptr<CDrag> _pDrag{nullptr};

	bool _IsMouseIn{false}; // 鼠标是否在区域内
	std::string _strHint;   // 控件提示

	void* _pVoid{nullptr};

protected:
	int _ScreenX(int x) { return (int)(((float)(x)) / _fScreenX); }
	int _ScreenY(int y) { return (int)(((float)(y)) / _fScreenY); }
	void _RenderHint(const char* str, int x, int y);

private:
	static float _fDrawX, _fDrawY, _fScreenX, _fScreenY;
	static CGuiPic _imgLevel, _imgVertical; // 用于画边框的水平线和垂直线
	static CItemObj* _pHintItem;

	static int _nMouseX, _nMouseY;   // 目前鼠标所在的位置
	static CCursor::eState _eCursor; // 显示在gui上的光标
};

class CGuiTime;
using GuiTimerEvent = void (*)(CGuiTime* pSender);

class CGuiTime {
	friend class CFormMgr;

public:
	static CGuiTime* Create(DWORD dwInterval = 0, GuiTimerEvent evt = nullptr);
	bool Release(); // 外部删除对象

	static void FrameMove(DWORD dwTime);
	static CGuiTime* Find(DWORD id);

	void SetInterval(DWORD v) { _dwInterval = v; }
	DWORD GetInterval() { return _dwInterval; }

	DWORD GetID() { return _dwIndex; }

	void SetUserID(DWORD v) { _nEventID = v; }
	DWORD GetUserID() { return _nEventID; }

	void SetUserPoint(PVOID v) { _lpData = v; }
	PVOID GetUserPoint() { return _lpData; }

	bool IsTime() { // 是否到达时间点,不能阻塞
		if (_IsTime) {
			_IsTime = false;
			return true;
		}
	}

public: // event
	GuiTimerEvent evtTime;

private:
	DWORD _dwIndex;	// 唯一ID
	DWORD _dwInterval; // 等于零,不激活;

	bool _IsTime;
	DWORD _dwLastTime;

	// 用户数据
	DWORD _nEventID;
	PVOID _lpData;

	bool _IsRelease;

private:
	CGuiTime(DWORD dwInterval = 0, GuiTimerEvent evt = nullptr);
	~CGuiTime();
	void OnTime(DWORD dwTime);

	using times = std::vector<CGuiTime*>;
	static times _times;
};

// 内联函数
inline bool CGuiData::IsNoDrag(int x, int y, MouseClickState key) {
	InRect(x, y);
	if (_pDrag &&
		_pDrag->BeginMouseRun(this, _IsMouseIn, x, y, key) == CDrag::eState::Drag) {
		return false;
	}

	return _IsMouseIn;
}

inline void CGuiData::_RenderHint(const char* str, int x, int y) {
	CGuiFont::s_Font.FrameRender(str, x + 40, y);
}

inline void CGuiTime::OnTime(DWORD dwTime) {
	if (!_IsRelease && _dwInterval > 0 && dwTime > _dwLastTime && (dwTime - _dwLastTime > _dwInterval)) {
		if (evtTime) {
			evtTime(this);
		} else {
			_IsTime = true;
		}

		_dwLastTime = dwTime;
	}
}
} // namespace GUI
