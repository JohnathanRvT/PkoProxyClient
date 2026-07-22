//----------------------------------------------------------------------
// 名称:技能框
// 作者:lh 2004-12-13
// 用途:双击可以使用技能,升级时显示一个升级按钮,有当前级别,魔法值
//      可拖动到快捷栏,可双击执行,因些有Command系列
//      有动态可显示按钮用于升级,因此要响应鼠标
//      有静态文字,表示技能名称
//      有动态文字,表示当前级别,消耗魔法值
//      有上下换行,换页,到头尾等,要响应键盘
//   注:本来这个类可以直接使用CList,只要派生CListItems即可,但考虑到功能较CList更多更复杂,
//      后期可能有改动,所以新开一个类,避免CList系列可能太过复杂
// 最后修改日期:
//----------------------------------------------------------------------
#pragma once
#include <vector>
#include "uicompent.h"
#include "uiskillcommand.h"
#include "UITextButton.h"
#include "UIScroll.h"

namespace GUI {

class CSkillList;
using GuiSkillListUpgrade = void (*)(CSkillList* pSender, const CSkillCommand* pSkill);

class CSkillList : public CCompent {
public:
	CSkillList(CForm& frmOwn);
	CSkillList(const CSkillList& rhs);
	CSkillList& operator=(const CSkillList& rhs);
	virtual ~CSkillList();
	GUI_CLONE(CSkillList)

	virtual void Init() override;
	virtual void Render() override;
	virtual void Refresh() override;
	virtual bool MouseRun(int x, int y, MouseClickState key) override;
	virtual bool MouseScroll(int nScroll) override;
	virtual void SetAlpha(BYTE alpha) override;
	virtual bool IsHandleMouse() override { return true; }
	virtual bool OnKeyDown(int key) override;
	virtual void DragRender() override;
	virtual CCompent* GetHintCompent(int x, int y) override;

	virtual void SetMargin(int left, int top, int right, int bottom) override;
	virtual CGuiPic* GetImage() override { return _pImage.get(); }

	CGuiPic* GetButtonImage() { return _pButton.get(); }
	void Clear();
	void SetRowHeight(int v) { _nRowHeight = v; }
	void SetUnitSize(int w, int h) {
		_nUnitHeight = h;
		_nUnitWidth = w;
	}
	void SetFontLeft(int x) { _nFontStart = x; }
	void SetFontColor(DWORD color) { _dwFontColor = color; }
	void SetIsShowUpgrade(bool v) { _IsShowUpgrade = v; }

	CSkillCommand* GetCommand(size_t v);
	bool AddCommand(std::unique_ptr<CSkillCommand> p);
	CSkillCommand* FindSkill(int nID);
	bool DelSkill(int nID);

	int GetCount() { return (int)_skills.size(); }

	CGuiPic* GetSelect() { return _pSelect.get(); }
	CScroll* GetScroll() { return _pScroll.get(); }

	int FindCommand(CSkillCommand* p);

public:
	GuiSkillListUpgrade evtUpgrade{nullptr};

private:
	void _SetSelf();
	void _Copy(const CSkillList& rhs);
	void _SetFirstShowRow(DWORD);
	int _GetHitSkill(int x, int y);

private:
	static void _ScrollChange(CGuiData* pSender) {
		((CSkillList*)(pSender->GetParent()))->_OnScrollChange();
	}
	void _OnScrollChange();
	void _ResetPageNum();

private:
	static void _DragEnd(CGuiData* pSender, int x, int y, MouseClickState key) {
		((CSkillList*)(pSender))->_DragEnd(x, y, key);
	}
	void _DragEnd(int x, int y, MouseClickState key);

protected:
	std::unique_ptr<CGuiPic> _pImage{nullptr};  // 背景图
	std::unique_ptr<CGuiPic> _pSelect{nullptr}; // 选择条
	std::unique_ptr<CGuiPic> _pButton{nullptr}; // 升级按钮
	std::unique_ptr<CScroll> _pScroll{nullptr};

	using skills = std::vector<std::unique_ptr<CSkillCommand>>;
	skills _skills;

	// 与周围边框的距离
	int _nLeftMargin{0};
	int _nTopMargin{0};
	int _nRightMargin{0};
	int _nBottomMargin{0};

	int _nRowHeight{36}; // 行高
	int _nFontStart{40}; // 字体开始位置
	DWORD _dwFontColor{COLOR_BLACK};

	int _nUnitHeight{32}, _nUnitWidth{32}; // 单元图片宽高

	int _nSelectIndex{0}; // 选择的行数
	bool _IsShowUpgrade{true};

private:
	int _nShowFirst{0}; // 显示的第一行
	int _nShowLast{0};  // 显示的最后一行
	int _nShowCount{0}; // 总共可以显示多少行

	// 用于显示技能的范围
	int _nSX1, _nSY1, _nSX2, _nSY2;

	int _nButtonX1;   // 按钮显示的X位置
	int _nButtonOffY; // 按钮显示的Y轴偏移

	int _nRowSpace;	// 行距
	int _nFontYOff{0}; // 字显示在一行的中部高度

private:
	int _nDragIndex{-1};
	int _nDragOffX, _nDragOffY;
	int _nMouseUpgradeID{-1};
};

// 内联函数
inline void CSkillList::_ResetPageNum() {
	_pScroll->SetPageNum((GetHeight() - _nTopMargin - _nBottomMargin) / _nRowHeight - 1);
}

inline CSkillCommand* CSkillList::GetCommand(size_t v) {
	if (v >= _skills.size()) {
		return nullptr;
	}
	return _skills[v].get();
}

inline int CSkillList::_GetHitSkill(int x, int y) {
	if (x >= _nSX1 && x <= _nSX2 && y >= _nSY1 && y <= _nSY2) {
		int h = (_nShowCount * _nRowHeight + _nSY1);
		if (y >= h)
			return false;

		// 得到行数
		int row = (y - _nSY1) / _nRowHeight + _nShowFirst;
		if (row >= 0 && row < (int)_skills.size())
			return row;
	}
	return -1;
}

} // namespace GUI
