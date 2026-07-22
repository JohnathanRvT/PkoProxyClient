//----------------------------------------------------------------------
// 名称:滚动条
// 作者:lh 2004-07-20
// 用途:用于配合树状控件,列表栏等的滚动
// 最后修改日期:2004-10-09
//----------------------------------------------------------------------
#pragma once
#include "uicompent.h"
#include "uitextbutton.h"
#include "uiimage.h"
#include "UIStep.h"
#include "uidragtitle.h"

namespace GUI {

class CScroll : public CCompent {
public:
	enum eStyle {
		btHorizontal = 0, // 水平的,从左自右的
		btVertical,		  // 垂直的，从下直上的
	};

public:
	CScroll(CForm& frmOwn);
	CScroll(const CScroll& rhs);
	CScroll& operator=(const CScroll& rhs);
	virtual ~CScroll();
	GUI_CLONE(CScroll)

	virtual void Render() override;
	virtual void Refresh() override;
	virtual bool MouseRun(int x, int y, MouseClickState key) override;
	virtual bool MouseScroll(int nScroll) override;
	virtual bool OnKeyDown(int key) override;
	virtual void Init() override;
	virtual void FrameMove(DWORD dwTime) override;
	virtual bool IsFrameMove() override { return true; }
	virtual void SetAlpha(BYTE alpha) override;
	virtual void RenderHint(int x, int y) override;
	virtual CCompent* GetHintCompent(int x, int y) override; // 返回有提示的控件

	virtual bool IsHandleMouse() override { return true; }

	bool LoadImage(const char* bkg, const char* up, const char* scroll, const char* down, int w, int h);
	CTextButton* GetUp() { return _up; }
	CTextButton* GetDown() { return _down; }
	CDragTitle* GetScroll() { return _pScroll; }
	CGuiPic* GetImage() override { return _pImage; }

	CStep& GetStep() { return _step; }
	bool SetRange(float min, float max);

	void SetStyle(eStyle v);
	void SetPageNum(unsigned int v) { _fPageNum = (float)v; }
	unsigned int GetPageNum() { return (unsigned int)_fPageNum; }

	void PageUp() { _step.Add(_fPageNum); }
	void PageDown() { _step.Sub(_fPageNum); }
	void Reset() override;
	void SetAutoHide(bool v) { _IsAutoHide = v; }
	bool GetAutoHide() { return _IsAutoHide; }

	// Add by lark.li 20080805 begin
	bool StepMove(int val);
	// End

public:
	GuiEvent evtChange; // 发生了滚动

protected:
	void _RefreshMidst();
	void _SetSelf();
	void _Copy(const CScroll& rhs);

protected:
	static void _UpClick(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CScroll*)(pSender->GetParent()))->_UpClick();
	}
	void _UpClick();

	static void _DownClick(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CScroll*)(pSender->GetParent()))->_DownClick();
	}
	void _DownClick();

	static void _DragBegin(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CScroll*)(pSender->GetParent()))->_DragBegin();
	}
	void _DragBegin();

	static void _DragMove(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CScroll*)(pSender->GetParent()))->_CheckScrollDrag();
	}
	void _CheckScrollDrag();

	void _UpdataStep(int v);

protected:
	CGuiPic* _pImage; // 背景图

	CTextButton* _up;
	CTextButton* _down;
	CDragTitle* _pScroll;

	eStyle _eStyle;

	CStep _step;
	int _nStart;

	float _fPageNum;  // 翻页时的间距
	bool _IsAutoHide; // 如果没有翻页是否自动隐藏

private:
	int _nMinDrag, _nMaxDrag;

	static CTextButton* _pContinueButton;
	static DWORD _dwStartTime;
};

} // namespace GUI
