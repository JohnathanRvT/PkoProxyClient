#include <iostream>
#include "GroupServerApp.h"
#include "GameCommon.h"

void GroupServerApp::CP_FRND_INVITE(Player* inviter_player, DataSocket* datasock, RPacket& pk) {
	if (inviter_player->m_CurrFriendNum >= const_frnd.FriendMax) {
		inviter_player->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00001));
		return;
	}

	unsigned short l_len;
	const char* invited_name = pk.ReadString(&l_len);
	if (!invited_name || l_len > CommConstants::Character::name_max_length) {
		return;
	}

	Invited* invited{nullptr};
	Player* invited_player = FindPlayerByChaName(invited_name);
	MutexArmor l_lockDB(m_mtxDB);
	if (!invited_player || invited_player->m_currcha < 0 || invited_player == inviter_player) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00002), invited_name);
		inviter_player->SendSysInfo(l_buf);
	} else if (invited = invited_player->FrndFindInvitedByInviterChaID(inviter_player->m_chaid[inviter_player->m_currcha])) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00003), invited_name);
		inviter_player->SendSysInfo(l_buf);
	} else if (invited = inviter_player->FrndFindInvitedByInviterChaID(invited_player->m_chaid[invited_player->m_currcha])) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00005), invited_name);
		inviter_player->SendSysInfo(l_buf);
	} else if (invited_player->m_CurrFriendNum >= const_frnd.FriendMax) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00006), invited_name);
		inviter_player->SendSysInfo(l_buf);
	} else if (m_tblfriends->GetFriendsCount(inviter_player->m_chaid[inviter_player->m_currcha], invited_player->m_chaid[invited_player->m_currcha]) > 0) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00007), invited_name);
		inviter_player->SendSysInfo(l_buf);
	} else {
		PtInviter l_ptinviter = invited_player->FrndBeginInvited(inviter_player);
		if (l_ptinviter) {
			char l_buf[256];
			_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00009), invited_name);
			l_ptinviter->SendSysInfo(l_buf);

			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PC_FRND_CANCEL);
			wpk.WriteChar(MSG_FRND_CANCLE_BUSY);
			wpk.WriteLong(l_ptinviter.m_chaid);
			SendToClient(invited_player, wpk);
		}
		WPacket wpk = GetWPacket();
		wpk.WriteCmd(CMD_PC_FRND_INVITE);
		wpk.WriteString(inviter_player->m_chaname[inviter_player->m_currcha].c_str());
		wpk.WriteLong(inviter_player->m_chaid[inviter_player->m_currcha]);
		wpk.WriteShort(inviter_player->m_icon[inviter_player->m_currcha]);
		SendToClient(invited_player, wpk);
	}
}
void GroupServerApp::CP_FRND_REFUSE(Player* ply, DataSocket* datasock, RPacket& pk) {
	const unsigned long l_inviter_chaid = pk.ReadLong();
	PtInviter l_inviter = ply->FrndEndInvited(l_inviter_chaid);
	if (l_inviter && l_inviter->m_currcha >= 0 && l_inviter.m_chaid == l_inviter->m_chaid[l_inviter->m_currcha]) {
		char l_buf[256];
		_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00010), ply->m_chaname[ply->m_currcha].c_str());
		l_inviter->SendSysInfo(l_buf);
	}
}
void GroupServerApp::CP_FRND_ACCEPT(Player* ply, DataSocket* datasock, RPacket& pk) {
	uLong l_inviter_chaid = pk.ReadLong();
	PtInviter l_inviter = ply->FrndEndInvited(l_inviter_chaid);
	if (l_inviter && l_inviter->m_currcha >= 0 && l_inviter.m_chaid == l_inviter->m_chaid[l_inviter->m_currcha]) {
		MutexArmor l_lockDB(m_mtxDB);
		if ((++(ply->m_CurrFriendNum)) > const_frnd.FriendMax) {
			--(ply->m_CurrFriendNum);
			ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00011));
		} else if ((++(l_inviter->m_CurrFriendNum)) > const_frnd.FriendMax) {
			--(ply->m_CurrFriendNum);
			--(l_inviter->m_CurrFriendNum);
			char l_buf[256];
			_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00012), l_inviter->m_chaname[l_inviter->m_currcha].c_str());
			ply->SendSysInfo(l_buf);
		} else if (m_tblfriends->GetFriendsCount(ply->m_chaid[ply->m_currcha], l_inviter->m_chaid[l_inviter->m_currcha]) > 0) {
			--(ply->m_CurrFriendNum);
			--(l_inviter->m_CurrFriendNum);
			char l_buf[256];
			_snprintf_s(l_buf, sizeof(l_buf), _TRUNCATE, RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00007), l_inviter->m_chaname[l_inviter->m_currcha].c_str());
			ply->SendSysInfo(l_buf);
		} else {
			LogLine l_line(g_LogFriend);
			l_line << newln << "player" << ply->m_chaname[ply->m_currcha] << "(" << ply->m_chaid[ply->m_currcha]
				   << ")and player" << l_inviter->m_chaname[l_inviter->m_currcha] << "(" << l_inviter_chaid << ") make friends"
				   << endln;

			if (m_tblfriends->GroupIsExsit(ply->m_chaid[ply->m_currcha], -1, Friend::GetStandardGroup())) {
				m_tblfriends->AddGroup(ply->m_chaid[ply->m_currcha], Friend::GetStandardGroup());
			}

			m_tblfriends->AddFriend(ply->m_chaid[ply->m_currcha], l_inviter.m_chaid);
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PC_FRND_REFRESH);
			wpk.WriteChar(MSG_FRND_REFRESH_ADD);
			wpk.WriteString(Friend::GetStandardGroup());
			WPacket wpk2 = wpk;
			wpk.WriteLong(ply->m_chaid[ply->m_currcha]);
			wpk.WriteString(ply->m_chaname[ply->m_currcha].c_str());
			wpk.WriteString(ply->m_motto[ply->m_currcha].c_str());
			wpk.WriteShort(ply->m_icon[ply->m_currcha]);
			SendToClient(l_inviter.m_ply, wpk);
			wpk2.WriteLong(l_inviter->m_chaid[l_inviter->m_currcha]);
			wpk2.WriteString(l_inviter->m_chaname[l_inviter->m_currcha].c_str());
			wpk2.WriteString(l_inviter->m_motto[l_inviter->m_currcha].c_str());
			wpk2.WriteShort(l_inviter->m_icon[l_inviter->m_currcha]);
			SendToClient(ply, wpk2);
		}
	}
}
void GroupServerApp::CP_FRND_DELETE(Player* ply, DataSocket* datasock, RPacket& pk) {
	uLong l_deleted_chaid = pk.ReadLong();
	MutexArmor l_lockDB(m_mtxDB);
	if (m_tblfriends->GetFriendsCount(ply->m_chaid[ply->m_currcha], l_deleted_chaid) < 1) {
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00018));
	} else {
		auto l_deleted_ply = ToPointer<Player>(m_tblfriends->GetFriendAddr(ply->m_chaid[ply->m_currcha], l_deleted_chaid));
		if (l_deleted_ply && l_deleted_ply->m_currcha >= 0) {
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PC_FRND_REFRESH);
			wpk.WriteChar(MSG_FRND_REFRESH_DEL);
			WPacket wpk2 = wpk;

			wpk.WriteLong(ply->m_chaid[ply->m_currcha]);
			SendToClient(l_deleted_ply, wpk);
			--(l_deleted_ply->m_CurrFriendNum);

			wpk2.WriteLong(l_deleted_chaid);
			SendToClient(ply, wpk2);
			--(ply->m_CurrFriendNum);
		} else {
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PC_FRND_REFRESH);
			wpk.WriteChar(MSG_FRND_REFRESH_DEL);

			wpk.WriteLong(l_deleted_chaid);
			SendToClient(ply, wpk);
			--(ply->m_CurrFriendNum);
		}
		m_tblfriends->DelFriend(ply->m_chaid[ply->m_currcha], l_deleted_chaid);
		LogLine l_line(g_LogFriend);
		l_line << newln << "player" << ply->m_chaname[ply->m_currcha] << "(" << ply->m_chaid[ply->m_currcha]
			   << ")and(" << l_deleted_chaid << ")free friends";
	}
}

void GroupServerApp::CP_FRND_DEL_GROUP(Player* ply, DataSocket* datasock, RPacket& pk) {
	unsigned short l_len;
	const char* l_grp = pk.ReadString(&l_len);
	if (!l_grp || l_len > CommConstants::Chat::groupname_max_length || !IsValidName(l_grp, l_len)) {
		return;
	}

	if (strcmp(l_grp, Friend::GetStandardGroup()) == 0) {
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00024));
		return;
	}

	MutexArmor l_lockDB(m_mtxDB);
	if (!strchr(l_grp, '\'') && (m_tblfriends->GetFriendsCount(ply->m_chaid[ply->m_currcha], -1, l_grp) == 0)) {
		if (m_tblfriends->DeleteGroup(ply->m_chaid[ply->m_currcha], l_grp)) {
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_PC_FRND_DEL_GROUP);
			wpk.WriteString(l_grp);
			SendToClient(ply, wpk);
		}
	}
}

void GroupServerApp::CP_FRND_ADD_GROUP(Player* ply, DataSocket* datasock, RPacket& pk) {
	unsigned short l_len;
	const char* l_grp = pk.ReadString(&l_len);
	if (!l_grp || l_len > CommConstants::Chat::groupname_max_length || !IsValidName(l_grp, l_len)) {
		return;
	}

	if (strcmp(l_grp, Friend::GetStandardGroup()) == 0) {
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00025));
		return;
	}

	MutexArmor l_lockDB(m_mtxDB);
	if (!strchr(l_grp, '\'')) {
		if (m_tblfriends->GroupIsExsit(ply->m_chaid[ply->m_currcha], -1, l_grp)) {
			ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00026));
		}

		const int l_grpnum = m_tblfriends->GetGroupCount(ply->m_chaid[ply->m_currcha]);
		if (l_grpnum < 0 || l_grpnum >= const_frnd.FriendGroupMax) {
			ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00021));
		} else {
			if (m_tblfriends->AddGroup(ply->m_chaid[ply->m_currcha], l_grp)) {
				WPacket wpk = GetWPacket();
				wpk.WriteCmd(CMD_PC_FRND_ADD_GROUP);
				wpk.WriteString(l_grp);
				SendToClient(ply, wpk);
			}
		}
	}
}

void GroupServerApp::CP_FRND_MOVE_GROUP(Player* ply, DataSocket* datasock, RPacket& pk) {
	unsigned short l_len1;
	unsigned short l_len2;
	const long l_changed_chaid = pk.ReadLong();
	const char* l_grp1 = pk.ReadString(&l_len1);
	const char* l_grp2 = pk.ReadString(&l_len2);

	if (!l_grp1 || !l_grp2 ||
		l_len1 > CommConstants::Chat::groupname_max_length || l_len2 > CommConstants::Chat::groupname_max_length ||
		!IsValidName(l_grp1, l_len1) || !IsValidName(l_grp2, l_len2)) {
		return;
	}

	MutexArmor l_lockDB(m_mtxDB);
	if (!strchr(l_grp1, '\'') && !strchr(l_grp2, '\'')) {
		if (
			(m_tblfriends->GroupIsExsit(ply->m_chaid[ply->m_currcha], l_changed_chaid, l_grp1) ||
			 (strcmp(l_grp1, Friend::GetStandardGroup()) == 0)) &&
			((m_tblfriends->GroupIsExsit(ply->m_chaid[ply->m_currcha], -1, l_grp2)) ||
			 (strcmp(l_grp2, Friend::GetStandardGroup()) == 0))) { //FIXED: Now friends from other friend groups can be moved from or to "Friends" group
			if (m_tblfriends->MoveGroup(ply->m_chaid[ply->m_currcha], l_changed_chaid, l_grp1, l_grp2)) {
				WPacket wpk = GetWPacket();
				wpk.WriteCmd(CMD_PC_FRND_MOVE_GROUP);
				wpk.WriteLong(l_changed_chaid);
				wpk.WriteString(l_grp1);
				wpk.WriteString(l_grp2);
				SendToClient(ply, wpk);
			}
		} else {
			ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00027));
		}
	}
}

void GroupServerApp::CP_FRND_CHANGE_GROUP(Player* ply, DataSocket* datasock, RPacket& pk) {
	unsigned short l_len1;
	unsigned short l_len2;
	const char* l_grp1 = pk.ReadString(&l_len1);
	const char* l_grp2 = pk.ReadString(&l_len2);

	if (!l_grp1 || !l_grp2 ||
		l_len1 > CommConstants::Chat::groupname_max_length || l_len2 > CommConstants::Chat::groupname_max_length ||
		!IsValidName(l_grp1, l_len1) || !IsValidName(l_grp2, l_len2)) {
		return;
	}

	if (strcmp(l_grp1, Friend::GetStandardGroup()) == 0) {
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00028));
		return;
	}

	if (strcmp(l_grp2, Friend::GetStandardGroup()) == 0) {
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00028));
		return;
	}

	MutexArmor l_lockDB(m_mtxDB);
	if (!strchr(l_grp1, '\'') && !strchr(l_grp2, '\'')) {
		if (!m_tblfriends->GroupIsExsit(ply->m_chaid[ply->m_currcha], -1, l_grp1) || m_tblfriends->GroupIsExsit(ply->m_chaid[ply->m_currcha], -1, l_grp2)) {
			ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00029));
			return;
		}

		if (m_tblfriends->GetFriendsCount(ply->m_chaid[ply->m_currcha], -1, l_grp1) > 0) {
			ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00029));
			return;
		}

		const int l_grpnum = m_tblfriends->GetGroupCount(ply->m_chaid[ply->m_currcha]);
		if (l_grpnum < 0 || l_grpnum > const_frnd.FriendGroupMax) {
			//ply->SendSysInfo("您当前的好友分组数已经达到系统允许上限了！");
			ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00021));
		} else {
			if (m_tblfriends->UpdateGroup(ply->m_chaid[ply->m_currcha], l_grp1, l_grp2)) {
				WPacket wpk = GetWPacket();
				wpk.WriteCmd(CMD_PC_FRND_CHANGE_GROUP);
				wpk.WriteString(l_grp1);
				wpk.WriteString(l_grp2);
				SendToClient(ply, wpk);
			}
		}
	}
}
void Player::FrndInvitedCheck(Invited* invited) {
	Player* l_inviter = invited->m_ptinviter.m_ply;
	if (m_currcha < 0) {
		FrndEndInvited(l_inviter);
	} else if (l_inviter->m_currcha < 0 || l_inviter->m_chaid[l_inviter->m_currcha] != invited->m_ptinviter.m_chaid) {
		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_FRND_CANCEL);
		wpk.WriteChar(MSG_FRND_CANCLE_OFFLINE);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		FrndEndInvited(l_inviter);
	} else if (l_inviter->m_CurrFriendNum >= g_gpsvr->const_frnd.FriendMax) {
		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_FRND_CANCEL);
		wpk.WriteChar(MSG_FRND_CANCLE_INVITER_ISFULL);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		FrndEndInvited(l_inviter);
	} else if (m_CurrFriendNum >= g_gpsvr->const_frnd.FriendMax) {
		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_FRND_CANCEL);
		wpk.WriteChar(MSG_FRND_CANCLE_SELF_ISFULL);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		FrndEndInvited(l_inviter);
	} else if (g_gpsvr->GetCurrentTick() - invited->m_tick >= g_gpsvr->const_frnd.PendTimeOut) {
		char l_buf[256];
		//sprintf(l_buf,"你对【%s】的好友邀请已超过%d秒钟没有回应，系统自动取消了你的邀请。",m_chaname[m_currcha].c_str(),g_gpsvr->const_frnd.PendTimeOut/1000);
		//sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00022),m_chaname[m_currcha].c_str(),g_gpsvr->const_frnd.PendTimeOut/1000);
		//_snprintf_s(l_buf,sizeof(l_buf),_TRUNCATE,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00022),m_chaname[m_currcha].c_str(),g_gpsvr->const_frnd.PendTimeOut/1000);
		CFormatParameter param(2);
		param.setString(0, m_chaname[m_currcha].c_str());
		param.setLong(1, g_gpsvr->const_frnd.PendTimeOut / 1000);
		RES_FORMAT_STRING(GP_GROUPSERVERAPPFRND_CPP_00022, param, l_buf);
		l_inviter->SendSysInfo(l_buf);

		WPacket wpk = g_gpsvr->GetWPacket();
		wpk.WriteCmd(CMD_PC_FRND_CANCEL);
		wpk.WriteChar(MSG_FRND_CANCLE_TIMEOUT);
		wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this, wpk);
		FrndEndInvited(l_inviter);
	}
}
/*	玩家上线时提取好友名单的SQL语句
cha_id-上线角色ID
select '' relation,count(*) addr,0 cha_id,'' cha_name,0 icon,'' motto from ( 
select distinct friends.relation relation from character INNER JOIN 
friends ON character.cha_id = friends.cha_id2 where friends.cha_id1 = 240 
) cc 

union select '' cha_name,0 addr, -1 cha_id,friends.relation relation,0 icon,'' motto from friends 
where friends.cha_id1 = 240 and friends.cha_id2 = -1

union select friends.relation relation,count(character.mem_addr) addr,0 
cha_id,'' cha_name,1 icon,'' motto from character INNER JOIN friends ON 
character.cha_id = friends.cha_id2 where friends.cha_id1 = 240 group by relation 
union select friends.relation relation,character.mem_addr addr,character.cha_id 
cha_id,character.cha_name cha_name,character.icon icon,character.motto motto 
from character INNER JOIN friends ON character.cha_id = friends.cha_id2 
where friends.cha_id1 = 240 order by relation,cha_id,icon  

*/
void GroupServerApp::PC_FRND_INIT(Player* ply) {
	friend_dat l_farray[210];
	int l_num = 210;
	m_tblX1->get_friend_dat(l_farray, l_num, ply->m_chaid[ply->m_currcha]);

	WPacket l_toFrnd = GetWPacket();
	l_toFrnd.WriteCmd(CMD_PC_FRND_REFRESH);
	l_toFrnd.WriteChar(MSG_FRND_REFRESH_ONLINE);
	l_toFrnd.WriteLong(ply->m_chaid[ply->m_currcha]);

	WPacket l_toSelf = GetWPacket();
	l_toSelf.WriteCmd(CMD_PC_FRND_REFRESH);
	l_toSelf.WriteChar(MSG_FRND_REFRESH_START);

	l_toSelf.WriteLong(ply->m_chaid[ply->m_currcha]);
	l_toSelf.WriteString(ply->m_chaname[ply->m_currcha].c_str());
	l_toSelf.WriteString(ply->m_motto[ply->m_currcha].c_str());
	l_toSelf.WriteShort(ply->m_icon[ply->m_currcha]);

	// Add by lark.li 20080804 begin
	int groupNum = 0;
	for (int i = 0; i < l_num; i++) {
		if (l_farray[i].cha_id == -1) {
			groupNum++;
		}
	}

	if (groupNum > 10)
		groupNum = 10;

	l_toSelf.WriteShort(groupNum);

	for (int i = 0; i < l_num; i++) {
		if (l_farray[i].cha_id == -1) {
			l_toSelf.WriteString(l_farray[i].cha_name.c_str());
		}
	}
	// End

	ply->m_CurrFriendNum = 0;

	Player* playerlst[10240];
	short playernum = 0;

	Player* player1;
	char l_currcha;
	for (int i = 0; i < l_num; i++) {
		// Add by lark.li 20080804 begin
		if (l_farray[i].cha_id == -1)
			continue;
		// End

		if (l_farray[i].cha_id == 0) {
			if (l_farray[i].icon_id == 0) {
				l_toSelf.WriteShort(uShort(l_farray[i].memaddr));
			} else {
				l_toSelf.WriteString(l_farray[i].relation.c_str());
				l_toSelf.WriteShort(uShort(l_farray[i].memaddr));
				ply->m_CurrFriendNum += l_farray[i].memaddr;
			}
		} else if ((player1 = ToPointer<Player>(l_farray[i].memaddr)) &&
				   ((l_currcha = player1->m_currcha) >= 0) &&
				   (player1->m_chaid[l_currcha] == l_farray[i].cha_id)) {
			playerlst[playernum] = player1;
			playernum++;

			l_toSelf.WriteLong(l_farray[i].cha_id);
			l_toSelf.WriteString(l_farray[i].cha_name.c_str());
			l_toSelf.WriteString(l_farray[i].motto.c_str());
			l_toSelf.WriteShort(l_farray[i].icon_id);
			l_toSelf.WriteChar(1);
		} else {
			l_toSelf.WriteLong(l_farray[i].cha_id);
			l_toSelf.WriteString(l_farray[i].cha_name.c_str());
			l_toSelf.WriteString(l_farray[i].motto.c_str());
			l_toSelf.WriteShort(l_farray[i].icon_id);
			l_toSelf.WriteChar(0);
		}
	}
	SendToClient(ply, l_toSelf);
	LogLine l_line(g_LogFriend);
	l_line << newln << "online friends num:" << playernum << endln;
	SendToClient(playerlst, playernum, l_toFrnd);
}
