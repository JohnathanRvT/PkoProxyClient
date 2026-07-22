#include <iostream>
#include "GroupServerApp.h"
#include "GameCommon.h"

//const cChar*	gc_master_group = RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00018);
//const cChar*	gc_prentice_group = RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00017);

void GroupServerApp::CP_MASTER_REFRESH_INFO(Player* ply, DataSocket* datasock, RPacket& pk) {
	uLong l_chaid = pk.ReadLong();
	MutexArmor l_lockDB(m_mtxDB);
	//if(m_tblmaster->HasMaster(ply->m_chaid[ply->m_currcha],l_chaid) < 1)
	if (HasMaster(ply->m_chaid[ply->m_currcha], l_chaid) < 1) {
		l_lockDB.unlock();
		//ply->SendSysInfo("你们不是师徒关系！");
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00001));
	} else if (m_tblcharaters->FetchRowByChaID(l_chaid) == 1) {
		WPacket wpk = GetWPacket();
		wpk.WriteCmd(CMD_PC_MASTER_REFRESH_INFO);
		wpk.WriteLong(l_chaid);
		wpk.WriteString(m_tblcharaters->GetMotto());
		wpk.WriteShort(m_tblcharaters->GetIcon());
		wpk.WriteShort(m_tblcharaters->GetDegree());
		wpk.WriteString(m_tblcharaters->GetJob());
		wpk.WriteString(m_tblcharaters->GetGuildName());
		l_lockDB.unlock();
		SendToClient(ply, wpk);
	}
}

void GroupServerApp::CP_PRENTICE_REFRESH_INFO(Player* ply, DataSocket* datasock, RPacket& pk) {
	uLong l_chaid = pk.ReadLong();
	MutexArmor l_lockDB(m_mtxDB);
	//if(m_tblmaster->HasMaster(l_chaid, ply->m_chaid[ply->m_currcha]) < 1)
	if (HasMaster(l_chaid, ply->m_chaid[ply->m_currcha]) < 1) {
		l_lockDB.unlock();
		//ply->SendSysInfo("你们不是师徒关系！");
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00001));
	} else if (m_tblcharaters->FetchRowByChaID(l_chaid) == 1) {
		WPacket wpk = GetWPacket();
		wpk.WriteCmd(CMD_PC_PRENTICE_REFRESH_INFO);
		wpk.WriteLong(l_chaid);
		wpk.WriteString(m_tblcharaters->GetMotto());
		wpk.WriteShort(m_tblcharaters->GetIcon());
		wpk.WriteShort(m_tblcharaters->GetDegree());
		wpk.WriteString(m_tblcharaters->GetJob());
		wpk.WriteString(m_tblcharaters->GetGuildName());
		l_lockDB.unlock();
		SendToClient(ply, wpk);
	}
}

void GroupServerApp::MP_MASTER_CREATE(Player* ply, DataSocket* datasock, RPacket& pk) {
	const char* szPrenticeName = pk.ReadString();
	unsigned long l_prentice_chaid = pk.ReadLong();
	const char* szMasterName = pk.ReadString();
	unsigned long l_master_chaid = pk.ReadLong();

	Player* pPrentice = FindPlayerByChaName(szPrenticeName);
	//Player *pPrentice = GetPlayerByChaID(l_prentice_chaid);
	Player* pMaster = FindPlayerByChaName(szMasterName);
	//Player *pMaster = GetPlayerByChaID(l_master_chaid);

	if (!pPrentice || !pMaster) {
		LogLine l_line(g_LogMaster);
		l_line << newln << "MP_MASTER_CREATE() member is offline!";
		return;
	}

	//邀请
	bool bInvited = false;
	if (pPrentice->m_CurrMasterNum >= const_master.MasterMax) {
		pMaster->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00003));
		pPrentice->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00004));
	} else if (m_tblmaster->GetPrenticeCount(l_master_chaid) >= const_master.PrenticeMax) {
		pMaster->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00005));
		pPrentice->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00006));
	} else {
		Invited* l_invited = nullptr;
		const unsigned short l_len = (uShort)strlen(szMasterName);
		const char* l_invited_name = szMasterName;
		if (!l_invited_name || l_len > CommConstants::Character::name_max_length) {
			return;
		}
		Player* l_invited_ply = pMaster;
		MutexArmor l_lockDB(m_mtxDB);
		if (!l_invited_ply || l_invited_ply->m_currcha < 0 || l_invited_ply == pPrentice) {
			char l_buf[256];
			_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00007), l_invited_name);
			pPrentice->SendSysInfo(l_buf);
		} else if (l_invited = l_invited_ply->MasterFindInvitedByInviterChaID(pPrentice->m_chaid[pPrentice->m_currcha])) {
			char l_buf[256];
			_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00008), l_invited_name);
			pPrentice->SendSysInfo(l_buf);
		} else if (l_invited = pPrentice->MasterFindInvitedByInviterChaID(l_invited_ply->m_chaid[l_invited_ply->m_currcha])) {
			char l_buf[256];
			_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00009), l_invited_name);
			pPrentice->SendSysInfo(l_buf);
			//}else if(m_tblmaster->HasMaster(pPrentice->m_chaid[pPrentice->m_currcha], l_invited_ply->m_chaid[l_invited_ply->m_currcha]) > 0)
		} else if (HasMaster(pPrentice->m_chaid[pPrentice->m_currcha], l_invited_ply->m_chaid[l_invited_ply->m_currcha]) > 0) {
			pPrentice->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00011));
			pMaster->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00011));
		} else {
			PtInviter l_ptinviter = l_invited_ply->MasterBeginInvited(pPrentice);
			if (l_ptinviter) {
				char l_buf[256];
				_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00012), l_invited_name);
				l_ptinviter->SendSysInfo(l_buf);
			}

			bInvited = true;
		}
	}

	//确认
	if (bInvited) {
		unsigned long l_inviter_chaid = l_prentice_chaid;
		PtInviter l_inviter = pMaster->MasterEndInvited(l_inviter_chaid);
		if (l_inviter && l_inviter->m_currcha >= 0 && l_inviter.m_chaid == l_inviter->m_chaid[l_inviter->m_currcha]) {
			MutexArmor l_lockDB(m_mtxDB);
			++(pMaster->m_CurrPrenticeNum);
			if ((++(pPrentice->m_CurrMasterNum)) > const_master.MasterMax) {
				--(pMaster->m_CurrPrenticeNum);
				--(pPrentice->m_CurrMasterNum);
				char l_buf[256];
				_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00013), l_inviter->m_chaname[l_inviter->m_currcha].c_str());
				pMaster->SendSysInfo(l_buf);
			} else {
				LogLine l_line(g_LogMaster);
				l_line << newln << "player" << pMaster->m_chaname[pMaster->m_currcha] << "(" << pMaster->m_chaid[pMaster->m_currcha]
					   << ")and player" << l_inviter->m_chaname[l_inviter->m_currcha] << "(" << l_inviter_chaid << ")become Master!"
					   << endln;
				m_tblmaster->AddMaster(l_prentice_chaid, l_master_chaid);
				AddMaster(l_prentice_chaid, l_master_chaid);
				pMaster->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00014));
				pPrentice->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00015));
				WPacket wpk = GetWPacket();
				wpk.WriteCmd(CMD_PC_MASTER_REFRESH);
				WPacket wpk2 = wpk;
				wpk.WriteChar(MSG_MASTER_REFRESH_ADD);
				wpk.WriteString(Master::GetMasterGroup());
				wpk.WriteLong(pMaster->m_chaid[pMaster->m_currcha]);
				wpk.WriteString(pMaster->m_chaname[pMaster->m_currcha].c_str());
				wpk.WriteString(pMaster->m_motto[pMaster->m_currcha].c_str());
				wpk.WriteShort(pMaster->m_icon[pMaster->m_currcha]);
				SendToClient(l_inviter.m_ply, wpk);
				wpk2.WriteChar(MSG_PRENTICE_REFRESH_ADD);
				wpk2.WriteString(Master::GetPrenriceGroup());
				wpk2.WriteLong(l_inviter->m_chaid[l_inviter->m_currcha]);
				wpk2.WriteString(l_inviter->m_chaname[l_inviter->m_currcha].c_str());
				wpk2.WriteString(l_inviter->m_motto[l_inviter->m_currcha].c_str());
				wpk2.WriteShort(l_inviter->m_icon[l_inviter->m_currcha]);
				SendToClient(pMaster, wpk2);
			}
		}
	} else {
		LogLine l_line(g_LogMaster);
		l_line << newln << "MP_MASTER_CREATE() invite failed";
	}
}

void GroupServerApp::MP_MASTER_DEL(Player* ply, DataSocket* datasock, RPacket& pk) {
	cChar* szPrenticeName = pk.ReadString();
	uLong l_prentice_chaid = pk.ReadLong();
	cChar* szMasterName = pk.ReadString();
	uLong l_master_chaid = pk.ReadLong();

	Player* pPrentice = FindPlayerByChaName(szPrenticeName);
	//Player *pPrentice = GetPlayerByChaID(l_prentice_chaid);
	Player* pMaster = FindPlayerByChaName(szMasterName);
	//Player *pMaster = GetPlayerByChaID(l_master_chaid);

	MutexArmor l_lockDB(m_mtxDB);
	//if(m_tblmaster->HasMaster(l_prentice_chaid,l_master_chaid) < 1)
	if (HasMaster(l_prentice_chaid, l_master_chaid) < 1) {
		return;
	} else {
		if (pPrentice) {
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PC_MASTER_REFRESH);
			wpk.WriteChar(MSG_MASTER_REFRESH_DEL);
			wpk.WriteLong(l_master_chaid);
			SendToClient(pPrentice, wpk);
			--(pPrentice->m_CurrMasterNum);
		}

		if (pMaster) {
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PC_MASTER_REFRESH);
			wpk.WriteChar(MSG_PRENTICE_REFRESH_DEL);
			wpk.WriteLong(l_prentice_chaid);
			SendToClient(pMaster, wpk);
			--(pMaster->m_CurrPrenticeNum);
		}

		m_tblmaster->DelMaster(l_prentice_chaid, l_master_chaid);
		DelMaster(l_prentice_chaid, l_master_chaid);
		LogLine l_line(g_LogMaster);
		l_line << newln << "player " << szMasterName << "(" << l_master_chaid
			   << ")and " << szPrenticeName << " (" << l_prentice_chaid << ")free master relation";
	}
}

void GroupServerApp::MP_MASTER_FINISH(Player* ply, DataSocket* datasock, RPacket& pk) {
	uLong l_prentice_chaid = pk.ReadLong();
	//if(m_tblmaster->GetMasterCount(l_prentice_chaid) > 0)

	MutexArmor l_lockDB(m_mtxDB);
	if (GetMasterCount(l_prentice_chaid) > 0) {
		m_tblmaster->FinishMaster(l_prentice_chaid);
		//FinishMaster(l_prentice_chaid);
	}
}

void Player::MasterInvitedCheck(Invited* invited) {
	Player* l_inviter = invited->m_ptinviter.m_ply;
	if (m_currcha < 0) {
		MasterEndInvited(l_inviter);
	} else if (l_inviter->m_currcha < 0 || l_inviter->m_chaid[l_inviter->m_currcha] != invited->m_ptinviter.m_chaid) {
		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_MASTER_CANCEL);
		wpk.WriteChar(MSG_MASTER_CANCLE_OFFLINE);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		MasterEndInvited(l_inviter);
	} else if (l_inviter->m_CurrMasterNum >= g_gpsvr->const_master.MasterMax) {
		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_MASTER_CANCEL);
		wpk.WriteChar(MSG_MASTER_CANCLE_INVITER_ISFULL);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		MasterEndInvited(l_inviter);
	} /*else if(m_CurrPrenticeNum >= g_gpsvr->const_master.PrenticeMax)
	{
		WPacket wpk	=g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_MASTER_CANCEL);
		wpk.WriteChar(MSG_MASTER_CANCLE_SELF_ISFULL);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this,wpk);
		MasterEndInvited(l_inviter);
	}*/
	else if (g_gpsvr->GetCurrentTick() - invited->m_tick >= g_gpsvr->const_master.PendTimeOut) {
		char l_buf[256];
		//sprintf(l_buf,"你对【%s】的拜师申请已超过%d秒钟没有回应，系统自动取消了你的申请。",m_chaname[m_currcha].c_str(),g_gpsvr->const_master.PendTimeOut/1000);
		//sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00016),m_chaname[m_currcha].c_str(),g_gpsvr->const_master.PendTimeOut/1000);
		//_snprintf_s(l_buf,sizeof(l_buf),_TRUNCATE,RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00016),m_chaname[m_currcha].c_str(),g_gpsvr->const_master.PendTimeOut/1000);
		CFormatParameter param(2);
		param.setString(0, m_chaname[m_currcha].c_str());
		param.setLong(1, g_gpsvr->const_master.PendTimeOut / 1000);
		RES_FORMAT_STRING(GP_GROUPSERVERAPPMASTER_CPP_00016, param, l_buf);
		l_inviter->SendSysInfo(l_buf);

		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_MASTER_CANCEL);
		wpk.WriteChar(MSG_MASTER_CANCLE_TIMEOUT);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		MasterEndInvited(l_inviter);
	}
}

void GroupServerApp::PC_MASTER_INIT(Player* ply) {
	TBLMaster::master_dat l_farray1[200];
	TBLMaster::master_dat l_farray2[200];
	int l_num1 = 200;
	int l_num2 = 200;

	//通知学徒
	m_tblmaster->GetPrenticeData(l_farray1, l_num1, ply->m_chaid[ply->m_currcha]);

	WPacket l_toPrentice = GetWPacket();
	l_toPrentice.WriteCmd(CMD_PC_MASTER_REFRESH);
	l_toPrentice.WriteChar(MSG_MASTER_REFRESH_ONLINE);
	l_toPrentice.WriteLong(ply->m_chaid[ply->m_currcha]);

	WPacket l_toSelf1 = GetWPacket();
	l_toSelf1.WriteCmd(CMD_PC_MASTER_REFRESH);
	l_toSelf1.WriteChar(MSG_PRENTICE_REFRESH_START);

	l_toSelf1.WriteLong(ply->m_chaid[ply->m_currcha]);
	l_toSelf1.WriteString(ply->m_chaname[ply->m_currcha].c_str());
	l_toSelf1.WriteString(ply->m_motto[ply->m_currcha].c_str());
	l_toSelf1.WriteShort(ply->m_icon[ply->m_currcha]);

	ply->m_CurrPrenticeNum = 0;

	Player* playerlst1[10240];
	short playernum1 = 0;

	Player* player11;
	char l_currcha1;
	for (int i = 0; i < l_num1; i++) {
		if (l_farray1[i].cha_id == 0) {
			if (l_farray1[i].icon_id == 0) {
				l_toSelf1.WriteShort(uShort(l_farray1[i].memaddr));
			} else {
				//l_toSelf1.WriteString(l_farray1[i].relation.c_str());
				//l_toSelf1.WriteString("学徒");
				l_toSelf1.WriteString(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00017));
				l_toSelf1.WriteShort(uShort(l_farray1[i].memaddr));
				ply->m_CurrPrenticeNum += l_farray1[i].memaddr;
			}
		} else if ((player11 = ToPointer<Player>(l_farray1[i].memaddr)) &&
				   ((l_currcha1 = player11->m_currcha) >= 0) &&
				   (player11->m_chaid[l_currcha1] == l_farray1[i].cha_id)) {
			playerlst1[playernum1] = player11;
			playernum1++;

			l_toSelf1.WriteLong(l_farray1[i].cha_id);
			l_toSelf1.WriteString(l_farray1[i].cha_name.c_str());
			l_toSelf1.WriteString(l_farray1[i].motto.c_str());
			l_toSelf1.WriteShort(l_farray1[i].icon_id);
			l_toSelf1.WriteChar(1);
		} else {
			l_toSelf1.WriteLong(l_farray1[i].cha_id);
			l_toSelf1.WriteString(l_farray1[i].cha_name.c_str());
			l_toSelf1.WriteString(l_farray1[i].motto.c_str());
			l_toSelf1.WriteShort(l_farray1[i].icon_id);
			l_toSelf1.WriteChar(0);
		}
	}
	SendToClient(ply, l_toSelf1);
	LogLine l_line1(g_LogMaster);
	l_line1 << newln << "online notice apprentice num:" << playernum1 << endln;
	SendToClient(playerlst1, playernum1, l_toPrentice);

	//通知导师
	m_tblmaster->GetMasterData(l_farray2, l_num2, ply->m_chaid[ply->m_currcha]);

	WPacket l_toMaster = GetWPacket();
	l_toMaster.WriteCmd(CMD_PC_MASTER_REFRESH);
	l_toMaster.WriteChar(MSG_PRENTICE_REFRESH_ONLINE);
	l_toMaster.WriteLong(ply->m_chaid[ply->m_currcha]);

	WPacket l_toSelf2 = GetWPacket();
	l_toSelf2.WriteCmd(CMD_PC_MASTER_REFRESH);
	l_toSelf2.WriteChar(MSG_MASTER_REFRESH_START);

	l_toSelf2.WriteLong(ply->m_chaid[ply->m_currcha]);
	l_toSelf2.WriteString(ply->m_chaname[ply->m_currcha].c_str());
	l_toSelf2.WriteString(ply->m_motto[ply->m_currcha].c_str());
	l_toSelf2.WriteShort(ply->m_icon[ply->m_currcha]);

	ply->m_CurrPrenticeNum = 0;

	Player* playerlst2[10240];
	short playernum2 = 0;

	Player* player12;
	char l_currcha2;
	for (int i = 0; i < l_num2; i++) {
		if (l_farray2[i].cha_id == 0) {
			if (l_farray2[i].icon_id == 0) {
				l_toSelf2.WriteShort(uShort(l_farray2[i].memaddr));
			} else {
				//l_toSelf2.WriteString(l_farray2[i].relation.c_str());
				//l_toSelf2.WriteString("导师");
				l_toSelf2.WriteString(RES_STRING(GP_GROUPSERVERAPPMASTER_CPP_00018));
				l_toSelf2.WriteShort(uShort(l_farray2[i].memaddr));
				ply->m_CurrMasterNum += l_farray2[i].memaddr;
			}
		} else if ((player12 = ToPointer<Player>(l_farray2[i].memaddr)) &&
				   ((l_currcha2 = player12->m_currcha) >= 0) &&
				   (player12->m_chaid[l_currcha2] == l_farray2[i].cha_id)) {
			playerlst2[playernum2] = player12;
			playernum2++;

			l_toSelf2.WriteLong(l_farray2[i].cha_id);
			l_toSelf2.WriteString(l_farray2[i].cha_name.c_str());
			l_toSelf2.WriteString(l_farray2[i].motto.c_str());
			l_toSelf2.WriteShort(l_farray2[i].icon_id);
			l_toSelf2.WriteChar(1);
		} else {
			l_toSelf2.WriteLong(l_farray2[i].cha_id);
			l_toSelf2.WriteString(l_farray2[i].cha_name.c_str());
			l_toSelf2.WriteString(l_farray2[i].motto.c_str());
			l_toSelf2.WriteShort(l_farray2[i].icon_id);
			l_toSelf2.WriteChar(0);
		}
	}
	SendToClient(ply, l_toSelf2);
	LogLine l_line2(g_LogMaster);
	l_line2 << newln << "online notice master num:" << playernum2 << endln;
	SendToClient(playerlst2, playernum2, l_toMaster);
}

bool GroupServerApp::InitMasterRelation() {
	if (m_tblmaster->InitMasterRelation(m_mapMasterRelation)) {
		return true;
	}
	return false;
}

int GroupServerApp::GetMasterCount(uLong cha_id) {
	auto it = m_mapMasterRelation.find(cha_id);
	if (it != m_mapMasterRelation.end()) {
		return 1;
	}
	return 0;
}

int GroupServerApp::GetPrenticeCount(uLong cha_id) {
	return 0;
}

int GroupServerApp::HasMaster(uLong cha_id1, uLong cha_id2) {
	auto it = m_mapMasterRelation.find(cha_id1);
	if (it != m_mapMasterRelation.end()) {
		if (m_mapMasterRelation[cha_id1] == cha_id2)
			return 1;
	}
	return 0;
}

bool GroupServerApp::AddMaster(uLong cha_id1, uLong cha_id2) {
	auto it = m_mapMasterRelation.find(cha_id1);
	if (it != m_mapMasterRelation.end()) {
		return false;
	}

	m_mapMasterRelation[cha_id1] = cha_id2;
	return true;
}

bool GroupServerApp::DelMaster(uLong cha_id1, uLong cha_id2) {
	auto it = m_mapMasterRelation.find(cha_id1);
	if (it != m_mapMasterRelation.end()) {
		if (m_mapMasterRelation[cha_id1] == cha_id2) {
			m_mapMasterRelation.erase(it);
			return true;
		}
	}
	return false;
}

bool GroupServerApp::FinishMaster(uLong cha_id) {
	auto it = m_mapMasterRelation.find(cha_id);
	if (it != m_mapMasterRelation.end()) {
		m_mapMasterRelation.erase(it);
		return true;
	}
	return false;
}
