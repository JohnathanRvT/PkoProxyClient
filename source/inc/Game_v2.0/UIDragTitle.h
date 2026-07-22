//----------------------------------------------------------------------
// 名称:静态文字
// 作者:lh 2004-07-19
// 最后修改日期:2004-10-09
//----------------------------------------------------------------------
#pragma once
#include "uicompent.h"

namespace GUI {
// 拖动控件条,用于支持外部拖动控件
class CDragTitle : public CCompent {
public:
	CDragTitle(CForm& frmOwn);
	CDragTitle(const CDragTitle& rhs);
	CDragTitle& operator=(const CDragTitle& rhs);
	virtual ~CDragTitle();
	GUI_CLONE(CDragTitle)

	virtual bool IsHandleMouse() override { return true; }
	virtual bool MouseRun(int x, int y, MouseClickState key) override;
	virtual void Render() override;
	virtual void Refresh() override;

	void DragRender() override;

	void SetIsShowDrag(bool v) { _IsShowDrag = v; }

public:
	CGuiPic* GetImage() override { return _pImage; }

private:
	void _SetSelf(const CDragTitle& rhs);

protected:
	CGuiPic* _pImage;
	bool _IsShowDrag;
};

} // namespace GUI
