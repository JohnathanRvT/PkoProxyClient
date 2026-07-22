#include "StdAfx.h"
#include "uiCozeform.h"
#include "uiformmgr.h"
#include "uigrid.h"
#include "UITeam.h"
#include "UIChat.h"
#include "UICombo.h"
#include "gameapp.h"
#include "character.h"
#include "netchat.h"
#include "packetcmd.h"
#include "uiheadsay.h"
#include "StringLib.h"
#include "UIBoatForm.h"
#include "UIMiniMapForm.h"

//const WORD					CCharMsg::m_wTotalChannelsCount;
//const DWORD					CCharMsg::m_dwChannelBufferSize;
CCharMsg::lstTextInfoType CCharMsg::m_lstMsgLink;
CCharMsg::sChannelInfo CCharMsg::m_sChannelInfo[m_wTotalChannelsCount] =
	{
		{
			CHANNEL_NONE,
			/*""	  ,	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"",
			0,
		}, //CHANNEL_NONE
		{
			CHANNEL_ALL,
			/*"RES_STRING(CL_LANGUAGE_MATCH_493)",	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"",
			0,
		}, //CHANNEL_ALL
		{
			CHANNEL_SIGHT,
			/*"RES_STRING(CL_LANGUAGE_MATCH_494)",	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"",
			0,
		}, //CHANNEL_SIGHT
		{
			CHANNEL_PRIVATE,
			/*"RES_STRING(CL_LANGUAGE_MATCH_495)",	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"@",
			0,
		}, //CHANNEL_PRIVATE
		{
			CHANNEL_WORLD,
			/*"RES_STRING(CL_LANGUAGE_MATCH_496)",	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"*",
			0,
		}, //CHANNEL_WORLD
		{
			CHANNEL_TRADE,
			/*"RES_STRING(CL_LANGUAGE_MATCH_497)",	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"^",
			0,
		}, //CHANNEL_TRADE
		{
			CHANNEL_TEAM,
			/*"RES_STRING(CL_LANGUAGE_MATCH_299)",	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"!",
			0,
		}, //CHANNEL_TEAM
		{
			CHANNEL_GUILD,
			/*"RES_STRING(CL_LANGUAGE_MATCH_468)",	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"%",
			0,
		}, //CHANNEL_GUILD
		{
			CHANNEL_SYSTEM,
			/*"RES_STRING(CL_LANGUAGE_MATCH_498)",	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"",
			0,
		}, //CHANNEL_SYSTEM
		{
			CHANNEL_PUBLISH,
			/*"RES_STRING(CL_LANGUAGE_MATCH_499)",	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"",
			0,
		}, //CHANNEL_PUBLISH
		{
			CHANNEL_SIDE,
			/*"RES_STRING(CL_LANGUAGE_MATCH_932)",	*/ 0xFFFFFFFF,
			0xFFFFFFFF,
			"|",
			0,
		}, //CHANNEL_SIDE
};

CCharMsg::lstInstanceType CCharMsg::m_lstThisInstanceLink;

CCharMsg::CCharMsg() {
	m_ecboShowChannels = CHANNEL_ALL | CHANNEL_SIGHT | CHANNEL_PRIVATE | CHANNEL_WORLD | CHANNEL_TRADE | CHANNEL_TEAM | CHANNEL_GUILD | CHANNEL_SYSTEM | CHANNEL_PUBLISH | CHANNEL_SIDE;
	m_bCurMsgAvailable = false;
	m_itCurrentMsgPos = m_lstMsgLink.end();
	m_lstThisInstanceLink.push_back(this);
}

CCharMsg::~CCharMsg() {
	m_lstThisInstanceLink.remove(this);
}

WORD CCharMsg::GetChannelIndex(eChannel channel) {
	DWORD dwChannel = (DWORD)channel;
	for (WORD i = 1; i <= m_wTotalChannelsCount; i++) {
		if (dwChannel & 1)
			return i;
		dwChannel = dwChannel >> 1;
	}
	return 0;
}

void CCharMsg::SetChannelColor(eChannel channel, DWORD dwColor) {
	m_sChannelInfo[GetChannelIndex(channel)].dwChannelColor = dwColor;
}

DWORD CCharMsg::GetChannelColor(eChannel channel) {
	return m_sChannelInfo[GetChannelIndex(channel)].dwChannelColor;
}

void CCharMsg::SetChannelPrefixColor(eChannel channel, DWORD dwColor) {
	m_sChannelInfo[GetChannelIndex(channel)].dwChannelPrefixColor = dwColor;
}

DWORD CCharMsg::GetChannelPrefixColor(eChannel channel) {
	return m_sChannelInfo[GetChannelIndex(channel)].dwChannelPrefixColor;
}

std::string CCharMsg::GetChannelCommand(WORD wChannelIndex) {
	return m_sChannelInfo[wChannelIndex].strChannelCommand;
}

std::string CCharMsg::GetChannelName(eChannel channel) {
	std::string str;

	switch (channel) {
	case CHANNEL_ALL:
		str = RES_STRING(CL_LANGUAGE_MATCH_493);
		break;
	case CHANNEL_SIGHT:
		str = RES_STRING(CL_LANGUAGE_MATCH_494);
		break;
	case CHANNEL_PRIVATE:
		str = RES_STRING(CL_LANGUAGE_MATCH_495);
		break;
	case CHANNEL_WORLD:
		str = RES_STRING(CL_LANGUAGE_MATCH_496);
		break;
	case CHANNEL_TRADE:
		str = RES_STRING(CL_LANGUAGE_MATCH_497);
		break;
	case CHANNEL_TEAM:
		str = RES_STRING(CL_LANGUAGE_MATCH_299);
		break;
	case CHANNEL_GUILD:
		str = RES_STRING(CL_LANGUAGE_MATCH_468);
		break;
	case CHANNEL_SYSTEM:
		str = RES_STRING(CL_LANGUAGE_MATCH_498);
		break;
	case CHANNEL_PUBLISH:
		str = RES_STRING(CL_LANGUAGE_MATCH_499);
		break;
	case CHANNEL_SIDE:
		str = RES_STRING(CL_LANGUAGE_MATCH_932);
		break;
#ifdef MANAGE_VER
	case CHANNEL_MANAGE:
		str = "LARK";
		break;
#endif
	default:
		break;
	}

	return str;
}

std::string CCharMsg::GetChannelShowName(eChannel channel) {
	return "[" + GetChannelName(channel) + "]";
}

CCharMsg::eChannel CCharMsg::GetChannelByIndex(WORD wChannelIndex) {
	return m_sChannelInfo[wChannelIndex].enumChannel;
}

WORD CCharMsg::GetTotalChannelCount() {
	return m_wTotalChannelsCount;
}

void CCharMsg::AddMsg(eChannel channel, std::string strWho, std::string strText, bool bSendTo) {
	sTextInfo sText;
	if (channel == CHANNEL_PRIVATE) {
		if (bSendTo) {
			sText.strShowText = GetChannelShowName(channel) + "<To " + strWho + ">: ";
		} else {
			sText.strShowText = GetChannelShowName(channel) + "<From " + strWho + ">: ";
		}
	} else {
		sText.strShowText = GetChannelShowName(channel) + strWho;
		if (!strWho.empty()) {
			sText.strShowText += ": ";
		}
	}
	sText.strShowText += strText;
	sText.eTextChannel = channel;
	sText.strText = strText;
	sText.bSendTo = bSendTo;
	sText.strWho = strWho;

	WORD wChannelIndex = GetChannelIndex(channel);
	if (m_sChannelInfo[wChannelIndex].dwTotalMsg >= m_dwChannelBufferSize) {
		//缓冲区满
		lstTextInfoType::iterator iterRemoveObject = m_sChannelInfo[wChannelIndex].itFirstMsgPos;
		while (m_sChannelInfo[wChannelIndex].itFirstMsgPos != m_lstMsgLink.end()) {
			m_sChannelInfo[wChannelIndex].itFirstMsgPos++;
			sTextInfo sTempText = *(m_sChannelInfo[wChannelIndex].itFirstMsgPos);
			if (wChannelIndex == GetChannelIndex(sTempText.eTextChannel)) {
				break;
			}
		}
		if (iterRemoveObject != m_lstMsgLink.end()) {
			for (lstInstanceType::iterator it = m_lstThisInstanceLink.begin(); it != m_lstThisInstanceLink.end(); it++) {
				if (iterRemoveObject == (*it)->m_itCurrentMsgPos) {
					(*it)->m_bCurMsgAvailable = false;
					if ((*it)->m_itCurrentMsgPos == m_lstMsgLink.begin()) {
						(*it)->m_itCurrentMsgPos = m_lstMsgLink.end();
					} else {
						(*it)->m_itCurrentMsgPos--;
					}
				}
			}
			m_lstMsgLink.erase(iterRemoveObject);
		}
		m_sChannelInfo[wChannelIndex].dwTotalMsg--;
	}

	m_lstMsgLink.push_back(sText);

	if (m_sChannelInfo[wChannelIndex].dwTotalMsg++ == 0) {
		m_sChannelInfo[wChannelIndex].itFirstMsgPos = m_lstMsgLink.end()--;
	}
}

void CCharMsg::ClearAllMsg() {
	m_lstMsgLink.clear();
	for (WORD i = 0; i < m_wTotalChannelsCount; i++) {
		m_sChannelInfo[i].dwTotalMsg = 0;
		m_sChannelInfo[i].itFirstMsgPos = m_lstMsgLink.end();
	}
	for (lstInstanceType::iterator it = m_lstThisInstanceLink.begin(); it != m_lstThisInstanceLink.end(); it++) {
		(*it)->m_bCurMsgAvailable = false;
		(*it)->m_itCurrentMsgPos = m_lstMsgLink.end();
	}
}

void CCharMsg::SetShowChannels(DWORD ecboShowChannels) {
	m_ecboShowChannels = ecboShowChannels;
}

bool CCharMsg::ModifyShowChannel(eChannel eShowChannel, bool bAddOrRemove, bool bShowTips) {
	bool bOrgShow = (m_ecboShowChannels & eShowChannel) ? true : false;
	if (bOrgShow != bAddOrRemove) {
		if (bAddOrRemove) {
			SetShowChannels(m_ecboShowChannels | eShowChannel);
			if (bShowTips) {
				CCozeForm::GetInstance()->OnSystemMsg(GetChannelName(eShowChannel) + RES_STRING(CL_LANGUAGE_MATCH_500));
			}
		} else {
			SetShowChannels(m_ecboShowChannels & ~eShowChannel);
			if (bShowTips) {
				CCozeForm::GetInstance()->OnSystemMsg(GetChannelName(eShowChannel) + RES_STRING(CL_LANGUAGE_MATCH_501));
			}
		}
		return true;
	}
	return false;
}

bool CCharMsg::IsShowChannel(eChannel channel) {
	return (m_ecboShowChannels & channel) ? true : false;
}

DWORD CCharMsg::GetShowChannels() {
	return m_ecboShowChannels;
}

bool CCharMsg::MoveToFirstMsg() {
	m_itCurrentMsgPos = m_lstMsgLink.begin();
	m_bCurMsgAvailable = !m_lstMsgLink.empty();
	if (!m_bCurMsgAvailable)
		return false;
	if ((*m_itCurrentMsgPos).eTextChannel & m_ecboShowChannels) {
		return true;
	} else {
		return MoveToNextMsg();
	}
}

bool CCharMsg::MoveToNextMsg() {
	if (m_itCurrentMsgPos == m_lstMsgLink.end()) {
		if (m_lstMsgLink.empty()) {
			return false;
		}
		m_itCurrentMsgPos = m_lstMsgLink.begin();
		if ((*m_itCurrentMsgPos).eTextChannel & m_ecboShowChannels) {
			m_bCurMsgAvailable = true;
			return true;
		}
	}

	lstTextInfoType::iterator iterTemp = m_itCurrentMsgPos;
	while (++iterTemp != m_lstMsgLink.end()) {
		sTextInfo sTempText = *(iterTemp);
		if (sTempText.eTextChannel & m_ecboShowChannels) {
			m_bCurMsgAvailable = true;
			m_itCurrentMsgPos = iterTemp;
			return true;
		}
	}
	return false;
}

CCharMsg::sTextInfo CCharMsg::GetMsgInfo() {
	sTextInfo sText;
	sText.eTextChannel = CHANNEL_ALL;
	sText.strWho = RES_STRING(CL_LANGUAGE_MATCH_2);
	sText.strText = RES_STRING(CL_LANGUAGE_MATCH_502);
	sText.strShowText = RES_STRING(CL_LANGUAGE_MATCH_502);
	sText.bSendTo = false;
	if (m_bCurMsgAvailable && m_itCurrentMsgPos != m_lstMsgLink.end()) {
		sText = *m_itCurrentMsgPos;
	}
	return sText;
}

CCardCase::CCardCase(WORD wMaxLimit) {
	m_wMaxLimit = wMaxLimit;
	MoveToFirstCard();
}

CCardCase::~CCardCase() {
}

void CCardCase::AddCard(std::string str) {
	if (str.empty())
		return;

	for (lstCardType::iterator it = m_lstCardData.begin(); it != m_lstCardData.end(); it++) {
		if ((*it) == str) {
			m_lstCardData.erase(it);
			m_lstCardData.push_back(str);
			return;
		}
	}

	m_lstCardData.push_back(str);
	if ((WORD)m_lstCardData.size() > m_wMaxLimit) {
		m_lstCardData.erase(m_lstCardData.begin());
	}
}

bool CCardCase::RemoveCard(std::string str) {
	for (lstCardType::iterator it = m_lstCardData.begin(); it != m_lstCardData.end(); it++) {
		if ((*it) == str) {
			m_lstCardData.erase(it);
			return true;
		}
	}
	return false;
}

void CCardCase::ClearAll() {
	m_lstCardData.clear();
	MoveToFirstCard();
}

int CCardCase::GetTotalCount() {
	return (int)m_lstCardData.size();
}

bool CCardCase::MoveToFirstCard() {
	m_itCurrentCardPos = m_lstCardData.begin();
	if (GetTotalCount() == 0) {
		return false;
	}
	return true;
}

bool CCardCase::MoveToNextCard() {
	lstCardType::iterator iterTemp = m_itCurrentCardPos;
	if (++iterTemp == m_lstCardData.end()) {
		return false;
	}
	m_itCurrentCardPos = iterTemp;
	return true;
}

bool CCardCase::MoveToLastCard() {
	m_itCurrentCardPos = m_lstCardData.end();
	if (GetTotalCount() == 0) {
		return false;
	}
	--m_itCurrentCardPos;
	return true;
}

bool CCardCase::MoveToPrevCard() {
	if (m_itCurrentCardPos == m_lstCardData.begin()) {
		return false;
	}
	--m_itCurrentCardPos;
	return true;
}

std::string CCardCase::GetCardInfo() {
	if (GetTotalCount() == 0) {
		return "";
	}

	return *m_itCurrentCardPos;
}

using namespace GUI;

CChannelSwitchForm::CChannelSwitchForm() {
}

CChannelSwitchForm::~CChannelSwitchForm() {
}

extern CChannelSwitchForm g_stUIChannelSwitch;
CChannelSwitchForm* CChannelSwitchForm::GetInstance() {
	//static CChannelSwitchForm s_ChannelSwitchForm;
	return (&g_stUIChannelSwitch);
}

void CChannelSwitchForm::SwitchCheck() {
	CCozeForm* pCozeForm = CCozeForm::GetInstance();
	if (m_frmChannelSwitch->GetIsShow()) {
		pCozeForm->m_chkChannelSwitch->SetIsChecked(false);
		m_frmChannelSwitch->SetIsShow(false);
	} else {
		CChannelSwitchForm::GetInstance()->m_chkPrivate->SetIsChecked(pCozeForm->m_cMainMsg.IsShowChannel(CCharMsg::CHANNEL_PRIVATE));
		CChannelSwitchForm::GetInstance()->m_chkSight->SetIsChecked(pCozeForm->m_cMainMsg.IsShowChannel(CCharMsg::CHANNEL_SIGHT));
		CChannelSwitchForm::GetInstance()->m_chkSystem->SetIsChecked(pCozeForm->m_lstSystemPage->GetIsShow());
		CChannelSwitchForm::GetInstance()->m_chkTeam->SetIsChecked(pCozeForm->m_cMainMsg.IsShowChannel(CCharMsg::CHANNEL_TEAM));
		CChannelSwitchForm::GetInstance()->m_chkGuild->SetIsChecked(pCozeForm->m_cMainMsg.IsShowChannel(CCharMsg::CHANNEL_GUILD));
		CChannelSwitchForm::GetInstance()->m_chkWorld->SetIsChecked(pCozeForm->m_cMainMsg.IsShowChannel(CCharMsg::CHANNEL_WORLD));
		CChannelSwitchForm::GetInstance()->m_chkTrade->SetIsChecked(pCozeForm->m_cMainMsg.IsShowChannel(CCharMsg::CHANNEL_TRADE));

		pCozeForm->m_chkChannelSwitch->SetIsChecked(true);
		m_frmChannelSwitch->SetIsShow(true);
	}
}

bool CChannelSwitchForm::Init() {
	FORM_LOADING_CHECK(m_frmChannelSwitch, "main.clu", "frmForbid");
	m_frmChannelSwitch->evtLost = EventLostFocus;
	FORM_CONTROL_LOADING_CHECK(m_chkPrivate, m_frmChannelSwitch, CCheckBox, "main.clu", "chkPersonal");
	FORM_CONTROL_LOADING_CHECK(m_chkSight, m_frmChannelSwitch, CCheckBox, "main.clu", "chkNear");
	FORM_CONTROL_LOADING_CHECK(m_chkSystem, m_frmChannelSwitch, CCheckBox, "main.clu", "chkSystem");
	FORM_CONTROL_LOADING_CHECK(m_chkTeam, m_frmChannelSwitch, CCheckBox, "main.clu", "chkTeam");
	FORM_CONTROL_LOADING_CHECK(m_chkGuild, m_frmChannelSwitch, CCheckBox, "main.clu", "chkGuild");
	FORM_CONTROL_LOADING_CHECK(m_chkWorld, m_frmChannelSwitch, CCheckBox, "main.clu", "chkWorld");
	FORM_CONTROL_LOADING_CHECK(m_chkTrade, m_frmChannelSwitch, CCheckBox, "main.clu", "chkTrade");
	m_chkPrivate->evtCheckChange = EventPrivateCheckChange;
	m_chkSight->evtCheckChange = EventSightCheckChange;
	m_chkSystem->evtCheckChange = EventSystemCheckChange;
	m_chkTeam->evtCheckChange = EventTeamCheckChange;
	m_chkGuild->evtCheckChange = EventGuildCheckChange;
	m_chkWorld->evtCheckChange = EventWorldCheckChange;
	m_chkTrade->evtCheckChange = EventTradeCheckChange;

	return true;
}

void CChannelSwitchForm::EventLostFocus(CGuiData* pSender) {
	CCozeForm* pCozeForm = CCozeForm::GetInstance();
	pCozeForm->m_chkChannelSwitch->SetIsChecked(false);
	pCozeForm->m_chkChannelSwitch->MouseRun(pCozeForm->m_chkChannelSwitch->GetX() - 1, pCozeForm->m_chkChannelSwitch->GetY() - 1, MouseClickState());
	CChannelSwitchForm::GetInstance()->m_frmChannelSwitch->SetIsShow(false);
}

void CChannelSwitchForm::EventPrivateCheckChange(CGuiData* pSender) {
	CCozeForm* pCozeForm = CCozeForm::GetInstance();
	pCozeForm->m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_PRIVATE, CChannelSwitchForm::GetInstance()->m_chkPrivate->GetIsChecked());
	pCozeForm->ResetPages();
}

void CChannelSwitchForm::EventSightCheckChange(CGuiData* pSender) {
	CCozeForm* pCozeForm = CCozeForm::GetInstance();
	pCozeForm->m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_SIGHT, CChannelSwitchForm::GetInstance()->m_chkSight->GetIsChecked());
	pCozeForm->ResetPages();
}

void CChannelSwitchForm::EventSystemCheckChange(CGuiData* pSender) {
	CCozeForm* pCozeForm = CCozeForm::GetInstance();

	bool bCheck = CChannelSwitchForm::GetInstance()->m_chkSystem->GetIsChecked();
	pCozeForm->m_lstSystemPage->SetIsShow(bCheck);
	pCozeForm->m_drgSystemPage->SetIsShow(bCheck);

	pCozeForm->m_cSystemMsg.ModifyShowChannel(CCharMsg::CHANNEL_SYSTEM, CChannelSwitchForm::GetInstance()->m_chkSystem->GetIsChecked());
	pCozeForm->ResetPages();
}

void CChannelSwitchForm::EventTeamCheckChange(CGuiData* pSender) {
	CCozeForm* pCozeForm = CCozeForm::GetInstance();
	pCozeForm->m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_TEAM, CChannelSwitchForm::GetInstance()->m_chkTeam->GetIsChecked());
	pCozeForm->ResetPages();
}

void CChannelSwitchForm::EventGuildCheckChange(CGuiData* pSender) {
	CCozeForm* pCozeForm = CCozeForm::GetInstance();
	pCozeForm->m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_GUILD, CChannelSwitchForm::GetInstance()->m_chkGuild->GetIsChecked());
	pCozeForm->ResetPages();
}

void CChannelSwitchForm::EventWorldCheckChange(CGuiData* pSender) {
	CCozeForm* pCozeForm = CCozeForm::GetInstance();
	pCozeForm->m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_WORLD, CChannelSwitchForm::GetInstance()->m_chkWorld->GetIsChecked());
	pCozeForm->ResetPages();
}

void CChannelSwitchForm::EventTradeCheckChange(CGuiData* pSender) {
	CCozeForm* pCozeForm = CCozeForm::GetInstance();
	pCozeForm->m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_TRADE, CChannelSwitchForm::GetInstance()->m_chkTrade->GetIsChecked());
	pCozeForm->ResetPages();
}

const int CCozeForm::m_nMainPageMinHeight = 70;
const int CCozeForm::m_nMainPageMaxHeight = 280;
const int CCozeForm::m_nSystemPageMinHeight = 55;
const int CCozeForm::m_nSystemPageMaxHeight = 240;

CCozeForm::CCozeForm() : m_cCallingCard(10), m_cSendMsgCard(10), m_bSendMsgCardSwitch(false) {
	m_eCurSelChannel = CCharMsg::CHANNEL_SIGHT;
}

CCozeForm::~CCozeForm() {
}

extern CCozeForm g_stUICozeForm;
CCozeForm* CCozeForm::GetInstance() {
	//static CCozeForm s_CozeForm;
	return &g_stUICozeForm;
}

void CCozeForm::OnSightMsg(std::string strName, std::string strMsg) {
	CCharMsg::AddMsg(CCharMsg::CHANNEL_SIGHT, strName, strMsg);
	UpdatePages();
}

void CCozeForm::OnSightMsg(CCharacter* pChar, std::string strMsg) {
	if (!pChar)
		return;

	char buf[10];
	//for (int i=0; i<=9; i++)
	for (int i = 0; i <= 50; i++) //Modify by sunny.sun 20080902
	{
		_snprintf_s(buf, _TRUNCATE, "***%d", i);
		if (strMsg == buf) {
			pChar->GetHeadSay()->SetFaceID(i);
			return;
		}
	}

	//Add by sunny.sun 20080902
	//Begin
	const std::string pcnsenderface = "#21";				 //服务器收不到的字符串
	const std::string preplaceface = "#99";					 //替换发送的字符串
	ReplaceSpecialFace(strMsg, pcnsenderface, preplaceface); //替换字符串
	//End
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
	OnSightMsg(pChar->getHumanName(), strMsg);

	std::string str = std::string(pChar->getHumanName()) + ": ";
	size_t player_name_string_length = str.length();
	str += strMsg;
	auto* item = new CItemEx(str, CCharMsg::GetChannelColor(CCharMsg::CHANNEL_SIGHT));
	item->ProcessString(player_name_string_length);
	pChar->GetHeadSay()->AddItem(item);
}

void CCozeForm::OnPrivateMsg(std::string strFromName, std::string strToName, std::string strMsg) {
	if (!CGameScene::GetMainCha())
		return;

	CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
	std::string strHumanName = CGameScene::GetMainCha()->getHumanName();
	if (strHumanName == strFromName) {
		if (strMsg != "{x}*") {
			CCharMsg::AddMsg(CCharMsg::CHANNEL_PRIVATE, strToName, strMsg, true);
			m_cCallingCard.AddCard(strToName);
		}
	} else {
		if (strMsg == "{x}*") {
			OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_505) + strFromName + RES_STRING(CL_LANGUAGE_MATCH_506));
		} else {
			CCharMsg::AddMsg(CCharMsg::CHANNEL_PRIVATE, strFromName, strMsg, false);
			if (m_cMainMsg.IsShowChannel(CCharMsg::CHANNEL_PRIVATE)) {
				m_cCallingCard.AddCard(strFromName);
			} else {
				std::string strContent = "{x}*";
				CS_Say2You(strFromName.c_str(), strContent.c_str());
			}
		}
	}
	UpdatePages();
}

void CCozeForm::OnWorldMsg(std::string strName, std::string strMsg) {
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
	CCharMsg::AddMsg(CCharMsg::CHANNEL_WORLD, strName, strMsg);
	UpdatePages();
}

void CCozeForm::OnTradeMsg(std::string strName, std::string strMsg) {
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
	CCharMsg::AddMsg(CCharMsg::CHANNEL_TRADE, strName, strMsg);
	UpdatePages();
}

void CCozeForm::OnTeamMsg(std::string strName, std::string strMsg) {
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
	CCharMsg::AddMsg(CCharMsg::CHANNEL_TEAM, strName, strMsg);
	UpdatePages();
}

void CCozeForm::OnTeamMsg(DWORD dwCharID, std::string strMsg) {
	if (!CGameScene::GetMainCha())
		return;

	std::string strName;
	if (dwCharID != CGameScene::GetMainCha()->getHumanID()) {
		CTeam* pTeam = g_stUIChat.GetTeamMgr()->Find(enumTeamGroup);
		if (!pTeam)
			return;
		CMember* pMember = pTeam->Find(dwCharID);
		if (!pMember)
			return;
		strName = pMember->GetName();
	} else {
		strName = CGameScene::GetMainCha()->getHumanName();
	}
	OnTeamMsg(strName, strMsg);
}

void CCozeForm::OnGuildMsg(std::string strName, std::string strMsg) {
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
	CCharMsg::AddMsg(CCharMsg::CHANNEL_GUILD, strName, strMsg);
	UpdatePages();
}

void CCozeForm::OnSystemMsg(std::string strMsg) {
	CCharMsg::AddMsg(CCharMsg::CHANNEL_SYSTEM, "", strMsg);
	UpdatePages();
}

void CCozeForm::OnSideMsg(std::string strName, std::string strMsg) {
	CCharMsg::AddMsg(CCharMsg::CHANNEL_SIDE, strName, strMsg);
	UpdatePages();
}

void CCozeForm::OnPublishMsg(std::string strName, std::string strMsg) {
	std::string str;
	str = RES_STRING(CL_LANGUAGE_MATCH_507) + std::string(strMsg);
	g_pGameApp->ShowStaticNotify(str.c_str(), CCharMsg::GetChannelColor(CCharMsg::CHANNEL_PUBLISH));
}
//Add by sunny.sun20080804
void CCozeForm::OnPublishMsg1(std::string strMsg, int setnum) {
	std::string str;
	str = std::string(strMsg);
	g_pGameApp->ShowScrollingNotify(str.c_str(), setnum, CCharMsg::GetChannelColor(CCharMsg::CHANNEL_PUBLISH));
}
//End
void CCozeForm::OnPrivateNameSet(std::string strName) {
	if (strName.empty())
		return;

	std::string strCurCmd = CCharMsg::GetChannelCommand(CCharMsg::GetChannelIndex(CCharMsg::CHANNEL_PRIVATE)) + strName + " ";
	m_edtMsg->SetCaption(strCurCmd.c_str());
	m_edtMsg->SetActive(m_edtMsg);

	m_cCallingCard.AddCard(strName);
}

void CCozeForm::OnResetAll() {
	m_chkChannelSwitch->SetIsChecked(false);
	m_lstCallingCard->SetIsShow(false);
	m_grdFacePanel->SetIsShow(false);
	m_grdBrowPanel->SetIsShow(false);
	m_grdActionPanel->SetIsShow(false);

	m_eCurSelChannel = CCharMsg::CHANNEL_SIGHT;
	m_cmbChannel->GetList()->GetItems()->Select(0);
	m_edtMsg->SetCaption("");
	m_lstMainPage->GetItems()->Clear();
	m_lstSystemPage->GetItems()->Clear();
	m_cMainMsg.ClearAllMsg();
	m_cSystemMsg.ClearAllMsg();
	m_cCallingCard.ClearAll();
	m_cSendMsgCard.ClearAll();
	m_bSendMsgCardSwitch = false;
}

bool CCozeForm::IsMouseOnList(int x, int y) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	if (pThis->m_lstSystemPage->GetScroll()->IsNormal()) {
		GetRender().ScreenConvert(x, y);
		if (pThis->m_lstSystemPage->GetScroll()->InRect(x, y))
			return false;
	}
	if (pThis->m_lstMainPage->GetScroll()->IsNormal()) {
		GetRender().ScreenConvert(x, y);
		if (pThis->m_lstMainPage->GetScroll()->InRect(x, y))
			return false;
	}
	return true;
}

void CCozeForm::AddToEdit(std::string strData) {
	auto* pEdit = dynamic_cast<CEdit*>(CCompent::GetActive());
	if (!pEdit) {
		if (!m_edtMsg)
			return;
		pEdit = m_edtMsg;
	}

	std::string strMsg = pEdit->GetCaption();
	strMsg += strData;
	if ((int)strlen(strMsg.c_str()) > pEdit->GetMaxNum())
		return;
	pEdit->SetCaption(strMsg.c_str());
}
//Add by sunny.sun 20080902
//Begin
//将strMsg中的strFace替换为strReplace
void CCozeForm::ReplaceSpecialFace(std::string& strMsg, const std::string& strReplace, const std::string& strFace) {
	size_t nPos = strMsg.find(strFace);

	while (nPos != std::string::npos) {
		strMsg.replace(nPos, strFace.length(), strReplace, 0, strReplace.length());
		nPos = strMsg.find(strFace, nPos + (strReplace.length()));
	}
}
//End

bool CCozeForm::Init() {
	m_frmMainChat = _FindForm("frmMain800");
	if (!m_frmMainChat)
		return false;
	CFormMgr::s_Mgr.AddKeyDownEvent(EventGlobalKeyDownHandle);

	FORM_LOADING_CHECK(m_frmPublish, "main.clu", "frmGM");
	m_frmPublish->evtBeforeShow = EventPublishShowForm;

	FORM_CONTROL_LOADING_CHECK(m_edtPublishMsg, m_frmPublish, CEdit, "coze.clu", "edtGMSay");
	m_edtPublishMsg->SetIsWrap(true);
	m_edtPublishMsg->evtEnter = EventPublishSendMsg;

	FORM_CONTROL_LOADING_CHECK(m_edtMsg, m_frmMainChat, CEdit, "main.clu", "edtSay");
	m_edtMsg->SetIsWrap(true);
	m_edtMsg->SetIsParseText(true);
	m_edtMsg->evtEnter = EventSendMsg;
	m_edtMsg->evtKeyDown = EventEditMsg;

	//Add by sunny.sun20080804
	//Begin
	FORM_LOADING_CHECK(m_frmPublish1, "main.clu", "Arena");
	m_frmPublish1->evtBeforeShow = EventPublishShowForm;
	//GM聊天输入表单Arena，edtArenaSay
	FORM_CONTROL_LOADING_CHECK(m_edtPublishMsg1, m_frmPublish1, CEdit, "coze.clu", "edtArenaSay");
	m_edtPublishMsg1->SetIsWrap(true);
	m_edtPublishMsg1->evtEnter = EventPublishSendMsg1;
	//End

	FORM_CONTROL_LOADING_CHECK(m_cmbChannel, m_frmMainChat, CCombo, "main.clu", "cboChannel");
	m_cmbChannel->evtSelectChange = EventSendChannelChange;
	m_cmbChannel->evtMouseClick = EventSendChannelSwitchClick;
	m_cmbChannel->GetList()->GetItems()->Select(0);

	FORM_CONTROL_LOADING_CHECK(m_lstMainPage, m_frmMainChat, CList, "main.clu", "lstOnSay");
	m_lstMainPage->SetIsChangeColor(false);
	m_lstMainPage->SetMouseAction(eMouseAction::Drill);
	m_lstMainPage->SetRowHeight(16);
	m_lstMainPage->evtListMouseDB = EventMainListKeyDown;

	FORM_CONTROL_LOADING_CHECK(m_lstSystemPage, m_frmMainChat, CList, "main.clu", "lstOnSaySystem");
	m_lstSystemPage->SetIsChangeColor(false);
	m_lstSystemPage->SetMouseAction(eMouseAction::Drill);
	m_lstSystemPage->SetRowHeight(16);
	m_cSystemMsg.SetShowChannels(CCharMsg::CHANNEL_SYSTEM);

	FORM_CONTROL_LOADING_CHECK(m_drgMainPage, m_frmMainChat, CDragTitle, "main.clu", "drpTitle");
	m_drgMainPage->SetIsShowDrag(false);
	m_drgMainPage->GetDrag()->SetYare(0);
	m_drgMainPage->GetDrag()->SetDragInCursor(CCursor::stVertical);
	m_drgMainPage->GetDrag()->evtMouseDragBegin = EventMainPageDragBegin;
	m_drgMainPage->GetDrag()->evtMouseDragMove = EventMainPageDragging;
	m_drgMainPage->GetDrag()->evtMouseDragEnd = EventMainPageDragEnd;

	FORM_CONTROL_LOADING_CHECK(m_drgSystemPage, m_frmMainChat, CDragTitle, "main.clu", "drpTitleSystem");
	m_drgSystemPage->SetIsShowDrag(false);
	m_drgSystemPage->GetDrag()->SetYare(0);
	m_drgSystemPage->GetDrag()->SetDragInCursor(CCursor::stVertical);
	m_drgSystemPage->GetDrag()->evtMouseDragBegin = EventSystemPageDragBegin;
	m_drgSystemPage->GetDrag()->evtMouseDragMove = EventSystemPageDragging;
	m_drgSystemPage->GetDrag()->evtMouseDragEnd = EventSystemPageDragEnd;

	FORM_CONTROL_LOADING_CHECK(m_chkChannelSwitch, m_frmMainChat, CCheckBox, "main.clu", "chkChatOn");
	m_chkChannelSwitch->SetIsChecked(false);
	m_chkChannelSwitch->evtCheckChange = EventChannelSwitchCheck;

	FORM_CONTROL_LOADING_CHECK(m_btnCallingCardSwitch, m_frmMainChat, CTextButton, "main.clu", "btnCard");
	m_btnCallingCardSwitch->evtMouseClick = EventCallingCardSwitchClick; //switch

	FORM_CONTROL_LOADING_CHECK(m_lstCallingCard, m_frmMainChat, CList, "main.clu", "lstCard");
	//m_lstCallingCard->evtLost=EventCallingCardLostFocus; -arcol 无效事件
	m_lstCallingCard->evtSelectChange = EventCardSelected;
	m_lstCallingCard->SetIsChangeColor(false);

	FORM_CONTROL_LOADING_CHECK(m_btnFaceSwitch, m_frmMainChat, CTextButton, "main.clu", "btnChatFace");
	m_btnFaceSwitch->evtMouseClick = EventFacePanelSwitchClick; //switch

	FORM_CONTROL_LOADING_CHECK(m_grdFacePanel, m_frmMainChat, CGrid, "main.clu", "grdFace");
	//m_grdFacePanel->evtLost=EventFacePanelLostFocus;
	m_grdFacePanel->evtSelectChange = EventFaceSelected;

	FORM_CONTROL_LOADING_CHECK(m_btnBrowSwitch, m_frmMainChat, CTextButton, "main.clu", "btnBrow");
	m_btnBrowSwitch->evtMouseClick = EventBrowPanelSwitchClick; //switch

	FORM_CONTROL_LOADING_CHECK(m_grdBrowPanel, m_frmMainChat, CGrid, "main.clu", "grdHeart");
	m_grdBrowPanel->evtSelectChange = EventBrowSelected;

	FORM_CONTROL_LOADING_CHECK(m_btnActionSwitch, m_frmMainChat, CTextButton, "main.clu", "btnAction");
	m_btnActionSwitch->evtMouseClick = EventActionPanelSwitchClick; //switch

	FORM_CONTROL_LOADING_CHECK(m_grdActionPanel, m_frmMainChat, CGrid, "main.clu", "grdAction");
	m_grdActionPanel->evtSelectChange = EventActionSelected;

	m_frmMainChat->SetIsShow(true);
	CChannelSwitchForm::GetInstance();

	m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_SYSTEM, false, false);
	ResetPages();
	return true;
}

CCharMsg::eChannel CCozeForm::GetCmdFromMsg(std::string strMsg) {
	std::string strCmd;
	strCmd.push_back(strMsg[0]);
	for (WORD i = 0; i < CCharMsg::GetTotalChannelCount(); i++) {
		if (strCmd == CCharMsg::GetChannelCommand(i)) {
			return CCharMsg::GetChannelByIndex(i);
		}
	}
	return CCharMsg::CHANNEL_NONE;
}

void CCozeForm::SendMsg() {
	CCharacter* pChar = CGameScene::GetMainCha();
	if (!pChar)
		return;

	std::string strMsg = m_edtMsg->GetCaption();
	if (strMsg.empty() || strMsg.length() == count(strMsg.begin(), strMsg.end(), ' ')) {
		m_lstSystemPage->OnKeyDown(VK_END);
		m_lstMainPage->OnKeyDown(VK_END);
		return;
	}

	if (g_stUIMap.IsPKSilver() && pChar->getGMLv() <= 0) {
		g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_901)); // 乱斗白银城中禁止发送消息
		return;
	}

	std::string strCurCmd = "";
	CCharMsg::eChannel enumChannel = GetCmdFromMsg(strMsg);
	if (enumChannel == CCharMsg::CHANNEL_PRIVATE) {
		int p = (int)(strMsg.find(" "));
		if (p != -1) {
			std::string strPersonName = strMsg.substr(1, p - 1); //私聊对象
			std::string strContent = strMsg.substr(p + 1);
			if (!strContent.empty()) {
				CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strContent);
				if (!strPersonName.empty() && strPersonName.length() <= 16) {
					m_cSendMsgCard.AddCard(strContent);
					m_bSendMsgCardSwitch = false;
					if (m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_PRIVATE)) {
						ResetPages();
					}
					strCurCmd = CCharMsg::GetChannelCommand(CCharMsg::GetChannelIndex(CCharMsg::CHANNEL_PRIVATE)) + strPersonName + " ";

					CCharacter* pMain = CGameScene::GetMainCha();
					if (pMain && (pMain->IsBoat() || pMain->getGameAttr()->get(ATTR_LV) >= 9)) {
						CS_Say2You(strPersonName.c_str(), strContent.c_str());
					} else {
						// 角色等级9级以下禁止发起密语私聊
						g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_868));
					}

					m_edtMsg->SetCaption(strCurCmd.c_str());
				}
			}
		}
		return;
	} else if (enumChannel == CCharMsg::CHANNEL_NONE) {
		enumChannel = m_eCurSelChannel;
	} else {
		strMsg.erase(0, 1);
		if (strMsg.empty())
			return;
	}
	m_cSendMsgCard.AddCard(strMsg);
	m_bSendMsgCardSwitch = false;

	switch (enumChannel) {
	case CCharMsg::CHANNEL_SIGHT: {
		static std::string preStr = "";
		static DWORD preTime = 0;
		if (preStr == strMsg && GetTickCount() - preTime < 1000) {
			OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_508));
		} else {
			if (m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_SIGHT)) {
				ResetPages();
			}
			CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
			//Add by sunny.sun 20080902
			//Begin
			const std::string pcnsenderface = "#21";
			const std::string preplaceface = "#99";
			ReplaceSpecialFace(strMsg, preplaceface, pcnsenderface);
			//End
			CS_Say(strMsg.c_str());
			preStr = strMsg;
			preTime = GetTickCount();
		}
		break;
	}
	case CCharMsg::CHANNEL_WORLD: {
		CCharacter* pChar = g_stUIBoat.GetHuman();
		if (!pChar || pChar->getGameAttr()->get(ATTR_LV) < 10) {
			OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_509));
		} else // modify by Philip.Wu  2006-06-09  避免 10 级以下使用世界频道
		{
			if (GetTickCount() - g_pGameApp->GetLoginTime() > 60000) {
				if (m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_WORLD)) {
					ResetPages();
				}
				CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
				CS_Say2All(strMsg.c_str());
			} else {
				OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_510));
			}
		}
		break;
	}
	case CCharMsg::CHANNEL_TRADE: {
		CCharacter* pChar = g_stUIBoat.GetHuman();
		if (!pChar || pChar->getGameAttr()->get(ATTR_LV) < 10) {
			OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_511));
		} else // modify by Philip.Wu  2006-06-09  避免 10 级以下使用贸易频道
		{
			if (GetTickCount() - g_pGameApp->GetLoginTime() > 60000) {
				if (m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_TRADE)) {
					ResetPages();
				}
				CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
				CS_Say2Trade(strMsg.c_str());
			} else {
				OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_512));
			}
		}
		break;
	}
	case CCharMsg::CHANNEL_TEAM: {
		CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
		if (!strMsg.empty()) {
			CTeam* pTeam = g_stUIChat.GetTeamMgr()->Find(enumTeamGroup);
			if (!pTeam || pTeam->GetCount() == 0) {
				OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_513));
			} else {
				if (m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_TEAM)) {
					ResetPages();
				}
				CS_Say2Team(strMsg.c_str());
			}
		}
		break;
	}
	case CCharMsg::CHANNEL_GUILD: {
		if (m_cMainMsg.ModifyShowChannel(CCharMsg::CHANNEL_GUILD)) {
			ResetPages();
		}
		CTextFilter::Filter(CTextFilter::DIALOG_TABLE, strMsg);
		CS_Say2Guild(strMsg.c_str());
		break;
	}
	case CCharMsg::CHANNEL_SYSTEM: {
		break;
	}
	case CCharMsg::CHANNEL_PUBLISH: {
		break;
	}
	case CCharMsg::CHANNEL_SIDE: {
		CS_Say2Camp(strMsg.c_str());
		break;
	}
	default: {
		OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_514));
		return;
	}
	}
	strCurCmd = CCharMsg::GetChannelCommand(CCharMsg::GetChannelIndex(enumChannel));
	m_edtMsg->SetCaption(strCurCmd.c_str());
}

void CCozeForm::UpdatePages() {
	bool bIndentFlag;
	CListItems* pListItems;
	while (m_cMainMsg.MoveToNextMsg()) {
		CCharMsg::sTextInfo msg = m_cMainMsg.GetMsgInfo();
		std::string strMsg = msg.strShowText;
		bIndentFlag = false;
		while (!strMsg.empty()) {
			if (bIndentFlag) {
				strMsg = "      " + strMsg;
			}
			std::string strShowText = CutFaceText(strMsg, 44); //TODO: Fix CutFaceText for UTF-8.
			auto* pItem = new CItemEx(strShowText, CCharMsg::GetChannelColor(msg.eTextChannel));
			pItem->SetAlignment(eAlignCenter);
			pItem->SetHeight(18);
			pItem->SetIsParseText(true);
			pItem->SetItemName(msg.strWho.c_str());
			if (!bIndentFlag) {

				// modify by Philip.Wu  2006-08-30  修正英文版颜色错位问题
				int nPos = (int)CCharMsg::GetChannelShowName(msg.eTextChannel).length();
				if (nPos > 0) {
					pItem->SetHasHeadText(nPos, CCharMsg::GetChannelPrefixColor(msg.eTextChannel));
				}
			}
			m_lstMainPage->NewItem()->SetBegin(pItem);
			bIndentFlag = true;
		}
	};
	pListItems = m_lstMainPage->GetItems();
	for (int i = pListItems->GetCount() - 200; i > 0; i--) {
		pListItems->Del(pListItems->GetItem(0));
	}

	static bool IsPressScroll = false;
	IsPressScroll = false;
	if (g_pGameApp->IsMouseButtonPress(0)) {
		if (m_lstMainPage->GetScroll()->IsNormal()) {
			int x = g_pGameApp->GetMouseX();
			int y = g_pGameApp->GetMouseY();
			GetRender().ScreenConvert(x, y);
			if (m_lstMainPage->GetScroll()->InRect(x, y)) {
				IsPressScroll = true;
			}
		}
	} else {
		if (m_lstMainPage->GetScroll()->IsNormal()) {
			CStep& step = m_lstMainPage->GetScroll()->GetStep();
			if (step.GetPosition() > step.GetMin() && step.GetPosition() + 5 < step.GetMax()) {
				IsPressScroll = true;
			}
		}
	}

	if (IsPressScroll)
		m_lstMainPage->Refresh();
	else
		m_lstMainPage->OnKeyDown(VK_END);

	while (m_cSystemMsg.MoveToNextMsg()) {
		CCharMsg::sTextInfo msg = m_cSystemMsg.GetMsgInfo();
		std::string strMsg = msg.strShowText;
		bIndentFlag = false;
		while (!strMsg.empty()) {
			if (bIndentFlag) {
				strMsg = "      " + strMsg;
			}
			std::string strShowText = CutFaceText(strMsg, 44);
			auto* pItem = new CItemEx(strShowText, CCharMsg::GetChannelColor(msg.eTextChannel));
			pItem->SetAlignment(eAlignCenter);
			pItem->SetHeight(18);
			pItem->SetIsParseText(true);
			pItem->SetItemName(msg.strWho.c_str());
			if (!bIndentFlag) {
				int nPos = (int)CCharMsg::GetChannelShowName(msg.eTextChannel).length();
				if (nPos > 0) {
					pItem->SetHasHeadText(nPos, CCharMsg::GetChannelPrefixColor(msg.eTextChannel));
				}
			}
			m_lstSystemPage->NewItem()->SetBegin(pItem);
			bIndentFlag = true;
		}
	};
	pListItems = m_lstSystemPage->GetItems();
	for (int i = pListItems->GetCount() - 200; i > 0; i--) {
		pListItems->Del(pListItems->GetItem(0));
	}
	m_lstSystemPage->Refresh();

	IsPressScroll = false;
	if (g_pGameApp->IsMouseButtonPress(0)) {
		if (m_lstSystemPage->GetScroll()->IsNormal()) {
			int x = g_pGameApp->GetMouseX();
			int y = g_pGameApp->GetMouseY();
			GetRender().ScreenConvert(x, y);
			if (!m_lstSystemPage->GetScroll()->InRect(x, y)) {
				IsPressScroll = true;
			}
		}
	} else {
		if (m_lstMainPage->GetScroll()->IsNormal()) {
			CStep& step = m_lstMainPage->GetScroll()->GetStep();
			if (step.GetPosition() > step.GetMin() && step.GetPosition() + 5 < step.GetMax()) {
				IsPressScroll = true;
			}
		}
	}

	if (IsPressScroll)
		m_lstSystemPage->Refresh();
	else
		m_lstSystemPage->OnKeyDown(VK_END);
}

void CCozeForm::ResetPages() {
	bool bIndentFlag;
	m_lstMainPage->GetItems()->Clear();
	if (m_cMainMsg.MoveToFirstMsg()) {
		CCharMsg::sTextInfo msg = m_cMainMsg.GetMsgInfo();
		std::string strMsg = msg.strShowText;
		bIndentFlag = false;
		while (!strMsg.empty()) {
			if (bIndentFlag) {
				strMsg = "      " + strMsg;
			}
			std::string strShowText = CutFaceText(strMsg, 44);
			auto* pItem = new CItemEx(strShowText, CCharMsg::GetChannelColor(msg.eTextChannel));
			pItem->SetAlignment(eAlignCenter);
			pItem->SetHeight(18);
			pItem->SetIsParseText(true);
			pItem->SetItemName(msg.strWho.c_str());
			if (!bIndentFlag) {
				int nPos = (int)CCharMsg::GetChannelShowName(msg.eTextChannel).length();
				if (nPos > 0) {
					pItem->SetHasHeadText(nPos, CCharMsg::GetChannelPrefixColor(msg.eTextChannel));
				}
			}
			m_lstMainPage->NewItem()->SetBegin(pItem);
			bIndentFlag = true;
		}
	}

	m_lstSystemPage->GetItems()->Clear();
	if (m_cSystemMsg.MoveToFirstMsg()) {
		CCharMsg::sTextInfo msg = m_cSystemMsg.GetMsgInfo();
		std::string strMsg = msg.strShowText;
		bIndentFlag = false;
		while (!strMsg.empty()) {
			if (bIndentFlag) {
				strMsg = "      " + strMsg;
			}
			std::string strShowText = CutFaceText(strMsg, 44);
			auto* pItem = new CItemEx(strShowText, CCharMsg::GetChannelColor(msg.eTextChannel));
			pItem->SetAlignment(eAlignCenter);
			pItem->SetHeight(18);
			pItem->SetIsParseText(true);
			pItem->SetItemName(msg.strWho.c_str());
			if (!bIndentFlag) {
				int nPos = (int)CCharMsg::GetChannelShowName(msg.eTextChannel).length();
				if (nPos > 0) {
					pItem->SetHasHeadText(nPos, CCharMsg::GetChannelPrefixColor(msg.eTextChannel));
				}
			}
			m_lstSystemPage->NewItem()->SetBegin(pItem);
			bIndentFlag = true;
		}
	}
	UpdatePages();
}

void CCozeForm::ChangePrivatePlayerName(std::string strName) {
	if (strName.empty()) {
		return;
	}

	std::string strChat = m_edtMsg->GetCaption();
	std::string strPrivateCmd = CCharMsg::GetChannelCommand(CCharMsg::GetChannelIndex(CCharMsg::CHANNEL_PRIVATE));
	int nLTrim = 0;
	if (strChat.find(CCharMsg::GetChannelCommand(CCharMsg::GetChannelIndex(CCharMsg::CHANNEL_WORLD))) == 0 ||
		strChat.find(CCharMsg::GetChannelCommand(CCharMsg::GetChannelIndex(CCharMsg::CHANNEL_TRADE))) == 0 ||
		strChat.find(CCharMsg::GetChannelCommand(CCharMsg::GetChannelIndex(CCharMsg::CHANNEL_TEAM))) == 0 ||
		strChat.find(CCharMsg::GetChannelCommand(CCharMsg::GetChannelIndex(CCharMsg::CHANNEL_GUILD))) == 0) {
		nLTrim = 1;
	} else if (strChat.find(strPrivateCmd) == 0) {
		int nPos = (int)(strChat.find(" "));
		if (nPos > 0)
			nLTrim = nPos + 1;
		else
			nLTrim = (int)strChat.length();
	}
	if (nLTrim > 0 && nLTrim <= (int)strChat.length()) {
		strChat = strChat.substr(nLTrim, strChat.length() - nLTrim);
	}
	strChat = strPrivateCmd + strName + " " + strChat;
	if ((int)strChat.length() > m_edtMsg->GetMaxNum()) {
		strChat = strPrivateCmd + strName + " ";
	}
	m_edtMsg->SetCaption(strChat.c_str());
}

void CCozeForm::EventPublishShowForm(CForm* pForm, bool& IsShow) {
	CCharacter* pCharacter = CGameScene::GetMainCha();
	if (!pCharacter)
		return;

	if (pCharacter->getGMLv() == 0) {
		IsShow = false;
	}
}

void CCozeForm::EventPublishSendMsg(CGuiData* pSender) {
	CCharacter* pCharacter = CGameScene::GetMainCha();
	if (!pCharacter)
		return;

	//如果全是空格，则不显示
	CCozeForm* pThis = CCozeForm::GetInstance();
	std::string strMsg = pThis->m_edtPublishMsg->GetCaption();
	if (strMsg.empty() || strMsg.length() == count(strMsg.begin(), strMsg.end(), ' ')) {
		return;
	}

	CS_GM1Say(strMsg.c_str());
	pThis->m_edtPublishMsg->SetCaption("");
}

//Add by sunny.sun20080804
void CCozeForm::EventPublishSendMsg1(CGuiData* pSender) {
	CCharacter* pCharacter = CGameScene::GetMainCha();
	if (!pCharacter)
		return;

	//如果全是空格，则不显示
	CCozeForm* pThis = CCozeForm::GetInstance();
	std::string strMsg = pThis->m_edtPublishMsg1->GetCaption();
	if (strMsg.empty() || strMsg.length() == count(strMsg.begin(), strMsg.end(), ' ')) {
		return;
	}

	CS_GM1Say1(strMsg.c_str());
	pThis->m_edtPublishMsg1->SetCaption("");
}
//End

bool CCozeForm::EventGlobalKeyDownHandle(int& key) {
	if (g_pGameApp->IsCtrlPress()) {
		WORD wFaceID = key - '0';
		if (wFaceID >= 0 && wFaceID <= 9) {
			CCharacter* pCharacter = CGameScene::GetMainCha();
			if (pCharacter) {
				char lpszBuf[20];
				_snprintf_s(lpszBuf, _TRUNCATE, "***%d", wFaceID);
				pCharacter->GetHeadSay()->SetFaceID(wFaceID);
				CS_Say(lpszBuf);
			}
		}
	}
	return false;
}

void CCozeForm::EventSendMsg(CGuiData* pSender) {
	CCozeForm::GetInstance()->SendMsg();
}

bool CCozeForm::EventEditMsg(CGuiData* pSender, int& key) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	if (key == VK_PRIOR || key == VK_NEXT) {
		pThis->m_lstMainPage->OnKeyDown(key);
		return true;
	} else if (key == VK_UP || key == VK_DOWN) {
		if (!pThis->m_bSendMsgCardSwitch) {
			if (pThis->m_cSendMsgCard.MoveToLastCard()) {
				pThis->m_bSendMsgCardSwitch = true;
				pThis->m_edtMsg->SetCaption(pThis->m_cSendMsgCard.GetCardInfo().c_str());
				return true;
			}
		} else {
			if (key == VK_UP) {
				if (pThis->m_cSendMsgCard.MoveToPrevCard()) {
					pThis->m_edtMsg->SetCaption(pThis->m_cSendMsgCard.GetCardInfo().c_str());
					return true;
				}
			} else {
				if (pThis->m_cSendMsgCard.MoveToNextCard()) {
					pThis->m_edtMsg->SetCaption(pThis->m_cSendMsgCard.GetCardInfo().c_str());
					return true;
				}
			}
		}
	}
	return false;
}

void CCozeForm::EventMainListKeyDown(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	if ((key & MouseClickState::RDown) != MouseClickState()) {
		CItemRow* pItemRow = pThis->m_lstMainPage->GetSelectItem();
		if (!pItemRow)
			return;
		auto* pItemEx = dynamic_cast<CItemEx*>(pItemRow->GetBegin());
		if (!pItemEx)
			return;
		std::string strName = pItemEx->GetItemName();
		CCharacter* pCharacter = CGameScene::GetMainCha();
		if (pCharacter) {
			if (strName == pCharacter->getHumanName()) {
				return;
			}
		}
		pThis->ChangePrivatePlayerName(strName);
	}
}

void CCozeForm::EventSendChannelSwitchClick(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	pThis->m_lstCallingCard->SetIsShow(false);
	pThis->m_grdFacePanel->SetIsShow(false);
	pThis->m_grdBrowPanel->SetIsShow(false);
	pThis->m_grdActionPanel->SetIsShow(false);
}

void CCozeForm::EventSendChannelChange(CGuiData* pSender) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	std::string str = pThis->m_cmbChannel->GetText();
	if (str == CCharMsg::GetChannelName(CCharMsg::CHANNEL_SIGHT)) {
		pThis->m_eCurSelChannel = CCharMsg::CHANNEL_SIGHT;
	} else if (str == CCharMsg::GetChannelName(CCharMsg::CHANNEL_TEAM)) {
		pThis->m_eCurSelChannel = CCharMsg::CHANNEL_TEAM;
	} else if (str == CCharMsg::GetChannelName(CCharMsg::CHANNEL_GUILD)) {
		pThis->m_eCurSelChannel = CCharMsg::CHANNEL_GUILD;
	} else if (str == CCharMsg::GetChannelName(CCharMsg::CHANNEL_TRADE)) {
		pThis->m_eCurSelChannel = CCharMsg::CHANNEL_TRADE;
	} else if (str == CCharMsg::GetChannelName(CCharMsg::CHANNEL_WORLD)) {
		pThis->m_eCurSelChannel = CCharMsg::CHANNEL_WORLD;
	} else if (str == CCharMsg::GetChannelName(CCharMsg::CHANNEL_SIDE)) {
		pThis->m_eCurSelChannel = CCharMsg::CHANNEL_SIDE;
	} else {
		pThis->m_eCurSelChannel = CCharMsg::CHANNEL_NONE;
	}
	pThis->m_edtMsg->SetCaption(CCharMsg::GetChannelCommand(CCharMsg::GetChannelIndex(pThis->m_eCurSelChannel)).c_str());
}

void CCozeForm::EventMainPageDragBegin(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	pThis->m_nHeightBeforeDrag = pThis->m_lstMainPage->GetHeight();
	pThis->m_nTopBeforeDrag = pThis->m_lstMainPage->GetTop();
}

void CCozeForm::EventMainPageDragging(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	int nDragTop = pThis->m_lstMainPage->GetDrag()->GetDragY();
	int nHeight = pThis->m_nHeightBeforeDrag - nDragTop;
	if (m_nMainPageMinHeight <= nHeight && nHeight <= m_nMainPageMaxHeight) {
		CList* pList = pThis->m_lstMainPage;
		pList->SetPos(pList->GetLeft(), pThis->m_nTopBeforeDrag + nDragTop);
		pList->SetSize(pList->GetWidth(), nHeight);
		pList->Init();
		pList->Refresh();
		pList->GetItems()->SetFirstShowRow(pList->GetItems()->GetCount() - pList->GetItems()->GetShowCount());
		pList->Refresh();

		pThis->m_drgMainPage->SetPos(pList->GetLeft(), pList->GetTop() - 4);
		pThis->m_drgMainPage->Refresh();

		pList = pThis->m_lstSystemPage;
		pList->SetPos(pList->GetLeft(), pThis->m_nTopBeforeDrag + nDragTop - pList->GetHeight());
		pList->Init();
		pList->Refresh();
		pList->GetItems()->SetFirstShowRow(pList->GetItems()->GetCount() - pList->GetItems()->GetShowCount());
		pList->Refresh();

		pThis->m_drgSystemPage->SetPos(pList->GetLeft(), pList->GetTop() - 4);
		pThis->m_drgSystemPage->Refresh();
	}
}

void CCozeForm::EventMainPageDragEnd(CGuiData* pSender, int x, int y, MouseClickState key) {
	EventMainPageDragging(pSender, x, y, key);
	CCozeForm* pThis = CCozeForm::GetInstance();
	CList* pList = pThis->m_lstMainPage;
	pThis->m_drgMainPage->SetPos(pList->GetLeft(), pList->GetTop() - 4);
	pThis->m_drgMainPage->Refresh();

	pList = pThis->m_lstSystemPage;
	pThis->m_drgSystemPage->SetPos(pList->GetLeft(), pList->GetTop() - 4);
	pThis->m_drgSystemPage->Refresh();
}

void CCozeForm::EventSystemPageDragBegin(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	pThis->m_nHeightBeforeDrag = pThis->m_lstSystemPage->GetHeight();
	pThis->m_nTopBeforeDrag = pThis->m_lstSystemPage->GetTop();
}

void CCozeForm::EventSystemPageDragging(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	int nDragTop = pThis->m_lstSystemPage->GetDrag()->GetDragY();
	int nHeight = pThis->m_nHeightBeforeDrag - nDragTop;
	if (m_nSystemPageMinHeight <= nHeight && nHeight <= m_nSystemPageMaxHeight) {
		CList* pList = pThis->m_lstSystemPage;
		pList->SetPos(pList->GetLeft(), pThis->m_nTopBeforeDrag + nDragTop);
		pList->SetSize(pList->GetWidth(), nHeight);
		pList->Init();
		pList->Refresh();
		pList->GetItems()->SetFirstShowRow(pList->GetItems()->GetCount() - pList->GetItems()->GetShowCount());
		pList->Refresh();

		pThis->m_drgSystemPage->SetPos(pList->GetLeft(), pList->GetTop() - 4);
		pThis->m_drgSystemPage->Refresh();
	}
}

void CCozeForm::EventSystemPageDragEnd(CGuiData* pSender, int x, int y, MouseClickState key) {
	EventSystemPageDragging(pSender, x, y, key);
	CCozeForm* pThis = CCozeForm::GetInstance();
	CList* pList = pThis->m_lstSystemPage;
	pThis->m_drgSystemPage->SetPos(pList->GetLeft(), pList->GetTop() - 4);
	pThis->m_drgSystemPage->Refresh();
}

void CCozeForm::EventChannelSwitchCheck(CGuiData* pSender) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	pThis->m_lstCallingCard->SetIsShow(false);
	pThis->m_grdFacePanel->SetIsShow(false);
	pThis->m_grdBrowPanel->SetIsShow(false);
	pThis->m_grdActionPanel->SetIsShow(false);
	pThis->m_cmbChannel->GetList()->SetIsShow(false);

	CChannelSwitchForm::GetInstance()->SwitchCheck();
}

void CCozeForm::EventCallingCardSwitchClick(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCozeForm* pThis = CCozeForm::GetInstance();

	pThis->m_lstCallingCard->GetItems()->Clear();
	if (pThis->m_cCallingCard.MoveToFirstCard()) {
		do {
			pThis->m_lstCallingCard->Add(pThis->m_cCallingCard.GetCardInfo().c_str());
		} while (pThis->m_cCallingCard.MoveToNextCard());
	}

	CItemRow* pRow = pThis->m_lstCallingCard->Add(RES_STRING(CL_LANGUAGE_MATCH_515));
	pRow->GetBegin()->SetColor(0xFF7F7F3F);
	pThis->m_lstCallingCard->SetPointer(pRow);
	pThis->m_lstCallingCard->Refresh();

	pThis->m_lstCallingCard->SetIsShow(!pThis->m_lstCallingCard->GetIsShow());
	pThis->m_grdFacePanel->SetIsShow(false);
	pThis->m_grdBrowPanel->SetIsShow(false);
	pThis->m_grdActionPanel->SetIsShow(false);
	pThis->m_cmbChannel->GetList()->SetIsShow(false);
}

//void CCozeForm::EventCallingCardLostFocus(CGuiData *pSender)
//{
//	CCozeForm *pThis=CCozeForm::GetInstance();
//	pThis->m_lstCallingCard->SetIsShow(false);
//	pThis->m_lstCallingCard->MouseRun(pThis->m_lstCallingCard->GetX()-1,pThis->m_lstCallingCard->GetY()-1,0);
//}

void CCozeForm::EventCardSelected(CGuiData* pSender) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	pThis->m_lstCallingCard->SetIsShow(false);
	CItemRow* pRow = pThis->m_lstCallingCard->GetSelectItem();
	if (!pRow) {
		return;
	}
	std::string strName = pRow->GetBegin()->GetString();
	if (strName.empty()) {
		return;
	}
	if (pThis->m_lstCallingCard->GetPointer() == pRow) {
		pThis->m_cCallingCard.ClearAll();
		return;
	}

	pThis->ChangePrivatePlayerName(strName);
}

void CCozeForm::EventFacePanelSwitchClick(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	pThis->m_grdFacePanel->SetIsShow(!pThis->m_grdFacePanel->GetIsShow());
	pThis->m_lstCallingCard->SetIsShow(false);
	pThis->m_grdBrowPanel->SetIsShow(false);
	pThis->m_grdActionPanel->SetIsShow(false);
	pThis->m_cmbChannel->GetList()->SetIsShow(false);
}

//void CCozeForm::EventFacePanelLostFocus(CGuiData *pSender)
//{
//	CCozeForm *pThis=CCozeForm::GetInstance();
//	pThis->m_grdFacePanel->SetIsShow(false);
//	pThis->m_grdFacePanel->MouseRun(pThis->m_grdFacePanel->GetX()-1, pThis->m_grdFacePanel->GetY()-1, 0);
//}

void CCozeForm::EventFaceSelected(CGuiData* pSender) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	pThis->m_grdFacePanel->SetIsShow(false);

	if ((int)strlen(pThis->m_edtMsg->GetCaption()) > pThis->m_edtMsg->GetMaxNum() - 3) {
		return;
	}

	if (pThis->m_grdFacePanel->GetSelect()) {
		pThis->m_edtMsg->SetActive(pThis->m_edtMsg);
		char lpszFace[10];
		_snprintf_s(lpszFace, _TRUNCATE, "#%02d", pThis->m_grdFacePanel->GetSelectIndex());
		pThis->m_edtMsg->ReplaceSel(lpszFace);
	}
}

void CCozeForm::EventBrowPanelSwitchClick(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	pThis->m_grdBrowPanel->SetIsShow(!pThis->m_grdBrowPanel->GetIsShow());
	pThis->m_lstCallingCard->SetIsShow(false);
	pThis->m_grdFacePanel->SetIsShow(false);
	pThis->m_grdActionPanel->SetIsShow(false);
	pThis->m_cmbChannel->GetList()->SetIsShow(false);
}

void CCozeForm::EventBrowSelected(CGuiData* pSender) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	CCharacter* pCharacter = CGameScene::GetMainCha();
	if (!pCharacter)
		return;
	pThis->m_grdBrowPanel->SetIsShow(false);

	if (pThis->m_grdBrowPanel->GetSelect()) {
		int nIndex = pThis->m_grdBrowPanel->GetSelectIndex();
		pCharacter->GetHeadSay()->SetFaceID(nIndex);
		char lpszFaceID[10] = {0};
		_snprintf_s(lpszFaceID, _TRUNCATE, "***%d", nIndex);
		CS_Say(lpszFaceID);
	}
}

void CCozeForm::EventActionPanelSwitchClick(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	pThis->m_grdActionPanel->SetIsShow(!pThis->m_grdActionPanel->GetIsShow());
	pThis->m_lstCallingCard->SetIsShow(false);
	pThis->m_grdFacePanel->SetIsShow(false);
	pThis->m_grdBrowPanel->SetIsShow(false);
	pThis->m_cmbChannel->GetList()->SetIsShow(false);
}

void CCozeForm::EventActionSelected(CGuiData* pSender) {
	CCozeForm* pThis = CCozeForm::GetInstance();
	CCharacter* pCharacter = CGameScene::GetMainCha();
	if (!pCharacter)
		return;
	pThis->m_grdActionPanel->SetIsShow(false);

	CGraph* graph = pThis->m_grdActionPanel->GetSelect();
	//int nIndex = pThis->m_grdActionPanel->GetSelectIndex();
	if (graph) {
		//pCharacter->GetActor()->PlayPose(graph->nTag, true, true);
		pCharacter->GetActor()->PlayPose(graph->nTag, true, true);
	}
}

#if 0

#define CARD_LIST_MAX 15


//static std::string  strPersonObject;     // 私聊的对象
CList* CCozeMgr::lstOnSay=NULL;
CList* CCozeMgr::lstOnSystemSay=NULL;
CCombo*	CCozeMgr::m_cmbChannel=NULL;
CGrid*	CCozeMgr::grdFace=NULL;
CEdit*	CCozeMgr::edtSay=NULL;
CEdit*	CCozeMgr::edtGmSay=NULL;
CCheckBox* CCozeMgr::m_pForbidCheck=NULL;
CTextButton* CCozeMgr::m_pbtnCard=NULL;
CList* CCozeMgr::m_pCardList=NULL;
vector<std::string> CCozeMgr::m_strCardListData;

CCheckBox* CCozeMgr::m_pPrivateCheck=NULL;
CCheckBox* CCozeMgr::m_pSightCheck=NULL;
CCheckBox* CCozeMgr::m_pSystemCheck=NULL;
CCheckBox* CCozeMgr::m_pTeamCheck=NULL;
CCheckBox* CCozeMgr::m_pGuildCheck=NULL;
CCheckBox* CCozeMgr::m_pWorldCheck=NULL;
CCheckBox* CCozeMgr::m_pTradeCheck=NULL;


CCozeMgr::eChannel CCozeMgr::m_CurSelectChannel=CCozeMgr::CHANNEL_ROAD;
std::string CCozeMgr::m_CurCommand="";
int CCozeMgr::m_TextPoint[CHANNELS_COUNT][2]={0};
CCozeMgr::sTextInfo CCozeMgr::m_TextRecords[CHANNELS_COUNT][DATA_RECORDS_SIZE]={CHANNEL_ROAD,"","",0};

//---------------------------------------------------------------------------
// class CCozeMgr
//---------------------------------------------------------------------------
bool CCozeMgr::Init()
{
	CCozeForm::GetInstance();
	frmMainChat = _FindForm("frmMain800");
	grdFace = dynamic_cast<CGrid*>(frmMainChat->Find("grdFace"));
	if( !grdFace ) return Error( RES_STRING(CL_LANGUAGE_MATCH_45), frmMainChat->GetName(), "grdFace" );
	return true;					

	//Arcol:Test-

	_nSelectSayRow = 0;
	_nOldSayRow = 0;
	for (int i=0;i<MAX_PAGE_SAY;i++)
	{
		g_stUICoze.oldSay[i]="";
	}

	//
	CForm* frmMain800 = _FindForm("frmMain800");

	CFormMgr &mgr = CFormMgr::s_Mgr;
	frmMainChat = frmMain800;//_FindForm("frmMainChat");
	if( !frmMainChat ) return false;
	
	frmMainChat->SetIsShow(true);
	//frmMainChat->SetPos( frmMainChat->GetLeft(), frmMain800->GetTop() );
	//frmMainChat->SetStyle(CForm::fsBottom);

	edtSay = dynamic_cast<CEdit*>(frmMainChat->Find("edtSay"));
	if( !edtSay ) return Error(RES_STRING(CL_LANGUAGE_MATCH_516), frmMainChat->GetName(), "edtSay");
	edtSay->SetIsWrap(true);
	edtSay->evtEnter = _evtSay;
	edtSay->evtKeyDown = _evtSayKeyDown;

	lstOnSystemSay = dynamic_cast<CList*>(frmMainChat->Find("lstOnSaySystem"));
	if( !lstOnSystemSay )	return Error( RES_STRING(CL_LANGUAGE_MATCH_516), frmMainChat->GetName(), "lstOnSaySystem" );
	lstOnSystemSay->evtListMouseDB = _MainOnLstKeyDownEvent;
	lstOnSystemSay->SetIsChangeColor(false);
	lstOnSystemSay->SetMouseAction( enumMA_Drill );

	lstOnSay = dynamic_cast<CList*>(frmMainChat->Find("lstOnSay"));
	if( !lstOnSay )	return Error( RES_STRING(CL_LANGUAGE_MATCH_516), frmMainChat->GetName(), "lstOnSay" );
	lstOnSay->evtListMouseDB = _MainOnLstKeyDownEvent;
	lstOnSay->SetIsChangeColor(false);
	lstOnSay->SetMouseAction( enumMA_Drill );
	
	lstChannel = dynamic_cast<CList*>(frmMainChat->Find("lstChannel"));
	if( !lstChannel ) return Error( RES_STRING(CL_LANGUAGE_MATCH_516), frmMainChat->GetName(), "lstChannel" );
	lstChannel->evtSelectChange = _evtChannelSelectChange;

	mgr.AddKeyDownEvent( _MainOnSayKeyDownEvent );

	drpTitleSystem = dynamic_cast<CDragTitle*>(frmMainChat->Find("drpTitleSystem"));
	if( !drpTitleSystem ) return Error( RES_STRING(CL_LANGUAGE_MATCH_516), frmMainChat->GetName(), "drpTitleSystem" );
	drpTitleSystem->SetIsShowDrag(false);
	
	drpTitle = dynamic_cast<CDragTitle*>(frmMainChat->Find("drpTitle"));
	if( !drpTitle ) return Error( RES_STRING(CL_LANGUAGE_MATCH_516), frmMainChat->GetName(), "drpTitle" );
	drpTitle->SetIsShowDrag(false);

	//CTextButton *btnChat = dynamic_cast<CTextButton*>(frmMainChat->Find("btnChat"));
	//if( !btnChat ) return Error( "coze.clu界面<%s>上找不到控件<%s>", frmMainChat->GetName(), "btnChat" );
	//btnChat->evtMouseClick=_evtShowChannelList;

	//CTextButton *btnChat = dynamic_cast<CTextButton*>(frmMainChat->Find("btnChat"));
	//if( !btnChat ) return Error( "coze.clu界面<%s>上找不到控件<%s>", frmMainChat->GetName(), "btnChat" );
	//btnChat->evtMouseClick=_evtShowChannelList;

	m_cmbChannel=dynamic_cast<CCombo*>(frmMainChat->Find("cboChannel"));
	if( !m_cmbChannel ) return Error( RES_STRING(CL_LANGUAGE_MATCH_516), frmMainChat->GetName(), "cboChannel" );
	m_cmbChannel->evtSelectChange=_evtChannelSelectChange;
	m_cmbChannel->GetList()->GetItems()->Select(0);


	m_pForbidCheck=dynamic_cast<CCheckBox*>(frmMainChat->Find("chkChatOn"));
	if( !m_pForbidCheck ) return Error( RES_STRING(CL_LANGUAGE_MATCH_516), frmMainChat->GetName(), "chkChatOn" );
	m_pForbidCheck->SetIsChecked(false);
	m_pForbidCheck->evtCheckChange=_evtForbidCheck;

	FORM_CONTROL_LOADING_CHECK(m_pbtnCard,frmMainChat,CTextButton,"coze.clu","btnCard");
	m_pbtnCard->evtMouseClick=_evtCardClick;

	FORM_CONTROL_LOADING_CHECK(m_pCardList,frmMainChat,CList,"coze.clu","lstCard");
	m_pCardList->evtSelectChange=_evtCardSelectChange;
	m_pCardList->SetIsChangeColor(false);


	FORM_LOADING_CHECK(frmGM,"coze.clu","frmGM");
	frmGM->evtBeforeShow=_evtGmForm;

	//Add by sunny.sun20080804
	FORM_LOADING_CHECK(Arena,"coze.clu","Arena");
	Arena->evtBeforeShow=_evtGmForm;
	//End

	FORM_LOADING_CHECK(frmForbid,"main.clu","frmForbid");
	frmForbid->evtLost=_evtForbidFormLost;
	FORM_CONTROL_LOADING_CHECK(m_pPrivateCheck,frmForbid,CCheckBox,"main.clu","chkPersonal");
	FORM_CONTROL_LOADING_CHECK(m_pSightCheck,frmForbid,CCheckBox,"main.clu","chkNear");
	FORM_CONTROL_LOADING_CHECK(m_pSystemCheck,frmForbid,CCheckBox,"main.clu","chkSystem");
	FORM_CONTROL_LOADING_CHECK(m_pTeamCheck,frmForbid,CCheckBox,"main.clu","chkTeam");
	FORM_CONTROL_LOADING_CHECK(m_pGuildCheck,frmForbid,CCheckBox,"main.clu","chkGuild");
	FORM_CONTROL_LOADING_CHECK(m_pWorldCheck,frmForbid,CCheckBox,"main.clu","chkWorld");
	FORM_CONTROL_LOADING_CHECK(m_pTradeCheck,frmForbid,CCheckBox,"main.clu","chkTrade");
	m_pSystemCheck->evtCheckChange=_evtForbidSystemChange;


	FORM_CONTROL_LOADING_CHECK(edtGmSay,frmGM,CEdit,"coze.clu","edtGMSay");
	edtGmSay->SetIsWrap(true);
	edtGmSay->evtEnter = _evtGmSay;
	//Add by sunny.sun20080804
	FORM_CONTROL_LOADING_CHECK(edtArenaSay,Arena,CEdit,"coze.clu","edtArenaSay");
	edtArenaSay->SetIsWrap(true);
	edtArenaSay->evtEnter = _evtGmSay;	
	//End
	CDrag *pDrag = drpTitleSystem->GetDrag();
	pDrag->evtMouseDragBegin = _DragBeginOnSystemTitle;
	pDrag->evtMouseDragMove = _DragMoveOnSystemTitle;
	pDrag->evtMouseDragEnd = _DragEndOnSystemTitle;
	pDrag->SetYare(0);
	pDrag->SetDragInCursor( CCursor::stVertical );
	//_nDragMinY = 70;
	//_nDragMaxY = 280;

	pDrag = drpTitle->GetDrag();
	pDrag->evtMouseDragBegin = _DragBeginOnSay;
	pDrag->evtMouseDragMove = _DragMoveOnSay;
	pDrag->evtMouseDragEnd = _DragEndOnSay;
	pDrag->SetYare(0);
	pDrag->SetDragInCursor( CCursor::stVertical );
	_nDragSystemMinY=55;
	_nDragSystemMaxY=280;
	_nDragMinY = 70;
	_nDragMaxY = 280;
	
	grdFace = dynamic_cast<CGrid*>(frmMainChat->Find("grdFace"));
	if( !grdFace ) return Error( RES_STRING(CL_LANGUAGE_MATCH_45), frmMainChat->GetName(), "grdFace" );
	grdFace->evtSelectChange =_evtSelectBudChange;   
	grdFace->evtLost = _evtHideSelf;
	
	g_stUICoze.lstOnSay->SetRowHeight(16);
	g_stUICoze.lstOnSystemSay->SetRowHeight(16);


	return true;
}

void CCozeMgr::End()
{
}

bool CCozeMgr::IsMouseOnSayList( int x, int y )
{
	if( CCompent::GetLastMouseCompent()==lstOnSay )
	{		
		if( lstOnSay->GetScroll()->IsNormal() )
		{
			GetRender().ScreenConvert( x, y );
			if( lstOnSay->GetScroll()->InRect( x, y ) )
				return false;
		}

		return true;
	}
	return false;
}

void CCozeMgr::_evtSay(CGuiData *pSender)
{
	CCharacter* pCha = CGameScene::GetMainCha();
	if( !pCha ) return;

	//如果全是空格，则不显示	
	const char* s = g_stUICoze.edtSay->GetCaption() ;
	int len =(int) strlen( s );
	int i = 0;
	for ( ; i <len  ;i++)
	{
		if 	(s[i]!= 32 )  //32是空格的ascii码
			break;
	}	
	if (i == len )
		return ;

	std::string strChat =  g_stUICoze.edtSay->GetCaption();
	g_stUICoze.oldSay[g_stUICoze._nOldSayRow++] = g_stUICoze.edtSay->GetCaption();
	g_stUICoze._nOldSayRow%=MAX_PAGE_SAY;
	g_stUICoze._nSelectSayRow = g_stUICoze._nOldSayRow;
	int p = (int)(strChat.find(" ") ) ;
	if ( strChat.find ("@") ==0 && p != -1)   //私聊
	{	
		std::string strPersonName=strChat.substr(1, p-1) ;//私聊对象
		std::string strPersonContent	 = strChat.substr( p+1, strChat.length() - p -1 );
		if (strPersonContent.length()>0)
		{
			//FilterText(strPersonContent,&g_stUIChat._strFilterTxt);
			CTextFilter::Filter(CTextFilter::DIALOG_TABLE,strPersonContent);
			if (!strPersonName.empty() && strPersonName.length()<=16)
			{
				m_CurCommand="@"+strPersonName+" ";
				CS_Say2You( strPersonName.c_str() ,strPersonContent.c_str() );
			}
		}
		///////////处理lstOnSay的显示
		//std::string str = pCha->getName();
		//str += ":";
		//str += strPersonContent ;
		//ShowLstOnSay( str, g_stUICoze.chatColor.person );
		//g_stUICoze.lstOnSay->OnKeyDown(VK_END);

		//g_stUICoze.oldSay[g_stUICoze._nOldSayRow] = g_stUICoze.edtSay->GetCaption();
		//++g_stUICoze._nOldSayRow;
		//if( g_stUICoze._nOldSayRow >= MAX_PAGE_SAY )
		//	g_stUICoze._nOldSayRow=0;

		//g_stUICoze._nSelectSayRow = g_stUICoze._nOldSayRow;
		//g_stUICoze.edtSay->SetCaption("");
		//strPersonObject = "";

		///////////处理lstOnSay的显示
		//str = pCha->getName();
		//str += ":";
		//str += g_stUICoze.edtSay->GetCaption() ;
		//ShowLstOnSay( str, g_stUICoze.chatColor.road );
		//g_stUICoze.lstOnSay->OnKeyDown(VK_END);

		////////////////////////////////////////////////////

		//g_stUICoze.oldSay[g_stUICoze._nOldSayRow] = g_stUICoze.edtSay->GetCaption();
		//++g_stUICoze._nOldSayRow;
		//if( g_stUICoze._nOldSayRow >= MAX_PAGE_SAY )
		//	g_stUICoze._nOldSayRow=0;

		//g_stUICoze._nSelectSayRow = g_stUICoze._nOldSayRow;


		//std::string  strheadSay;
		//std::string  strName ;
		//strName = pCha->getName() ;   //游戏的主角
		//strName = strName + ":" ;
		//strheadSay =strName  +  g_stUICoze.edtSay->GetCaption();
		//CItemEx * item_headSay = new CItemEx(strheadSay, g_stUICoze.chatColor.road );

		//if ( strheadSay.size() > 32 )  //如果多于16个字符，则分行显示
		//{
		//	item_headSay->SetIsMultiLine(true );
		//	item_headSay->ProcessString( (int) strName.size());
		//}
		//if (  strheadSay.find ("#")!= -1 )
		//	item_headSay->SetIsParseText(true);

		//item_headSay->SetColor( g_stUICoze.chatColor.road );
		//pCha->GetHeadSay()->AddItem( item_headSay );	
		//g_stUICoze.edtSay->SetCaption("");

		//return;
	}	
	else
	{
		//FilterText(strChat,&g_stUIChat._strFilterTxt);
		if (strChat.find("*")==0)
		{
			if (GetTickCount()-g_pGameApp->GetLoginTime()>60000)
			{
				CTextFilter::Filter(CTextFilter::DIALOG_TABLE,strChat);
				if (strChat.length()>1) CS_Say2All( strChat.substr(1).c_str() );
			}
			else
			{
				OnSystemSay(RES_STRING(CL_LANGUAGE_MATCH_510));
			}
		}
		else if (strChat.find("^")==0)
		{
			if (GetTickCount()-g_pGameApp->GetLoginTime()>60000)
			{
				CTextFilter::Filter(CTextFilter::DIALOG_TABLE,strChat);
				if (strChat.length()>1) CS_Say2Trade( strChat.substr(1).c_str() );
			}
			else
			{
				OnSystemSay(RES_STRING(CL_LANGUAGE_MATCH_512));
			}
		}
		else if (strChat.find("%")==0)
		{
			CTextFilter::Filter(CTextFilter::DIALOG_TABLE,strChat);
			if (strChat.length()>1) CS_Say2Guild( strChat.substr(1).c_str() );
			//g_pGameApp->SysInfo("此频道尚未开放,您可以使用聊天系统中的公会栏发送消息");
		}
		else if (strChat.find("!")==0)
		{
			CTextFilter::Filter(CTextFilter::DIALOG_TABLE,strChat);
			if (strChat.length()>1)
			{
				CTeam *pTeam=g_stUIChat.GetTeamMgr()->Find(enumTeamGroup);
				if (!pTeam || pTeam->GetCount()==0)
				{
					PrintToList(CHANNEL_TEAM,"",RES_STRING(CL_LANGUAGE_MATCH_517),CCharMsg::GetChannelColor(CCharMsg::CHANNEL_TEAM));
				}
				else
					CS_Say2Team( strChat.substr(1).c_str() );
			}
		}
		else
		{
			// 2005.5.27 增加需求：在所有频道（即路人频道）屏蔽连续重复的发言，限制为8秒
			// 2005.6.2 修改需求：在所有频道（即路人频道）屏蔽连续重复的发言，限制为1秒
			static std::string preStr="";
			static DWORD preTime=0;
			if (preStr==strChat && GetTickCount()-preTime<1000)
			{
				OnSystemSay(RES_STRING(CL_LANGUAGE_MATCH_518));
			}
			else
			{
				CTextFilter::Filter(CTextFilter::DIALOG_TABLE,strChat);
				CS_Say( strChat.c_str() );
				preStr=strChat;
				preTime=GetTickCount();
			}
		}
		std::string str=m_cmbChannel->GetText();
		if (str==RES_STRING(CL_LANGUAGE_MATCH_496))
		{
			m_CurCommand="*";
		}
		else if (str==RES_STRING(CL_LANGUAGE_MATCH_497))
		{
			m_CurCommand="^";
		}
		else if (str==RES_STRING(CL_LANGUAGE_MATCH_468))
		{
			m_CurCommand="%";
		}
		else if (str==RES_STRING(CL_LANGUAGE_MATCH_519))
		{
			m_CurCommand="!";
		}
		else if (str==RES_STRING(CL_LANGUAGE_MATCH_493))
		{
			m_CurCommand="";
		}

		//std::string str=m_cmbChannel->GetText();
		//if (str=="世界")
		//{
		//	CS_Say2All( g_stUICoze.edtSay->GetCaption() );
		//}
		//else if (str=="交易")
		//{
		//	CS_Say2All( g_stUICoze.edtSay->GetCaption() );
		//}
		//else if (str=="所有")
		//{
		//	CS_Say( g_stUICoze.edtSay->GetCaption() );
		//}
		//else if (str=="公会")
		//{
		//	//CS_Say( g_stUICoze.edtSay->GetCaption() );
		//}
		//else if (str=="队友")
		//{
		//	CTeam *pTeam=g_stUIChat.GetTeamMgr()->Find(enumTeamGroup);
		//	if (!pTeam || pTeam->GetCount()==0)
		//	{
		//		PrintToList(CHANNEL_TEAM,"","发送失败:您目前还没有组队",g_stUICoze.chatColor.team);
		//		return;
		//	}
		//	CS_Say2Team( g_stUICoze.edtSay->GetCaption() );
		//}
		//else
		//{
		//	CS_Say( g_stUICoze.edtSay->GetCaption() );
		//}
	}
	g_stUICoze.edtSay->SetCaption(m_CurCommand.c_str());
}

bool CCozeMgr::_evtSayKeyDown(CGuiData *pSender, int& key)
{
	m_pCardList->SetIsShow( false );
	grdFace->SetIsShow( false );
	switch( key )
	{
	case VK_DOWN:
		{
			++g_stUICoze._nSelectSayRow;
			if( g_stUICoze._nSelectSayRow>=MAX_PAGE_SAY )
				g_stUICoze._nSelectSayRow = 0;
		}
		break;
	case VK_UP:
		{
			--g_stUICoze._nSelectSayRow;
			if( g_stUICoze._nSelectSayRow < 0 )
				g_stUICoze._nSelectSayRow = MAX_PAGE_SAY - 1;
		}
		break;
	default: return false;
	}
	g_stUICoze.edtSay->SetCaption( g_stUICoze.oldSay[g_stUICoze._nSelectSayRow].c_str() );
	return true;
}

void CCozeMgr::_evtChannelSelectChange(CGuiData *pSender)
{
	std::string str=m_cmbChannel->GetText();
	if (str==RES_STRING(CL_LANGUAGE_MATCH_493))
	{
		m_CurSelectChannel=CHANNEL_ROAD;
		m_CurCommand="";
	}
	else if (str==RES_STRING(CL_LANGUAGE_MATCH_519))
	{
		m_CurSelectChannel=CHANNEL_TEAM;
		m_CurCommand="!";
	}
	else if (str==RES_STRING(CL_LANGUAGE_MATCH_468))
	{
		m_CurSelectChannel=CHANNEL_GUILD;
		m_CurCommand="%";
	}
	else if (str==RES_STRING(CL_LANGUAGE_MATCH_497))
	{
		m_CurSelectChannel=CHANNEL_TRADE;
		m_CurCommand="^";
	}
	else if (str==RES_STRING(CL_LANGUAGE_MATCH_496))
	{
		m_CurSelectChannel=CHANNEL_WORLD;
		m_CurCommand="*";
	}
	else if (str == RES_STRING(CL_LANGUAGE_MATCH_932))
	{
		m_CurSelectChannel=CHANNEL_SIDE;
		m_CurCommand="|";
	}

	g_stUICoze.edtSay->SetCaption(m_CurCommand.c_str());
	g_stUICoze.lstOnSystemSay->GetItems()->Clear();
	g_stUICoze.lstOnSay->GetItems()->Clear();
	for (int i=m_TextPoint[m_CurSelectChannel][0];i!=m_TextPoint[m_CurSelectChannel][1];i=(i+1)%DATA_RECORDS_SIZE)
	{
		if ((m_CurSelectChannel==CHANNEL_ROAD && IsForbidChannel(m_TextRecords[m_CurSelectChannel][i].eTextChannel)) ||
			(m_TextRecords[m_CurSelectChannel][i].eTextChannel==CHANNEL_PRIVATE && IsForbidChannel(CHANNEL_PRIVATE)))
		{
			continue;
		}
		std::string text=m_TextRecords[m_CurSelectChannel][i].stText;
		bool indentFlag=false;
		while (!text.empty())
		{
			if (indentFlag)
			{
				text="      "+text;
			}
			std::string str=CutFaceText(text,44);
			CItemRow* pItemRow;
			if (m_TextRecords[m_CurSelectChannel][i].eTextChannel==CHANNEL_SYSTEM)
			{
				pItemRow=g_stUICoze.lstOnSystemSay->NewItem();
			}
			else
			{
				pItemRow=g_stUICoze.lstOnSay->NewItem();
			}
			CItemEx* pItem=new CItemEx(str,m_TextRecords[m_CurSelectChannel][i].dwColor);
			pItem->SetAlignment(eAlignCenter);
			pItem->SetHeight(18);
			pItem->SetItemName(m_TextRecords[m_CurSelectChannel][i].stName.c_str());
			pItem->SetIsParseText(true);
			if (!indentFlag)
			{
				pItem->SetHasHeadText(6,0xFFFFFF00);
			}
			pItemRow->SetBegin(pItem);
			indentFlag=true;
		}
	}
	if( CCompent::GetActive()!=lstOnSay->GetScroll() ) lstOnSay->OnKeyDown(VK_END);
	if( CCompent::GetActive()!=lstOnSystemSay->GetScroll() ) lstOnSystemSay->OnKeyDown(VK_END);
	g_stUICoze.lstOnSystemSay->Refresh();
	g_stUICoze.lstOnSay->Refresh();

	//CList* p = dynamic_cast<CList*>(pSender);
	//if( !p ) return;

	//CItemRow* selectItem = p->GetSelectItem();

	//int nSelect = p->GetItems()->GetIndex(selectItem);

	//switch(nSelect)
	//{
	//case 0:
	//	g_stUICoze.edtSay->SetCaption("");
	//	break;
	//case 1:
	//	g_stUICoze.edtSay->SetCaption("@");
	//	break;
	//case 2:
	//	g_stUICoze.edtSay->SetCaption("!");
	//	break;
	//case 3:
	//	g_stUICoze.edtSay->SetCaption("^");
	//	break;
	//case 4:
	//	g_stUICoze.edtSay->SetCaption("$");
	//	break;
	//}
	//p->SetIsShow(false);
}

bool CCozeMgr::_MainOnSayKeyDownEvent( int& key )
{
	if( g_pGameApp->IsCtrlPress() )
	{
		CCharacter* pCha = CGameScene::GetMainCha();
		if( !pCha ) return false;

		pCha->GetHeadSay()->SetFaceID( key-'0');

		unsigned int id = key-'0' ;	

		if ( id<0 || id > 9)
			return false;
		
		char szFaceID[10] = {0} ;
		_snprintf_s( szFaceID , _TRUNCATE, "***%d" ,id);	
		CS_Say( szFaceID);
		return false;
	}

	if( key==VK_UP || key==VK_DOWN ) return false;
	if( key==VK_LEFT || key==VK_RIGHT ) return false;

	g_stUICoze.lstOnSay->OnKeyDown( key );
	return false;
}

void CCozeMgr::_DragBeginOnSystemTitle(CGuiData *pSender, int x, int y, DWORD key)
{
	g_stUICoze._nBeginOnSayHeight = g_stUICoze.lstOnSystemSay->GetHeight();
	g_stUICoze._nBeginOnSayY = g_stUICoze.lstOnSystemSay->GetTop();
}

void CCozeMgr::_DragMoveOnSystemTitle(CGuiData *pSender, int x, int y, DWORD key)
{
	int height = g_stUICoze._nBeginOnSayHeight - g_stUICoze.lstOnSystemSay->GetDrag()->GetDragY();
	if( height >= g_stUICoze._nDragSystemMinY && height <= g_stUICoze._nDragSystemMaxY )
	{
		CList *lst = g_stUICoze.lstOnSystemSay;
		lst->SetPos( lst->GetLeft(), g_stUICoze._nBeginOnSayY + g_stUICoze.lstOnSystemSay->GetDrag()->GetDragY() );		
		lst->SetSize( lst->GetWidth(), height );
		lst->Refresh();

		CDragTitle* title = g_stUICoze.drpTitleSystem ;
		title->SetPos( lst->GetLeft(), lst->GetTop() -4 );
		title->Refresh() ;

		CListItems *items = g_stUICoze.lstOnSystemSay->GetItems();
		items->SetFirstShowRow( items->GetCount() - items->GetShowCount() );
	}

	g_stUICoze.lstOnSystemSay->Init();
	g_stUICoze.lstOnSystemSay->Refresh();
}

void CCozeMgr::_DragEndOnSystemTitle(CGuiData *pSender, int x, int y, DWORD key)
{
	_DragMoveOnSystemTitle( pSender,  g_stUICoze.drpTitleSystem->GetLeft(), y, key );
	g_stUICoze.drpTitleSystem->SetPos( g_stUICoze.lstOnSystemSay->GetLeft(), g_stUICoze.lstOnSystemSay->GetTop() -4 );
	g_stUICoze.drpTitleSystem->Refresh();
}

void CCozeMgr::_DragBeginOnSay(CGuiData *pSender, int x, int y, DWORD key)
{
	g_stUICoze._nBeginOnSayHeight = g_stUICoze.lstOnSay->GetHeight();
	g_stUICoze._nBeginOnSayY = g_stUICoze.lstOnSay->GetTop();
//	g_stUICoze._DragMoveOnSay( pSender, x, y, key );
}

void CCozeMgr::_DragEndOnSay(CGuiData *pSender, int x, int y, DWORD key)
{
	_DragMoveOnSay( pSender,  g_stUICoze.drpTitle->GetLeft(), y, key );
	g_stUICoze.drpTitle->SetPos( g_stUICoze.lstOnSay->GetLeft(), g_stUICoze.lstOnSay->GetTop() -4 );
	g_stUICoze.drpTitle->Refresh();
	//g_stUICoze.lstOnSay->Init();
	//g_stUICoze.lstOnSay->Refresh();
}

void CCozeMgr::_DragMoveOnSay(CGuiData *pSender, int x, int y, DWORD key)
{
	int height = g_stUICoze._nBeginOnSayHeight - g_stUICoze.drpTitle->GetDrag()->GetDragY();
	if( height >= g_stUICoze._nDragMinY && height <= g_stUICoze._nDragMaxY )
	{
		CList *lst = g_stUICoze.lstOnSystemSay;
		lst->SetPos( lst->GetLeft(), g_stUICoze._nBeginOnSayY + g_stUICoze.drpTitle->GetDrag()->GetDragY() - lst->GetHeight() );		
		lst->Refresh();

		CDragTitle* title = g_stUICoze.drpTitleSystem ;
		title->SetPos( lst->GetLeft(), lst->GetTop() -4 );
		title->Refresh() ;

		lst = g_stUICoze.lstOnSay;
		lst->SetPos( lst->GetLeft(), g_stUICoze._nBeginOnSayY + g_stUICoze.drpTitle->GetDrag()->GetDragY() );		
		lst->SetSize( lst->GetWidth(), height );
		lst->Refresh();

		title = g_stUICoze.drpTitle ;
		title->SetPos( lst->GetLeft(), lst->GetTop() -4 );
		title->Refresh() ;

		CListItems *items = g_stUICoze.lstOnSay->GetItems();
		items->SetFirstShowRow( items->GetCount() - items->GetShowCount() );
	}
	g_stUICoze.lstOnSystemSay->Init();
	g_stUICoze.lstOnSystemSay->Refresh();

	g_stUICoze.lstOnSay->Init();
	g_stUICoze.lstOnSay->Refresh();
}

void CCozeMgr::_evtSelectBudChange(CGuiData *pSender)
{
	pSender->SetIsShow( false );

	CGrid *p = dynamic_cast<CGrid*>(pSender);
	if( !p ) return;

	if ((int)(strlen( g_stUICoze.edtSay->GetCaption()))>g_stUICoze.edtSay->GetMaxNum()-3 )
		return;

	CGraph * r = p->GetSelect();
	int nIndex = p->GetSelectIndex();
    if( r )
	{
		g_stUICoze.edtSay->SetIsParseText(true);
		g_stUICoze.edtSay->SetActive( g_stUICoze.edtSay);
		//std::string sSource = g_stUICoze.edtSay->GetCaption();
		char pszChar[10] = {0};
		if ( nIndex <10)
			_snprintf_s( pszChar , _TRUNCATE,"#0%d",nIndex);
		else
			_snprintf_s( pszChar , _TRUNCATE,"#%d",nIndex);
		std::string sAdd   = pszChar;   //需要根据图元进行匹配
		//std::string sDes = sSource +sAdd;
		g_stUICoze.edtSay->ReplaceSel(sAdd.c_str());
		//g_stUICoze.edtSay->SetCaption(sDes.c_str());	
	}		
}

void CCozeMgr::_evtForbidFormLost(CGuiData *pSender)
{
	//if (m_pForbidCheck->GetActive()!=m_pForbidCheck)
	//{
	//	g_stUICoze.frmForbid->SetIsShow(false);
	//	g_stUICoze.m_pForbidCheck->SetIsChecked(false);
	//}
	g_stUICoze.m_pForbidCheck->SetIsChecked(false);
	g_stUICoze._evtForbidCheck( g_stUICoze.m_pForbidCheck );
	g_stUICoze.m_pForbidCheck->MouseRun( g_stUICoze.m_pForbidCheck->GetX() - 1, g_stUICoze.m_pForbidCheck->GetY() - 1, 0 );
}

void CCozeMgr::_evtForbidSystemChange(CGuiData *pSender)
{
	if (g_stUICoze.m_pSystemCheck->GetIsChecked())
	{
		lstOnSystemSay->SetIsShow(false);
		g_stUICoze.drpTitleSystem->SetIsShow(false);
	}
	else
	{
		lstOnSystemSay->SetIsShow(true);
		g_stUICoze.drpTitleSystem->SetIsShow(true);
	}
}

void CCozeMgr::_evtHideSelf(CGuiData *pSender)
{
	pSender->SetIsShow( false );	
}

void CCozeMgr::_evtForbidCheck(CGuiData *pSender)
{
	g_stUICoze.frmForbid->SetIsShow( g_stUICoze.m_pForbidCheck->GetIsChecked() );

	//CCheckBox *pChecker=dynamic_cast<CCheckBox*>(pSender);
	//if (!pChecker) return;
	//bool bChecked=pChecker->GetIsChecked();
	//g_stUICoze.frmForbid->SetIsShow(bChecked);

	//CP_RefuseToMe(bChecked);
	//if( bChecked )
	//	g_pGameApp->ShowMidText( "您已经关掉了密语功能，别人将不能在聊天框里跟你私聊，再次点击后打开" );
	//else
	//	g_pGameApp->ShowMidText( "您已经打开了密语功能，再次点击后关闭" );
}

//void CCozeMgr::ShowLstOnSay(std::string str , DWORD color) 
//{			
//	//CItemRow* p1 ;
//	//CItemRow* p2 ;
//	//CItemEx *item_lstOnSay1;
//	//CItemEx *item_lstOnSay2;
//
//	while (!str.empty())
//	{
//		std::string text;
//		text=CutFaceText(str,46);
//		CItemRow* pItemRow=g_stUICoze.lstOnSay->NewItem();
//		CItemEx* pItem=new CItemEx(text,color);
//		pItem->SetAlignment(eAlignCenter);
//		pItem->SetHeight(18);
//		if (text.find ("#") !=-1)	
//		{
//			pItem->SetIsParseText(true);
//		}
//		pItemRow->SetBegin(pItem);
//	}
//	if( CCompent::GetActive()!=lstOnSay->GetScroll() ) lstOnSay->OnKeyDown(VK_END);
//	g_stUICoze.lstOnSay->Refresh();
//
//
//
//	//if ( str.length() > 40 )
//	//{
//	//	const char * s = str.c_str() ;
//	//	int len = (int )  str.length() ;
//	//	
//	//	std::string str1 = GetSelfString( str , 40 ,true) ;
//	//	std::string str2 = GetSelfString( str, len - 40 , false );
//
//	//	p1 = g_stUICoze.lstOnSay->NewItem();
//	//	item_lstOnSay1 = new CItemEx(  str1, color );
//	//	item_lstOnSay1->SetAlignment(eAlignCenter);
//	//	item_lstOnSay1->SetHeight( 18);
//	//	if (  str.find ("#") !=-1)	
//	//	{
//	//		item_lstOnSay1->SetIsParseText(true);
//	//	}
//
//	//	p1->SetBegin(item_lstOnSay1);		
//	//			
//	//	p2 = g_stUICoze.lstOnSay->NewItem();
//	//	item_lstOnSay2 = new CItemEx( str2, color);
//	//	item_lstOnSay2->SetAlignment(eAlignCenter);
//	//	item_lstOnSay2->SetHeight( 18);
//	//
//	//	if (  str.find ("#") !=-1)
//	//	{
//	//		item_lstOnSay2->SetIsParseText(true);
//	//	}
//	//	p2->SetBegin(item_lstOnSay2);
//	//}
//	//else
//	//{
//	//	p1 = g_stUICoze.lstOnSay->NewItem();
//	//	item_lstOnSay1 = new CItemEx(  str, color );
//	//	item_lstOnSay1->SetAlignment(eAlignCenter);
//	//	item_lstOnSay1->SetHeight(18);
//
//	//	if (  str.find ("#") !=-1)	
//	//	{
//	//		item_lstOnSay1->SetIsParseText(true);
//	//	}
//	//	p1->SetBegin(item_lstOnSay1);
//	//}
//	//if( CCompent::GetActive()!=lstOnSay ) lstOnSay->OnKeyDown(VK_END);
//	////g_stUICoze.lstOnSay->OnKeyDown(VK_END);
//	//g_stUICoze.lstOnSay->Refresh();
//}

void CCozeMgr::OnWorldSay( const char* name, const char* say ) //向所有人发广播信息:CS_Say2All
{
	CCozeForm::GetInstance()->OnWorldMsg(name, say);
	return;										//Arcol:Test-

	/////////处理lstOnSay的显示
	//std::string str = name;
	//str += ":";
	//str += say ;
	

	//std::string str=m_cmbChannel->GetText();
	//DWORD color;
	//if (str=="交易")
	//{
	//	str="[交易]"+std::string(name)+":"+say;
	//	color=g_stUICoze.chatColor.trade;
	//}
	//else
	//{
	//	str="[世界]"+std::string(name)+":"+say;
	//	color=g_stUICoze.chatColor.world;
	//}

	std::string filter=say;
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE,filter);
	PrintToList(CHANNEL_WORLD,name,filter,CCharMsg::GetChannelColor(CCharMsg::CHANNEL_WORLD));

	//ShowLstOnSay( str, g_stUICoze.chatColor.world );
//	if( CCompent::GetActive()!=lstOnSay ) 
//		lstOnSay->OnKeyDown(VK_END);

}

void CCozeMgr::OnTradeSay( const char* name, const char* say )
{
	CCozeForm::GetInstance()->OnTradeMsg(name, say);
	return;										//Arcol:Test-

	std::string filter=say;
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE,filter);
	PrintToList(CHANNEL_TRADE,name,filter,CCharMsg::GetChannelColor(CCharMsg::CHANNEL_TRADE));

	//ShowLstOnSay( str, g_stUICoze.chatColor.world );
	//	if( CCompent::GetActive()!=lstOnSay ) 
	//		lstOnSay->OnKeyDown(VK_END);

}

void CCozeMgr::OnPrivateSay( const char* srcName, const char* dstName,const char* say )   //私聊 
{
	CCozeForm::GetInstance()->OnPrivateMsg(srcName, dstName, say);
	return;										//Arcol:Test-

	if( !CGameScene::GetMainCha() ) return;

	CCharacter* pCha = CGameScene::GetMainCha();
	std::string selfName=pCha->getHumanName();
	std::string name;
	std::string str;
	std::string filter=say;
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE,filter);
	if (selfName==srcName)
	{
		name=std::string(dstName);
		str="<To "+name+">:"+filter;
	}
	else
	{
		name=std::string(srcName);
		str="<From "+name+">:"+filter;
	}
	AddNameToCard(name);
	PrintToList(CHANNEL_PRIVATE,name,str,CCharMsg::GetChannelColor(CCharMsg::CHANNEL_PRIVATE));
	//ShowLstOnSay( str, g_stUICoze.chatColor.person );
	//if( CCompent::GetActive()!=lstOnSay ) lstOnSay->OnKeyDown(VK_END);
}

void CCozeMgr::OnSystemSay( const char* say )
{
	CCozeForm::GetInstance()->OnSystemMsg(say);
	return;										//Arcol:Test-

	PrintToList(CHANNEL_SYSTEM,"",say,CCharMsg::GetChannelColor(CCharMsg::CHANNEL_SYSTEM));

	// CItemRow* p = lstOnSay->Add( "系统信息" );
	// p->GetIndex(0)->SetColor( g_stUIMain.chatColor.world );	

//	ShowLstOnSay( say, g_stUICoze.chatColor.world );
//	CItem *item = new CItem( say, g_stUIMain.chatColor.world );
//	p->SetBegin( item );

//	if( CCompent::GetActive()!=lstOnSay ) 
	lstOnSay->OnKeyDown(VK_END);	
}

void CCozeMgr::OnRoadSay( CCharacter* c, const char* say )
{
	CCozeForm::GetInstance()->OnSightMsg(c, say);
	return;										//Arcol:Test-

	if ( strcmp( say ,"***0")== 0 )
	{		c->GetHeadSay()->SetFaceID( 0);  return ;  }	
	else if ( strcmp( say ,"***1")== 0 )
	{		c->GetHeadSay()->SetFaceID( 1);  return ;  }
	else if ( strcmp( say ,"***2")== 0 )
	{		c->GetHeadSay()->SetFaceID( 2);  return ;  }
	else if ( strcmp( say ,"***3")== 0 )
	{		c->GetHeadSay()->SetFaceID( 3);  return ;  }
	else if ( strcmp( say ,"***4")== 0 )
	{		c->GetHeadSay()->SetFaceID( 4);  return ;  }
	else if ( strcmp( say ,"***5")== 0 )
	{		c->GetHeadSay()->SetFaceID( 5);  return ;  }
	else if ( strcmp( say ,"***6")== 0 )
	{		c->GetHeadSay()->SetFaceID( 6);  return ;  }
	else if ( strcmp( say ,"***7")== 0 )
	{		c->GetHeadSay()->SetFaceID( 7);  return ;  }
	else if ( strcmp( say ,"***8")== 0 )
	{		c->GetHeadSay()->SetFaceID( 8);  return ;  }
	else if ( strcmp( say ,"***9")== 0 )
	{		c->GetHeadSay()->SetFaceID( 9);  return ;  }
	/////////处理lstOnSay的显示
	//std::string str = c->getName();
	//str += ":";
	//str += say ;
	
	std::string filter=say;
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE,filter);
	PrintToList( CHANNEL_ROAD,c->getHumanName(),filter,CCharMsg::GetChannelColor(CCharMsg::CHANNEL_SIGHT));

	//ShowLstOnSay( str, g_stUICoze.chatColor.road );
	//if( CCompent::GetActive()!=lstOnSay ) 
	//	lstOnSay->OnKeyDown(VK_END);	



	//对家头像上的聊天显示
	std::string str = std::string(c->getHumanName())+":";
	size_t player_name_string_length = str.length();
	str += filter;
	CItemEx *item = new CItemEx( str, CCharMsg::GetChannelColor(CCharMsg::CHANNEL_SIGHT));
	item->ProcessString(player_name_string_length);
	c->GetHeadSay()->AddItem( item );
}

void CCozeMgr::_evtShowChannelList(CGuiData *pSender, int x, int y, DWORD key)
{
	//lstChannel->SetisShow
}

void CCozeMgr::OnTeamSay(DWORD cha_id, const char* say )
{
	CCozeForm::GetInstance()->OnTeamMsg(cha_id, say);
	return;				//Arcol:Test-

	if( !CGameScene::GetMainCha() ) return;

	std::string name;
	if ( cha_id!=CGameScene::GetMainCha()->getHumanID() )
	{
		CTeam *pTeam=g_stUIChat.GetTeamMgr()->Find(enumTeamGroup);
		if (!pTeam) return;
		CMember *pMember=pTeam->Find(cha_id);
		if (!pMember) return;
		name=pMember->GetName();
	}
	else
	{
		name=CGameScene::GetMainCha()->getHumanName();
	}
	std::string filter=say;
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE,filter);
	PrintToList(CHANNEL_TEAM,name,filter,CCharMsg::GetChannelColor(CCharMsg::CHANNEL_TEAM));
}

bool CCozeMgr::IsForbidChannel(eChannel channel)
{
	switch(channel)
	{
	case CHANNEL_ROAD:
		return m_pSightCheck->GetIsChecked();
	case CHANNEL_PRIVATE:
		return m_pPrivateCheck->GetIsChecked();
	case CHANNEL_TEAM:
		return m_pTeamCheck->GetIsChecked();
	case CHANNEL_GUILD:
		return m_pGuildCheck->GetIsChecked();
	case CHANNEL_WORLD:
		return m_pWorldCheck->GetIsChecked();
	case CHANNEL_SYSTEM:
		//系统消息永远不过滤，只隐藏窗口
		//return m_pSystemCheck->GetIsChecked();
		return false;
	case CHANNEL_TRADE:
		return m_pTradeCheck->GetIsChecked();
	}
	return false;
}

void CCozeMgr::PrintToList(eChannel channel,std::string name, std::string str, DWORD color)
{
	return;						//Arcol:Test-

	std::string strPrint="";
	switch(channel)
	{
	case CHANNEL_ROAD:
		strPrint=RES_STRING(CL_LANGUAGE_MATCH_520)+name+":"+str;
		break;
	case CHANNEL_TEAM:
		if (name.empty())
		{
			strPrint=RES_STRING(CL_LANGUAGE_MATCH_521)+str;
		}
		else
		{
			strPrint=RES_STRING(CL_LANGUAGE_MATCH_521)+name+":"+str;
		}
		break;
	case CHANNEL_GUILD:
		if (name.empty())
		{
			strPrint=RES_STRING(CL_LANGUAGE_MATCH_522)+str;
		}
		else
		{
			strPrint=RES_STRING(CL_LANGUAGE_MATCH_522)+name+":"+str;
		}
		break;
	case CHANNEL_TRADE:
		strPrint=RES_STRING(CL_LANGUAGE_MATCH_523)+name+":"+str;
		break;
	case CHANNEL_WORLD:
		strPrint=RES_STRING(CL_LANGUAGE_MATCH_524)+name+":"+str;
		break;
	case CHANNEL_PRIVATE:
		strPrint=RES_STRING(CL_LANGUAGE_MATCH_525)+str;
		break;
	case CHANNEL_SYSTEM:
		if (name.empty())
		{
			strPrint=RES_STRING(CL_LANGUAGE_MATCH_526)+str;
		}
		else
		{
			strPrint=RES_STRING(CL_LANGUAGE_MATCH_526)+name+":"+str;
		}
		break;
	}
	if (channel==CHANNEL_SYSTEM || channel==CHANNEL_PRIVATE)
	{
		for (int i=CHANNEL_PRIVATE;i<=CHANNEL_TRADE;i++)
		{
			m_TextRecords[i][m_TextPoint[i][1]].eTextChannel=channel;
			m_TextRecords[i][m_TextPoint[i][1]].stText=strPrint;
			m_TextRecords[i][m_TextPoint[i][1]].stName=name;
			m_TextRecords[i][m_TextPoint[i][1]].dwColor=color;
			m_TextPoint[i][1]=(m_TextPoint[i][1]+1)%DATA_RECORDS_SIZE;
			if (m_TextPoint[i][1]==m_TextPoint[i][0])
			{
				m_TextPoint[i][0]=(m_TextPoint[i][0]+1)%DATA_RECORDS_SIZE;
			}
		}
	}
	else if ( channel!=CHANNEL_ROAD )
	{
		m_TextRecords[channel][m_TextPoint[channel][1]].eTextChannel=channel;
		m_TextRecords[channel][m_TextPoint[channel][1]].stText=strPrint;
		m_TextRecords[channel][m_TextPoint[channel][1]].stName=name;
		m_TextRecords[channel][m_TextPoint[channel][1]].dwColor=color;
		m_TextPoint[channel][1]=(m_TextPoint[channel][1]+1)%DATA_RECORDS_SIZE;
		if (m_TextPoint[channel][1]==m_TextPoint[channel][0])
		{
			m_TextPoint[channel][0]=(m_TextPoint[channel][0]+1)%DATA_RECORDS_SIZE;
		}
	}
	m_TextRecords[CHANNEL_ROAD][m_TextPoint[CHANNEL_ROAD][1]].eTextChannel=channel;
	m_TextRecords[CHANNEL_ROAD][m_TextPoint[CHANNEL_ROAD][1]].stText=strPrint;
	m_TextRecords[CHANNEL_ROAD][m_TextPoint[CHANNEL_ROAD][1]].stName=name;
	m_TextRecords[CHANNEL_ROAD][m_TextPoint[CHANNEL_ROAD][1]].dwColor=color;
	m_TextPoint[CHANNEL_ROAD][1]=(m_TextPoint[CHANNEL_ROAD][1]+1)%DATA_RECORDS_SIZE;
	if (m_TextPoint[CHANNEL_ROAD][1]==m_TextPoint[CHANNEL_ROAD][0])
	{
		m_TextPoint[CHANNEL_ROAD][0]=(m_TextPoint[CHANNEL_ROAD][0]+1)%DATA_RECORDS_SIZE;
	}

	//if (m_CurSelectChannel==channel || m_CurSelectChannel==CHANNEL_ROAD || channel==CHANNEL_SYSTEM || channel==CHANNEL_PRIVATE))
	if ( (m_CurSelectChannel!=CHANNEL_ROAD && (m_CurSelectChannel==channel || channel==CHANNEL_SYSTEM || (channel==CHANNEL_PRIVATE && !IsForbidChannel(CHANNEL_PRIVATE)))) ||
		 (m_CurSelectChannel==CHANNEL_ROAD && !IsForbidChannel(channel)) )
	{
		bool indentFlag=false;
		while (!strPrint.empty())
		{
			if (indentFlag)
			{
				strPrint="      "+strPrint;
			}
			std::string text=CutFaceText(strPrint,44);
			CItemRow* pItemRow;
			if (channel==CHANNEL_SYSTEM)
			{
				pItemRow=g_stUICoze.lstOnSystemSay->NewItem();
			}
			else
			{
				pItemRow=g_stUICoze.lstOnSay->NewItem();
			}
			CItemEx* pItem=new CItemEx(text,color);
			pItem->SetAlignment(eAlignCenter);
			pItem->SetHeight(18);
			pItem->SetItemName(name.c_str());
			pItem->SetIsParseText(true);
			if (!indentFlag)
			{
				pItem->SetHasHeadText(6,0xFFFFFF00);
			}
			pItemRow->SetBegin(pItem);
			indentFlag=true;
		}

		CListItems* pListItems=g_stUICoze.lstOnSystemSay->GetItems();
		if (pListItems->GetCount()>200)
		{
			pListItems->Del(pListItems->GetItem(0));
		}
		if( CCompent::GetActive()!=lstOnSystemSay->GetScroll() ) lstOnSystemSay->OnKeyDown(VK_END);
		g_stUICoze.lstOnSystemSay->Refresh();

		pListItems=g_stUICoze.lstOnSay->GetItems();
		if (pListItems->GetCount()>200)
		{
			pListItems->Del(pListItems->GetItem(0));
		}
		if( CCompent::GetActive()!=lstOnSay->GetScroll() ) lstOnSay->OnKeyDown(VK_END);
		g_stUICoze.lstOnSay->Refresh();
	}
}

void CCozeMgr::_MainOnLstKeyDownEvent(CGuiData *pSender, int x, int y, DWORD key)
{
	if( key & Mouse_RDown )
	{
		CItemRow* pItemRow=lstOnSay->GetSelectItem();
		if (!pItemRow) return;
		CItemEx* pItemEx=dynamic_cast<CItemEx*>(pItemRow->GetBegin());
		if (!pItemEx) return;
		std::string str=pItemEx->GetItemName();
		if (str.empty()) return;
		//std::string str=pItemEx->GetString();
		//if (!GetNameFormString(str,str)) return;
		m_CurCommand="@"+str+" ";
		g_stUICoze.edtSay->SetCaption(m_CurCommand.c_str());
	}
}

void CCozeMgr::SetPrivateName(const char* srcName)
{
	if (!strlen(srcName)) return;
	m_CurCommand="@"+std::string(srcName)+" ";
	g_stUICoze.edtSay->SetCaption(m_CurCommand.c_str());
	CCompent::SetActive(g_stUICoze.edtSay);
	AddNameToCard(srcName);
}

void CCozeMgr::CloseForm()
{
	if ( grdFace )
	{
		grdFace->SetIsShow( false );
	}
	if ( m_pCardList )
	{
		 m_pCardList->SetIsShow( false );
	}
	if ( frmForbid )
	{
		frmForbid->SetIsShow( false );
	}

}

void CCozeMgr::ResetAll()
{
	return;										//Arcol:Test-

	g_stUICoze._nSelectSayRow = 0;
	g_stUICoze._nOldSayRow = 0;
	int i=0;
	for (;i<MAX_PAGE_SAY;i++)
	{
		g_stUICoze.oldSay[i]="";
	}
	m_CurSelectChannel=CHANNEL_ROAD;
	m_cmbChannel->GetList()->GetItems()->Select(0);
	m_CurCommand="";
	for (i=0;i<CHANNELS_COUNT;i++)
	{
		for (int j=0;j<DATA_RECORDS_SIZE;j++)
		{
			m_TextRecords[i][j].stName=m_TextRecords[i][j].stText="";
			m_TextRecords[i][j].dwColor=0;
		}
	}
	for (i=CHANNEL_ROAD;i<CHANNEL_TRADE;i++)
	{
		m_TextPoint[i][1]=m_TextPoint[i][0]=0;
	}
	m_pForbidCheck->SetIsChecked(false);
	g_stUICoze.lstOnSay->GetItems()->Clear();
	g_stUICoze.frmMainChat->Refresh();
}

void CCozeMgr::AddInput( const char* str )
{
	CEdit* pEdit = dynamic_cast<CEdit*>( CCompent::GetActive() );
	if( !pEdit )
	{
		if( !edtSay ) return;

		pEdit = edtSay;
	}

	std::string name = pEdit->GetCaption();
	name += str;
	UINT len=pEdit->GetMaxNum();
	if (strlen(name.c_str())>len)
	{
		return;
	}
	pEdit->SetCaption( name.c_str() );
}

void CCozeMgr::_evtCardClick(CGuiData *pSender, int x, int y, DWORD key)
{
	m_pCardList->GetItems()->Clear();
	for (vector<std::string>::reverse_iterator it=m_strCardListData.rbegin();it!=m_strCardListData.rend();it++)
	{
		m_pCardList->Add((*it).c_str());
	}
	CItemRow *pRow=m_pCardList->Add(RES_STRING(CL_LANGUAGE_MATCH_515));
	pRow->GetBegin()->SetColor(0xFF7F7F3F);
	m_pCardList->SetPointer(pRow);
	m_pCardList->Refresh();
	m_pCardList->SetIsShow(!m_pCardList->GetIsShow());
}

void CCozeMgr::_evtCardSelectChange(CGuiData *pSender)
{
	CList *pList=dynamic_cast<CList*>(pSender);
	pList->SetIsShow(false);
	CItemRow *pRow=pList->GetSelectItem();
	if (!pRow) return;
	std::string strName=pRow->GetBegin()->GetString();
	if (strName.length()==0) return;
	if (strName==RES_STRING(CL_LANGUAGE_MATCH_515) && pList->GetPointer()==pRow)
	{
		m_strCardListData.clear();
		return;
	}
	std::string strChat=edtSay->GetCaption();
	int nLTrim=0;
	if (strChat[0]=='*' || strChat[0]=='^' || strChat[0]=='!' || strChat[0]=='*' || strChat[0]=='%')
	{
		nLTrim=1;
	}
	else if ( strChat[0]=='@' )
	{
		nLTrim = (int)(strChat.find(" ")) +1;
	}
	if (nLTrim>0 && nLTrim<=(int)strChat.length())
	{
		strChat=strChat.substr(nLTrim,strChat.length()-nLTrim);
	}
	strChat="@"+strName+" "+strChat;
	if (strChat.length()>80)
	{
		strChat="@"+strName+" ";
	}
	edtSay->SetCaption(strChat.c_str());
}

void CCozeMgr::AddNameToCard(std::string strName)
{
	if (strName.length()==0) return;
	for (vector<std::string>::iterator it=m_strCardListData.begin();it!=m_strCardListData.end();it++)
	{
		if ((*it)==strName)
		{
			m_strCardListData.erase(it);
			m_strCardListData.push_back(strName);
			return;
		}
	}
	m_strCardListData.push_back(strName);
	if (m_strCardListData.size()>CARD_LIST_MAX)
	{
		m_strCardListData.erase(m_strCardListData.begin());
	}
}

void CCozeMgr::_evtGmForm( CForm* pForm, bool& IsShow )
{
	CCharacter* pMain = CGameScene::GetMainCha();
	if( !pMain ) return;

	if(pMain->getGMLv()==0)
	{
		IsShow=false;
	}
}


void CCozeMgr::_evtGmSay(CGuiData *pSender)
{
	CCharacter* pCha = CGameScene::GetMainCha();
	if( !pCha ) return;
	
	//如果全是空格，则不显示	
	const char* s = g_stUICoze.edtGmSay->GetCaption() ;
	int len =(int) strlen( s );
	int i = 0;
	for ( ; i <len  ;i++)
	{
		if 	(s[i]!= 32 )  //32是空格的ascii码
			break;
	}	
	if (i == len )
		return ;


	CS_GM1Say(s);
	g_stUICoze.edtGmSay->SetCaption("");
}

void CCozeMgr::OnGmSay( const char* name, const char* say )
{
	std::string str;
	//str="[系统公告]"+std::string(name)+":"+std::string(say);
	str=RES_STRING(CL_LANGUAGE_MATCH_507)+std::string(say);
	//str=std::string(say);
	g_pGameApp->ShowNotify(str.c_str(),CCharMsg::GetChannelColor(CCharMsg::CHANNEL_PUBLISH));
}

void CCozeMgr::OnGuildSay( const char* name, const char* say )
{
	CCozeForm::GetInstance()->OnGuildMsg(name, say);
	return;				//Arcol:Test-

	std::string filter=say;
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE,filter);
	PrintToList(CHANNEL_GUILD,name,filter,CCharMsg::GetChannelColor(CCharMsg::CHANNEL_GUILD));
}

#endif
