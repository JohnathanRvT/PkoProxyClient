#pragma once
#include "GuildData.h"
#include "uiglobalvar.h"

namespace GUI {

class CUIGuildApply : public CUIInterface {
public:
	CUIGuildApply();
	~CUIGuildApply();
	static void ShowForm(CGuildData::eType type);

protected:
	virtual bool Init() override;

private:
	static void OnConfirm(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);
	static void OnShowForm(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);
	static void OnEscClose(CForm* pForm);

	static CForm* m_pGuildNameInputForm;
	static CEdit* m_pGuildNameEdit;
	static CEdit* m_pGuildPasswordEdit;
	static CEdit* m_pGuildConfirmEdit;
	static CGuildData::eType m_eType;
};

} // namespace GUI
