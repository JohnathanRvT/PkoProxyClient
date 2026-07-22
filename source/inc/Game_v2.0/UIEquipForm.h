#pragma once
#include "UIGlobalVar.h"
#include "NetProtocol.h"
#include "SkillRecord.h"

class CCharacter;
class CSkillStateRecord;
struct stNetSkillBag;

namespace GUI {
class CItemCommand;

// 技能，道具，快捷方式，装备
class CEquipMgr : public CUIInterface {
public:
	void ShowRepairMsg(const char* pItemName, long lMoney);
	void SynSkillBag(DWORD dwCharID, stNetSkillBag* pSSkillBag);

	void UpdataEquip(const stNetChangeChaPart& SPart, CCharacter* pCha);
	void UpdataEquipData(const stNetChangeChaPart& SPart, CCharacter* pCha);

	void UpdateShortCut(stNetShortCut& stShortCut); // 更新快捷栏
	void DelFastCommand(CCommandObj* pObj);			// 删除当前快捷栏
	bool ExecFastKey(int key);

	void CloseAllForm(); // 关闭窗体

	// 将快捷栏与道具栏比较,与服务器不相同的快捷栏,向服务器发送协议更新快捷栏,返回更新的个数
	// 用于道具在道具栏交换时
	int RefreshServerShortCut();

	CGoodsGrid* GetGoodsGrid() { return grdItem; } // 操作道具栏表格
	CForm* GetItemForm() { return frmItem; }	   // 操作道具表单

	// 从装备栏卸载道具到道具栏
	void UnfixToGrid(CCommandObj* pItem, int nGridID, int nLink);

	CSkillRecord* FindSkill(int nID);

	CItemCommand* GetEquipItem(unsigned int nLink);

	bool IsEquipCom(COneCommand* pCom) const;

	void SetIsLock(bool bLock);
	bool GetIsLock() const;

	// 获得道具在当前背包中总的个数
	int GetItemCount(int nID);

	static CGuiPic* GetChargePic(unsigned int n);

	static void evtThrowItemEvent(CGuiData* pSender, int id, bool& isThrow);				// 从道具栏中扔出道具
	static void evtSwapItemEvent(CGuiData* pSender, int nFirst, int nSecond, bool& isSwap); // 道具栏中交换道具

protected:
	virtual bool Init() override;
	virtual void End() override;
	virtual void LoadingCall() override;
	virtual void FrameMove(DWORD dwTime) override;

private:
	CSkillList* GetSkillList(char nType);
	void RefreshUpgrade();
	void RefreshSkillJob(int nJob);

	bool _GetThrowPos(int& x, int& y);
	bool _GetCommandShortCutType(CCommandObj* pItem, char& chType, short& sGridID);

	static void _evtSkillFormShow(CGuiData* pSender);
	static void _evtItemFormShow(CGuiData* pSender);
	static void _evtItemFormClose(CForm* pForm, bool& IsClose);

	static void _evtSkillUpgrade(CSkillList* pSender, const CSkillCommand* pSkill);
	static void _evtFastChange(CGuiData* pSender, CCommandObj* pItem, bool& isAccept);	 // 快捷栏发生变化
	static void _evtEquipEvent(CGuiData* pSender, CCommandObj* pItem, bool& isAccept);	 // 从道具栏中装备到装备栏
	static void _evtThrowEquipEvent(CGuiData* pSender, CCommandObj* pItem, bool& isThrow); // 从装备栏扔出装备

	static void _evtButtonClickEvent(CGuiData* pSender, int x, int y, MouseClickState key);
	static void _evtRMouseGridEvent(CGuiData* pSender, CCommandObj* pItem, int nGridID);

	bool _UpdataEquip(SItemGrid& Item, int nLink);
	void _ActiveFast(int num); // 激活第几栏

	static void _evtItemFormMouseEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);
	static void _evtRepairEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);

	// 加锁 MSGBOX 确认
	static void _CheckLockMouseEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);

private:
	CForm* frmSkill{nullptr};

	CLabel* labPoint{nullptr};			// 剩余技能点数
	CLabel* labPointLife{nullptr};		// 剩余生活技能点数
	CSkillList* lstFightSkill{nullptr}; // 战斗技能栏
	CSkillList* lstLifeSkill{nullptr};  // 生活技能栏
	CSkillList* lstSailSkill{nullptr};  // 航海技能栏

	CForm* frmItem{nullptr};							// 道具表单
	CGoodsGrid* grdItem{nullptr};						// 道具栏
	std::array<COneCommand*, enumEQUIP_NUM> cnmEquip{}; // 装备栏
	stNetChangeChaPart stEquip;
	stNetShortCut _stShortCut;

	int _nFastCur; // 快捷栏目前在第几位
	CFastCommand** _pFastCommands{nullptr};
	CLabel* _pActiveFastLabel{nullptr}; // 用于激活快捷栏的页码

	CLabel* lblGold{nullptr};  // 金钱
	CImage* imgItem4{nullptr}; // 道具前4格的图片

	CImage* imgLock{nullptr};
	CImage* imgUnLock{nullptr};

private:
	struct stThrow {
		int nX{0}, nY{0};
		int nGridID{0};
		CCharacter* pSelf{nullptr};
	} _sThrow;
	static void _evtThrowDialogEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);
	static void _evtThrowBoatDialogEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);
	void _SendThrowData(const stThrow& sthrow, int nThrowNum = 1);

	struct stUnfix {
		void Reset() {
			nGridID = -1;
			nLink = -1;
			pItem = nullptr;
		}

		int nLink{0};
		int nGridID{0};
		int nX{0}, nY{0};
		CCommandObj* pItem{nullptr};
	} _sUnfix;

	static void _evtUnfixDialogEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);
	void _SendUnfixData(const stUnfix& unfix, int nUnfixNum = 1);
	void _StartUnfix(stUnfix& unfix);

	static void _evtLostYesNoEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);
	int _nLostGridID;

	using vskill = std::vector<CSkillRecord*>;
	vskill _skills;

	using ints = std::vector<int>;
	vskill _cancels; // 可被击活的技能状态

	using states = std::vector<CSkillStateRecord*>;
	states _charges; // 充电的技能状态

	static CGuiPic _imgCharges[enumEQUIP_NUM];

	struct SSplitItem {
		static void _evtSplitItemEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey); // 拆分道具回调
		int nFirst;
		int nSecond;
		CCharacter* pSelf;
	};
	static SSplitItem SSplit;
};

inline CSkillList* CEquipMgr::GetSkillList(char nType) {
	switch (nType) {
	case enumSKILL_SAIL:
		return lstSailSkill;
	case enumSKILL_FIGHT:
		return lstFightSkill;
	default:
		return lstLifeSkill;
	}
}

} // namespace GUI
