//----------------------------------------------------------------------
// 名称:技能
// 作者:lh 2004-11-07
// 最后修改日期:
//----------------------------------------------------------------------
#pragma once
#include "UICommand.h"
#include "SkillRecord.h"

class CAniClock;
class CSkillRecord;
class CCharacter;

namespace GUI {

class CSkillCommand : public CCommandObj {
public:
	CSkillCommand(CSkillRecord* pSkill);
	CSkillCommand(const CSkillCommand& rhs);
	CSkillCommand& operator=(const CSkillCommand& rhs);
	~CSkillCommand() = default;
	ITEM_CLONE(CSkillCommand)

	virtual void Render(int x, int y) override;
	virtual bool UseCommand() override;
	virtual bool StartCommand() override;
	virtual bool IsAllowUse() const override;

	virtual const char* GetName() const override;
	const char* GetSkillName() const;
	int GetSkillID() const { return _pSkill->sID; }
	CSkillRecord* GetSkillRecord() { return _pSkill; }

	static CGuiPic* GetActiveImage() { return &_imgActive; }

	enum eSpecialType {
		enumHighLight = 1, // 一直显示高亮
		enumNotUpgrade = 2 // 不允许手动升级
	};

	bool GetIsSpecial(eSpecialType SpecialType);

protected:
	virtual bool IsAtOnce() override;
	virtual bool ReadyUse() override;
	virtual void Error() override;
	virtual void AddHint(int x, int y) override;

protected:
	int _GetSkillTime() const;
	bool _WriteNeed(int nType, int nValue, const char* szStr);

protected:
	std::unique_ptr<CGuiPic> _pImage{nullptr};
	CAniClock* _pAniClock{nullptr};
	CSkillRecord* _pSkill{nullptr};

	DWORD _dwPlayTime{0};

	static CGuiPic _imgActive; // 显示激活的边框
};

// 内联函数
inline int CSkillCommand::_GetSkillTime() const {
	return _pSkill->GetFireSpeed();
}

} // namespace GUI
