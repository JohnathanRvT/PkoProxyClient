//----------------------------------------------------------------------
// 名称:用于游戏中的命令式控件
// 作者:lh 2004-10-28
//----------------------------------------------------------------------
#pragma once

#include "uicompent.h"

namespace GUI {

class CCommandCompent : public CCompent {
public:
	CCommandCompent(CForm& frmOwn);
	CCommandCompent(const CCommandCompent& rhs);
	CCommandCompent& operator=(const CCommandCompent& rhs);
	GUI_CLONE(CCommandCompent)

	bool IsHandleMouse() override { return true; }
	CCompent* GetHitCommand(int x, int y) override;
};

} // namespace GUI
