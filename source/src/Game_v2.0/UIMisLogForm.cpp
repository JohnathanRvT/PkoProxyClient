#include "StdAfx.h"
#include "UIMisLogForm.h"
#include "uiform.h"
#include "UITreeView.h"
#include "UIMemo.h"
#include "PacketCmd.h"
#include "UIItem.h"
#include "Script.h"
#include "lua_platform.h"
#ifdef CALUA_COMPAT
#include "caLua.h" //CHANGED: Removed CaLua dependency
#endif
#include "uiboxform.h"
#include "uiboatform.h"
#include "MapSet.h"
#include "FindPath.h"
#include "UIStartForm.h"
#include "UIList.h"
#include "NPCHelper.h"
using namespace GUI;

extern lua_State* g_pLuaState;

//---------------------------------------------------------------------------
// class CMisLogForm
//---------------------------------------------------------------------------
CMisLogForm::CMisLogForm() {
	m_pMisTree = nullptr;
	m_pMisInfo = nullptr;
	m_pNormal = nullptr;
	m_pHistory = nullptr;
	m_pGuild = nullptr;
	m_pMain = nullptr; //	rain added to test main task.
	m_pInvalid = nullptr;
	m_wMisID = 0xFFFF;
	m_dwUpdateTick = 0;
}

CMisLogForm::~CMisLogForm() {
}

bool CMisLogForm::Init() {
	m_pForm = _FindForm("frmMission");
	if (!m_pForm) {
		LG("gui", RES_STRING(CL_LANGUAGE_MATCH_721));
		return false;
	}

	m_pForm->evtEntrustMouseEvent = _MouseEvent;
	m_pForm->evtShow = _Show;
	m_pMisTree = dynamic_cast<CTreeView*>(m_pForm->Find("trvMission"));

	if (!m_pMisTree) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_473), m_pForm->GetName(), "trvMission");
	}

	m_pMisTree->evtMouseDown = _MouseDown;
	m_pMisTree->SetSelectColor(0xFF00FFFF);
	m_pMisTree->SetColSpace(1);

	m_pMisInfo = dynamic_cast<CMemoEx*>(m_pForm->Find("memMiss"));
	if (!m_pMisInfo) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_473), m_pForm->GetName(), "memMiss");
	}
	m_pMisInfo->SetRowNum(64);
	m_pMisInfo->Refresh();
	m_pMisInfo->evtClickItem = _ItemClickEvent;
	auto* pButton = dynamic_cast<CTextButton*>(m_pForm->Find("btnBreak"));
	if (pButton) {
		pButton->SetIsEnabled(false);
	}

	return true;
}

void CMisLogForm::End() {
}

void CMisLogForm::_Show(CGuiData* pSender) {
	if (_stricmp("frmMission", pSender->GetName()) == 0) {
		g_stUIMisLog.MisRefresh();
	}
}

void CMisLogForm::_MouseEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	std::string strName = pSender->GetName();
	if (_stricmp("frmMission", pSender->GetForm()->GetName()) == 0) {
		// 如果是退出按钮,则关闭该表单
		if (strName == "btnNo" || strName == "btnClose") {
			pSender->GetForm()->Close();
			return;
		} else if (strName == "btnBreak") {
			// 取消一个任务
			CTreeNodeObj* pNode = g_stUIMisLog.m_pMisTree->GetSelectNode();
			if (pNode == nullptr)
				return;

			char szData[128];
			BYTE byType = 0;
			_snprintf_s(szData, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_722), g_stUIMisLog.m_wMisID);

			// 获取任务信息
			MisData pRet = g_stUIMisLog.GetMisData(g_stUIMisLog.m_wMisID);

			char szBuf[256] = {0};
			//if( byType==mission::MIS_TREENODE_RAND )
			//{
			//	int nMoney = 100;
			//	if( CCharacter* pMain = g_stUIBoat.GetHuman() )
			//	{
			//		nMoney += pMain->getGameAttr()->get(ATTR_LV) * pMain->getGameAttr()->get(ATTR_LV);
			//	}
			//	_snprintf_s( szBuf, _TRUNCATE, "中断[%s]\n将扣除金钱[%d$]?", szData, nMoney );
			//}
			//else
			//{
			_snprintf_s(szBuf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_723), szData);
			//}

			stSelectBox* pBox = CBoxMgr::ShowSelectBox(_evtBreakYesNoEvent, szBuf, true);
			pBox->dwTag = pNode->GetTag();
		}
	}
}

void CMisLogForm::_evtBreakYesNoEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType != CForm::mrYes)
		return;

	stSelectBox* pBox = (stSelectBox*)pSender->GetForm()->GetPointer();
	if (!pBox)
		return;

	CS_MisClear((WORD)pBox->dwTag);
}

void CMisLogForm::_MouseDown(CGuiData* pSender, int x, int y, MouseClickState key) {
	auto* pTree = dynamic_cast<CTreeView*>(pSender);
	if (!pTree)
		return;

	CTreeNodeObj* pNode = pTree->GetHitNode(x, y);
	if (pNode) {
		DWORD dwData = pNode->GetTag();
		if (dwData == -1) {
			return;
		}

		auto* pButton = dynamic_cast<CTextButton*>(g_stUIMisLog.m_pForm->Find("btnBreak"));
		if (pButton) {
			pButton->SetIsEnabled(true);
		} else {
			pButton->SetIsEnabled(false);
		}
		DWORD dwTick = GetTickCount();
		if (dwTick - g_stUIMisLog.m_dwUpdateTick > MISLOG_REFRESH_TIME || dwData != g_stUIMisLog.m_wMisID) {
			CS_MisLogInfo((WORD)dwData);
			g_stUIMisLog.m_dwUpdateTick = dwTick;
		}
	}
}

void CMisLogForm::MisRefresh() {
	DWORD dwTick = GetTickCount();
	if (dwTick - m_dwUpdateTick > MISLOG_REFRESH_TIME) {
		if (m_wMisID != 0xFFFF) {
			CS_MisLogInfo(m_wMisID);
			m_dwUpdateTick = dwTick;
		}
	}
}

BOOL CMisLogForm::AddNode(WORD wMisID, BYTE byState, BYTE& byType) {
	USHORT sMaxNum = 28;
	char szData[128];
	byType = 0;
	_snprintf_s(szData, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_722), wMisID);
	CTreeNodeObj* pNode = m_pMisTree->GetRootNode();

	// 获取任务信息
	MisData pRet = GetMisData(wMisID);
	if (pRet.byRet == LUA_TRUE)
		byType = pRet.byType;

	std::string strData = "";

	if (byState == ROLE_MIS_PENDING_FLAG) {
		strData += RES_STRING(CL_LANGUAGE_MATCH_724);
	} else if (byState == ROLE_MIS_COMPLETE_FLAG) {
		strData += RES_STRING(CL_LANGUAGE_MATCH_725);
	} else if (byState == ROLE_MIS_FAILURE_FALG) {
		strData += RES_STRING(CL_LANGUAGE_MATCH_726);
	} else {
		strData += RES_STRING(CL_LANGUAGE_MATCH_727);
	}

	strData += "<p";
	if (pRet.byRet == LUA_TRUE)
		strData += pRet.pszName;
	else
		strData += szData;
	strData += ">";
	strData += "<r";
	int nLen = sMaxNum - (int)strlen(szData) - 6;
	for (int i = 0; i < nLen; i++) {
		strData += ".";
	}
	strData += ">";

	// 根据任务信息类型添加的分类节点中
	auto* pItem = new CColorItem;
	pItem->SetString(strData.c_str());
	CTreeNodeObj* pTreeNode = new CTreeNode(m_pMisTree, pItem);
	DWORD dwData = wMisID;
	pTreeNode->SetTag(dwData);

	// 任务显示节点类型
	if (byType == mission::MIS_TREENODE_NORMAL) {
		if (m_pNormal == nullptr) {
			auto* pTemp = new CItem(RES_STRING(CL_LANGUAGE_MATCH_728), COLOR_BLACK);
			m_pNormal = new CTreeNode(m_pMisTree, pTemp);
			m_pNormal->SetTag(-1);
			pNode->AddNode(m_pNormal);
		}
		m_pNormal->AddNode(pTreeNode);
	} else if (byType == mission::MIS_TREENODE_HISTORY) {
		if (m_pHistory == nullptr) {
			auto* pTemp = new CItem(RES_STRING(CL_LANGUAGE_MATCH_729), COLOR_BLACK);
			m_pHistory = new CTreeNode(m_pMisTree, pTemp);
			m_pHistory->SetTag(-1);
			pNode->AddNode(m_pHistory);
		}
		m_pHistory->AddNode(pTreeNode);
	} else if (byType == mission::MIS_TREENODE_GUILD) {
		if (m_pGuild == nullptr) {
			auto* pTemp = new CItem(RES_STRING(CL_LANGUAGE_MATCH_730), COLOR_BLACK);
			m_pGuild = new CTreeNode(m_pMisTree, pTemp);
			m_pGuild->SetTag(-1);
			pNode->AddNode(m_pGuild);
		}
		m_pGuild->AddNode(pTreeNode);
	}
	// rain added to test main task.
	else if (byType == mission::MIS_TREENODE_MAIN) {
		if (m_pMain == nullptr) {
			auto* pTemp = new CItem(RES_STRING(CL_LANGUAGE_MATCH_976), COLOR_BLACK);
			m_pMain = new CTreeNode(m_pMisTree, pTemp);
			m_pMain->SetTag(-1);
			pNode->AddNode(m_pMain);
		}
		m_pMain->AddNode(pTreeNode);
	} else {
		if (m_pInvalid == nullptr) {
			auto* pTemp = new CItem(RES_STRING(CL_LANGUAGE_MATCH_731), COLOR_BLACK);
			m_pInvalid = new CTreeNode(m_pMisTree, pTemp);
			m_pInvalid->SetTag(-1);
			pNode->AddNode(m_pInvalid);
		}

		m_pInvalid->AddNode(pTreeNode);
	}

	m_pMisTree->Refresh();

	return TRUE;
}

void CMisLogForm::ClearAllNode() {
	m_pMisTree->ClearAllNode();
	m_pNormal = nullptr;
	m_pHistory = nullptr;
	m_pGuild = nullptr;
	m_pMain = nullptr; //	rain added to test main task
	m_pInvalid = nullptr;
	m_pMisInfo->Init();
}

void CMisLogForm::MisLogList(const NET_MISLOG_LIST& List) {
	ClearAllNode();

	memcpy(&m_LogList, &List, sizeof(NET_MISLOG_LIST));

	for (BYTE i = 0; i < m_LogList.byNumLog; i++) {
		AddNode(m_LogList.MisLog[i].wMisID, m_LogList.MisLog[i].byState, m_LogList.MisLog[i].byType);
	}

	if (List.byNumLog > 0) {
		CS_MisLogInfo(List.MisLog[0].wMisID);
	}
}

void CMisLogForm::MissionLog(WORD wMisID, const NET_MISPAGE& page) {
	if (m_pMisInfo == nullptr) {
		return;
	}

	m_pMisInfo->Init();
	m_pMisInfo->SetMisPage(page);
	m_pMisInfo->SetIsShow(true);
	m_wMisID = wMisID;
}

CMisLogForm::MisData CMisLogForm::GetMisData(WORD wMisID) {
	CMisLogForm::MisData pRet;
#ifdef CALUA_COMPAT
	pRet = (RetType*)CLU_CallScriptFunction("GetMisData", "uchar,uchar,char*", "ushort", wMisID);
	CLU_DllFree(pRet); //TODO: Check if this is right - also not updated to use std::string.
#else
	lua_getglobal(L, "GetMisData");
	lua_pushnumber(L, wMisID);
	lua_handled_pcall(L, 1, 3, 0);
	pRet.byRet = lua_tonumber(L, 1);
	pRet.byType = lua_tonumber(L, 2);
	pRet.pszName = (char*)lua_tostring(L, 3);
	//TODO: Handle the extra parameters used in lua scripts.
	lua_settop(L, 0);
#endif
	return pRet;
}

void CMisLogForm::MisClear(WORD wMisID) {
	int nIndex = -1;
	for (int i = 0; i < m_LogList.byNumLog; i++) {
		if (m_LogList.MisLog[i].wMisID == wMisID) {
			nIndex = i;
			break;
		}
	}

	if (nIndex == -1) {
		g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_732), wMisID);
		return;
	}

	CTreeNodeObj* pNode = nullptr;
	if (m_LogList.MisLog[nIndex].byType == mission::MIS_TREENODE_NORMAL) {
		pNode = m_pNormal;
	} else if (m_LogList.MisLog[nIndex].byType == mission::MIS_TREENODE_HISTORY) {
		pNode = m_pHistory;
	} else if (m_LogList.MisLog[nIndex].byType == mission::MIS_TREENODE_GUILD) {
		pNode = m_pGuild;
	}
	//	rain added to test main task.
	else if (m_LogList.MisLog[nIndex].byType == mission::MIS_TREENODE_MAIN) {
		pNode = m_pMain;
	} else {
		pNode = m_pInvalid;
	}

	if (pNode) {
		USHORT sNum = pNode->GetChildCount();
		for (USHORT n = 0; n < sNum; n++) {
			CTreeNodeObj* pChild = pNode->GetChildNode(n);
			if (pChild == nullptr) {
				g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_733), wMisID, n);
				break;
			}
			DWORD dwData = pChild->GetTag();
			if (dwData == wMisID) {
				m_pMisTree->DelNode(pChild);
				m_pMisTree->Refresh();
				break;
			}
		}
		if (pNode->GetChildCount() <= 0) {
			m_pMisTree->DelNode(pNode);
			m_pMisTree->Refresh();
			if (pNode == m_pInvalid) {
				m_pInvalid = nullptr;
			} else if (pNode == m_pNormal) {
				m_pNormal = nullptr;
			} else if (pNode == m_pHistory) {
				m_pHistory = nullptr;
			} else if (pNode == m_pGuild) {
				m_pGuild = nullptr;
			}
			//	rain added to test main task.
			else if (m_LogList.MisLog[nIndex].byType == mission::MIS_TREENODE_MAIN) {
				pNode = m_pMain;
			}
		}
	}

	NET_MISLOG Log[ROLE_MAXNUM_MISSION];
	memset(Log, 0, sizeof(NET_MISLOG) * ROLE_MAXNUM_MISSION);
	memcpy(Log, m_LogList.MisLog, sizeof(NET_MISLOG) * m_LogList.byNumLog);
	memset(m_LogList.MisLog + nIndex, 0, sizeof(NET_MISLOG) * (m_LogList.byNumLog - nIndex));
	memcpy(m_LogList.MisLog + nIndex, Log + nIndex + 1, sizeof(NET_MISLOG) * (m_LogList.byNumLog - nIndex - 1));
	m_LogList.byNumLog--;

	if (m_wMisID == wMisID) {
		m_wMisID = 0xFFFF;
		m_pMisInfo->Init();
		if (m_LogList.byNumLog > 0) {
			CS_MisLogInfo(m_LogList.MisLog[0].wMisID);
		}
	}
}

void CMisLogForm::MisAddLog(WORD wMisID, BYTE byState) {
	m_LogList.MisLog[m_LogList.byNumLog].byState = byState;
	m_LogList.MisLog[m_LogList.byNumLog].wMisID = wMisID;

	AddNode(wMisID, byState, m_LogList.MisLog[m_LogList.byNumLog].byType);

	m_LogList.byNumLog++;

	if (m_wMisID == 0xFFFF) {
		CS_MisLogInfo(m_LogList.MisLog[0].wMisID);
	}
}

void CMisLogForm::MisLogState(WORD wMisID, BYTE byState) {
	int i = 0;
	for (; i < m_LogList.byNumLog; i++) {
		if (m_LogList.MisLog[i].wMisID == wMisID) {
			if (m_LogList.MisLog[i].byState == byState)
				return;

			// 更新任务状态
			m_LogList.MisLog[i].byState = byState;
			break;
		}
	}

	CTreeNodeObj* pNode = nullptr;
	if (m_LogList.MisLog[i].byType == mission::MIS_TREENODE_NORMAL) {
		pNode = m_pNormal;
	} else if (m_LogList.MisLog[i].byType == mission::MIS_TREENODE_HISTORY) {
		pNode = m_pHistory;
	} else if (m_LogList.MisLog[i].byType == mission::MIS_TREENODE_GUILD) {
		pNode = m_pGuild;
	}
	//	rain added
	else if (m_LogList.MisLog[i].byType == mission::MIS_TREENODE_MAIN) {
		pNode = m_pMain;
	} else {
		pNode = m_pInvalid;
	}

	CTreeNode* pFind = nullptr;
	if (pNode) {
		USHORT sNum = pNode->GetChildCount();
		for (USHORT n = 0; n < sNum; n++) {
			CTreeNodeObj* pChild = pNode->GetChildNode(n);
			if (pChild == nullptr) {
				g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_734), wMisID, n);
				break;
			}
			DWORD dwData = pChild->GetTag();
			if (dwData == wMisID) {
				pFind = dynamic_cast<CTreeNode*>(pChild);
				break;
			}
		}
	}

	if (pFind == nullptr) {
		g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_735), wMisID);
		return;
	}

	auto* pItem = dynamic_cast<CColorItem*>(pFind->GetItem());

	// 获取任务信息
	char szData[128];
	BYTE byType = 0;
	_snprintf_s(szData, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_722), wMisID);
	MisData pRet = GetMisData(wMisID);
	if (pRet.byRet == LUA_TRUE)
		byType = pRet.byType;

	std::string strData = "";

	if (byState == ROLE_MIS_PENDING_FLAG) {
		strData += RES_STRING(CL_LANGUAGE_MATCH_724);
	} else if (byState == ROLE_MIS_COMPLETE_FLAG) {
		strData += RES_STRING(CL_LANGUAGE_MATCH_725);
	} else if (byState == ROLE_MIS_FAILURE_FALG) {
		strData += RES_STRING(CL_LANGUAGE_MATCH_726);
	} else {
		strData += RES_STRING(CL_LANGUAGE_MATCH_727);
	}

	if (byState == ROLE_MIS_PENDING_FLAG) {
		strData += RES_STRING(CL_LANGUAGE_MATCH_736);
	} else if (byState == ROLE_MIS_COMPLETE_FLAG) {
		strData += RES_STRING(CL_LANGUAGE_MATCH_737);
	} else if (byState == ROLE_MIS_FAILURE_FALG) {
		strData += RES_STRING(CL_LANGUAGE_MATCH_738);
	} else {
		strData += RES_STRING(CL_LANGUAGE_MATCH_739);
	}

	strData = +"<p";

	if (pRet.byRet == LUA_TRUE)
		strData += pRet.pszName;
	else
		strData += szData;
	strData += ">";
	strData += "<r";
	int nLen = 24 - (int)strlen(szData) - 6;
	for (int i = 0; i < nLen; i++) {
		strData += ".";
	}
	strData += ">";

	pItem->SetString(strData.c_str());
}

void CMisLogForm::_ItemClickEvent(std::string strItem) {
	const char* pStr = strItem.c_str();
	const char* p = pStr;
	const char* q;
	char* map;
	char* x;
	char* y;

	bool bmap = false;
	bool bx = false;
	bool by = false;

	int index = 0;
	int num = 0;
	q = p;
	while (((*p) != '(') && index < strItem.length()) {
		num++;
		p++;
		index++;
	}
	if ((*p) == '(') {
		map = new char[num + 1];
		memcpy(map, q, sizeof(char) * num);
		map[num] = '\0';
		p++;
		index++;
		bmap = true;
	}

	num = 0;
	q = p;
	while (((*p) != ',') && index < strItem.length()) {
		num++;
		p++;
		index++;
	}
	if ((*p) == ',') {
		x = new char[num + 1];
		memcpy(x, q, sizeof(char) * num);
		x[num] = '\0';
		p++;
		index++;
		bx = true;
	}

	num = 0;
	q = p;
	while (((*p) != ')') && index < strItem.length()) {
		num++;
		p++;
		index++;
	}
	if ((*p) == ')') {
		y = new char[num + 1];
		memcpy(y, q, sizeof(char) * num);
		y[num] = '\0';
		p++;
		index++;
		by = true;
	}

	if (bmap && bx && by && index == strItem.length()) {
		//check npc is valid or not
		const char* targetmap = nullptr;
		if (strcmp((const char*)map, (const char*)RES_STRING(CL_LANGUAGE_MATCH_56)) == 0) {
			targetmap = new char[9];
			targetmap = RES_STRING(CL_LANGUAGE_MATCH_56);
		} else if (strcmp((const char*)map, (const char*)RES_STRING(CL_LANGUAGE_MATCH_57)) == 0) {
			targetmap = new char[9];
			targetmap = RES_STRING(CL_LANGUAGE_MATCH_57);
		} else if (strcmp((const char*)map, (const char*)RES_STRING(CL_LANGUAGE_MATCH_58)) == 0) {
			targetmap = new char[9];
			targetmap = RES_STRING(CL_LANGUAGE_MATCH_58);
		} else if (strcmp((const char*)map, (const char*)RES_STRING(CL_UIMISLOGFORM_CPP_00001)) == 0) {
			targetmap = new char[9];
			targetmap = RES_STRING(CL_UIMISLOGFORM_CPP_00001);
		} else {
			int nTotalIndex = NPCHelper::I()->GetLastID() + 1;
			bool bfindnpc = false;
			for (int i = 0; i < nTotalIndex; ++i) {
				NPCData* p = GetNPCDataInfo(i);

				if (p == nullptr)
					return;

				const char* npc = p->szName;

				if (strcmp(npc, map) == 0) {
					if (strcmp(p->szName, "jialebi") == 0) {
						targetmap = RES_STRING(CL_UIMISLOGFORM_CPP_00002);
					} else {
						targetmap = p->szMapName;
					}
					bfindnpc = true;
					break;
				}
			}

			if (!bfindnpc)
				return;
		}

		auto* pScene = dynamic_cast<CWorldScene*>(CGameApp::GetCurScene());
		if (!pScene)
			return;
		const char* curmap = pScene->GetCurMapInfo()->szName;

		if (pScene->GetMainCha()->IsBoat()) {
			g_stUIBox.ShowMsgBox(nullptr, RES_STRING(CL_UIMISLOGFORM_CPP_00003));
			return;
		}

		if (targetmap && strcmp((const char*)targetmap, curmap) != 0) {
			g_stUIBox.ShowMsgBox(nullptr, RES_STRING(CL_UIMISLOGFORM_CPP_00004));
			return;
		}

		int cx = (int)pScene->GetMainCha()->GetPos().x;
		int cy = (int)pScene->GetMainCha()->GetPos().y;

		//check x,y is valid or not
		int tx = atoi(x);
		int ty = atoi(y);
		if (tx < 0 || tx > 4096)
			return;

		if (ty < 0 || ty > 4096)
			return;

		//if(!g_cFindPathEx.HaveTarget())
		//{
		g_cFindPathEx.Reset();
		g_cFindPathEx.ClearDestDirection();

		g_cFindPathEx.SetDestDirection(cx, cy, tx, ty);
		g_cFindPathEx.SetTarget(cx, cy, tx, ty);
		//}
		D3DXVECTOR3 target((float)tx, (float)ty, 0);
		CNavigationBar::g_cNaviBar.SetTarget((char*)"", target);
		CNavigationBar::g_cNaviBar.Show(true);
	}
}
