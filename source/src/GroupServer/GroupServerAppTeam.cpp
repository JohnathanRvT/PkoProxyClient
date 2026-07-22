#include <iostream>
#include "GroupServerApp.h"
#include "GameCommon.h"

#include "Parser.h"

// Add by lark.li 20080715 begin
bool GroupServerApp::CheckFunction(std::string mapName, std::string funName) {
	int i = 0;
	if (g_CParser.DoString("CheckFunction", enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_STRING, 1, mapName.c_str(), enumSCRIPT_PARAM_STRING, 1, funName.c_str(), DOSTRING_PARAM_END)) {
		//NOTE: i is unused, function incomplete? Always returns false
		i = g_CParser.GetReturnNumber(0);
	}
	return false;
}
// End

void GroupServerApp::CP_TEAM_INVITE(Player* ply, DataSocket* datasock, RPacket& pk) {
	//// Add by lark.li 20080715 begin
	//if(!CheckFunction("garner", "Team_Invite"))
	//{
	//	ply->SendSysInfo("You can't invite!");
	//	return;
	//}
	//// End

	if (ply->GetTeam() && ply->GetLeader() != ply) {
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00001));
		return;
	}
	if (ply->GetTeam() && ply->GetTeam()->GetTotal() >= const_team.MemberMax) {
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00002));
		return;
	}

	unsigned short l_len;
	const char* invited_name = pk.ReadString(&l_len);
	if (!invited_name || l_len > CommConstants::Character::name_max_length) {
		return;
	}

	Player* invited_player = FindPlayerByChaName(invited_name);
	if (!invited_player || invited_player->m_currcha < 0 || invited_player == ply) {
		char l_buf[256];
		//sprintf(l_buf,"你所邀请的玩家【%s】当前不在线上。",l_invited_name);
		//sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00003),l_invited_name);
		ply->SendSysInfo(l_buf);
		return;
	}

	if (invited_player->GetTeam()) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00004), invited_name);
		ply->SendSysInfo(l_buf);
		return;
	}

	Invited* invited = invited_player->TeamFindInvitedByInviterChaID(ply->m_chaid[ply->m_currcha]);
	if (invited) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00005), invited_name);
		ply->SendSysInfo(l_buf);
		return;
	}

	PtInviter l_ptinviter = invited_player->TeamBeginInvited(ply);
	if (l_ptinviter) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00007), invited_name);
		l_ptinviter->SendSysInfo(l_buf);

		WPacket wpk = GetWPacket();
		wpk.WriteCmd(CMD_PC_TEAM_CANCEL);
		wpk.WriteChar(MSG_TEAM_CANCLE_BUSY);
		wpk.WriteLong(l_ptinviter.m_chaid);
		SendToClient(invited_player, wpk);
	}

	WPacket wpk = GetWPacket();
	wpk.WriteCmd(CMD_PC_TEAM_INVITE);
	wpk.WriteString(ply->m_chaname[ply->m_currcha].c_str());
	wpk.WriteLong(ply->m_chaid[ply->m_currcha]);
	wpk.WriteShort(ply->m_icon[ply->m_currcha]);
	SendToClient(invited_player, wpk);
}
void GroupServerApp::CP_TEAM_REFUSE(Player* ply, DataSocket* datasock, RPacket& pk) {
	const unsigned long l_inviter_chaid = pk.ReadLong();
	PtInviter l_inviter = ply->TeamEndInvited(l_inviter_chaid);
	if (l_inviter && l_inviter->m_currcha >= 0 && l_inviter.m_chaid == l_inviter->m_chaid[l_inviter->m_currcha]) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00008), ply->m_chaname[ply->m_currcha].c_str());
		l_inviter->SendSysInfo(l_buf);
	}
}
void GroupServerApp::MP_TEAM_CREATE(Player* ply, DataSocket* datasock, RPacket& pk) {
	bool bInvited = false;
	const char* invited_name = pk.ReadString(); // 队员
	const char* inviter_name = pk.ReadString(); // 队长

	Player* inviter = FindPlayerByChaName(inviter_name);
	Player* invited = FindPlayerByChaName(invited_name);
	if (!inviter || !invited) {
		LogLine l_line(g_LogMaster);
		l_line << newln << "MP_TEAM_CREATE() member is offline!";
		return;
	}

	//邀请
	if (inviter->GetTeam() && inviter->GetLeader() != inviter) {
		inviter->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00010));
		invited->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00011));
	} else if (inviter->GetTeam() && inviter->GetTeam()->GetTotal() >= const_team.MemberMax) {
		inviter->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00002));
		invited->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00012));
	} else {
		const size_t l_len = strlen(invited_name);
		if (!invited_name || l_len > CommConstants::Character::name_max_length) {
			LogLine l_line(g_LogMaster);
			l_line << newln << "MP_TEAM_CREATE() name length is invalid!";
			return;
		}
		if (!invited || invited->m_currcha < 0 || invited == inviter) {
			char l_buf[256];
			_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00014), invited_name);
			inviter->SendSysInfo(l_buf);
		} else if (invited->GetTeam()) {
			char l_buf[256];
			_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00015), invited_name);
			inviter->SendSysInfo(l_buf);
		} else if (invited->TeamFindInvitedByInviterChaID(inviter->m_chaid[inviter->m_currcha])) {
			char l_buf[256];
			_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00005), invited_name);
			inviter->SendSysInfo(l_buf);
		} else {
			PtInviter l_ptinviter = invited->TeamBeginInvited(inviter);
			if (l_ptinviter) {
				char l_buf[256];
				_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00016), invited_name);
				l_ptinviter->SendSysInfo(l_buf);
			}
			bInvited = true;
		}
	}

	//接受邀请
	if (bInvited) {
		const long l_count = invited->JoinTeam(inviter_name);
		if (l_count && (l_count > const_team.MemberMax)) {
			invited->LeaveTeam();
			invited->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00017));
		} else if (l_count) {
			LogLine l_line(g_LogMaster);
			l_line << newln << "player " << invited->m_chaname[invited->m_currcha] << " add team " << invited->GetLeader()->m_chaname[invited->GetLeader()->m_currcha] << endln;
			//通知Client组成员变化
			{
				Team* l_team = invited->GetTeam();

				WPacket wpk = GetWPacket();
				wpk.WriteCmd(CMD_PC_TEAM_REFRESH);
				wpk.WriteChar(TEAM_MSG_ADD);
				wpk.WriteChar(static_cast<unsigned char>(l_count));

				Player* playerlst[10240];
				short playernum = 0;

				Player* player1{nullptr};
				char l_currcha;
				RunChainGetArmor<TeamMember> l(*l_team);
				for (int i = 0; i < l_count && (player1 = static_cast<Player*>(l_team->GetNextItem())); i++) {
					if ((l_currcha = player1->m_currcha) >= 0) {
						wpk.WriteLong(player1->m_chaid[l_currcha]);
						wpk.WriteString(player1->m_chaname[l_currcha].c_str());
						wpk.WriteString(player1->m_motto[l_currcha].c_str());
						wpk.WriteShort(player1->m_icon[l_currcha]);

						playerlst[playernum] = player1;
						playernum++;
					}
				}
				l.unlock();

				SendToClient(playerlst, playernum, wpk);
			}
			//通知GameServer组成员变化
			{
				Team* l_team = invited->GetTeam();
				Player* playerr{nullptr};
				GateServer* l_gate[30];
				std::fill(std::begin(l_gate), std::end(l_gate), nullptr);
				char l_gtnum = 0;

				WPacket wpk = GetWPacket();
				wpk.WriteCmd(CMD_PM_TEAM);
				wpk.WriteChar(TEAM_MSG_ADD);
				wpk.WriteChar(uChar(l_count));
				RunChainGetArmor<TeamMember> l(*l_team);
				for (int i = 0; i < l_count && (playerr = static_cast<Player*>(l_team->GetNextItem())); i++) {
					wpk.WriteString(playerr->m_gate->m_name.c_str());
					wpk.WriteAddress(playerr->m_gtAddr);
					wpk.WriteLong(playerr->m_chaid[playerr->m_currcha]);
					for (int j = 0; j < 30; j++) {
						if (l_gate[j] == playerr->m_gate) {
							break;
						}
						if (!l_gate[j]) {
							l_gate[j] = playerr->m_gate;
							l_gtnum++;
							break;
						}
					}
				}
				l.unlock();
#if 1
				l_gtnum = l_gtnum ? 1 : 0;
#endif
				for (int j = 0; j < l_gtnum; j++) {
					l_gate[j]->GetDataSock()->SendData(wpk);
					LogLine l_line(g_LogMaster);
					l_line << newln << "MP_TEAM_CREATE() send ToGameServer data to GateServer";
				}
			}
		}
	} else {
		LogLine l_line(g_LogMaster);
		l_line << newln << "MP_TEAM_CREATE() invite failed";
	}
}
void GroupServerApp::CP_TEAM_ACCEPT(Player* ply, DataSocket* datasock, RPacket& pk) {
	const unsigned long l_inviter_chaid = pk.ReadLong();
	const long l_count = ply->JoinTeam(l_inviter_chaid);
	if (l_count && (l_count > const_team.MemberMax)) {
		ply->LeaveTeam();
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00017));
	} else if (l_count) {
		LogLine l_line(g_LogTeam);
		l_line << newln << "player " << ply->m_chaname[ply->m_currcha] << "add team" << ply->GetLeader()->m_chaname[ply->GetLeader()->m_currcha] << endln;

		//通知Client组成员变化
		{
			Team* l_team = ply->GetTeam();

			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PC_TEAM_REFRESH);
			wpk.WriteChar(TEAM_MSG_ADD);
			wpk.WriteChar(uChar(l_count));

			Player* playerlst[10240];
			short playernum = 0;

			Player* player1{nullptr};
			char l_currcha{0};
			RunChainGetArmor<TeamMember> l(*l_team);
			for (int i = 0; i < l_count && (player1 = static_cast<Player*>(l_team->GetNextItem())); i++) {
				if ((l_currcha = player1->m_currcha) >= 0) {
					wpk.WriteLong(player1->m_chaid[l_currcha]);
					wpk.WriteString(player1->m_chaname[l_currcha].c_str());
					wpk.WriteString(player1->m_motto[l_currcha].c_str());
					wpk.WriteShort(player1->m_icon[l_currcha]);

					playerlst[playernum] = player1;
					playernum++;
				}
			}
			l.unlock();

			SendToClient(playerlst, playernum, wpk);
		}

		//通知GameServer组成员变化
		{
			Team* l_team = ply->GetTeam();
			Player* playerr{nullptr};
			GateServer* l_gate[30];
			std::fill(std::begin(l_gate), std::end(l_gate), nullptr);
			char l_gtnum = 0;

			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PM_TEAM);
			wpk.WriteChar(TEAM_MSG_ADD);
			wpk.WriteChar(uChar(l_count));
			RunChainGetArmor<TeamMember> l(*l_team);
			for (int i = 0; i < l_count && (playerr = static_cast<Player*>(l_team->GetNextItem())); i++) {
				wpk.WriteString(playerr->m_gate->m_name.c_str());
				wpk.WriteAddress(playerr->m_gtAddr);
				wpk.WriteLong(playerr->m_chaid[playerr->m_currcha]);
				for (auto& gate : l_gate) {
					if (gate == playerr->m_gate) {
						break;
					}
					if (!gate) {
						gate = playerr->m_gate;
						++l_gtnum;
						break;
					}
				}
			}
			l.unlock();
#if 1
			l_gtnum = l_gtnum ? 1 : 0;
#endif
			for (int j = 0; j < l_gtnum; j++) {
				l_gate[j]->GetDataSock()->SendData(wpk);
				LogLine l_line(g_LogTeam);
				l_line << newln << "MP_TEAM_CREATE() send ToGameServer data to GateServer";
			}
		}
	}
}
void GroupServerApp::CP_TEAM_LEAVE(Player* ply, DataSocket* datasock, RPacket& pk) {
	Team* l_team = ply->GetTeam();
	if (!l_team) {
		return;
	}
	Player* l_leader = l_team->GetLeader();
	const long l_count = ply->LeaveTeam();
	if (l_count) {
		//通知Client组成员变化
		{
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PC_TEAM_REFRESH);
			wpk.WriteChar(TEAM_MSG_LEAVE);
			wpk.WriteChar(uChar(l_count));

			Player* playerlst[10240];
			short playernum = 0;

			Player* player1;
			char l_currcha;
			RunChainGetArmor<TeamMember> l(*l_team);
			for (int i = 0; i < l_count && ((player1 = static_cast<Player*>(l_team->GetNextItem())) || (player1 = ply)); i++) {
				if ((l_currcha = player1->m_currcha) >= 0) {
					wpk.WriteLong(player1->m_chaid[l_currcha]);
					wpk.WriteString(player1->m_chaname[l_currcha].c_str());
					wpk.WriteString(player1->m_motto[l_currcha].c_str());
					wpk.WriteShort(player1->m_icon[l_currcha]);

					playerlst[playernum] = player1;
					playernum++;
				}
			}
			l.unlock();
			SendToClient(playerlst, playernum, wpk);
		}
		//通知GameServer组成员变化
		{
			Player* playerr;
			GateServer* l_gate[30];
			std::fill(std::begin(l_gate), std::end(l_gate), nullptr);
			char l_gtnum = 0;

			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PM_TEAM);
			wpk.WriteChar(TEAM_MSG_LEAVE);
			wpk.WriteChar(uChar(l_count));
			RunChainGetArmor<TeamMember> l(*l_team);
			for (int i = 0; i < l_count && ((playerr = static_cast<Player*>(l_team->GetNextItem())) || (playerr = ply)); i++) {
				wpk.WriteString(playerr->m_gate->m_name.c_str());
				wpk.WriteAddress(playerr->m_gtAddr);
				wpk.WriteLong(playerr->m_chaid[playerr->m_currcha]);
				for (auto& gate : l_gate) {
					if (gate == playerr->m_gate) {
						break;
					}
					if (!gate) {
						gate = playerr->m_gate;
						++l_gtnum;
						break;
					}
				}
			}
			l.unlock();
#if 1
			l_gtnum = l_gtnum ? 1 : 0;
#endif
			for (int j = 0; j < l_gtnum; j++) {
				l_gate[j]->GetDataSock()->SendData(wpk);
			}

			if (l_count == 2) {
				l_team->GetLeader()->LeaveTeam();
			}
			LogLine l_line(g_LogTeam);
			l_line << newln << "player " << ply->m_chaname[ply->m_currcha] << "leave team "
				   << l_leader->m_chaname[l_leader->m_currcha] << (l_count == 2 ? ",free team." : ".")
				   << endln;
		}
	}
}
void GroupServerApp::CP_TEAM_KICK(Player* ply, DataSocket* datasock, RPacket& pk) {
	Team* l_team = ply->GetTeam();
	if (!l_team) {
		return;
	}
	Player* l_leader = l_team->GetLeader();
	if (ply != l_leader) {
		return;
	}
	const unsigned long dwKickedID = pk.ReadLong();
	if (dwKickedID == (ply->m_currcha >= 0 ? ply->m_chaid[ply->m_currcha] : 0)) {
		return;
	}
	Player* pKicker = l_team->GetMember(dwKickedID);
	if (!pKicker) {
		LogLine l_line(g_LogTeam);
		l_line << newln << "captain " << ply->m_chaname[ply->m_currcha] << "killed member not exsit! ID["
			   << dwKickedID << "]" << endln;
		return;
	}
	const long l_count = pKicker->LeaveTeam();
	if (l_count) {
		//通知Client组成员变化
		{
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PC_TEAM_REFRESH);
			wpk.WriteChar(TEAM_MSG_KICK);
			wpk.WriteChar(static_cast<unsigned char>(l_count));

			Player* playerlst[10240];
			short playernum = 0;

			Player* player1{nullptr};
			char l_currcha;
			RunChainGetArmor<TeamMember> l(*l_team);
			for (int i = 0; i < l_count && ((player1 = static_cast<Player*>(l_team->GetNextItem())) || (player1 = pKicker)); i++) {
				if ((l_currcha = player1->m_currcha) >= 0) {
					wpk.WriteLong(player1->m_chaid[l_currcha]);
					wpk.WriteString(player1->m_chaname[l_currcha].c_str());
					wpk.WriteString(player1->m_motto[l_currcha].c_str());
					wpk.WriteShort(player1->m_icon[l_currcha]);

					playerlst[playernum] = player1;
					playernum++;
				}
			}
			l.unlock();
			SendToClient(playerlst, playernum, wpk);
		}
		//通知GameServer组成员变化
		{
			Player* playerr{nullptr};
			GateServer* l_gate[30];
			std::fill(std::begin(l_gate), std::end(l_gate), nullptr);
			char l_gtnum = 0;

			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PM_TEAM);
			wpk.WriteChar(TEAM_MSG_LEAVE);
			wpk.WriteChar(uChar(l_count));
			RunChainGetArmor<TeamMember> l(*l_team);
			for (int i = 0; i < l_count && ((playerr = static_cast<Player*>(l_team->GetNextItem())) || (playerr = pKicker)); i++) {
				wpk.WriteString(playerr->m_gate->m_name.c_str());
				wpk.WriteAddress(playerr->m_gtAddr);
				wpk.WriteLong(playerr->m_chaid[playerr->m_currcha]);
				for (auto& gate : l_gate) {
					if (gate == playerr->m_gate) {
						break;
					}
					if (!gate) {
						gate = playerr->m_gate;
						++l_gtnum;
						break;
					}
				}
			}
			l.unlock();
#if 1
			l_gtnum = l_gtnum ? 1 : 0;
#endif
			for (int j = 0; j < l_gtnum; j++) {
				l_gate[j]->GetDataSock()->SendData(wpk);
			}

			if (l_count == 2) {
				l_team->GetLeader()->LeaveTeam();
			}
			LogLine l_line(g_LogTeam);
			l_line << newln << "player" << pKicker->m_chaname[ply->m_currcha] << "killed by captain"
				   << l_leader->m_chaname[l_leader->m_currcha] << (l_count == 2 ? ",free team." : ".")
				   << endln;
		}
	}
}
void Player::TeamInvitedCheck(Invited* invited) {
	Player* l_inviter = invited->m_ptinviter.m_ply;
	if (m_currcha < 0) {
		TeamEndInvited(l_inviter);
	} else if (l_inviter->m_currcha < 0 || l_inviter->m_chaid[l_inviter->m_currcha] != invited->m_ptinviter.m_chaid) {
		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_TEAM_CANCEL);
		wpk.WriteChar(MSG_TEAM_CANCLE_OFFLINE);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		TeamEndInvited(l_inviter);
	} else if (l_inviter->GetTeam() && l_inviter->GetTeam()->GetTotal() >= g_gpsvr->const_team.MemberMax) {
		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_TEAM_CANCEL);
		wpk.WriteChar(MSG_TEAM_CANCLE_ISFULL);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		TeamEndInvited(l_inviter);
	} else if (l_inviter->GetTeam() && l_inviter->GetLeader() != l_inviter) {
		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_TEAM_CANCEL);
		wpk.WriteChar(MSG_TEAM_CANCLE_CANCEL);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		TeamEndInvited(l_inviter);
	} else if (GetTeam() && GetTeam()->GetTotal() > 1) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPTEAM_CPP_00028), m_chaname[m_currcha].c_str());
		l_inviter->SendSysInfo(l_buf);

		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_TEAM_CANCEL);
		wpk.WriteChar(MSG_TEAM_CANCLE_CANCEL);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		TeamEndInvited(l_inviter);
	} else if (g_gpsvr->GetCurrentTick() - invited->m_tick >= g_gpsvr->const_team.PendTimeOut) {
		char l_buf[256];
		CFormatParameter param(2);
		param.setString(0, m_chaname[m_currcha].c_str());
		param.setLong(1, g_gpsvr->const_team.PendTimeOut / 1000);
		RES_FORMAT_STRING(GP_GROUPSERVERAPPTEAM_CPP_00029, param, l_buf);
		l_inviter->SendSysInfo(l_buf);

		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_TEAM_CANCEL);
		wpk.WriteChar(MSG_TEAM_CANCLE_TIMEOUT);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		TeamEndInvited(l_inviter);
	}
}
void GroupServerApp::MP_SWITCH(Player* ply) {
	Team* l_team = ply->GetTeam();
	if (!l_team) {
		return;
	}
	Player* l_leader = l_team->GetLeader();
	const long l_count = l_team->GetTotal();
	{
		Player* playerr;
		GateServer* l_gate[3];
		std::fill(std::begin(l_gate), std::end(l_gate), nullptr);
		char l_gtnum = 0;

		WPacket wpk = GetWPacket();
		wpk.WriteCmd(CMD_PM_TEAM);
		wpk.WriteChar(TEAM_MSG_UPDATE);
		wpk.WriteChar(uChar(l_count));
		RunChainGetArmor<TeamMember> l(*l_team);
		for (int i = 0; i < l_count && ((playerr = static_cast<Player*>(l_team->GetNextItem())) || (playerr = ply)); i++) {
			wpk.WriteString(playerr->m_gate->m_name.c_str());
			wpk.WriteAddress(playerr->m_gtAddr);
			wpk.WriteLong(playerr->m_chaid[playerr->m_currcha]);
			for (auto& gate : l_gate) {
				if (gate == playerr->m_gate) {
					break;
				}
				if (!gate) {
					gate = playerr->m_gate;
					++l_gtnum;
					break;
				}
			}
		}
		l.unlock();
#if 1
		l_gtnum = l_gtnum ? 1 : 0;
#endif
		for (int j = 0; j < l_gtnum; j++) {
			l_gate[j]->GetDataSock()->SendData(wpk);
		}

		LogLine l_line(g_LogTeam);
		l_line << newln << "player" << ply->m_chaname[ply->m_currcha] << "refresh team by switch map" << l_leader->m_chaname[l_leader->m_currcha];
	}
}
