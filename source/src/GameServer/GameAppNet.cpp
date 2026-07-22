#include "Stdafx.h"
#include "GameApp.h"
#include "GameAppNet.h"
#include "SubMap.h"
#include "MapEntry.h"
#include "CharTrade.h"
#include "GameDB.h"
#include "Guild.h"

#include "lua_gamectrl.h" //ADDED: better lua error handling (deguix)
extern std::string g_strLogName;

//	2008-8-20	yangyinyu	add	begin!

//	进出地图(上下线)。
void lgtool_printCharacter(const char* sType, CCharacter* pCCha) {
	//
	static char sText[2048];
	static char sTextEx[2048];

	//_snprintf(	sText,	2048,	"%s%s;%d;%d;%d;%d;%d;%d;%d;%d;",
	//	sType,							//	标题。
	//	pCCha->GetName(),				//	人物名字。
	//	pCCha->GetID(),					//	人物ID。
	//	pCCha->GetLevel(),				//	人物等级。
	//	pCCha->getAttr(	ATTR_STR	),	//	人物的力量属性。
	//	pCCha->getAttr(	ATTR_DEX	),	//	人物的敏捷属性。
	//	pCCha->getAttr(	ATTR_AGI	),	//
	//	pCCha->getAttr(	ATTR_CON	),	//
	//	pCCha->getAttr(	ATTR_STA	),	//
	//	pCCha->getAttr( ATTR_AP	)		//	显示剩余属性点。
	//	);
	_snprintf_s(sText, sizeof(sText), _TRUNCATE, "%s%s;%d;%d;%d;%d;%d;%d;%d;%d;",
				sType,							//	标题。
				pCCha->GetName(),				//	人物名字。
				pCCha->GetID(),					//	人物ID。
				pCCha->GetLevel(),				//	人物等级。
				(long)pCCha->getAttr(ATTR_STR), //	人物的力量属性。
				(long)pCCha->getAttr(ATTR_DEX), //	人物的敏捷属性。
				(long)pCCha->getAttr(ATTR_AGI), //
				(long)pCCha->getAttr(ATTR_CON), //
				(long)pCCha->getAttr(ATTR_STA), //
				(long)pCCha->getAttr(ATTR_AP)   //	显示剩余属性点。
	);

	//	打印背包道具。
	USHORT sNum = pCCha->GetKitbag()->GetUseGridNum();

	for (int i = 0; i < sNum; i++) {
		SItemGrid* pGrid = pCCha->GetKitbag()->GetGridContByNum(i);
		if (pGrid) {
			CItemRecord* pItem = GetItemRecordInfo(pGrid->sID);
			if (pItem) {
				//_snprintf(	sTextEx,	2048,	"%s(%d),",	pItem->szName,	pGrid->sNum	);
				_snprintf_s(sTextEx, sizeof(sTextEx), _TRUNCATE, "%s(%d),", pItem->szName, pGrid->sNum);
				//strncat(	sText,	sTextEx,	2048	);
				strncat_s(sText, sizeof(sText), sTextEx, _TRUNCATE);
			};
		};
	};
	//strncat(	sText,	";",	2048	);
	strncat_s(sText, sizeof(sText), ";", _TRUNCATE);

	//	打印银行道具。
	if (pCCha->GetPlayer()) {
		CKitbag* bank = pCCha->GetPlayer()->GetBank();

		if (bank) {
			for (int i = 0; i < bank->GetUseGridNum(); i++) {
				SItemGrid* pGrid = bank->GetGridContByNum(i);
				if (pGrid) {
					CItemRecord* pItem = GetItemRecordInfo(pGrid->sID);
					if (pItem) {
						if (pGrid->dwDBID)
							//_snprintf(	sTextEx,	2048,	"%s-%d-%d,",	pItem->szName,	pGrid->sNum,	pGrid->dwDBID	);
							_snprintf_s(sTextEx, sizeof(sTextEx), _TRUNCATE, "%s-%d-%d,", pItem->szName, pGrid->sNum, (long)pGrid->dwDBID);
						else
							//_snprintf(	sTextEx,	2048,	"%s-%d,",	pItem->szName,	pGrid->sNum	);
							_snprintf_s(sTextEx, sizeof(sTextEx), _TRUNCATE, "%s-%d,", pItem->szName, pGrid->sNum);
						//strncat(	sText,	sTextEx,	2048	);
						strncat_s(sText, sizeof(sText), sTextEx, _TRUNCATE);
					};
				};
			};
		};
	};

	//	结束时打印换行。
	//strncat(	sText,	"\n",	2048	);
	strncat_s(sText, sizeof(sText), "\n", _TRUNCATE);

	//
	LG("query_cha", sText);
};

//	打印船只的道具( 进出海，建造，销毁 )。
void lgtool_printBoat(const char* sType, CCharacter* pCCha) {
	//	取该人物当前的船只。
	CCharacter* pBoat = pCCha->GetBoat();

	if (!pBoat) {
		return;
	};

	//	打印基本信息。
	static char sText[2048];
	static char sTextEx[2048];

	//_snprintf(	sText,	2048,	"%s%s;%s;%d;",
	//	sType,				//	标题。
	//	pCCha->GetName(),	//	人物名字。
	//	pBoat->GetName(),	//	船名字。
	//	pBoat->GetLevel()	//	船等级。
	//	);
	_snprintf_s(sText, sizeof(sText), _TRUNCATE, "%s%s;%s;%d;",
				sType,			  //	标题。
				pCCha->GetName(), //	人物名字。
				pBoat->GetName(), //	船名字。
				pBoat->GetLevel() //	船等级。
	);

	//	打印所有道具。
	USHORT sNum = pBoat->GetKitbag()->GetUseGridNum();

	for (int i = 0; i < sNum; i++) {
		SItemGrid* pGrid = pBoat->GetKitbag()->GetGridContByNum(i);
		if (pGrid) {
			CItemRecord* pItem = GetItemRecordInfo(pGrid->sID);
			if (pItem) {
				//_snprintf(	sTextEx,	2048,	"%s(%d),",
				//	pItem->szName,		//	道具名字。
				//	pGrid->sNum			//	道具数量。
				//	);
				_snprintf_s(sTextEx, sizeof(sTextEx), _TRUNCATE, "%s(%d),",
							pItem->szName, //	道具名字。
							pGrid->sNum	//	道具数量。
				);

				//strncat(	sText,	sTextEx,	2048	);
				strncat_s(sText, sizeof(sText), sTextEx, _TRUNCATE);
			};
		};
	};

	//strncat(	sText,	"\n",	2048	);
	strncat_s(sText, sizeof(sText), "\n", _TRUNCATE);

	//	写LOG。
	LG("query_boat", sText);
};
//	2008-8-20	yangyinyu	add	end!

// 计时退出切换
//#define CHAEXIT_ONTIME

//  获取网络消息的唯一入口
void CGameApp::ProcessNetMsg(int nMsgType, GateServer* pGate, RPACKET pkt) {
	try {
		switch (nMsgType) {
		case NETMSG_GATE_CONNECTED: { // 连接上Gate
			LG("Connect", "Exec OnGateConnected()\n");
			OnGateConnected(pGate, pkt);
			break;
		}

		case NETMSG_GATE_DISCONNECT: { // 与Gate断开连接
			LG("Connect", "Exec OnGateDisconnect()\n");
			OnGateDisconnect(pGate, pkt);
			break;
		}

		case NETMSG_PACKET: { // 收到消息包
			ProcessPacket(pGate, pkt);
			break;
		}
		}
	}
	T_E
}

void CGameApp::ProcessInfoMsg(pNetMessage msg, short sType, TradeServer* pInfo) {
	try {
		switch (sType) {
		case TradeServer::CMD_FM_CONNECTED: {
			OnInfoConnected(pInfo);
		} break;

		case TradeServer::CMD_FM_DISCONNECTED: {
			OnInfoDisconnected(pInfo);
		} break;

		case TradeServer::CMD_FM_MSG: {
			ProcessMsg(msg, pInfo);
		} break;

		default: {
		} break;
		}
	}
	T_E
}

void CGameApp::OnInfoConnected(TradeServer* pInfo) {
	try {
		//登录TradeServer
		pInfo->Login();
	}
	T_E
}

void CGameApp::OnInfoDisconnected(TradeServer* pInfo) {
	try {
		// 商城系统关闭
		g_StoreSystem.InValid();
		pInfo->InValid();
	}
	T_E
}

void CGameApp::ProcessMsg(pNetMessage msg, TradeServer* pInfo) {
	try {
		if (msg) {
			switch (msg->msgHead.msgID) {
			case INFO_LOGIN: { // 登录TradeServer
				if (msg->msgHead.subID == INFO_SUCCESS) {
					LG("Store_data", "TradeServer Login Success!\n");

					pInfo->SetValid();
					//g_StoreSystem.SetValid();

					//向TradeServer索取商城列表和公告列表
					g_StoreSystem.GetItemList();
					g_StoreSystem.GetAfficheList();
				} else if (msg->msgHead.subID == INFO_FAILED) {
					LG("Store_data", "TradeServer Login Failed!\n");
					pInfo->InValid();
				} else {
					LG("Store_data", "enter TradeServer message data error!\n");
				}
			} break;

			case INFO_REQUEST_ACCOUNT: { // 获取帐户信息
				if (msg->msgHead.subID == INFO_SUCCESS) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]succeed to obtain account information!\n", lOrderID);

					RoleInfo* ChaInfo = (RoleInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptRoleInfo(lOrderID, ChaInfo);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]obtain account information failed!\n", lOrderID);

					g_StoreSystem.CancelRoleInfo(lOrderID);
				} else {
					LG("Store_data", "account information message data error");
				}
			} break;

			case INFO_REQUEST_STORE: { // 获取商城信息
				LG("Store_data", "get store list!\n");
				if (msg->msgHead.subID == INFO_SUCCESS) { // 商城信息反馈
					//商品个数
					const short lComNum = LOWORD(msg->msgHead.msgExtend);
					//分类个数
					const short lClassNum = HIWORD(msg->msgHead.msgExtend);
					//设置分类列表
					g_StoreSystem.SetItemClass((ClassInfo*)(msg->msgBody), lClassNum);
					//设置商品列表
					g_StoreSystem.SetItemList((StoreStruct*)((char*)msg->msgBody + lClassNum * sizeof(ClassInfo)), lComNum);

					g_StoreSystem.SetValid();
				} else if (msg->msgHead.subID == INFO_FAILED) { // 商城信息更新
					//商品个数
					const short lComNum = LOWORD(msg->msgHead.msgExtend);
					//分类个数
					const short lClassNum = HIWORD(msg->msgHead.msgExtend);
					//设置分类列表
					g_StoreSystem.SetItemClass((ClassInfo*)(msg->msgBody), lClassNum);
					//设置商品列表
					g_StoreSystem.SetItemList((StoreStruct*)((char*)msg->msgBody + lClassNum * sizeof(ClassInfo)), lComNum);
				} else {
					LG("Store_data", "store list message data error!\n");
				}
			} break;

			case INFO_REQUEST_AFFICHE: { // 获取公告信息
				LG("Store_data", "get offiche information!\n");
				if (msg->msgHead.subID == INFO_SUCCESS) { // 公告信息反馈
					//公告个数
					const long lAfficheNum = msg->msgHead.msgExtend;
					//设置公告列表
					g_StoreSystem.SetAfficheList((AfficheInfo*)msg->msgBody, lAfficheNum);
				} else if (msg->msgHead.subID == INFO_FAILED) { // 公告信息更新
					//公告个数
					const long lAfficheNum = msg->msgHead.msgExtend;
					//设置公告列表
					g_StoreSystem.SetAfficheList((AfficheInfo*)msg->msgBody, lAfficheNum);
				} else {
					//LG("Store_data", "公告信息报文数据错误!\n");
					LG("Store_data", "offiche information message data error!\n");
				}
			} break;

				// Add by lark.li 20090218 begin
			case INFO_STORE_BUY_RETURN: {
				if (msg->msgHead.subID == INFO_SUCCESS) { // 购买成功
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]succeed to buy return item!\n", lOrderID);

					RoleInfo* ChaInfo = (RoleInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptReturn(lOrderID, ChaInfo);
				} else if (msg->msgHead.subID == INFO_FAILED) { // 购买失败
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]buy item return failed!\n", lOrderID);

					RoleInfo* ChaInfo = (RoleInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptReturn(lOrderID, ChaInfo);
				} else {
					LG("Store_data", "confirm information that buy item return message data error!\n");
				}
			} break;
				// End

			case INFO_STORE_BUY: {						  // 购买道具
				if (msg->msgHead.subID == INFO_SUCCESS) { // 购买成功
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]succeed to buy item!\n", lOrderID);

					RoleInfo* ChaInfo = (RoleInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.Accept(lOrderID, ChaInfo);
				} else if (msg->msgHead.subID == INFO_FAILED) { // 购买失败
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]buy item failed!\n", lOrderID);
					g_StoreSystem.Cancel(lOrderID);
				} else {
					LG("Store_data", "confirm information that buy item message data error!\n");
				}
			} break;

			case INFO_REGISTER_VIP: {
				if (msg->msgHead.subID == INFO_SUCCESS) {
					const long long lOrderID = *(long long*)msg->msgBody;
					//LG("Store_data", "[%I64i]购买VIP成功!\n", lOrderID);
					LG("Store_data", "[%I64i] buy VIP succeed !\n", lOrderID);

					RoleInfo* ChaInfo = (RoleInfo*)((char*)msg->msgBody + sizeof(long long));

					g_StoreSystem.AcceptVIP(lOrderID, ChaInfo, msg->msgHead.msgExtend);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i] buy VIP failed !\n", lOrderID);

					g_StoreSystem.CancelVIP(lOrderID);
				} else {
					LG("Store_data", "buy VIP confirm information message data error !\n");
				}
			} break;

			case INFO_EXCHANGE_MONEY: // 兑换代币
			{
				if (msg->msgHead.subID == INFO_SUCCESS) {
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]change token succeed !\n", lOrderID);

					RoleInfo* ChaInfo = (RoleInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptChange(lOrderID, ChaInfo);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]change token failed!\n", lOrderID);

					g_StoreSystem.CancelChange(lOrderID);
				} else {
					LG("Store_data", "change token confirm information message data error !\n");
				}
			} break;

			case INFO_REQUEST_HISTORY: // 查询交易记录
			{
				if (msg->msgHead.subID == INFO_SUCCESS) {
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]succeed to query trade note!\n", lOrderID);

					HistoryInfo* pRecord = (HistoryInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptRecord(lOrderID, pRecord);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]query trade note failed!\n", lOrderID);

					g_StoreSystem.CancelRecord(lOrderID);
				} else {
					LG("Store_data", "trade note query resoibsuib nessage data error!\n");
				}
			} break;

			case INFO_SND_GM_MAIL: {
				if (msg->msgHead.subID == INFO_SUCCESS) {
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]send GM mail success!\n", lOrderID);

					const long lMailID = msg->msgHead.msgExtend;
					g_StoreSystem.AcceptGMSend(lOrderID, lMailID);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]send GM mail failed!\n", lOrderID);

					g_StoreSystem.CancelGMSend(lOrderID);
				} else {
					LG("Store_data", "send GM mail message data error!\n");
				}
			} break;

			case INFO_RCV_GM_MAIL: {
				if (msg->msgHead.subID == INFO_SUCCESS) {
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]receive GM mail success!\n", lOrderID);

					MailInfo* pMi = (MailInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptGMRecv(lOrderID, pMi);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					const long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]reciveGMmail failed!\n", lOrderID);

					g_StoreSystem.CancelGMRecv(lOrderID);
				} else {
					LG("Store_data", "receive GM mail message data error!\n");
				}
			} break;

			case INFO_EXCEPTION_SERVICE: //拒绝服务
			{
				LG("Store_data", "TradeServer refuse serve!\n");
				g_StoreSystem.InValid();
				pInfo->InValid();
			} break;
//以下2016注释 水晶功能完善不能启用
#ifdef SHUI_JING
			case INFO_REQUEST_ACTINFO: {
				if (msg->msgHead.subID == INFO_SUCCESS) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]succeed to obtain account information!\n", lOrderID);

					RoleInfo* ChaInfo = (RoleInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptActInfo(lOrderID, ChaInfo);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]obtain account information failed!\n", lOrderID);

					g_StoreSystem.CancelActInfo(lOrderID);
				} else {
					LG("Store_data", "account information message data error");
				}
			} break;
			case INFO_CRYSTAL_OP_ADD: {
				if (msg->msgHead.subID == INFO_SUCCESS) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]succeed to obtain account information!\n", lOrderID);
					CrystalInfo* cryInfo = (CrystalInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptCrystalAdd(lOrderID, cryInfo);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]obtain account information failed!\n", lOrderID);
					CrystalInfo* cryInfo = (CrystalInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.CancelCrystalAdd(lOrderID, cryInfo);
				} else {
					LG("Store_data", "AddCrystal information message data error");
				}
			} break;
			case INFO_CRYSTAL_OP_DEL: {
				if (msg->msgHead.subID == INFO_SUCCESS) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]succeed to obtain account information!\n", lOrderID);
					CrystalInfo* cryInfo = (CrystalInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptCrystalDel(lOrderID, cryInfo);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]obtain account information failed!\n", lOrderID);
					CrystalInfo* cryInfo = (CrystalInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.CancelCrystalDel(lOrderID, cryInfo);
				} else {
					LG("Store_data", "DelCrystal information message data error");
				}
			} break;

			case INFO_CRYSTAL_OP_ADD_RETURN: {
				if (msg->msgHead.subID == INFO_SUCCESS) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]succeed to obtain account information!\n", lOrderID);
					CrystalInfo* cryInfo = (CrystalInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptCrystalAddReturn(lOrderID, cryInfo);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]obtain account information failed!\n", lOrderID);
					CrystalInfo* cryInfo = (CrystalInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.CancelCrystalAddReturn(lOrderID, cryInfo);
				} else {
					LG("Store_data", "Addcrystal return  information message data error");
				}
			} break;
			case INFO_CRYSTAL_OP_DEL_RETURN: {
				if (msg->msgHead.subID == INFO_SUCCESS) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]succeed to obtain account information!\n", lOrderID);
					CrystalInfo* cryInfo = (CrystalInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.AcceptCrystalDelReturn(lOrderID, cryInfo);
				} else if (msg->msgHead.subID == INFO_FAILED) {
					long long lOrderID = *(long long*)msg->msgBody;
					LG("Store_data", "[%I64i]obtain account information failed!\n", lOrderID);
					CrystalInfo* cryInfo = (CrystalInfo*)((char*)msg->msgBody + sizeof(long long));
					g_StoreSystem.CancelCrystalDelReturn(lOrderID, cryInfo);
				} else {
					LG("Store_data", "Delcrystal return information message data error");
				}
			} break;
#endif
			default: {
				LG("Store_data", "get unknown information type!\n");
			} break;
			}

			FreeNetMessage(msg);
		}
	}
	T_E
}

// 与Gate连接上的处理函数
void CGameApp::OnGateConnected(GateServer* pGate, RPACKET pkt) {
	try {
		// 在GateServer注册本GameServer
		WPACKET wpk = GETWPACKET();
		WRITE_CMD(wpk, CMD_MT_LOGIN);
		WRITE_STRING(wpk, GETGMSVRNAME());
		WRITE_STRING(wpk, g_pGameApp->m_strMapNameList.c_str());

		LG("Connect", "[%s]\n", g_pGameApp->m_strMapNameList.c_str());

		pGate->SendData(wpk);
	}
	T_E
}

// 与Gate断开连接的处理函数
void CGameApp::OnGateDisconnect(GateServer* pGate, RPACKET pkt) {
	try {
		bool ret = VALIDRPACKET(pkt);
		if (!ret)
			return; // 无效的packet

		auto* tmp = ToPointer<GatePlayer>(READ_ADDRESS(pkt));

		while (tmp != nullptr) {
			auto player = static_cast<CPlayer*>(tmp);
			if (player->IsValid()) {
				GoOutGame(player, true);
				tmp->OnLogoff();
			}
			tmp = tmp->Next;
		}

		pGate->Invalid();
	}
	T_E
}

// 处理网络消息包
void CGameApp::ProcessPacket(GateServer* pGate, RPACKET pkt) {
	try {
		const unsigned short cmd = READ_CMD(pkt);
		// 测试性能用代码，发布不要
		MONITOR_VALUE(MESSAGE_NAME(cmd))

		//DWORD dwLastTick = GetTickCount();

		//GameServerLG(g_strLogName.c_str(), "CGameApp::ProcessPacket (cmd = %d) Begin...\n", cmd);

		//LG("Thread", "ProcessPacket %d\n", ::GetCurrentThreadId());

		switch (cmd) {
		case CMD_TM_LOGIN_ACK: {
			const short sErrCode = READ_SHORT(pkt);
			if (sErrCode != 0) {
				LG("GameLogin", "enter GateServer: %s:%d failed [%s], register map[%s]\n",
				   pGate->GetIP().c_str(), pGate->GetPort(), g_GameGateConnError(sErrCode),
				   g_pGameApp->m_strMapNameList.c_str());

				DISCONNECT(pGate->GetDataSock());
				break;
			}

			pGate->GetName() = READ_STRING(pkt);
			if (pGate->GetName().empty()) {
				LG("GameLogin", "entry GateServer: [%s:%d]success but do not get his name，so disconnection and entry again\n",
				   pGate->GetName().c_str(), pGate->GetIP().c_str(), pGate->GetPort(),
				   g_pGameApp->m_strMapNameList.c_str());

				DISCONNECT(pGate->GetDataSock());
				break;
			}

			LG("GameLogin", "entry GateServer: %s [%s:%d]success [MapName:%s]\n",
			   pGate->GetName().c_str(), pGate->GetIP().c_str(), pGate->GetPort(),
			   g_pGameApp->m_strMapNameList.c_str());
		} break;
			//	// Add by lark.li 20080921 begin
			//case CMD_TM_DELETEMAP:
			//	{
			//		int reason = READ_LONG(pkt);

			//		LG("GameLogin", "Gate %s Ip %s %d deleted\r\n", pGate->GetName(), pGate->GetIP(), reason);

			//		pGate->Invalid();

			//		break;
			//	}
			//	//End

		case CMD_TM_ENTERMAP: {
			//LG("XXX", ",1\n");

			const unsigned long l_actid = READ_LONG(pkt);
			const char* pszPassword = READ_STRING(pkt);
			if (!pszPassword) {
				break;
			}
			const unsigned long l_dbid = READ_LONG(pkt);
			const unsigned long l_worldid = READ_LONG(pkt);
			const char* l_map = READ_STRING(pkt);
			if (!l_map) {
				break;
			}
			const long lMapCpyNO = READ_LONG(pkt);
			const unsigned long l_x = READ_LONG(pkt);
			const unsigned long l_y = READ_LONG(pkt);
			const char chLogin = READ_CHAR(pkt); // 角色上线(0)
			const short swiner = READ_SHORT_R(pkt);
			const uintptr_t l_gtaddr = READ_ADDRESS_R(pkt);

			//让玩家ulChaDBId进入地图
			LG("enter_map", "start entry map cha_id = %d enter--------------------------\n", l_dbid);
			//EnterServerCall * pCall	= g_gmsvr->m_EnterProc.Get();
			//if(pCall)
			//{
			//	pCall->pGate = pGate;
			//	pCall->l_actid = l_actid;
			//	pCall->pszPassword = pszPassword;
			//	pCall->l_dbid = l_dbid;
			//	pCall->l_worldid = l_worldid;
			//	pCall->l_map = l_map;
			//	pCall->lMapCpyNO = lMapCpyNO;
			//	pCall->l_x=l_x ;
			//	pCall->l_y = l_y;
			//	pCall->chLogin = chLogin;
			//	pCall->swiner = swiner;
			//	pCall->l_gtaddr = l_gtaddr;
			//	g_gmsvr->GetProcessor()->AddTask(pCall);
			//}

			CPlayer* player = CreateGamePlayer(pszPassword, l_dbid, l_worldid, l_map, chLogin == 0 ? 0 : 1);
			if (!player) {
				WPACKET pkret = GETWPACKET();
				WRITE_CMD(pkret, CMD_MC_ENTERMAP);
				WRITE_SHORT(pkret, ERR_MC_ENTER_ERROR);
				WRITE_LONG(pkret, l_dbid);
				WRITE_ADDRESS(pkret, l_gtaddr);
				WRITE_SHORT(pkret, 1);
				pGate->SendData(pkret);
				LG("error", "when create new player(ID = %u),assign memory failed \n", l_dbid);
				return;
			}

			player->SetActLoginID(l_actid);
			player->SetGarnerWiner(swiner);
			player->GetLifeSkillinfo() = "";
			player->SetInLifeSkill(false);

			if (!chLogin) { // 上线（而不是切换地图）
				player->MisLogin();
			}

			//////////////////////////////////////////////////////////////////////////
			// 添加gate server对应的维护信息
			ADDPLAYER(player, pGate, l_gtaddr);
			player->OnLogin();
			//////////////////////////////////////////////////////////////////////////

			CCharacter* pCCha = player->GetMainCha();
			if (pCCha->Cmd_EnterMap(l_map, lMapCpyNO, l_x, l_y, chLogin)) {
				player->MisEnterMap();

				if (chLogin == 0) { // 角色上线
					NoticePlayerLogin(player);
				}
			}

			LG("enter_map", "end up entry map  [%s]================\n\n", pCCha->GetLogName());

			//	2008-8-20	yangyinyu	add	begin!
			//lgtool_printCharacter(	RES_STRING(GM_CHARACTER_CPP_00147),	pCCha	);
			//	2008-8-20	yangyinyu	add	end!

			//change by zcj
			//game_db.SavePlayer(l_player, enumSAVE_TYPE_SWITCH);
			// 因为这个是为了防止复制而加入的，速度提升，只用保存出生地即可
			game_db.OnlySavePosWhenBeSaved(player);

			// Add by lark.li 20090112 begin
			//extern ToMMS* g_ToMMS;
			//g_ToMMS->EnterMap(l_actid, pCCha->m_ID, l_map);
			// End
		} break;
		case CMD_TM_GOOUTMAP: {
			//LG("XXX", ",0\n");

			auto player = ToPointer<CPlayer>(READ_ADDRESS_R(pkt));
			if (!player) {
				LG("error", "CMD_TM_GOOUTMAP\n");
				break;
			}

			try {
				const uintptr_t gateaddr = READ_ADDRESS_R(pkt);
				if (player->GetGateAddr() != gateaddr) {
					LG("error", "DB ID: %u, address not matching,local :%zx, gate:%zx,cmd=%d, validity(%d).\n", player->GetDBChaId(), player->GetGateAddr(), gateaddr, cmd, player->IsValidFlag());
					break;
				}
			} catch (...) {
				LG("error", "===========================from Gate player's address error %p,cmd =%d\n", player, cmd);
				break;
			}
			if (!player->IsValid()) {
				LG("error", "this player already invalid\n");
				break;
			}
			if (player->GetMainCha()->GetPlayer() != player) {
				LG("error", "two player not matching(character name:%s,Gate address [local %p, guest %p]),cmd=%u\n", player->GetMainCha()->GetLogName(), player->GetMainCha()->GetPlayer(), player, cmd);
			}

			LG("enter_map", "start leave map--------\n");
			const char chOffLine = READ_CHAR(pkt); // 角色下线(0)

			LG("enter_map", "Delete Player [%s]\n", player->GetMainCha()->GetLogName());

			//	2008-8-20	yangyinyu	add	begin!
			//lgtool_printCharacter(	RES_STRING(GM_CHARACTER_CPP_00148),	l_player->GetMainCha()	);
			//	2008-8-20	yangyinyu	add	end!

#ifdef OUT_MUL_THREAD
			//GoOutServerCall * pCall	= g_gmsvr->m_GoOutProc.Get();
			//if(pCall)
			//{
			//	pCall->Init(l_player, !chOffLine);
			//	g_gmsvr->GetProcessor()->AddTask(pCall);
			//}
#else
			GoOutGame(player, !chOffLine);
#endif
			LG("enter_map", "end and leave the map========\n\n");

			// Add by lark.li 20090112 begin
			//extern ToMMS* g_ToMMS;
			//g_ToMMS->LeaveMap(l_player->GetActLoginID(), l_player->GetID());
			// End

			//LG("OutMap", "%s离开地图\n", szLogName);
		} break;
		case CMD_PM_SAY2ALL: {
			const unsigned long ulChaID = pkt.ReadLong();
			const char* szContent = pkt.ReadString();
			const long lChatMoney = pkt.ReadLong();

			CPlayer* pPlayer = g_pGameApp->GetPlayerByDBID(ulChaID);
			if (pPlayer) {
				CCharacter* pCha = pPlayer->GetMainCha();
				if (!pCha->HasMoney(lChatMoney)) {
					pCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00007));

					WPacket wpk = GETWPACKET();
					wpk.WriteCmd(CMD_MP_SAY2ALL);
					wpk.WriteChar(0);
					pCha->ReflectINFof(pCha, wpk);
					break;
				}

				pCha->setAttr(ATTR_GD, (pCha->getAttr(ATTR_GD) - lChatMoney));
				pCha->SynAttr(enumATTRSYN_TASK);
				pCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00006), lChatMoney);

				WPacket wpk = GETWPACKET();
				wpk.WriteCmd(CMD_MP_SAY2ALL);
				wpk.WriteChar(1);
				wpk.WriteString(pCha->GetName());
				wpk.WriteString(szContent);
				pCha->ReflectINFof(pCha, wpk);
			}
		} break;
		case CMD_PM_SAY2TRADE: {
			const unsigned long ulChaID = pkt.ReadLong();
			const char* szContent = pkt.ReadString();
			long lChatMoney = pkt.ReadLong();

			CPlayer* pPlayer = g_pGameApp->GetPlayerByDBID(ulChaID);
			if (pPlayer) {
				CCharacter* pCha = pPlayer->GetMainCha();
				if (!pCha->HasMoney(lChatMoney)) {
					pCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00005));

					WPacket wpk = GETWPACKET();
					wpk.WriteCmd(CMD_MP_SAY2TRADE);
					wpk.WriteChar(0);
					pCha->ReflectINFof(pCha, wpk);
					break;
				}

				pCha->setAttr(ATTR_GD, (pCha->getAttr(ATTR_GD) - lChatMoney));
				pCha->SynAttr(enumATTRSYN_TASK);
				pCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00004), lChatMoney);

				WPacket wpk = GETWPACKET();

				wpk.WriteCmd(CMD_MP_SAY2TRADE);
				wpk.WriteChar(1);
				wpk.WriteString(pCha->GetName());
				wpk.WriteString(szContent);
				pCha->ReflectINFof(pCha, wpk);
			}
		} break;
		case CMD_PM_TEAM: { // GroupServer通知组队信息变化
			ProcessTeamMsg(pGate, pkt);
		} break;
		case CMD_PM_GUILDINFO: { // GroupServer通知公会信息变化
			ProcessGuildMsg(pGate, pkt);
		} break;
		case CMD_PM_GUILD_CHALLMONEY: {
			ProcessGuildChallMoney(pGate, pkt);
		} break;
		case CMD_PM_GUILD_CHALL_PRIZEMONEY: {
			//ProcessGuildChallPrizeMoney( pGate, pkt );

			WPACKET WtPk = GETWPACKET();
			WRITE_CMD(WtPk, CMD_MM_GUILD_CHALL_PRIZEMONEY);
			WRITE_LONG(WtPk, 0);
			WRITE_LONG(WtPk, READ_LONG(pkt));
			WRITE_LONG(WtPk, READ_LONG(pkt));
			WRITE_SHORT(WtPk, 0);
			WRITE_LONG(WtPk, 0);
			WRITE_LONG(WtPk, 0);
			pGate->SendData(WtPk);
		} break;
		case CMD_PM_GUILD_INVITE: {
			const auto invited_chaid = pkt.ReadLong();
			const auto inviter_chaid = pkt.ReadLong();
			CPlayer* invited = g_pGameApp->GetPlayerByDBID(invited_chaid);
			CPlayer* inviter = g_pGameApp->GetPlayerByDBID(inviter_chaid);
			if (invited && inviter) {
				CCharacter* invited_cha = invited->GetMainCha();
				CCharacter* inviter_cha = inviter->GetMainCha();
				if (invited_cha && inviter_cha) {
					const unsigned long guild_id = inviter_cha->GetGuildID();
					const char guild_type = game_db.GetGuildTypeByID(inviter_cha, guild_id);
					invited_cha->SetGuildType(guild_type);
					invited_cha->GetPlayer()->m_GuildState.SetBit(emGuildReplaceOldTry);
					invited_cha->GetPlayer()->m_lTempGuildID = guild_id;
					strncpy_s(invited_cha->GetPlayer()->m_szTempGuildName, sizeof(invited_cha->GetPlayer()->m_szTempGuildName), inviter_cha->GetPlayer()->m_szGuildName, _TRUNCATE);
					Guild::cmd_GuildCancelTryFor(invited_cha);
					//Guild::cmd_GuildTryFor( pCha,guildid );
					Guild::cmd_GuildTryForComfirm(invited_cha, 1);
					Guild::cmd_GuildApprove(inviter_cha, invited_chaid);
				}
			}
		} break;
		case CMD_TM_MAPENTRY: {
			ProcessDynMapEntry(pGate, pkt);
		} break;
		case CMD_TM_MAP_ADMIN: {
			ProcessMapAdmin(pGate, pkt);
		} break;
		case CMD_TM_MAPENTRY_NOMAP: {
		} break;
		case CMD_PM_GARNER2_UPDATE: {
			ProcessGarner2Update(pkt);
		} break;
		case CMD_TM_STATE: {
			std::string str;

			const uintptr_t addr = pkt.ReverseReadAddress();
			const unsigned long dbID = pkt.ReverseReadLong();

			char buffer[256];
			CFormatParameter param(13);
			param.setLong(0, this->m_pCPlySpace->GetMaxHoldPlyNum());
			param.setLong(1, this->m_pCEntSpace->GetMaxHoldChaNum());
			param.setLong(2, this->m_pCEntSpace->GetMaxHoldItemNum());
			param.setLong(3, this->m_pCEntSpace->GetMaxHoldTNpcNum());

			param.setLong(4, this->m_pCPlySpace->GetAllocPlyNum());
			param.setLong(5, this->m_pCEntSpace->GetAllocChaNum());
			param.setLong(6, this->m_pCEntSpace->GetAllocItemNum());
			param.setLong(7, this->m_pCEntSpace->GetAllocTNpcNum());

			param.setLong(8, this->m_pCPlySpace->GetHoldPlyNum());
			param.setLong(9, this->m_pCEntSpace->GetMaxHoldChaNum());
			param.setLong(10, this->m_pCEntSpace->GetMaxHoldItemNum());
			param.setLong(11, this->m_pCEntSpace->GetMaxHoldTNpcNum());

			param.setString(12, g_Config.m_szName);

			CResourceBundleManage::Instance()->FormatString("{12} (Ply,Cha,Item,TNpc)[Max,Alloc,Hold] [{0} {1} {2} {3}] [{4} {5} {6} {7}] [{8} {9} {10} {11}]", param, buffer);
			str.append(buffer);

			WPACKET wpk = GETWPACKET();
			wpk.WriteCmd(CMD_MT_STATE);
			wpk.WriteString(str.c_str());
			wpk.WriteLong(dbID);
			wpk.WriteAddress(addr);
			pGate->SendData(wpk);
		} break;
		case CMD_PM_EXPSCALE: {
			//  防沉迷
			const unsigned long ulChaID = pkt.ReadLong();
			const unsigned long ulTime = pkt.ReadLong();

			CPlayer* pPlayer = g_pGameApp->GetPlayerByDBID(ulChaID);
			if (pPlayer) {
				CCharacter* pCha = pPlayer->GetMainCha();
				if (pCha->IsScaleFlag()) {
					break;
				}
				pCha->SetNoticeState(ulTime);
			}
		} break;

		default: {
			if (cmd / 500 == CMD_MM_BASE / 500) {
				ProcessInterGameMsg(cmd, pGate, pkt);
			} else {
				auto player = ToPointer<CPlayer>(READ_ADDRESS_R(pkt));
				if (cmd / 500 == CMD_PM_BASE / 500 && !player) {
					ProcessGroupBroadcast(cmd, pGate, pkt);
				} else {
					if (!player)
						break;
					try {
						uintptr_t l_gateaddr = READ_ADDRESS_R(pkt);
						if (player->GetGateAddr() != l_gateaddr) {
							LG("error", "DB ID:%u, address not matching,local :%zu, gate:%zu,cmd=%d, validity (%d)\n", player->GetDBChaId(), player->GetGateAddr(),
							   l_gateaddr, cmd, player->IsValidFlag());
							break;
						}
					} catch (...) {
						LG("error", "===========================Player address error that come from Gate %p,cmd =%d\n", player, cmd);
						break;
					}
					if (!player->IsValid()) {
						break;
					}
					if (player->GetMainCha()->GetPlayer() != player) {
						LG("error", "two player not matching(character name:%s,Gate address [local %p, guest %p]),cmd=%u\n", player->GetMainCha()->GetLogName(), player->GetMainCha()->GetPlayer(), player, cmd);
					}

					CCharacter* pCCha = player->GetCtrlCha();
					if (!pCCha) {
						break;
					}
					if (g_pGameApp->IsValidEntity(pCCha->GetID(), pCCha->GetHandle())) {
						g_pNoticeChar = pCCha;

						g_ulCurID = pCCha->GetID();
						g_lCurHandle = pCCha->GetHandle();

						pCCha->ProcessPacket(cmd, pkt);

						g_ulCurID = defINVALID_CHA_ID;
						g_lCurHandle = defINVALID_CHA_HANDLE;

						g_pNoticeChar = nullptr;
					} else {
						LG("error", "when receive CMD_CM_BASE message[%d], find character pCCha is null\n", cmd);
					}
				}
			}
		} break;
		}

		//DWORD dwTick = GetTickCount() - dwLastTick;
		//if(dwTick > 10)
		//	LG("TickCount", ",%d,%ld\n", cmd, dwTick);

		//GameServerLG(g_strLogName.c_str(), "CGameApp::ProcessPacket (cmd = %d) End!\n", cmd);
	}
	T_E
}

// 处理公会投标退钱
void CGameApp::ProcessGuildChallMoney(GateServer* pGate, RPACKET pkt) {
	try {
		const unsigned long dwChaDBID = pkt.ReadLong();
		const unsigned long dwMoney = pkt.ReadLong();
		const char* pszGuild1 = pkt.ReadString();
		if (!pszGuild1) {
			return;
		}
		const char* pszGuild2 = READ_STRING(pkt);
		if (!pszGuild2) {
			return;
		}

		//	2007-8-4	yangyinyu	change	begin!	//	任意时间收到这个消息，将导致退钱，而且
		CPlayer* pPlayer = GetPlayerByDBID(dwChaDBID);
		if (pPlayer) {
			CCharacter* pCha = pPlayer->GetMainCha();
			if (pCha) {
				pCha->AddMoney(RES_STRING(GM_GAMEAPPNET_CPP_00017), dwMoney);
				char szData[128];
				CFormatParameter param(3);
				param.setString(0, pszGuild1);
				param.setString(1, pszGuild2);
				param.setDouble(2, dwMoney);
				RES_FORMAT_STRING(GM_GAMEAPPNET_CPP_00009, param, szData);
				pCha->SystemNotice(szData);
				LG("challenge consortia result", "《%s》bidder and consortia《%s》battle was consortia《%s》replace,your consortia gold (%u)had back to you!\n", pCha->GetGuildName(), pszGuild1, pszGuild2, dwMoney);
			}
		} else {
			LG("challenge consortia result", "not find deacon information finger,cannot back gold DBID[%u],how much money[%u].\n", dwChaDBID, dwMoney);
		}
	}
	T_E
}

void CGameApp::ProcessGuildChallPrizeMoney(GateServer* pGate, RPACKET pkt) {
	try {
		const unsigned long dwChaDBID = pkt.ReadLong();
		const unsigned long dwMoney = pkt.ReadLong();
		CPlayer* pPlayer = GetPlayerByDBID(dwChaDBID);
		if (pPlayer) {
			CCharacter* pCha = pPlayer->GetMainCha();
			pCha->AddMoney(RES_STRING(GM_CHARACTER_CPP_00012), dwMoney);
			char szData[128];
			CFormatParameter param(2);
			param.setString(0, pCha->GetGuildName());
			param.setDouble(1, dwMoney);
			RES_FORMAT_STRING(GM_GAMEAPPNET_CPP_00010, param, szData);
			pCha->SystemNotice(szData);
			LG("challenge consortia result", "congratulate you have leading the consortia《%s》get win in consortia battle!gain bounty(%u)!", pCha->GetGuildName(), dwMoney);
		} else {
			LG("challenge consortia result", "cannot find deacon information finger,cannot hortation DBID[%u],how much money[%u]", dwChaDBID, dwMoney);
		}
	}
	T_E
}

// 处理公会信息
void CGameApp::ProcessGuildMsg(GateServer* pGate, RPACKET pkt) {
	try {
		const unsigned long dwChaDBID = pkt.ReadLong();
		CPlayer* pPlayer = GetPlayerByDBID(dwChaDBID);
		if (pPlayer) {
			CCharacter* pCha = pPlayer->GetCtrlCha();
			BYTE byType = READ_CHAR(pkt);
			const unsigned long dwGuildID = pkt.ReadLong();
			const unsigned long dwLeaderID = pkt.ReadLong();
			const char* pszGuildName = READ_STRING(pkt);
			const char* pszGuildMotto = READ_STRING(pkt);
			pCha->SetGuildType(byType);

			if (!pszGuildName) {
				pCha->SetGuildName("");
			} else {
				pCha->SetGuildName(pszGuildName);
			}

			if (!pszGuildMotto) {
				pCha->SetGuildMotto(pszGuildMotto);
			} else {
				pCha->SetGuildMotto("");
			}

			pCha->SyncGuildInfo();
		}
	}
	T_E
}

// 处理组队有关消息
void CGameApp::ProcessTeamMsg(GateServer* pGate, RPACKET pkt) {
	try {
		//LG("team", "开始处理组队消息\n");

		const char cTeamMsgType = pkt.ReadChar();

		switch (cTeamMsgType) {
		case TEAM_MSG_ADD: { /*LG("team", "收到组队 [新加队员] 消息\n");*/
			break;
		}
		case TEAM_MSG_LEAVE: { /*LG("team", "收到组队 [队员离队] 消息\n");*/
			break;
		}
		case TEAM_MSG_UPDATE: { /*LG("team", "收到组队 [队员刷新] 消息\n");*/
			break;
		}
		default:
			//LG("team", "收到无效的Team消息 [%d]\n", cTeamMsgType);
			return;
		}

		const char cMemberCnt = pkt.ReadChar();
		if (cMemberCnt > MAX_TEAM_MEMBER) {
			LG("team", "team count [%d] error\n", cMemberCnt);
			return;
		}
		//LG("team", "当前队员总数量[%d]\n", cMemberCnt); // 如果收到数量 < 2则表示GroupServer的队伍信息出问题了.

		uplayer Team[MAX_TEAM_MEMBER];
		CPlayer* PlayerList[MAX_TEAM_MEMBER];
		bool CanSeenO[MAX_TEAM_MEMBER][2];
		bool CanSeenN[MAX_TEAM_MEMBER][2];

		// 读取包信息并查找出所有的Player
		for (char i = 0; i < cMemberCnt; i++) {
			const char* pszGateName = pkt.ReadString();
			if (!pszGateName) {
				return;
			}

			const uintptr_t dwGateAddr = pkt.ReadAddress();
			const unsigned long dwChaDBID = pkt.ReadLong();
			Team[i].Init(pszGateName, dwGateAddr, dwChaDBID);
			if (!Team[i].pGate) {
				LG("team", "GameServer can't find matched Gate:%s, addr = 0x%zX, chaid = %d.\n", pszGateName, dwGateAddr, dwChaDBID);
				BEGINGETGATE();
				while (GateServer* pGateServer = GETNEXTGATE()) {
					LG("team", "\t%s\n", pGateServer->GetName().c_str());
				}
			}

			PlayerList[i] = GetPlayerByDBID(dwChaDBID);

			//LG("team", "队员: %s, %d %d 来自Gate [%s]\n", PlayerList[i]!=NULL ? PlayerList[i]->GetCtrlCha()->GetLogName():"(此人不在本Server!)", dwChaDBID, dwGateAddr, pszGateName);
		}

		//RefreshTeamEyeshot(PlayerList, cMemberCnt, cTeamMsgType);
		CheckSeeWithTeamChange(CanSeenO, PlayerList, cMemberCnt);
		//if(PlayerList[0]==NULL)
		//{
		//	LG("team", "队长不在本game server上了\n");
		//}

		int nLeftMember = cMemberCnt;
		if (cTeamMsgType == TEAM_MSG_LEAVE) { // 队员离队或下线
			nLeftMember -= 1;
			CPlayer* pLeave = PlayerList[cMemberCnt - 1];
			if (pLeave) {
				pLeave->LeaveTeam();
			}
		}
		// 彼此之间添加队友, 如果cMember此时为1, 则等价于解除了队伍
		for (int i = 0; i < nLeftMember; i++) {
			if (!PlayerList[i]) {
				continue;
			}

			PlayerList[i]->ClearTeamMember();
			for (int j = 0; j < nLeftMember; j++) {
				if (i == j) {
					continue;
				}
				PlayerList[i]->AddTeamMember(&Team[j]);
			}
			if (nLeftMember != 1) {
				PlayerList[i]->setTeamLeaderID(Team[0].m_dwDBChaId);
				PlayerList[i]->NoticeTeamLeaderID();
			}
		}

		CheckSeeWithTeamChange(CanSeenN, PlayerList, cMemberCnt);
		RefreshTeamEyeshot(CanSeenO, CanSeenN, PlayerList, cMemberCnt, cTeamMsgType);

		//add by jilinlee 2007/07/11

		for (char i = 0; i < cMemberCnt; i++) {
			if (i < 5) {
				if (PlayerList[i]) {
					CCharacter* pCtrlCha = PlayerList[i]->GetCtrlCha();
					if (pCtrlCha) {
						SubMap* pSubMap = pCtrlCha->GetSubMap();
						if (pSubMap && pSubMap->GetMapRes()) {
							if (!(pSubMap->GetMapRes()->CanTeam())) {
								pCtrlCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00011));

								LG("team", "MoveCity()\n");
								//Modify by lark.li 20090114 begin
								//pCtrlCha ->MoveCity("garner");
								// 打回到出生地
								pCtrlCha->MoveCity("");
								// End
							}
						}
					}
				}
			}
		}

		//if(nLeftMember==1) LG("team", "nLeftMember==1, 队伍解散!\n");

		//LG("team", "结束处理组队消息\n\n");
	}
	T_E
}

// 确定玩家间是否可见
void CGameApp::CheckSeeWithTeamChange(bool CanSeen[][2], CPlayer** pCPlayerList, char chMemberCnt) {
	try {
		if (chMemberCnt <= 1) {
			return;
		}

		CPlayer* pCProcPly = pCPlayerList[chMemberCnt - 1];
		if (!pCProcPly) {
			return;
		}

		CCharacter *pCProcCha = pCProcPly->GetCtrlCha(), *pCCurCha;
		for (char i = 0; i < chMemberCnt - 1; i++) {
			CPlayer* pCCurPly = pCPlayerList[i];
			if (!pCCurPly) {
				continue;
			}
			pCCurCha = pCCurPly->GetCtrlCha();
			if (pCProcCha->IsInEyeshot(pCCurCha)) {
				pCProcCha->CanSeen(pCCurCha) ? CanSeen[i][0] = true : CanSeen[i][0] = false;
				pCCurCha->CanSeen(pCProcCha) ? CanSeen[i][1] = true : CanSeen[i][1] = false;
			}
		}
	}
	T_E
}

// 根据当前是否可见与之前的进行比较来刷新视野
void CGameApp::RefreshTeamEyeshot(bool CanSeenOld[][2], bool CanSeenNew[][2], CPlayer** pCPlayerList, char chMemberCnt, char chRefType) {
	try {
		if (chMemberCnt <= 1) {
			return;
		}

		CPlayer* pCProcPly = pCPlayerList[chMemberCnt - 1];
		if (!pCProcPly) {
			return;
		}

		CCharacter *pCProcCha = pCProcPly->GetCtrlCha(), *pCCurCha;
		for (char i = 0; i < chMemberCnt - 1; i++) {
			CPlayer* pCCurPly = pCPlayerList[i];
			if (!pCCurPly) {
				continue;
			}
			pCCurCha = pCCurPly->GetCtrlCha();
			if (pCProcCha->IsInEyeshot(pCCurCha)) {
				if (chRefType == TEAM_MSG_ADD) {
					if (!CanSeenOld[i][0] && CanSeenNew[i][0]) {
						pCCurCha->BeginSee(pCProcCha);
					}
					if (!CanSeenOld[i][1] && CanSeenNew[i][1]) {
						pCProcCha->BeginSee(pCCurCha);
					}
				} else if (chRefType == TEAM_MSG_LEAVE) {
					if (CanSeenOld[i][0] && !CanSeenNew[i][0]) {
						pCCurCha->EndSee(pCProcCha);
					}
					if (CanSeenOld[i][1] && !CanSeenNew[i][1]) {
						pCProcCha->EndSee(pCCurCha);
					}
				}
			}
		}
	}
	T_E
}

// 根据当前是否可见来刷新视野
void CGameApp::RefreshTeamEyeshot(CPlayer** pCPlayerList, char chMemberCnt, char chRefType) {
	try {
		if (chMemberCnt <= 1) {
			return;
		}

		CPlayer* pCProcPly = pCPlayerList[chMemberCnt - 1];
		if (!pCProcPly) {
			return;
		}

		CCharacter *pCProcCha = pCProcPly->GetCtrlCha(), *pCCurCha;
		bool bCurChaHide;
		bool bProcChaHide = pCProcCha->IsHide();
		for (char i = 0; i < chMemberCnt - 1; i++) {
			CPlayer* pCCurPly = pCPlayerList[i];
			if (!pCCurPly) {
				continue;
			}
			pCCurCha = pCCurPly->GetCtrlCha();
			bCurChaHide = pCCurCha->IsHide();
			if (bProcChaHide || bCurChaHide) { // 存在隐身
				if (pCProcCha->IsInEyeshot(pCCurCha)) {
					if (chRefType == TEAM_MSG_ADD) {
						if (bProcChaHide) {
							pCCurCha->BeginSee(pCProcCha);
						}
						if (bCurChaHide) {
							pCProcCha->BeginSee(pCCurCha);
						}
					} else if (chRefType == TEAM_MSG_LEAVE) {
						if (bProcChaHide) {
							pCCurCha->EndSee(pCProcCha);
						}
						if (bCurChaHide) {
							pCProcCha->EndSee(pCCurCha);
						}
					}
				}
			}
		}
	}
	T_E
}

BOOL CGameApp::AddVolunteer(CCharacter* pCha) {
	try {
		if (pCha->IsVolunteer()) {
			return false;
		}
		pCha->SetVolunteer(true);

		SVolunteer volNode;
		volNode.lJob = (long)pCha->getAttr(ATTR_JOB);
		volNode.lLevel = pCha->GetLevel();
		volNode.ulID = pCha->GetID();
		strncpy_s(volNode.szMapName, sizeof(volNode.szMapName), pCha->GetSubMap()->GetName(), _TRUNCATE);
		strncpy_s(volNode.szName, sizeof(volNode.szName), pCha->GetName(), _TRUNCATE);

		m_vecVolunteerList.push_back(volNode);

		return true;
	}
	T_E
}

BOOL CGameApp::DelVolunteer(CCharacter* pCha) {
	try {
		if (!pCha->IsVolunteer()) {
			return false;
		}
		pCha->SetVolunteer(false);

		const char* volunteer_name = pCha->GetName();
		auto it = std::find_if(m_vecVolunteerList.begin(), m_vecVolunteerList.end(), [&volunteer_name](const SVolunteer& vol) {
			return strcmp(vol.szName, volunteer_name) == 0 ? true : false;
		});
		if (it != m_vecVolunteerList.end()) {
			m_vecVolunteerList.erase(it);
			return true;
		}
		return false;
	}
	T_E
}

int CGameApp::GetVolNum() {
	try {
		return (int)m_vecVolunteerList.size();
	}
	T_E
}

SVolunteer* CGameApp::GetVolInfo(int nIndex) {
	try {
		if (nIndex < 0 || nIndex >= (int)m_vecVolunteerList.size()) {
			return nullptr;
		}

		return &m_vecVolunteerList[nIndex];
	}
	T_E
}

SVolunteer* CGameApp::FindVolunteer(const char* szName) {
	try {
		auto it = std::find_if(m_vecVolunteerList.begin(), m_vecVolunteerList.end(), [&szName](const SVolunteer& vol) {
			return strcmp(vol.szName, szName) == 0 ? true : false;
		});
		return it != m_vecVolunteerList.end() ? static_cast<SVolunteer*>(&(*it)) : nullptr;
	}
	T_E
}

void CGameApp::ProcessInterGameMsg(unsigned short usCmd, GateServer* pGate, RPACKET pkt) {
	try {
		const long lSrcID = pkt.ReadLong();
		const short sNum = pkt.ReverseReadShort();
		const uintptr_t GatePlayerAddr = pkt.ReverseReadAddress();
		const long GatePlayerID = pkt.ReverseReadLong();

		switch (usCmd) {
		case CMD_MM_GUILD_MOTTO: {
			const unsigned long& l_gldid = lSrcID;
			const char* pszMotto = pkt.ReadString();
			{ //来自于FindPlayerChaByID

				if (!pszMotto) {
					pszMotto = "";
				}

				BEGINGETGATE();
				while (GateServer* pGateServer = GETNEXTGATE()) {
					if (!BEGINGETPLAYER(pGateServer)) {
						continue;
					}
					int nCount = 0;
					while (auto pCPlayer = static_cast<CPlayer*>(GETNEXTPLAYER(pGateServer))) {
						if (++nCount > GETPLAYERCOUNT(pGateServer)) {
							LG("player list error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "ProcessInterGameMsg::CMD_MM_GUILD_DISBAND");
							break;
						}

						CCharacter* pCha = pCPlayer->GetMainCha();
						if (!pCha) {
							continue;
						}
						if (pCha->GetGuildID() == l_gldid) { // 找到角色
							pCha->SetGuildMotto(pszMotto);
							pCha->SyncGuildInfo();
							pCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00012));
						}
					}
				}
			}
		} break;
		case CMD_MM_GUILD_DISBAND: {
			const unsigned long& l_gldid = lSrcID;
			{ //来自于FindPlayerChaByID
				BEGINGETGATE();
				while (GateServer* pGateServer = GETNEXTGATE()) {
					if (!BEGINGETPLAYER(pGateServer)) {
						continue;
					}
					int nCount = 0;
					while (auto pCPlayer = static_cast<CPlayer*>(GETNEXTPLAYER(pGateServer))) {
						if (++nCount > GETPLAYERCOUNT(pGateServer)) {
							//LG("玩家链表错误", "玩家数目:%u, %s\n", GETPLAYERCOUNT(pGateServer), "ProcessInterGameMsg::CMD_MM_GUILD_DISBAND");
							LG("player list error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "ProcessInterGameMsg::CMD_MM_GUILD_DISBAND");
							break;
						}
						CCharacter* pCha = pCPlayer->GetMainCha();
						if (!pCha) {
							continue;
						}
						if (pCha->GetGuildID() == l_gldid) { // 找到角色
							pCha->m_CChaAttr.ResetChangeFlag();

							pCha->SetGuildID(0);
							pCha->SetGuildState(0);
							pCha->SynAttr(enumATTRSYN_TRADE);

							pCha->SetGuildName("");
							pCha->SetGuildMotto("");
							pCha->SyncGuildInfo();
							pCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00013));
						}
					}
				}
			}
		} break;
		case CMD_MM_GUILD_KICK: {
			const unsigned long& l_chaid = lSrcID;
			CCharacter* pCha = FindMainPlayerChaByID(l_chaid);
			if (pCha) {
				pCha->SetGuildName("");
				const char* l_gldname = pkt.ReadString();
				if (!l_gldname) {
					l_gldname = "";
				}

				pCha->SetGuildID(0);   //设置公会ID
				pCha->SetGuildType(0); //设置公会Type
				pCha->SetGuildState(0);
				pCha->SetGuildName("");
				pCha->SetGuildMotto("");
				pCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00014), l_gldname);
				pCha->SyncGuildInfo();
			}
		} break;
		case CMD_MM_GUILD_APPROVE: {
			const unsigned long& l_chaid = lSrcID;
			CCharacter* pCha = FindMainPlayerChaByID(l_chaid);
			if (pCha) {
				pCha->SetGuildID(pkt.ReadLong()); //设置公会ID
				pCha->SetGuildType(pkt.ReadChar());
				pCha->SetGuildState(0); //设置公会Type
				const char* l_gldname = pkt.ReadString();

				if (!l_gldname) {
					l_gldname = "";
				}

				pCha->SetGuildName(l_gldname);
				const char* l_gldmotto = pkt.ReadString();
				if (!l_gldmotto) {
					l_gldmotto = "";
				}

				pCha->SetGuildMotto(l_gldmotto);
				pCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00015), l_gldname);
				pCha->SyncGuildInfo();
			}
		} break;
		case CMD_MM_GUILD_REJECT: {
			const unsigned long& l_chaid = lSrcID;
			CCharacter* pCha = FindMainPlayerChaByID(l_chaid);
			if (pCha) {
				pCha->SetGuildID(0);
				pCha->SetGuildState(0);
				pCha->SetGuildName("");

				const char* cszMsg = pkt.ReadString();
				if (cszMsg) {
					pCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00016), cszMsg);
				}
			}
		} break;
		case CMD_MM_QUERY_CHAPING: {
			const char* cszChaName = pkt.ReadString();
			if (!cszChaName) {
				break;
			}

			CCharacter* pCCha = FindPlayerChaByName(cszChaName);
			if (!pCCha) {
				break;
			}

			WPacket wpk = GETWPACKET();
			wpk.WriteCmd(CMD_MC_PING);
			wpk.WriteLong(GetTickCount());
			wpk.WriteAddress(ToAddress(pGate));
			wpk.WriteLong(lSrcID);
			wpk.WriteLong(GatePlayerID);
			wpk.WriteAddress(GatePlayerAddr);
			wpk.WriteShort(1);
			pCCha->ReflectINFof(pCCha, wpk);
		} break;
		case CMD_MM_QUERY_CHA: {
			const char* cszChaName = pkt.ReadString();
			if (!cszChaName) {
				break;
			}

			CCharacter* pCCha = FindPlayerChaByName(cszChaName);
			if (!pCCha || !pCCha->GetSubMap()) {
				break;
			}

			WPacket wpk = GETWPACKET();
			wpk.WriteCmd(CMD_MC_QUERY_CHA);
			wpk.WriteLong(lSrcID);
			wpk.WriteString(pCCha->GetName());
			wpk.WriteString(pCCha->GetSubMap()->GetName());
			wpk.WriteLong(pCCha->GetPos().x);
			wpk.WriteLong(pCCha->GetPos().y);
			wpk.WriteLong(pCCha->GetID());
			wpk.WriteLong(GatePlayerID);
			wpk.WriteAddress(GatePlayerAddr);
			wpk.WriteShort(1);
			pGate->SendData(wpk);
		} break;
		case CMD_MM_QUERY_CHAITEM: {
			const char* cszChaName = pkt.ReadString();
			if (!cszChaName) {
				break;
			}

			CCharacter* pCCha = FindPlayerChaByName(cszChaName);
			if (!pCCha) {
				break;
			}
			pCCha->GetKitbag()->SetChangeFlag();

			WPacket wpk = GETWPACKET();
			wpk.WriteCmd(CMD_MC_QUERY_CHA);
			wpk.WriteLong(lSrcID);
			pCCha->WriteKitbag(pCCha->GetKitbag(), wpk, enumSYN_KITBAG_INIT);
			wpk.WriteLong(GatePlayerID);
			wpk.WriteAddress(GatePlayerAddr);
			wpk.WriteShort(1);
			pGate->SendData(wpk);
		} break;
		case CMD_MM_CALL_CHA: {
			const char* cszChaName = pkt.ReadString();
			if (!cszChaName) {
				break;
			}

			CCharacter* pCCha = FindPlayerChaByName(cszChaName);
			if (!pCCha || !pCCha->GetSubMap()) {
				break;
			}
			const bool bTarIsBoat = pkt.ReadChar() ? true : false;
			if (bTarIsBoat != pCCha->IsBoat()) { // 不同区域类型
				break;
			}
			const char* cszMapName = pkt.ReadString();
			if (!cszMapName) {
				break;
			}

			const long lPosX = pkt.ReadLong();
			const long lPosY = pkt.ReadLong();
			const long lCopyNO = pkt.ReadLong();
			pCCha->SwitchMap(pCCha->GetSubMap(), cszMapName, lPosX, lPosY, true, enumSWITCHMAP_CARRY, lCopyNO);
		} break;
		case CMD_MM_GOTO_CHA: {
			const char* cszChaName = pkt.ReadString();
			if (!cszChaName) {
				break;
			}

			CCharacter* pCCha = FindPlayerChaByName(cszChaName);
			if (!pCCha || !pCCha->GetSubMap()) {
				break;
			}
			switch (pkt.ReadChar()) {
			case 1: { // 请求查找目标角色
				const char* cszSrcName = pkt.ReadString();
				if (!cszSrcName) {
					break;
				}

				WPacket wpk = GETWPACKET();
				wpk.WriteCmd(CMD_MM_GOTO_CHA);
				wpk.WriteLong(lSrcID);
				wpk.WriteString(cszSrcName);
				wpk.WriteChar(2);
				pCCha->IsBoat() ? wpk.WriteChar(1) : wpk.WriteChar(0);
				wpk.WriteString(pCCha->GetSubMap()->GetName());
				wpk.WriteLong(pCCha->GetPos().x);
				wpk.WriteLong(pCCha->GetPos().y);
				wpk.WriteLong(pCCha->GetSubMap()->GetCopyNO());
				wpk.WriteLong(GatePlayerID);
				wpk.WriteAddress(GatePlayerAddr);
				wpk.WriteShort(1);
				pGate->SendData(wpk);
			} break;
			case 2: { // 找到了目标角色，原角色进行跳转
				const bool bTarIsBoat = pkt.ReadChar() ? true : false;
				if (bTarIsBoat != pCCha->IsBoat()) { // 不同区域类型
					break;
				}
				const char* cszMapName = pkt.ReadString();
				if (!cszMapName) {
					break;
				}

				const long lPosX = pkt.ReadLong();
				const long lPosY = pkt.ReadLong();
				const long lCopyNO = pkt.ReadLong();
				pCCha->SwitchMap(pCCha->GetSubMap(), cszMapName, lPosX, lPosY, true, enumSWITCHMAP_CARRY, lCopyNO);
			} break;
			}
		} break;
		case CMD_MM_KICK_CHA: {
			const char* cszChaName = pkt.ReadString();
			if (!cszChaName) {
				break;
			}

			const unsigned long lTime = pkt.ReadLong();
			CCharacter* pCCha = FindPlayerChaByName(cszChaName);
			if (!pCCha || !pCCha->GetSubMap()) {
				break;
			}

			KICKPLAYER(pCCha->GetPlayer(), lTime);
			g_pGameApp->GoOutGame(pCCha->GetPlayer(), true);
		} break;
		case CMD_MM_NOTICE: {
			LocalNotice(pkt.ReadString());
		} break;
		case CMD_MM_CHA_NOTICE: {
			const char* cszNotiCont = pkt.ReadString();
			if (!cszNotiCont) {
				break;
			}
			const char* cszChaName = pkt.ReadString();
			if (!cszChaName) {
				break;
			}

			if (strcmp(cszChaName, "") == 0) {
				LocalNotice(cszNotiCont);
			} else {
				CCharacter* pCCha = FindPlayerChaByName(cszChaName);
				if (!pCCha) {
					break;
				}

				WPacket wpk = GETWPACKET();
				wpk.WriteCmd(CMD_MC_SYSINFO);
				wpk.WriteString(cszNotiCont);
				pCCha->ReflectINFof(pCCha, wpk);
			}
		} break;
		case CMD_MM_DO_STRING: {
			luaL_handled_dostring(g_pLuaState, pkt.ReadString()); //CHANGED: better lua error handling (deguix)
		} break;
		case CMD_MM_LOGIN: {
			g_pGameApp->AfterPlayerLogin(pkt.ReadString());
		} break;
		case CMD_MM_GUILD_CHALL_PRIZEMONEY: {
			const unsigned long dwChaDBID = pkt.ReadLong();
			const unsigned long dwMoney = pkt.ReadLong();
			CPlayer* pPlayer = GetPlayerByDBID(dwChaDBID);
			if (pPlayer) {
				CCharacter* pCha = pPlayer->GetMainCha();
				pCha->AddMoney(RES_STRING(GM_GAMEAPPNET_CPP_00017), dwMoney);
				pCha->SystemNotice(RES_STRING(GM_GAMEAPPNET_CPP_00010), pCha->GetGuildName(), dwMoney);
				LG("challenge consortia result", "congratulate you leading consortia《%s》ID(%u)get win in consortia battle!gain bounty(%u)!\n", pCha->GetGuildName(),
				   pCha->GetGuildID(), dwMoney);
			}
			//else
			//{
			//	LG( "挑战公会结果", "未发现公会会长信息指针，无法奖励DBID[%u],钱数[%u]\n", dwChaDBID, dwMoney );
			//}
		} break;
		case CMD_MM_ADDCREDIT: {
			const unsigned long dwChaDBID = pkt.ReadLong();
			const unsigned long lCredit = pkt.ReadLong();
			CPlayer* pPlayer = GetPlayerByDBID(dwChaDBID);
			if (pPlayer) {
				CCharacter* pCha = pPlayer->GetMainCha();
				pCha->SetCredit((long)pCha->GetCredit() + lCredit);
				pCha->SynAttr(enumATTRSYN_TASK);
			}
		} break;
		case CMD_MM_STORE_BUY: {
			const unsigned long dwChaDBID = pkt.ReadLong();
			const long lComID = pkt.ReadLong();
			//long lMoBean = READ_LONG(pkt);
			const long lRplMoney = pkt.ReadLong();
			CPlayer* pPlayer = GetPlayerByDBID(dwChaDBID);
			if (pPlayer) {
				CCharacter* pCha = pPlayer->GetMainCha();
				g_StoreSystem.Accept(pCha, lComID);
				//pCha->GetPlayer()->SetMoBean(lMoBean);
				pCha->GetPlayer()->SetRplMoney(lRplMoney);
			}
		} break;
		case CMD_MM_ADDMONEY: {
			const unsigned long dwChaID = pkt.ReadLong();
			const unsigned long dwMoney = pkt.ReadLong();
			CPlayer* pPlayer = g_pGameApp->GetPlayerByDBID(dwChaID);
			if (pPlayer) {
				CCharacter* pCha = pPlayer->GetMainCha();
				if (pCha) {
					pCha->AddMoney(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00023), dwMoney);
				}
			}
		} break;
		case CMD_MM_AUCTION: {
			//add by ALLEN 2007-10-19
			const unsigned long dwChaID = pkt.ReadLong();
			CPlayer* pPlayer = g_pGameApp->GetPlayerByDBID(dwChaID);
			if (pPlayer) {
				CCharacter* pCha = pPlayer->GetMainCha();
				if (pCha) {
					g_CParser.DoString("AuctionEnd", enumSCRIPT_RETURN_NONE, 0,
									   enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCha, DOSTRING_PARAM_END);
				}
			}
		} break;
#ifdef SHUI_JING
		case CMD_MM_NOTICETOCHA: {
			long dwChaID = lSrcID;
			int type = READ_SHORT(pkt);
			DWORD Price = READ_LONG(pkt);
			DWORD Num = READ_LONG(pkt);
			DWORD tMoney = READ_LONG(pkt);
			CCharacter* pCha = NULL;
			CPlayer* pPlayer = g_pGameApp->GetPlayerByDBID(dwChaID);
			if (type == 1) {
				if (pPlayer) {
					pCha = pPlayer->GetMainCha();
					if (pCha) {
						DWORD flatmoney = 0;
						//if( game_db.GetdwFlatMoney( pCha->GetName(), flatmoney ) )
						//{
						flatmoney = pCha->GetFlatMoney();
						pCha->SetFlatMoney(flatmoney + Price * Num);
						pCha->Cmd_TradeInfoAsr(type, true, Num, Price);
						pCha->Cmd_CrystalBuyAndSaleList();
					}
					//else
					//	game_db.SetdwFlatMoney( dwChaID, Price * Num );
				}
			} else if (type == 0) {
				if (pPlayer) {
					pCha = pPlayer->GetMainCha();
					if (pCha) {
						pPlayer->SetRplMoney(tMoney);
						pCha->Cmd_TradeInfoAsr(type, true, Num, Price);
						pCha->Cmd_CrystalBuyAndSaleList();
					}
				}
			}
			break;
		}
#endif
		case CMD_MM_EXCHEXTERNVALUE: {
			const char* szScriptName = pkt.ReadString();
			const long m_lValue = pkt.ReadLong();
			g_CParser.DoString(szScriptName, enumSCRIPT_RETURN_NONE, 0,
							   enumSCRIPT_PARAM_NUMBER, 1, m_lValue, DOSTRING_PARAM_END);
		} break;
		}
	}
	T_E
}
void CGameApp::ProcessGroupBroadcast(unsigned short usCmd, GateServer* pGate, RPACKET pkt) {
	try {
	}
	T_E
}
void CGameApp::ProcessGarner2Update(RPACKET pkt) //CMD_PM_GARNER2_UPDATE
{
	try {
		long cha_ids[6];
		cha_ids[0] = pkt.ReverseReadLong();
		cha_ids[1] = pkt.ReadLong();
		cha_ids[2] = pkt.ReadLong();
		cha_ids[3] = pkt.ReadLong();
		cha_ids[4] = pkt.ReadLong();
		cha_ids[5] = pkt.ReadLong();
		if (0 != cha_ids[0]) {
			CPlayer* pplay = FindPlayerByDBChaID(cha_ids[0]);
			if (pplay) {
				pplay->SetGarnerWiner(0);
			}
		}

		for (size_t i = 1; i < std::size(cha_ids) && cha_ids[i]; i++) {
			CPlayer* pplay = FindPlayerByDBChaID(cha_ids[0]);
			if (pplay) {
				pplay->SetGarnerWiner(i);
			}
		}
	}
	T_E
}

void CGameApp::ProcessMapAdmin(GateServer* pGate, RPACKET pkt) {
	try {

		LG("map_entrance_flow", "ProcessMapAdmin\n");

		if (!pGate) {
			return;
		}

		const char* szMap = pkt.ReadString();
		if (!szMap) {
			return;
		}

		CMapRes* pCMapRes = FindMapByName(szMap);
		if (!pCMapRes) {
			return;
		}

		const unsigned short id = pkt.ReverseReadShort();
		const uintptr_t addr = pkt.ReverseReadAddress();
		const unsigned long dbID = pkt.ReverseReadLong();

		switch (pkt.ReadChar()) {
		case enumMAPADMIN_OPEN_MAP: {
			pCMapRes->OpenMap();
		} break;
		case enumMAPADMIN_CLOSE_MAP: {
			pCMapRes->CloseMap();
		} break;
		case enumMAPADMIN_OPEN_MAP_ENTRY: {
			pCMapRes->OpenMapEntry();
		} break;
		case enumMAPADMIN_CLOSE_MAP_ENTRY: {
			pCMapRes->CloseMapEntry();
		} break;
		case enumMAPADMIN_CHECK_MAP: {
			std::string str;
			pCMapRes->CheckMapState(str);

			WPacket wpk = GETWPACKET();
			wpk.WriteCmd(CMD_MC_SYSINFO);
			wpk.WriteString(str.c_str());
			wpk.WriteLong(dbID);
			wpk.WriteAddress(addr);
			wpk.WriteShort(1);
			pGate->SendData(wpk);
		} break;
		}
	}
	T_E
}

void CGameApp::ProcessDynMapEntry(GateServer* pGate, RPACKET pkt) {
	try {
		const char* szTarMapN = pkt.ReadString();
		if (!szTarMapN) {
			return;
		}

		const char* szSrcMapN = pkt.ReadString();
		if (!szSrcMapN) {
			return;
		}

		switch (pkt.ReadChar()) {
		case enumMAPENTRY_CREATE: {
			CMapRes* pCMapRes = FindMapByName(szTarMapN);
			if (!pCMapRes) {
				break;
			}
			SubMap* pCMap = pCMapRes->GetCopy();
			const long lPosX = pkt.ReadLong();
			const long lPosY = pkt.ReadLong();
			const short sMapCopyNum = pkt.ReadShort();
			const short sCopyPlyNum = pkt.ReadShort();
			CDynMapEntryCell CEntryCell;
			CEntryCell.SetMapName(szTarMapN);
			CEntryCell.SetTMapName(szSrcMapN);
			CEntryCell.SetEntiPos(lPosX, lPosY);
			CDynMapEntryCell* pCEntry = g_CDMapEntry.Add(&CEntryCell);
			if (pCEntry) {
				if (g_cchLogMapEntry) {
					LG("map_entrance_flow", "receive request to create entry:position %s --> %s[%u, %u]\n",
					   szSrcMapN, szTarMapN, lPosX, lPosY);
				}
				pCEntry->SetCopyNum(sMapCopyNum);
				pCEntry->SetCopyPlyNum(sCopyPlyNum);
				std::string strScript;
				const char* cszSctLine{nullptr};
				short sLineNum = pkt.ReverseReadShort();
				while (--sLineNum >= 0) {
					cszSctLine = pkt.ReadString();
					strScript += cszSctLine;
					strScript += " ";
				}
				luaL_handled_dostring(g_pLuaState, strScript.c_str()); //CHANGED: better lua error handling (deguix)
				g_CParser.DoString("config_entry", enumSCRIPT_RETURN_NONE, 0,
								   enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCEntry, DOSTRING_PARAM_END);

				if (pCEntry->GetEntiID() > 0) {
					SItemGrid SItemCont;
					SItemCont.sID = static_cast<short>(pCEntry->GetEntiID());
					SItemCont.sNum = 1;
					SItemCont.SetDBParam(-1, 0);
					SItemCont.chForgeLv = 0;
					SItemCont.SetInstAttrInvalid();
					CItem* pCItem = pCMap->ItemSpawn(&SItemCont, lPosX, lPosY, enumITEM_APPE_NATURAL,
													 0, g_pCSystemCha->GetID(), g_pCSystemCha->GetHandle(), -1, -1,
													 pCEntry->GetEvent());
					if (pCItem) {
						pCItem->SetOnTick(0);
						pCEntry->SetEnti(pCItem);
					} else {
						if (g_cchLogMapEntry) {
							LG("map_entrance_flow", "create entry failed:position %s --> %s[%u, %u],item %u create failed\n",
							   szSrcMapN, szTarMapN, lPosX, lPosY, SItemCont.sID);
						}
						g_CDMapEntry.Del(pCEntry);
						break;
					}
				}
				// 通知源方，创建成功
				WPacket wpk = GETWPACKET();
				wpk.WriteCmd(CMD_MT_MAPENTRY);
				wpk.WriteString(szSrcMapN);
				wpk.WriteString(szTarMapN);
				wpk.WriteChar(enumMAPENTRY_RETURN);
				wpk.WriteChar(enumMAPENTRYO_CREATE_SUC);

				BEGINGETGATE();
				while (GateServer* pGateServer = GETNEXTGATE()) {
					pGateServer->SendData(wpk);
					break;
				}
				if (g_cchLogMapEntry) {
					LG("map_entrance_flow", "create entry success:position %s --> %s[%u, %u] \n",
					   szSrcMapN, szTarMapN, lPosX, lPosY);
				}

				g_CParser.DoString("after_create_entry", enumSCRIPT_RETURN_NONE, 0,
								   enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCEntry, DOSTRING_PARAM_END);
			} else {
				if (g_cchLogMapEntry) {
					LG("map_entrance_flow", "create entry error:position %s --> %s[%u, %u] \n", szSrcMapN, szTarMapN, lPosX, lPosY);
				}
			}
		} break;
		case enumMAPENTRY_SUBPLAYER: {
			const short sCopyNO = pkt.ReadShort();
			const short sSubNum = pkt.ReadShort();

			CDynMapEntryCell* pCEntry = g_CDMapEntry.GetEntry(szSrcMapN);
			if (pCEntry) {
				CMapEntryCopyCell* pCCopyInfo = pCEntry->GetCopy(sCopyNO);
				if (pCCopyInfo) {
					pCCopyInfo->AddCurPlyNum(-1 * sSubNum);
				}
			}
		} break;
		case enumMAPENTRY_SUBCOPY: {
			const short sCopyNO = pkt.ReadShort();

			CDynMapEntryCell* pCEntry = g_CDMapEntry.GetEntry(szSrcMapN);
			if (pCEntry) {
				pCEntry->ReleaseCopy(sCopyNO);
			}
			// 通知源方，副本关闭成功
			WPacket wpk = GETWPACKET();
			wpk.WriteCmd(CMD_MT_MAPENTRY);
			wpk.WriteString(szSrcMapN);
			wpk.WriteString(szTarMapN);
			wpk.WriteChar(enumMAPENTRY_RETURN);
			wpk.WriteChar(enumMAPENTRYO_COPY_CLOSE_SUC);
			wpk.WriteShort(sCopyNO);

			BEGINGETGATE();
			while (GateServer* pGateServer = GETNEXTGATE()) {
				pGateServer->SendData(wpk);
				break;
			}
		} break;
		case enumMAPENTRY_DESTROY: {
			CDynMapEntryCell* pCEntry = g_CDMapEntry.GetEntry(szSrcMapN);
			if (g_cchLogMapEntry) {
				LG("map_entrance_flow", "receive request to destroy entry:position %s --> %s\n",
				   szSrcMapN, szTarMapN);
			}
			if (pCEntry) {
				std::string strScript = "after_destroy_entry_";
				strScript += szSrcMapN;
				g_CParser.DoString(strScript.c_str(), enumSCRIPT_RETURN_NONE, 0,
								   enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCEntry, DOSTRING_PARAM_END);
				g_CDMapEntry.Del(pCEntry);

				// 通知源方，销毁成功
				WPacket wpk = GETWPACKET();
				wpk.WriteCmd(CMD_MT_MAPENTRY);
				wpk.WriteString(szSrcMapN);
				wpk.WriteString(szTarMapN);
				wpk.WriteChar(enumMAPENTRY_RETURN);
				wpk.WriteChar(enumMAPENTRYO_DESTROY_SUC);

				BEGINGETGATE();
				while (GateServer* pGateServer = GETNEXTGATE()) {
					pGateServer->SendData(wpk);
					break;
				}
				if (g_cchLogMapEntry) {
					LG("map_entrance_flow", "destroy entry success:position %s --> %s\n", szSrcMapN, szTarMapN);
				}
			} else {
				LG("map_entrance_flow", "destroy entry error:position %s --> %s\n", szSrcMapN, szTarMapN);
			}
		} break;
		case enumMAPENTRY_COPYPARAM: {
			CMapRes* pCMapRes = FindMapByName(szTarMapN);
			if (!pCMapRes) {
				break;
			}
			SubMap* pCMap = pCMapRes->GetCopy(pkt.ReadShort());
			if (!pCMap) {
				break;
			}
			for (dbc::Char i = 0; i < defMAPCOPY_INFO_PARAM_NUM; i++) {
				pCMap->SetInfoParam(i, pkt.ReadLong());
			}
		} break;
		case enumMAPENTRY_COPYRUN: {
			CMapRes* pCMapRes = FindMapByName(szTarMapN);
			if (!pCMapRes) {
				break;
			}
			SubMap* pCMap = pCMapRes->GetCopy(pkt.ReadShort());
			if (!pCMap) {
				break;
			}

			const char chType = pkt.ReadChar();
			const long lVal = pkt.ReadLong();
			pCMapRes->SetCopyStartCondition(chType, lVal);
		} break;
		case enumMAPENTRY_RETURN: {
			CMapRes* pCMap = FindMapByName(szTarMapN, true);
			if (!pCMap) {
				break;
			}
			switch (pkt.ReadChar()) {
			case enumMAPENTRYO_CREATE_SUC: {
				pCMap->CheckEntryState(enumMAPENTRY_STATE_OPEN);
				if (g_cchLogMapEntry) {
					LG("map_entrance_flow", "receive entry create success :position %s --> %s\n",
					   szSrcMapN, szTarMapN);
				}
			} break;
			case enumMAPENTRYO_DESTROY_SUC: {
				if (g_cchLogMapEntry) {
					LG("map_entrance_flow", "receive entry destroy success:position %s --> %s\n",
					   szSrcMapN, szTarMapN);
				}
				pCMap->CheckEntryState(enumMAPENTRY_STATE_CLOSE_SUC);
			} break;
			case enumMAPENTRYO_COPY_CLOSE_SUC: {
				pCMap->CopyClose(READ_SHORT(pkt));
			} break;
			default: {
			} break;
			}
		} break;
		default: {
		} break;
		};
	}
	T_E
}
