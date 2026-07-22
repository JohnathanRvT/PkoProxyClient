#include <iostream>
#include "GroupServerApp.h"
#include "GameCommon.h"

void GroupServerApp::CP_QUERY_PERSONINFO(Player* ply, DataSocket* datasock, RPacket& pk) {
	stQueryPersonInfo info;

	unsigned short l_len;
	strncpy_s(info.sChaName, sizeof(info.sChaName), pk.ReadString(&l_len), _TRUNCATE);
	info.bHavePic = (pk.ReadChar() == 1 ? true : false);
	strncpy_s(info.cSex, sizeof(info.cSex), pk.ReadString(&l_len), _TRUNCATE);
	info.nMinAge[0] = pk.ReadLong();
	info.nMinAge[1] = pk.ReadLong();
	strncpy_s(info.szAnimalZodiac, sizeof(info.szAnimalZodiac), pk.ReadString(&l_len), _TRUNCATE);
	info.iBirthday[0] = pk.ReadLong();
	info.iBirthday[1] = pk.ReadLong();
	strncpy_s(info.szState, sizeof(info.szState), pk.ReadString(&l_len), _TRUNCATE);
	strncpy_s(info.szCity, sizeof(info.szCity), pk.ReadString(&l_len), _TRUNCATE);
	strncpy_s(info.szConstellation, sizeof(info.szConstellation), pk.ReadString(&l_len), _TRUNCATE);
	strncpy_s(info.szCareer, sizeof(info.szCareer), pk.ReadString(&l_len), _TRUNCATE);
	info.nPageItemNum = pk.ReadLong();
	info.nCurPage = pk.ReadLong();

	int num;
	int totalpage;
	int totalrecord;

	MutexArmor l_lockDB(m_mtxDB);

	stQueryResoultPersonInfo result[10];
	m_tblpersoninfo->Query(&info, result, num, totalpage, totalrecord);
	l_lockDB.unlock();

	WPacket wpk = GetWPacket();
	wpk.WriteCmd(CMD_PC_QUERY_PERSONINFO);
	wpk.WriteShort(num);
	wpk.WriteLong(totalpage);
	wpk.WriteLong(totalrecord);

	for (int i = 0; i < num; i++) {
		wpk.WriteString(result[i].sChaName);
		wpk.WriteShort(result[i].nMinAge);
		wpk.WriteString(result[i].cSex);
		wpk.WriteString(result[i].szState);
		wpk.WriteString(result[i].nCity);
	}
	SendToClient(ply, wpk);
}

// End

void GroupServerApp::CP_CHANGE_PERSONINFO(Player* ply, DataSocket* datasock, RPacket& pk) {
	stPersonInfo info;

	unsigned short l_len;
	strncpy_s(info.szMotto, sizeof(info.szMotto), pk.ReadString(&l_len), _TRUNCATE);
	info.bShowMotto = (pk.ReadChar() == 1 ? true : false);
	strncpy_s(info.szSex, sizeof(info.szSex), pk.ReadString(&l_len), _TRUNCATE);
	info.sAge = (short)pk.ReadLong();
	strncpy_s(info.szName, sizeof(info.szName), pk.ReadString(&l_len), _TRUNCATE);
	strncpy_s(info.szAnimalZodiac, sizeof(info.szAnimalZodiac), pk.ReadString(&l_len), _TRUNCATE);
	strncpy_s(info.szBloodType, sizeof(info.szBloodType), pk.ReadString(&l_len), _TRUNCATE);
	info.iBirthday = pk.ReadLong();
	strncpy_s(info.szState, sizeof(info.szState), pk.ReadString(&l_len), _TRUNCATE);
	strncpy_s(info.szCity, sizeof(info.szCity), pk.ReadString(&l_len), _TRUNCATE);
	strncpy_s(info.szConstellation, sizeof(info.szConstellation), pk.ReadString(&l_len), _TRUNCATE);
	strncpy_s(info.szCareer, sizeof(info.szCareer), pk.ReadString(&l_len), _TRUNCATE);
	info.iSize = pk.ReadLong();

	if (info.iSize > 0 && info.iSize < 8 * 1024) {
		memcpy(info.pAvatar, pk.ReadSequence(l_len), info.iSize);
	}

	info.bPprevent = (pk.ReadChar() == 1 ? true : false);
	info.iSupport = pk.ReadLong();
	info.iOppose = pk.ReadLong();

	MutexArmor l_lockDB(m_mtxDB);
	ply->m_refuse_sess = info.bPprevent ? true : false;
	m_tblpersoninfo->DelInfo(ply->m_chaid[ply->m_currcha]);
	m_tblpersonavatar->DelInfo(ply->m_chaid[ply->m_currcha]);
	m_tblpersoninfo->AddInfo(ply->m_chaid[ply->m_currcha], &info);
	m_tblpersonavatar->AddInfo(ply->m_chaid[ply->m_currcha], &info);

	//
	const char* l_motto = pk.ReadString(&l_len);
	m_tblcharaters->UpdateInfo(ply->m_chaid[ply->m_currcha], 1, l_motto);

	l_lockDB.unlock();

	WPacket wpk = GetWPacket();
	wpk.WriteCmd(CMD_PC_CHANGE_PERSONINFO);
	wpk.WriteString(l_motto);
	wpk.WriteShort(1);
	wpk.WriteChar(ply->m_refuse_sess ? 1 : 0);
	SendToClient(ply, wpk);

	//uShort	l_len;
	//cChar *l_motto	=pk.ReadString(&l_len);
	//if(!l_motto ||l_len >16 || !IsValidName(l_motto,l_len))
	//{
	//	return;
	//}
	//uShort		l_icon	=pk.ReadShort();
	//if(l_icon >const_cha.MaxIconVal)
	//{
	//	ply->SendSysInfo("个性图标值非法");
	//	ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPPRSN_CPP_00001));
	//}else if(strchr(l_motto,'\'') || strlen(l_motto) !=l_len || !CTextFilter::IsLegalText(CTextFilter::NAME_TABLE,l_motto))
	//{
	//	ply->SendSysInfo("座右铭格式非法");
	//	ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPPRSN_CPP_00002));
	//}else
	//{
	//	MutexArmor l_lockDB(m_mtxDB);
	//	ply->m_refuse_sess	=pk.ReadChar()?true:false;
	//	m_tblcharaters->UpdateInfo(ply->m_chaid[ply->m_currcha],l_icon,l_motto);
	//	l_lockDB.unlock();

	//	WPacket	wpk	=GetWPacket();
	//	wpk.WriteCmd(CMD_PC_CHANGE_PERSONINFO);
	//	wpk.WriteString(l_motto);
	//	wpk.WriteShort(l_icon);
	//	wpk.WriteChar(ply->m_refuse_sess?1:0);
	//	SendToClient(ply,wpk);
	//}
	// End
}
void GroupServerApp::CP_FRND_REFRESH_INFO(Player* ply, DataSocket* datasock, RPacket& pk) {
	const unsigned long l_chaid = pk.ReadLong();
	MutexArmor l_lockDB(m_mtxDB);

	if (ply->m_chaid[ply->m_currcha] != l_chaid) {
		if (m_tblfriends->GetFriendsCount(ply->m_chaid[ply->m_currcha], l_chaid) != 2) {
			l_lockDB.unlock();
			ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPPRSN_CPP_00003));
			return;
		}
	}

	stPersonInfo st;

	if (m_tblcharaters->FetchRowByChaID(l_chaid) == 1) {
		WPacket wpk = GetWPacket();
		wpk.WriteCmd(CMD_PC_FRND_REFRESH_INFO);

		std::string act_name = "";
		if (m_tblcharaters->FetchActNameRowByChaID(l_chaid, act_name) != 1) {
			act_name = "";
		}
		wpk.WriteString(act_name.c_str());

		wpk.WriteLong(l_chaid);

		wpk.WriteString(m_tblcharaters->GetMotto());
		wpk.WriteShort(m_tblcharaters->GetIcon());
		wpk.WriteShort(m_tblcharaters->GetDegree());
		wpk.WriteString(m_tblcharaters->GetJob());
		wpk.WriteString(m_tblcharaters->GetGuildName());

		if (m_tblpersoninfo->GetInfo(l_chaid, &st) && m_tblpersonavatar->GetInfo(l_chaid, &st)) {
			wpk.WriteChar(true);
			wpk.WriteString(st.szMotto);			 // 签名
			wpk.WriteChar(st.bShowMotto);			 // 显示签名开关
			wpk.WriteString(st.szSex);				 // 性别
			wpk.WriteLong(st.sAge);					 // 年龄
			wpk.WriteString(st.szName);				 // 名字
			wpk.WriteString(st.szAnimalZodiac);		 // 属相
			wpk.WriteString(st.szBloodType);		 // 血型
			wpk.WriteLong(st.iBirthday);			 // 生日
			wpk.WriteString(st.szState);			 // 州（省）
			wpk.WriteString(st.szCity);				 // 城市（区）
			wpk.WriteString(st.szConstellation);	 // 星座
			wpk.WriteString(st.szCareer);			 // 职业
			wpk.WriteLong(st.iSize);				 // 头像大小
			wpk.WriteSequence(st.pAvatar, st.iSize); // 头像
			wpk.WriteChar(st.bPprevent);			 // 是否阻止消息
			wpk.WriteLong(st.iSupport);				 // 鲜花数
			wpk.WriteLong(st.iOppose);				 // 臭鸡蛋数
		} else {
			wpk.WriteChar(false);
		}

		l_lockDB.unlock();
		SendToClient(ply, wpk);
	} else {
		ply->SendSysInfo("Failed!");
	}

	//uLong	l_chaid	=pk.ReadLong();
	//MutexArmor l_lockDB(m_mtxDB);
	//if(m_tblfriends->GetFriendsCount(ply->m_chaid[ply->m_currcha],l_chaid) !=2)
	//{
	//	l_lockDB.unlock();
	//	//ply->SendSysInfo("你们不是好友关系！");
	//	ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPPRSN_CPP_00003));
	//}else if(m_tblcharaters->FetchRowByChaID(l_chaid) ==1)
	//{
	//	WPacket	wpk	=GetWPacket();
	//	wpk.WriteCmd(CMD_PC_FRND_REFRESH_INFO);
	//	wpk.WriteLong(l_chaid);
	//	wpk.WriteString(m_tblcharaters->GetMotto());
	//	wpk.WriteShort(m_tblcharaters->GetIcon());
	//	wpk.WriteShort(m_tblcharaters->GetDegree());
	//	wpk.WriteString(m_tblcharaters->GetJob());
	//	wpk.WriteString(m_tblcharaters->GetGuildName());
	//	l_lockDB.unlock();
	//	SendToClient(ply,wpk);
	//}
	// End
}
void GroupServerApp::CP_REFUSETOME(Player* ply, DataSocket* datasock, RPacket& pk) {
	ply->m_refuse_tome = pk.ReadChar() ? true : false;
}
