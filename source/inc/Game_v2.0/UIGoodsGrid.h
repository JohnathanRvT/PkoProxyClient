//----------------------------------------------------------------------
// 锟斤拷锟斤拷:锟斤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷:lh 2004-10-27
// 锟斤拷途:锟斤拷锟斤拷锟斤拷锟斤拷每一锟斤拷锟斤拷锟斤拷,装锟斤拷锟斤拷锟斤拷锟较讹拷,每一装锟斤拷锟铰讹拷锟斤拷锟斤拷锟街达拷锟斤拷锟斤拷锟?
// 锟斤拷锟斤拷薷锟斤拷锟斤拷锟?
//----------------------------------------------------------------------
#pragma once
#include "UICommandCompent.h"
#include "UIScroll.h"

namespace GUI {

using GuiThrowItemEvent = void (*)(CGuiData* pSender, int id, bool& isThrow);
using GuiSwapItemEvent = void (*)(CGuiData* pSender, int nFirst, int nSecond, bool& isSwap);
using GuiDragInGridEvent = void (*)(CGuiData* pSender, CCommandObj* pItem, int nGridID, bool& isAccept);
using GuiRMouseGridEvent = void (*)(CGuiData* pSender, CCommandObj* pItem, int nGridID);

class CCommandObj;
class CGoodsGrid : public CCommandCompent {
public:
	enum class eShowStyle {
		Small, // 锟斤拷锟斤拷锟斤拷示图锟疥方式
		Sale,  // 锟斤拷示锟桔筹拷
		OwnDef // 锟皆讹拷锟斤拷锟斤拷示
	};

	CGoodsGrid(CForm& frmOwn);
	CGoodsGrid(const CGoodsGrid& rhs);
	CGoodsGrid& operator=(const CGoodsGrid& rhs);
	~CGoodsGrid();
	GUI_CLONE(CGoodsGrid)

	virtual void Init() override;
	virtual void Render() override;
	virtual void Refresh() override;
	virtual bool MouseRun(int x, int y, MouseClickState key) override;
	virtual bool MouseScroll(int nScroll) override;
	virtual eAccept SetCommand(CCommandObj* p, int x, int y) override;
	virtual void Reset() override;

	virtual bool IsHandleMouse() override { return true; }
	virtual CGuiPic* GetImage() override { return _pImage.get(); }
	virtual void SetMargin(int left, int top, int right, int bottom) override;

	virtual void SetAlpha(BYTE alpha) override;
	virtual void DragRender() override;
	virtual CCompent* GetHintCompent(int x, int y) override;
	virtual void RenderHint(int x, int y) override;

	virtual UseComandEvent GetUseCommantEvent() override { return evtUseCommand; }

	CScroll* GetScroll() const { return _pScroll.get(); }

	CGuiPic* GetUnitImage() const { return _pUnit.get(); }
	void SetIsHint(bool v) { _IsShowHint = v; }

	void Clear();

public:
	void SetShowStyle(eShowStyle v) { _eShowStyle = v; }

	bool SetContent(int nRow, int nCol);
	void SetSpace(int x, int y);
	void SetUnitSize(int w, int h);

	size_t GetFreeIndex() const;
	void GetFreeIndex(int* nFreeIndexes, size_t& nCount, size_t nSize) const; // nFree为int锟斤拷锟斤拷,nCount为锟斤拷锟斤拷锟叫讹拷锟劫革拷值锟斤拷效,nSize为nFree锟侥伙拷锟斤拷锟斤拷锟?
	bool DelItem(int nIndex);
	bool SetItem(unsigned int nIndex, CCommandObj* pItem);
	bool SwapItem(unsigned int nFirst, unsigned int nSecond);

public:
	int GetDragIndex() const { return _nDragIndex; }
	int GetCol() const { return _nCol; }
	int GetRow() const { return _nRow; }

	CCommandObj* GetItem(unsigned int nIndex);
	int GetMaxNum() const { return _nMaxNum; }
	int GetCurNum() const { return _nCurNum; }

	size_t FindCommand(CCommandObj* p) const; // 锟斤拷锟截达拷锟斤拷锟斤拷晒锟?失锟杰凤拷锟斤拷-1

	void SetItemValid(bool v);

	int GetFirstShow() const { return _nFirst; }

	int GetEmptyGridCount(); // 锟斤拷锟斤拷锌崭锟斤拷锟接革拷锟斤拷
	int GetUsedGridCount();  // 锟斤拷锟斤拷锟斤拷锟绞癸拷酶锟斤拷痈锟斤拷锟?

public: // 锟铰硷拷
	GuiThrowItemEvent evtThrowItem{nullptr};
	GuiSwapItemEvent evtSwapItem{nullptr};
	GuiDragInGridEvent evtBeforeAccept{nullptr};
	GuiRMouseGridEvent evtRMouseEvent{nullptr};

	UseComandEvent evtUseCommand{nullptr};

private:
	void _Copy(const CGoodsGrid& rhs);
	void _SetSelf();
	void _ClearItem();
	int _GetHitItem(int x, int y); // 锟斤拷锟斤拷-1失锟杰ｏ拷锟斤拷锟津返伙拷锟斤拷锟斤拷

protected:
	static void _OnScrollChange(CGuiData* pSender) {
		((CGoodsGrid*)(pSender->GetParent()))->_OnScrollChange();
	}
	void _OnScrollChange();

protected:
	std::unique_ptr<CGuiPic> _pImage{nullptr}; // 锟斤拷锟斤拷
	std::unique_ptr<CScroll> _pScroll{nullptr};
	std::unique_ptr<CGuiPic> _pUnit{nullptr}; // 锟斤拷元锟斤拷图片

	CCommandObj** _pItems{nullptr};

protected:
	eShowStyle _eShowStyle{eShowStyle::Small};
	int _nUnitHeight{32},
		_nUnitWidth{32};		  // 锟斤拷元锟斤拷锟?
	int _nSpaceX{2}, _nSpaceY{2}; // 锟斤拷元锟斤拷锟?

	int _nRow{0}, _nCol{0}; // 锟斤拷示锟斤拷锟斤拷锟斤拷
	int _nMaxNum{0};		// 锟斤拷锟斤拷锟斤拷,锟斤拷锟叫碉拷锟斤拷锟斤拷锟斤拷

	// 锟斤拷锟斤拷围锟竭匡拷木锟斤拷锟?
	int _nLeftMargin{0};
	int _nTopMargin{0};
	int _nRightMargin{0};
	int _nBottomMargin{0};

private:
	static void _DragEnd(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CGoodsGrid*)(pSender))->_DragEnd(x, y, key);
	}
	void _DragEnd(int x, int y, MouseClickState key);

private:
	int _nStartX, _nStartY;
	int _nPageShowNum; // 一页锟斤拷锟斤拷示锟侥革拷锟斤拷
	int _nTotalW, _nTotalH;

	int _nFirst{0}, _nLast{0}; // 锟斤拷示时锟侥碉拷一锟斤拷锟斤拷锟斤拷锟揭伙拷锟?

	CCommandObj* _pDragItem;
	int _nDragIndex; // 锟较讹拷时锟斤拷锟斤拷锟斤拷
	int _nDragOffX, _nDragOffY;
	int _nDragRow, _nDragCol;

	int _nCurNum{0}; // 锟斤拷前锟斤拷品锟斤拷锟斤拷
	bool _IsShowHint{true};

private:
	static int _nTmpX, _nTmpY, _nTmpRow, _nTmpCol;

private:
	int _nTmpIndex{0};
};

// 锟斤拷锟斤拷锟斤拷锟斤拷
inline void CGoodsGrid::SetSpace(int x, int y) {
	if (x >= 0) {
		_nSpaceX = x;
	}
	if (y >= 0) {
		_nSpaceY = y;
	}
}

inline void CGoodsGrid::SetUnitSize(int w, int h) {
	if (w > 0) {
		_nUnitWidth = w;
	}
	if (h > 0) {
		_nUnitHeight = h;
	}
}

inline CCommandObj* CGoodsGrid::GetItem(unsigned int nIndex) {
	if (nIndex >= (unsigned int)_nMaxNum) {
		return nullptr;
	}
	return _pItems[nIndex];
}

} // namespace GUI
