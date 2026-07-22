//----------------------------------------------------------------------
// 名称:翻页表格
// 作者:lh 2004-07-29
// 用途:内部显示图元为CGraph,按翻页数字可翻页,可改变表格大小
// 最后修改日期:2004-10-09
//----------------------------------------------------------------------
#pragma once
#include "uicompent.h"
#include "uitextbutton.h"
#include "uiimage.h"
#include "UIGraph.h"
#include "uidragtitle.h"

namespace GUI {
class CGrid : public CCompent {
public:
	CGrid(CForm& frmOwn);
	CGrid(const CGrid& rhs);
	CGrid& operator=(const CGrid& rhs);
	virtual ~CGrid();
	GUI_CLONE(CGrid)

	virtual void Init() override;
	virtual void Render() override;
	virtual void Refresh() override;
	virtual bool MouseRun(int x, int y, MouseClickState key) override;
	virtual bool IsHandleMouse() override { return true; }
	virtual void SetAlpha(BYTE alpha) override;
	virtual void SetMargin(int left, int top, int right, int bottom) override { _nMargin = left; }
	virtual void SetIsDrag(bool v) override;
	virtual void FrameMove(DWORD dwTime) override;
	virtual bool IsFrameMove() override { return true; }

public:
	void SetUnitSize(int w, int h);
	int GetUnitWidth() { return _nUnitWidth; }
	int GetUnitHeight() { return _nUnitHeight; }
	void SetSpace(int x, int y);
	void SetIsDragSize(bool v);

	void Add(CGraph* p);

	int GetCount() { return (int)_memory.size(); }
	CGraph* GetGraph(unsigned int v) { return (v < _memory.size()) ? _memory[v] : NULL; }

public:
	virtual bool OnKeyDown(int key) override;

public:
	CGuiPic* GetImage() override { return _pImage.get(); }
	CImage* GetSelectImage() { return _pSelectImage; }
	CTextButton* GetNext() { return _pNextPage; }
	CTextButton* GetPrior() { return _pPriorPage; }

	CGraph* GetSelect() { return _pSelect; }
	int GetSelectIndex() { return _nSelectIndex; }

public:
	GuiEvent evtSelectChange{nullptr}; // 选择发生了变化

protected:
	void _SetSelf();
	void _Copy(const CGrid& rhs);
	void _Clear();
	void _PageChange();

	static void _NextClick(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CGrid*)(pSender->GetParent()))->_NextClick();
	}
	void _NextClick();

	static void _PriorClick(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CGrid*)(pSender->GetParent()))->_PriorClick();
	}
	void _PriorClick();

	static void _DragBegin(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CGrid*)(pSender->GetParent()))->_DragBegin();
	}
	void _DragBegin();

	static void _DragEnd(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CGrid*)(pSender->GetParent()))->_RefreshSize();
	}

	static void _DragMove(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CGrid*)(pSender->GetParent()))->_RefreshSize();
	}
	void _RefreshSize();

protected:
	std::unique_ptr<CGuiPic> _pImage;
	CImage* _pSelectImage{nullptr}; // 用于画选择框

	CDragTitle* _pSizeImg;

	CTextButton* _pNextPage;  // 翻到下一页
	CTextButton* _pPriorPage; // 翻到上一页

	char _strPage[10]; // 用于显示当前多少页，总共多少页
	int _nStrX, _nStrY;
	int _nStrWidth;

	int _nUnitHeight{32}, _nUnitWidth{32}; // 单元宽高
	int _nSpaceX{2}, _nSpaceY{2};		   // 行间距

	using memory = std::vector<CGraph*>;
	memory _memory;
	int _nSize{0};
	int _nFirst{0}, _nLast{0};   // 在显示区的第一个ID和最后一个ID
	int _nPage{0}, _nMaxPage{0}; // 当前页数，最大页数
	int _nColNum{0};			 // 一行可以显示多少列
	int _nRowNum;				 // 一页可以显示多少行
	int _nPageNum{0};			 // 一页可以显示多少个

	int _nMargin{5}; // 与周围边框的距离
	int _nStartX, _nStartY;

private:
	int _nDragWidth, _nDragHeight;
	int _nTotalW, _nTotalH;

	CGraph* _pSelect{nullptr};
	int _nSelectIndex{-1};
	bool _IsSelect;
	DWORD _clPageTextColor{0xff0000ff};
};

// 内联函数
inline void CGrid::SetUnitSize(int w, int h) {
	if (w > 0)
		_nUnitWidth = w;
	if (h > 0)
		_nUnitHeight = h;
	_pSelectImage->SetSize(_nUnitWidth, _nUnitHeight);
}

inline void CGrid::SetSpace(int x, int y) {
	if (x >= 0)
		_nSpaceX = x;
	if (y >= 0)
		_nSpaceY = y;
}

inline void CGrid::Add(CGraph* p) {
	_memory.push_back(p);
	_nSize = (int)_memory.size();
	_pSelect = p;
}

inline void CGrid::_PriorClick() {
	_nPage--;
	if (_nPage < 0)
		_nPage = _nMaxPage;
	Refresh();
}

inline void CGrid::_PageChange() {
	_nFirst = _nPage * _nPageNum;
	_nLast = _nFirst + _nPageNum;
	if (_nLast > _nSize)
		_nLast = _nSize;
	_snprintf_s(_strPage, _TRUNCATE, "%d/%d", _nPage + 1, _nMaxPage + 1);
}

inline void CGrid::_NextClick() {
	_nPage++;
	if (_nPage > _nMaxPage)
		_nPage = 0;
	Refresh();
}

inline void CGrid::SetIsDragSize(bool v) {
	_pSizeImg->SetIsShow(v);
}

} // namespace GUI
