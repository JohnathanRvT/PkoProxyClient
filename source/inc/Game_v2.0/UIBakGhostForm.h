#pragma once

#include "UIForm.h"
#include "UIGlobalVar.h"

namespace GUI {

class CBakGhostMgr : public CUIInterface {
	enum {
		REQUIRED_ITEM_COUNT = 2
	};

	CForm* frmBakGhost;

	static int HATCH_SPIRITE_ID;
	static COneCommand* cmdRequirement[REQUIRED_ITEM_COUNT];
	static int RequirementGridIDs[REQUIRED_ITEM_COUNT];
	static CTextButton* btnHatch;

public:
	CBakGhostMgr() : frmBakGhost(nullptr) {}
	virtual ~CBakGhostMgr() {}

	void CreateSpiriteResponse(bool Success);

private:
	virtual bool Init() override;

	void Clear();
	void PushItem(int Index, CItemCommand& Item);
	void PopItem(int Index);

	static void evtBakGhostFormShow(CGuiData* pSender);
	static void evtBakGhostFormClose(CForm* pForm, bool& IsClose);
	static void evtRequirementEvent(CGuiData* pSender, CCommandObj* pItem, bool& isAccept);
	static void evtCreateSpirite(CGuiData* pSender, int x, int y, MouseClickState key);
	static void evtShowHelpInfo(CGuiData* pSender, int x, int y, MouseClickState key);
};

} // namespace GUI
