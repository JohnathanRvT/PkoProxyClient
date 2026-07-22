#include "Friend.h"
#include "Master.h"
#include "DBConnect.h"
#include "Guild.h"
#include "GroupServerApp.h"
#include "GameCommon.h"

SQLRETURN Exec_sql_direct(const char* pszSQL, cfl_rs* pTable) {
	LG("group_sql", "Table [%s], begin execute SQL [%s]\n", pTable->get_table(), pszSQL);
	const SQLRETURN r = pTable->exec_sql_direct(pszSQL);
	if (DBOK(r)) {
		LG("group_sql", "execute SQL success!");
	} else if (DBNODATA(r)) {
		LG("group_sql", "execute SQL, no result \n");
	} else {
		LG("group_sql", "execute SQL, failed!\n");
	}
	return r;
}

SQLRETURN Exec_sql(const char* pszSQL, char const* pdata, size_t len, cfl_rs* pTable) {
	LG("group_sql", "Table [%s], begin execute SQL [%s]\n", pTable->get_table(), pszSQL);
	const SQLRETURN r = pTable->exec_param_sql(pszSQL, pdata, len);
	if (DBOK(r)) {
		LG("group_sql", "execute SQL success!");
	} else if (DBNODATA(r)) {
		LG("group_sql", "execute SQL, no result \n");
	} else {
		LG("group_sql", "execute SQL, failed!\n");
	}
	return r;
}

//==========TBLSystem===============================
bool TBLAccounts::IsReady() {
	char sql[SQL_MAXLEN];
	strncpy_s(sql, sizeof(sql), "drop trigger [TR_D_Character_Friends]", _TRUNCATE);
	SQLRETURN l_ret = Exec_sql_direct(sql, this);
	if (!DBOK(l_ret)) {
		LogLine l_line(g_LogDB);
		l_line << newln << "SQL:" << sql << " execute failed !";
	}
	strncpy_s(sql, sizeof(sql), "drop trigger [TR_I_Character]", _TRUNCATE);
	l_ret = Exec_sql_direct(sql, this);
	if (!DBOK(l_ret)) {
		LogLine l_line(g_LogDB);
		l_line << newln << "SQL:" << sql << " execute failed !";
	}
	//strcpy(sql,"CREATE TRIGGER TR_D_Character_Friends ON character \n\
	//			FOR DELETE \n\
	//			AS\n\
	//			BEGIN\n\
	//				declare @@stat tinyint\n\
	//				declare @@gid  int\n\
	//				select @@stat =guild_stat,@@gid =guild_id from deleted\n\
	//				DELETE friends where friends.cha_id1 IN(select cha_id from deleted)\n\
	//				if(@@gid >0)\n\
	//				BEGIN\n\
	//					update guild set try_total =try_total -(case when @@stat>0 then 1 else 0 end),\n\
	//							member_total =member_total -(case when @@stat >0 then 0 else 1 end)\n\
	//						where guild_id >0 and guild_id =@@gid\n\
	//				END\n\
	//			END\n\
	//	");
	strncpy_s(sql, sizeof(sql), "CREATE TRIGGER TR_D_Character_Friends ON character \n\
			   FOR DELETE \n\
			   AS\n\
			   BEGIN\n\
			   declare @@stat tinyint\n\
			   declare @@gid  int\n\
			   select @@stat =guild_stat,@@gid =guild_id from deleted\n\
			   DELETE friends where friends.cha_id1 IN(select cha_id from deleted)\n\
			   if(@@gid >0)\n\
			   BEGIN\n\
			   update guild set try_total =try_total -(case when @@stat>0 then 1 else 0 end),\n\
			   member_total =member_total -(case when @@stat >0 then 0 else 1 end)\n\
			   where guild_id >0 and guild_id =@@gid\n\
			   END\n\
			   END\n\
			   ",
			  _TRUNCATE);
	l_ret = Exec_sql_direct(sql, this);
	if (!DBOK(l_ret)) {
		LogLine l_line(g_LogDB);
		l_line << newln << "SQL:" << sql << " execute failed !";
		return false;
	}
	//strcpy(sql,"CREATE TRIGGER TR_I_Character ON character\n\
	//			FOR INSERT\n\
	//			AS\n\
	//			BEGIN\n\
	//				declare @l_icon smallint\n\
	//				select @l_icon =convert(smallint,SUBSTRING(inserted.look,5,1)) from inserted\n\
	//				update character set icon =@l_icon where cha_id in (select cha_id from inserted)\n\
	//			END\n\
	//	");
	strncpy_s(sql, sizeof(sql), "CREATE TRIGGER TR_I_Character ON character\n\
			   FOR INSERT\n\
			   AS\n\
			   BEGIN\n\
			   declare @l_icon smallint\n\
			   select @l_icon =convert(smallint,SUBSTRING(inserted.look,5,1)) from inserted\n\
			   update character set icon =@l_icon where cha_id in (select cha_id from inserted)\n\
			   END\n\
			   ",
			  _TRUNCATE);

	l_ret = Exec_sql_direct(sql, this);
	if (!DBOK(l_ret)) {
		LogLine l_line(g_LogDB);
		l_line << newln << "SQL:" << sql << " execute failed !";
		return false;
	}
	return true;
} /*
int TBLSystem::Increment()
{
   char sql[SQL_MAXLEN];

    // account_save 表的 id 字段是主键
    sprintf(sql, "update %s set group_startup =group_startup +1",
            _get_table());
    Exec_sql_direct(sql, this);

	int l_retrow	=0;
	char* param = "group_startup";
	char filter[80];
	sprintf(filter, "group_startup = group_startup");
	if(_get_row(m_buf, 1, param, filter,&l_retrow) && l_retrow ==1 && get_affected_rows() ==1)
	{
		return atoi(m_buf[0].c_str());
	}else
	{
		return -1;
	}

}
void TBLSystem::Decrement()
{
   char sql[SQL_MAXLEN];

    // account_save 表的 id 字段是主键
    sprintf(sql, "update %s set group_startup =group_startup -1",
            _get_table());
    Exec_sql_direct(sql, this);
}
*/
//==========TBLAccounts===============================
void TBLAccounts::AddStatLog(long login, long play, long wgplay) {
	char sql[SQL_MAXLEN];

	// account_save 表的 id 字段是主键
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "insert stat_log (login_num , play_num, wgplay_num) values (%d, %d, %d)", login, play, wgplay);
	Exec_sql_direct(sql, this);
}
bool TBLAccounts::SetDiscInfo(int actid, const char* cli_ip, const char* reason) {
	char sql[SQL_MAXLEN];

	// account_save 表的 id 字段是主键
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set last_ip='%s',disc_reason ='%s',last_leave =getdate() where act_id =%d",
				_get_table(), cli_ip, reason, actid);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}
//bool TBLAccounts::InsertRow(int act_id,const char *act_name,const char *cha_ids)
bool TBLAccounts::InsertRow(int act_id, const char* act_name, const char* cha_ids, bool equal) {
	char sql[SQL_MAXLEN];

	DWORD dwActID = 0;

	if (equal) {
		dwActID = act_id;
	} else {
		std::string buf[1];
		char param[80];
		//sprintf(param, "TOP 1 act_id");
		_snprintf_s(param, sizeof(param), _TRUNCATE, "TOP 1 act_id");
		char filter[80];
		//sprintf(filter, "ORDER BY act_id DESC");
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "ORDER BY act_id DESC");
		int r1 = 0;
		int r = _get_rowOderby(buf, 1, param, filter, &r1);
		if (DBOK(r) && r1 > 0) {
			dwActID = atol(buf[0].c_str()) + 1;
		} else {
			dwActID = 1;
		}
	}

	// account_save 表的 id 字段是主键
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "insert %s (act_id, act_name, cha_ids) values (%d, '%s', '%s')",
				_get_table(), dwActID, act_name, cha_ids);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}
bool TBLAccounts::UpdateRow(int act_id, const char* cha_ids) {
	char sql[SQL_MAXLEN];

	// account_save 表的 id 字段是主键
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set cha_ids='%s' where act_id=%d",
				_get_table(), cha_ids, act_id);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}
bool TBLAccounts::UpdatePassword(int act_id, const char* szPassword) {
	char sql[SQL_MAXLEN];

	// Add by lark.li 20090527 begin
	char password[40]{};
	size_t size;
	const int nResult = Util::ConvertDBParam(szPassword, password, sizeof(password), size);
	// End

	// account_save 表的 id 字段是主键
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set password='%s' where act_id=%d",
				_get_table(), password, act_id);

	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

int TBLAccounts::FetchRowByActName(const char szAccount[]) {
	int l_retrow = 0;
	const char* param = "act_id,gm,cha_ids,password,last_ip,disc_reason,convert(varchar(20),last_leave,120)";
	char filter[200];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "act_name='%s'", szAccount);
	if (_get_row(m_buf, 7, param, filter, &l_retrow)) {
		if (l_retrow == 1 && get_affected_rows() == 1) {
			return l_retrow;
		} else {
			return 0;
		}
	} else {
		return -1;
	}
}

int TBLAccounts::FetchRowByActID(int act_id) {
	int l_retrow = 0;
	const char* param = "act_name,gm,cha_ids,password,last_ip,disc_reason,convert(varchar(20),last_leave,120)";
	char filter[200];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "act_id=%d", act_id);
	if (_get_row(m_buf, 7, param, filter, &l_retrow)) {
		if (l_retrow == 1 && get_affected_rows() == 1) {
			return l_retrow;
		} else {
			return 0;
		}
	} else {
		return -1;
	}
}

//==========TBLCharacters===============================
bool TBLCharacters::ZeroAddr() {
	char sql[SQL_MAXLEN];

	// account_save 表的 id 字段是主键
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set mem_addr =0 where mem_addr != 0", _get_table());
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

//bool TBLCharacters::ZeroAddr()
//{
//	char sql[SQL_MAXLEN];
//
//	for( int i = 0; i < 200; i++ )
//	{
//		// account_save 表的 id 字段是主键
//		int nMinID = i * 10000;
//		int nMaxID = (i + 1) * 10000;
//		sprintf(sql, "update %s set mem_addr = 0 where cha_id > %d and cha_id < %d and mem_addr != 0",_get_table(), nMinID, nMaxID );
//		SQLRETURN l_ret =Exec_sql_direct(sql, this, 60);
//		if( !DBOK(l_ret) )
//		{
//			return false;
//		}
//	}
//	return true;
//}

bool TBLCharacters::SetAddr(long cha_id, uintptr_t addr) {
	char sql[SQL_MAXLEN];

	// account_save 表的 id 字段是主键
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set mem_addr =%zu where cha_id =%d", _get_table(), addr, cha_id);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}
bool TBLCharacters::InsertRow(const char* cha_name, int act_id, const char* birth, const char* map, const char* look) {
	char sql[SQL_MAXLEN];

	// account_save 表的 id 字段是主键
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "insert %s (cha_name, act_id, birth,map,look) values ('%s', %d, '%s','%s', '%s')",
				_get_table(), cha_name, act_id, birth, map, look);

	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}
bool TBLCharacters::UpdateInfo(unsigned long cha_id, unsigned short icon, const char* motto) {
	char sql[SQL_MAXLEN];

	// account_save 表的 id 字段是主键
	char buff[255]{};
	size_t count;
	int size = Util::ConvertDBParam(motto, buff, sizeof(buff), count);

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set icon =%d,motto ='%s' where cha_id =%d", _get_table(), icon, buff, cha_id);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

int TBLCharacters::FetchRowByChaName(const char* cha_name) {
	int l_retrow = 0;
	const char* param = "cha_id,motto,icon";
	char filter[200];
	char buff[100]{};
	size_t size;
	const int nResult = Util::ConvertDBParam(cha_name, buff, sizeof(buff), size);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_name='%s'", buff);
	if (_get_row(m_buf, 3, param, filter, &l_retrow)) {
		if (l_retrow == 1 && get_affected_rows() == 1) {
			return l_retrow;
		} else {
			return 0;
		}
	} else {
		return -1;
	}
}
bool TBLCharacters::FetchAccidByChaName(const char* cha_name, int& cha_accid) {
	int l_retrow = 0;
	const char* param = "act_id";
	char filter[200];
	char buff[100]{};

	size_t size;
	const int nResult = Util::ConvertDBParam(cha_name, buff, sizeof(buff), size);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_name='%s'", buff);
	if (_get_row(m_buf, 1, param, filter, &l_retrow)) {
		if (l_retrow == 1 && get_affected_rows() == 1) {
			cha_accid = atoi(m_buf[0].c_str());
			return true;
		} else {
			return false;
		}
	}
	return false;
}

bool TBLCharacters::StartEstopTime(int cha_id) {
	char sql[SQL_MAXLEN];
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set estop = getdate() where cha_id =%d", _get_table(), cha_id);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLCharacters::EndEstopTime(int cha_id) {
	char sql[SQL_MAXLEN];
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set estoptime = estoptime - datediff(minute, estop, getdate()) where cha_id =%d", _get_table(), cha_id);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLCharacters::IsEstop(int cha_id) {
	int l_retrow = 0;
	const char* param = "estop";
	char filter[200];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d and dateadd(minute, estoptime, estop) > getdate()", cha_id);
	if (_get_row(m_buf, 1, param, filter, &l_retrow)) {
		if (l_retrow == 1 && get_affected_rows() == 1) {
			return true;
		} else {
			return false;
		}
	}
	return true;
}

bool TBLCharacters::Estop(const char* cha_name, uLong lTimes) {
	char sql[SQL_MAXLEN];
	char buff[100]{};
	size_t size;

	const int nResult = Util::ConvertDBParam(cha_name, buff, sizeof(buff), size);
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set estop = getdate(), estoptime = %d where cha_name ='%s'", _get_table(), lTimes, buff);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLCharacters::AddMoney(int cha_id, DWORD dwMoney) {
	char sql[SQL_MAXLEN];
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set gd = gd + %d where cha_id ='%d'", _get_table(), dwMoney, cha_id);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLCharacters::DelEstop(const char* cha_name) {
	char sql[SQL_MAXLEN];
	char buff[100]{};
	size_t size;
	const int nResult = Util::ConvertDBParam(cha_name, buff, sizeof(buff), size);
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set estoptime = %d where cha_name ='%s'", _get_table(), 0, buff);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

int TBLCharacters::FetchRowByChaID(int cha_id) {
	int l_retrow = 0;
	/*
	char* param = "c.cha_name,c.motto,c.icon,\
				  case when c.guild_stat =0 then c.guild_id else 0 end,\
				  case when c.guild_stat <>0 or c.guild_id =0 then '[无]' else g.guild_name end,\
				  c.job,c.degree,c.map,c.map_x,c.map_y,c.look,c.str,c.dex,c.agi,c.con,c.sta,c.luk\
				  ";
*/
	const std::string param = std::string("c.cha_name,c.motto,c.icon,\
								case when c.guild_stat =0 then c.guild_id else 0 end,\
								case when c.guild_stat <>0 or c.guild_id =0 then '") +
							  std::string(RES_STRING(GP_DBCONNECT_CPP_00001)) +
							  std::string("' else g.guild_name end,\
								c.job,c.degree,c.map,c.map_x,c.map_y,c.look,c.str,c.dex,c.agi,c.con,c.sta,c.luk");
	char filter[200];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "c.guild_id =g.guild_id and c.cha_id=%d", cha_id);
	std::string l_tblname = _tbl_name;
	_tbl_name = "character c,guild g";
	bool l_bret = false;
	try {
		l_bret = _get_row(m_buf, CHA_MAXCOL, param.c_str(), filter, &l_retrow);
	} catch (...) {
		LG("group_sql", "TBLCharacters::FetchRowByChaID execute SQL, failed!,cha_id =%d\n", cha_id);
	}
	_tbl_name = l_tblname;
	if (l_bret) {
		if (l_retrow == 1 && get_affected_rows() == 1) {
			return l_retrow;
		} else {
			return 0;
		}
	} else {
		return -1;
	}
}
int TBLCharacters::FetchActNameRowByChaID(int cha_id, std::string& act_name) {
	int l_retrow = 0;
	const char* param = "a.act_name";
	char filter[200];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "c.cha_id = %d and c.act_id = a.act_id ", cha_id);
	std::string l_tblname = _tbl_name;
	_tbl_name = "character c,account a";
	bool l_bret = false;
	try {
		l_bret = _get_row(m_buf, CHA_MAXCOL, param, filter, &l_retrow);
	} catch (...) {
		LG("group_sql", "TBLCharacters::FetchActNameRowByChaID execute SQL, failed!,cha_id =%d\n", cha_id);
	}
	_tbl_name = l_tblname;
	if (l_bret) {
		if (l_retrow == 1 && get_affected_rows() == 1) {
			act_name = m_buf[0];
			return l_retrow;
		} else {
			return 0;
		}
	} else {
		return -1;
	}
}

bool TBLCharacters::BackupRow(int cha_id) {
	char sql[SQL_MAXLEN];
	//insert into character_log (cha_id, cha_name, act_id, guild_id, job, degree, exp, hp, sp, ap, tp, gd, str, dex, agi, con, sta, luk, map, map_x, map_y, radius, look)
	//	select * from character where cha_id =11

	std::string buf[3];
	char filter[80];
	const char* param = "guild_id, guild_stat";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id =%d", cha_id);
	int l_retrow = 0;
	DWORD dwGuildID;
	BYTE byType;
	bool bret = _get_row(buf, 2, param, filter, &l_retrow);
	if (l_retrow == 1) {
		dwGuildID = atoi(buf[0].c_str());
		byType = atoi(buf[1].c_str());
		if (dwGuildID > 0) {
			// 减少公会信息计数
			if (byType == emGldMembStatNormal) {
				// 已经是会员
				_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set member_total =member_total -1 where guild_id =%d and member_total > 0", dwGuildID);
				const SQLRETURN l_sqlret = Exec_sql_direct(sql, this);
				if (!DBOK(l_sqlret)) {
					LG("GuildSystem", "1>Reject:delete cha，update guild count failed! database sql failed .ret = ", l_sqlret);
					return false;
				} else {
					if (get_affected_rows() != 1) {
						LG("GuildSystem", "2>Reject:delete cha，update guild count failed! database sql failed .ret = ", l_sqlret);
						return false;
					} else {
					}
				}
			} else {
				// 正在申请
				_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set try_total =try_total -1 where guild_id =%d and try_total > 0", dwGuildID);
				const SQLRETURN l_sqlret = Exec_sql_direct(sql, this);
				if (!DBOK(l_sqlret)) {
					LG("GuildSystem", "1>BackupRow:delete cha，update guild count failed! database sql failed .ret = ", l_sqlret);
					return false;
				} else {
					if (get_affected_rows() != 1) {
						LG("GuildSystem", "2>BackupRow:delete cha，update guild count failed! database sql failed .ret = ", l_sqlret);
						return false;
					} else {
					}
				}
			}
		}
	} else {
		LG("GuildSystem", "BackupRow:delete cha，get guild info failed! database sql failed.cha_id = ", cha_id);
		return false;
	}

	//sprintf(sql, "delete from %s where cha_id=%d",_get_table(), cha_id);
	//sprintf(sql, "update %s set delflag =1,deldate =getdate() where cha_id=%d",_get_table(), cha_id);   //  删除时间独立
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set delflag =1,deldate =getdate() where cha_id=%d", _get_table(), cha_id); //  删除时间独立
	SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLPersonInfo::AddInfo(int cha_id, stPersonInfo* pInfo) {
	char sql[SQL_MAXLEN];
	//INSERT INTO [personinfo] ([cha_id],[motto],[showmotto],[sex],[age],[name],[animal_zodiac],[blood_type]
	//           ,[birthday],[state],[city],[constellation],[career],[avatar],[prevent],[support],[oppose])
	//     VALUES(2,'签名',1,'男',30,'名字','属相', '血型','9月16号','德克萨斯','盐湖城','狮子','无业',?
	//           ,0 ,10 ,100)
	// account_save 表的 id 字段是主键
	//sprintf(sql, "INSERT INTO [%s] ([cha_id],[motto],[showmotto],[sex],[age],[name],[animal_zodiac],[blood_type] \
	//	,[birthday],[state],[city],[constellation],[career],[avatarsize],[prevent],[support],[oppose])	\
	//	 VALUES( %d, '%s',%d, '%s', %d, '%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', %d , %d, %d, %d)",
	//		_get_table(), cha_id, pInfo->szMotto, pInfo->bShowMotto, pInfo->szSex, pInfo->sAge, pInfo->szName, pInfo->szAnimalZodiac,
	//		pInfo->szBloodType, pInfo->iBirthday, pInfo->szState, pInfo->szCity, pInfo->szConstellation, pInfo->szCareer,  pInfo->iSize,
	//		pInfo->bPprevent, pInfo->iSupport, pInfo->iOppose);

	char buszMotto[40]{};
	char buszName[50]{};
	char buszAnimalZodiac[50]{};
	char buszBloodType[50]{};
	char buszState[50]{};
	char buszCity[50]{};
	char buszConstellation[50]{};
	char buszCareer[50]{};

	size_t sizeMotto, sizeName, sizeAnimalZodiac, sizeBloodType, sizeState, sizeCity, sizeConstellation, sizeCareer;
	const int nResultMotto = Util::ConvertDBParam(pInfo->szMotto, buszMotto, sizeof(buszMotto), sizeMotto);
	const int nResultName = Util::ConvertDBParam(pInfo->szName, buszName, sizeof(buszName), sizeName);
	const int nResultAnimalZodiac = Util::ConvertDBParam(pInfo->szAnimalZodiac, buszAnimalZodiac, sizeof(buszAnimalZodiac), sizeAnimalZodiac);
	const int nResultBloodType = Util::ConvertDBParam(pInfo->szBloodType, buszBloodType, sizeof(buszBloodType), sizeBloodType);
	const int nResultState = Util::ConvertDBParam(pInfo->szState, buszState, sizeof(buszState), sizeState);
	const int nResultCity = Util::ConvertDBParam(pInfo->szCity, buszCity, sizeof(buszCity), sizeCity);
	const int nResultConstellation = Util::ConvertDBParam(pInfo->szConstellation, buszConstellation, sizeof(buszConstellation), sizeConstellation);
	const int nResultCareer = Util::ConvertDBParam(pInfo->szCareer, buszCareer, sizeof(buszCareer), sizeCareer);

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "INSERT INTO [%s] ([cha_id],[motto],[showmotto],[sex],[age],[name],[animal_zodiac],[blood_type] \
				 ,[birthday],[state],[city],[constellation],[career],[avatarsize],[prevent],[support],[oppose])	\
				 VALUES( %d, '%s',%d, '%s', %d, '%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', %zu , %d, %d, %d)",
				_get_table(), cha_id, buszMotto, pInfo->bShowMotto, pInfo->szSex, pInfo->sAge, buszName, buszAnimalZodiac,
				buszBloodType, pInfo->iBirthday, buszState, buszCity, buszConstellation, buszCareer, pInfo->iSize,
				pInfo->bPprevent, pInfo->iSupport, pInfo->iOppose);

	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLPersonInfo::GetInfo(int cha_id, stPersonInfo* pInfo) {
	int l_retrow = 0;
	const char* param = "[motto],[showmotto],[sex],[age],[name],[animal_zodiac],[blood_type],[birthday],[state],[city],[constellation],[career],[avatarsize],[prevent],[support],[oppose]";
	char filter[200];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id=%d", cha_id);
	if (_get_row(m_buf, 16, param, filter, &l_retrow)) {
		if (l_retrow == 1 && get_affected_rows() == 1) {
			int index = 0;
			strncpy_s(pInfo->szMotto, sizeof(pInfo->szMotto), m_buf[index++].c_str(), _TRUNCATE);

			pInfo->bShowMotto = (atoi(m_buf[index++].c_str()) == 1 ? true : false);
			strncpy_s(pInfo->szSex, sizeof(pInfo->szSex), m_buf[index++].c_str(), _TRUNCATE);

			pInfo->sAge = atoi(m_buf[index++].c_str());
			strncpy_s(pInfo->szName, sizeof(pInfo->szName), m_buf[index++].c_str(), _TRUNCATE);

			strncpy_s(pInfo->szAnimalZodiac, sizeof(pInfo->szAnimalZodiac), m_buf[index++].c_str(), _TRUNCATE);

			strncpy_s(pInfo->szBloodType, sizeof(pInfo->szBloodType), m_buf[index++].c_str(), _TRUNCATE);

			pInfo->iBirthday = atoi(m_buf[index++].c_str());
			strncpy_s(pInfo->szState, sizeof(pInfo->szState), m_buf[index++].c_str(), _TRUNCATE);

			strncpy_s(pInfo->szCity, sizeof(pInfo->szCity), m_buf[index++].c_str(), _TRUNCATE);

			strncpy_s(pInfo->szConstellation, sizeof(pInfo->szConstellation), m_buf[index++].c_str(), _TRUNCATE);

			strncpy_s(pInfo->szCareer, sizeof(pInfo->szCareer), m_buf[index++].c_str(), _TRUNCATE);

			pInfo->iSize = atoi(m_buf[index++].c_str());
			pInfo->bPprevent = (atoi(m_buf[index++].c_str()) == 1 ? true : false);
			pInfo->iSupport = atoi(m_buf[index++].c_str());
			pInfo->iOppose = atoi(m_buf[index++].c_str());

			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}

	return true;
}

bool TBLPersonInfo::DelInfo(int cha_id) {
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "delete %s where (cha_id=%d )", _get_table(), cha_id);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}
//char* param, int pagesize, int pageindex, char* filter, char* sort, int sorttype, int& totalpage, int& totalrecord,
//					  vector< vector< string > > &data, unsigned short timeout
bool TBLPersonInfo::Query(stQueryPersonInfo* pQuery, stQueryResoultPersonInfo pResult[], int& num, int& totalpage, int& totalrecord) {
	//取到页数
	char filter[1024];
	int sorttype = 1;
	totalpage = 0;
	totalrecord = 0;

	std::vector<std::vector<std::string>> rowV;

	if (pQuery->nCurPage < 0) {
		pQuery->nCurPage = 1;
	}

	char busChaName[100]{};
	size_t size;
	const int nResult = Util::ConvertDBParam(pQuery->sChaName, busChaName, sizeof(busChaName), size);



	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "where cha_name = '%s'", busChaName);

	char tablename[16] = "VW_Person";
	char param[64] = "cha_id, cha_name, age, sex, state, city";
	char sort[8] = "cha_id";
	if (get_page_data(tablename, param, pQuery->nPageItemNum, pQuery->nCurPage, filter, sort, sorttype, totalpage, totalrecord, rowV)) {
		num = (int)rowV.size();
		int i = 0;
		for (std::vector<std::vector<std::string>>::iterator it = rowV.begin(); it != rowV.end(); it++, i++) {
			// 第一个字段不要
			strncpy_s(pResult[i].sChaName, sizeof(pResult[i].sChaName), (*it)[1].c_str(), _TRUNCATE);

			if ((*it)[2].empty()) {
				pResult[i].nMinAge = -1;
			} else {
				pResult[i].nMinAge = atoi((*it)[2].c_str());
			}

			strncpy_s(pResult[i].cSex, sizeof(pResult[i].cSex), (*it)[3].c_str(), _TRUNCATE);
			strncpy_s(pResult[i].szState, sizeof(pResult[i].szState), (*it)[4].c_str(), _TRUNCATE);
			strncpy_s(pResult[i].nCity, sizeof(pResult[i].nCity), (*it)[5].c_str(), _TRUNCATE);
		}
		return true;
	}
	return false;
}

bool TBLPersonAvatar::AddInfo(int cha_id, stPersonInfo* pInfo) {
	char sql[SQL_MAXLEN];
	//INSERT INTO [personinfo] ([cha_id],[motto],[showmotto],[sex],[age],[name],[animal_zodiac],[blood_type]
	//           ,[birthday],[state],[city],[constellation],[career],[avatar],[prevent],[support],[oppose])
	//     VALUES(2,'签名',1,'男',30,'名字','属相', '血型','9月16号','德克萨斯','盐湖城','狮子','无业',?
	//           ,0 ,10 ,100)
	// account_save 表的 id 字段是主键
	//sprintf(sql, "INSERT INTO [%s] ([cha_id],[avatar]) VALUES( %d, ?)",	_get_table(), cha_id);
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "INSERT INTO [%s] ([cha_id],[avatar]) VALUES( %d, ?)", _get_table(), cha_id);

	const SQLRETURN l_ret = Exec_sql(sql, pInfo->pAvatar, pInfo->iSize, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLPersonAvatar::GetInfo(int cha_id, stPersonInfo* pInfo) {
	int l_retrow = 0;
	char param[16] = "[avatar]";
	char filter[200];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id=%d", cha_id);

	if (_get_bin_field(pInfo->pAvatar, pInfo->iSize, param, filter, &l_retrow)) {
		if (l_retrow == 1 && get_affected_rows() == 1) {
			//pInfo->pAvatar = new byte[pInfo->iSize];
			return true;
		} else {

			return false;
		}
	} else {
		return false;
	}

	return true;
}

bool TBLPersonAvatar::DelInfo(int cha_id) {
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "delete %s where (cha_id=%d )", _get_table(), cha_id);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

//==========TBLFriends===============================
int TBLFriends::GetFriendsCount(long cha_id1, long cha_id2, const char* groupname) {
	int l_retrow = 0;
	char filter[200];

	const char* param = "count(*) num";

	if (!groupname) {
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "(cha_id1=%d AND cha_id2 =%d)OR(cha_id1=%d AND cha_id2 =%d)", cha_id1, cha_id2, cha_id2, cha_id1);
	} else {
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "(cha_id1=%d AND cha_id2 <> -1) AND (relation='%s')", cha_id1, groupname);
	}

	if (_get_row(m_buf, FRD_MAXCOL, param, filter, &l_retrow) && l_retrow == 1 && get_affected_rows() == 1) {
		return atoi(m_buf[0].c_str());
	} else {
		return -1;
	}
}

// Add by lark.li 20080806 begin
bool TBLFriends::GroupIsExsit(long cha_id1, long cha_id2, const char* groupname) {
	int l_retrow = 0;
	char filter[200];

	const char* param = "count(*) num";

	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "(cha_id1=%d AND cha_id2 = %d) AND (relation='%s')", cha_id1, cha_id2, groupname);

	if (_get_row(m_buf, FRD_MAXCOL, param, filter, &l_retrow) && l_retrow == 1 && get_affected_rows() == 1) {
		return (atoi(m_buf[0].c_str()) == 1);
	} else {
		return false;
	}
}

bool TBLFriends::MoveGroup(long cha_id1, long cha_id2, const char* oldgroup, const char* newgroup) {
	//update friends set relation = 'Test1' where cha_id1 = 240 and cha_id2 = 242 and relation = '黑名单'
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set relation = '%s' where cha_id1 = %d and cha_id2 = %d and relation = '%s'",
				_get_table(), newgroup, cha_id1, cha_id2, oldgroup);

	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

int TBLFriends::GetGroupCount(long cha_id1) {
	int l_retrow = 0;
	char filter[200];
	char buffer[255];
	memset(buffer, 0, sizeof(buffer));

	const char* param = "count(*) num";
	_snprintf_s(filter, sizeof(filter), "1=1");
	_tbl_name = "(select distinct friends.relation relation from friends\
						where friends.cha_id1 =%d and friends.cha_id2 = -1) cc";

	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, _tbl_name.c_str(), cha_id1);

	_tbl_name = buffer;

	if (_get_row(m_buf, FRD_MAXCOL, param, filter, &l_retrow) && l_retrow == 1 && get_affected_rows() == 1) {
		_tbl_name = "friends";
		return atoi(m_buf[0].c_str());
	} else {
		_tbl_name = "friends";
		return -1;
	}
}
unsigned long TBLFriends::GetFriendAddr(long cha_id1, long cha_id2) {
	int l_retrow = 0;
	char filter[200];

	const char* param = "character.mem_addr addr";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "(friends.cha_id1=%d AND friends.cha_id2 =%d)", cha_id1, cha_id2);
	_tbl_name = "character (nolock) INNER JOIN friends ON character.cha_id = friends.cha_id2";
	if (_get_row(m_buf, FRD_MAXCOL, param, filter, &l_retrow) && l_retrow == 1 && get_affected_rows() == 1) {
		_tbl_name = "friends";
		return atoi(m_buf[0].c_str());
	} else {
		_tbl_name = "friends";
		return 0;
	}
}
bool TBLFriends::UpdateGroup(long cha_id1, const char* oldgroup, const char* newgroup) {
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set relation ='%s' where cha_id1=%d AND relation = '%s'",
				_get_table(), newgroup, cha_id1, oldgroup);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLFriends::DeleteGroup(long cha_id, const char* groupname) {
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "delete %s where cha_id1=%d AND cha_id2 = -1 AND relation ='%s' ",
				_get_table(), cha_id, groupname);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLFriends::AddGroup(long cha_id, const char* groupname) {
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "insert into %s (cha_id1,cha_id2,relation,createtime) values(%d , -1 , '%s', getdate())",
				_get_table(), cha_id, groupname);
	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLFriends::AddFriend(long cha_id1, long cha_id2) {
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "insert %s (cha_id1,cha_id2,relation) values(%d,%d,'%s')",
				_get_table(), cha_id1, cha_id2, Friend::GetStandardGroup());

	SQLRETURN l_ret = Exec_sql_direct(sql, this);

	if (DBOK(l_ret) && get_affected_rows() == 1) {
		_snprintf_s(sql, sizeof(sql), _TRUNCATE, "insert %s (cha_id1,cha_id2,relation) values(%d,%d,'%s')",
					_get_table(), cha_id2, cha_id1, Friend::GetStandardGroup());

		const SQLRETURN l_ret = Exec_sql_direct(sql, this);
		return (DBOK(l_ret)) ? true : false;
	}
	return false;
}
bool TBLFriends::DelFriend(long cha_id1, long cha_id2) {
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "delete %s where (cha_id1=%d AND cha_id2 =%d)OR(cha_id1=%d AND cha_id2 =%d)",
				_get_table(), cha_id1, cha_id2, cha_id2, cha_id1);

	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

//==========TBLMaster===============================
int TBLMaster::GetMasterCount(long cha_id) {
	int l_retrow = 0;
	char filter[200];

	const char* param = "count(*) num";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "(cha_id1=%d)", cha_id);
	if (_get_row(m_buf, MASTER_MAXCOL, param, filter, &l_retrow) && l_retrow == 1 && get_affected_rows() == 1) {
		return atoi(m_buf[0].c_str());
	} else {
		return -1;
	}
}

int TBLMaster::GetPrenticeCount(long cha_id) {
	int l_retrow = 0;
	char filter[200];

	const char* param = "count(*) num";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "(cha_id2=%d AND finish=0)", cha_id);
	if (_get_row(m_buf, MASTER_MAXCOL, param, filter, &l_retrow) && l_retrow == 1 && get_affected_rows() == 1) {
		return atoi(m_buf[0].c_str());
	} else {
		return -1;
	}
}

int TBLMaster::HasMaster(long cha_id1, long cha_id2) {
	int l_retrow = 0;
	char filter[200];

	const char* param = "count(*) num";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "(cha_id1=%d AND cha_id2=%d)", cha_id1, cha_id2);
	if (_get_row(m_buf, MASTER_MAXCOL, param, filter, &l_retrow) && l_retrow == 1 && get_affected_rows() == 1) {
		return atoi(m_buf[0].c_str());
	} else {
		return -1;
	}
}

bool TBLMaster::AddMaster(long cha_id1, long cha_id2) {
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "insert %s (cha_id1,cha_id2,finish) values(%d,%d,%d)",
				_get_table(), cha_id1, cha_id2, 0);

	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLMaster::DelMaster(long cha_id1, long cha_id2) {
	char sql[SQL_MAXLEN];

	//	sprintf(sql, "delete %s where (cha_id1=%d AND cha_id2 =%d)",
	//		_get_table(),cha_id1,cha_id2);
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "delete %s where (cha_id1=%d AND cha_id2 =%d)",
				_get_table(), cha_id1, cha_id2);

	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLMaster::FinishMaster(long cha_id) {
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set finish=1 where cha_id1=%d",
				_get_table(), cha_id);

	const SQLRETURN l_ret = Exec_sql_direct(sql, this);
	return (DBOK(l_ret)) ? true : false;
}

bool TBLMaster::InitMasterRelation(std::map<uLong, uLong>& mapMasterRelation) {
	static char const query_master_format[SQL_MAXLEN] =
		"select cha_id1 cha_id1,cha_id2 cha_id2 from %s";

	bool ret = false;
	char sql[SQL_MAXLEN];
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, query_master_format, _get_table());

	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try {
		do {
			sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
			if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
				handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
				throw 1;
			}

			sqlret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
			if (sqlret != SQL_SUCCESS) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				if (sqlret != SQL_SUCCESS_WITH_INFO)
					throw 2;
			}

			sqlret = SQLNumResultCols(hstmt, &col_num);
			col_num = min(col_num, MAX_COL);
			col_num = min(col_num, _max_col);

			// Bind Column
			int i = 0;
			for (i = 0; i < col_num; ++i) {
				SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
			}

			// Fetch each Row
			for (i = 0; ((sqlret = SQLFetch(hstmt)) == SQL_SUCCESS) || (sqlret == SQL_SUCCESS_WITH_INFO); ++i) {
				if (sqlret != SQL_SUCCESS) {
					handle_err(hstmt, SQL_HANDLE_STMT, sqlret, sql);
				}

				const uLong ulPID = atoi((char const*)_buf[0]);
				const uLong ulMID = atoi((char const*)_buf[1]);

				mapMasterRelation[ulPID] = ulMID;
			}

			SQLFreeStmt(hstmt, SQL_CLOSE);
			SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
			SQLFreeStmt(hstmt, SQL_UNBIND);
			ret = true;

		} while (false);
	} catch (...) {
		LogLine l_line(g_LogMaster);
		l_line << newln << "Unknown Exception raised when InitMasterRelation()";
	}

	if (hstmt != SQL_NULL_HSTMT) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return ret;
}

bool TBLMaster::GetMasterData(master_dat* farray, int& array_num, unsigned int cha_id) {
	static char const query_master_format[SQL_MAXLEN] =
		"select '' relation,count(*) addr,0 cha_id,'' cha_name,0 icon,'' motto from ( \
		select distinct master.relation relation from character INNER JOIN \
		master ON character.cha_id = master.cha_id2 where master.cha_id1 = %d \
		) cc union select master.relation relation,count(character.mem_addr) addr,0 \
		cha_id,'' cha_name,1 icon,'' motto from character INNER JOIN master ON \
		character.cha_id = master.cha_id2 where master.cha_id1 = %d group by relation \
		union select master.relation relation,character.mem_addr addr,character.cha_id \
		cha_id,character.cha_name cha_name,character.icon icon,character.motto motto \
		from character INNER JOIN master ON character.cha_id = master.cha_id2 \
		where master.cha_id1 = %d order by relation,cha_id,icon";

	if (!farray || array_num <= 0 || cha_id == 0) {
		return false;
	}

	bool ret = false;
	char sql[SQL_MAXLEN];
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, query_master_format, cha_id, cha_id, cha_id);
	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try {
		do {
			sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
			if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
				handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
				throw 1;
			}

			sqlret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
			if (sqlret != SQL_SUCCESS) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				if (sqlret != SQL_SUCCESS_WITH_INFO) {
					throw 2;
				}
			}

			sqlret = SQLNumResultCols(hstmt, &col_num);
			col_num = min(col_num, MAX_COL);
			col_num = min(col_num, _max_col);

			// Bind Column
			int i = 0;
			for (i = 0; i < col_num; ++i) {
				SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
			}

			// Fetch each Row
			for (i = 0; ((sqlret = SQLFetch(hstmt)) == SQL_SUCCESS) || (sqlret == SQL_SUCCESS_WITH_INFO); ++i) {
				if (i >= array_num) {
					break;
				}

				if (sqlret != SQL_SUCCESS) {
					handle_err(hstmt, SQL_HANDLE_STMT, sqlret, sql);
				}

				farray[i].relation = (char const*)_buf[0];
				farray[i].memaddr = atoi((char const*)_buf[1]);
				farray[i].cha_id = atoi((char const*)_buf[2]);
				farray[i].cha_name = (char const*)_buf[3];
				farray[i].icon_id = atoi((char const*)_buf[4]);
				farray[i].motto = (char const*)_buf[5];
			}

			array_num = i; // 取出的行数

			SQLFreeStmt(hstmt, SQL_CLOSE);
			SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
			SQLFreeStmt(hstmt, SQL_UNBIND);
			ret = true;

		} while (false);
	} catch (...) {
		LogLine l_line(g_LogMaster);
		l_line << newln << "Unknown Exception raised when GetMasterData()";
	}

	if (hstmt != SQL_NULL_HSTMT) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return ret;
}

bool TBLMaster::GetPrenticeData(master_dat* farray, int& array_num, unsigned int cha_id) {
	static char const query_prentice_format[SQL_MAXLEN] =
		"select '' relation,count(*) addr,0 cha_id,'' cha_name,0 icon,'' motto from ( \
		select distinct master.relation relation from character INNER JOIN \
		master ON character.cha_id = master.cha_id1 where master.cha_id2 = %d \
		) cc union select master.relation relation,count(character.mem_addr) addr,0 \
		cha_id,'' cha_name,1 icon,'' motto from character INNER JOIN master ON \
		character.cha_id = master.cha_id1 where master.cha_id2 = %d group by relation \
		union select master.relation relation,character.mem_addr addr,character.cha_id \
		cha_id,character.cha_name cha_name,character.icon icon,character.motto motto \
		from character INNER JOIN master ON character.cha_id = master.cha_id1 \
		where master.cha_id2 = %d order by relation,cha_id,icon";

	if (!farray || array_num <= 0 || cha_id == 0) {
		return false;
	}

	bool ret = false;
	char sql[SQL_MAXLEN];
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, query_prentice_format, cha_id, cha_id, cha_id);
	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try {
		do {
			sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
			if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
				handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
				throw 1;
			}

			sqlret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
			if (sqlret != SQL_SUCCESS) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				if (sqlret != SQL_SUCCESS_WITH_INFO) {
					throw 2;
				}
			}

			sqlret = SQLNumResultCols(hstmt, &col_num);
			col_num = min(col_num, MAX_COL);
			col_num = min(col_num, _max_col);

			// Bind Column
			int i = 0;
			for (i = 0; i < col_num; ++i) {
				SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
			}

			// Fetch each Row
			for (i = 0; ((sqlret = SQLFetch(hstmt)) == SQL_SUCCESS) || (sqlret == SQL_SUCCESS_WITH_INFO); ++i) {
				if (i >= array_num) {
					break;
				}

				if (sqlret != SQL_SUCCESS)
					handle_err(hstmt, SQL_HANDLE_STMT, sqlret, sql);

				farray[i].relation = (const char*)_buf[0];
				farray[i].memaddr = atoi((const char*)_buf[1]);
				farray[i].cha_id = atoi((const char*)_buf[2]);
				farray[i].cha_name = (const char*)_buf[3];
				farray[i].icon_id = atoi((const char*)_buf[4]);
				farray[i].motto = (const char*)_buf[5];
			}

			array_num = i; // 取出的行数

			SQLFreeStmt(hstmt, SQL_CLOSE);
			SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
			SQLFreeStmt(hstmt, SQL_UNBIND);
			ret = true;

		} while (false);
	} catch (...) {
		LogLine l_line(g_LogMaster);
		l_line << newln << "Unknown Exception raised when GetPrenticeData()";
	}

	if (hstmt != SQL_NULL_HSTMT) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return ret;
}

//==========TBLGuilds===============================
bool TBLGuilds::IsReady() {
	int l_retrow = 0;
	const char* param = "count(*)";
	if (_get_row(m_buf, 1, param, nullptr, &l_retrow)) {
		if (l_retrow == 1 && get_affected_rows() == 1 && atoi(m_buf[0].c_str()) == 199) {
			return true;
		}
	}
	return false;
}
int TBLGuilds::FetchRowByName(const char* guild_name) {
	int l_retrow = 0;
	const char* param = "guild_id";
	char filter[200];
	char buff[66]{};
	size_t size;
	const int nResult = Util::ConvertDBParam(guild_name, buff, sizeof(buff), size);

	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_name='%s'", buff);

	if (_get_row(m_buf, 1, param, filter, &l_retrow)) {
		if (l_retrow == 1 && get_affected_rows() == 1) {
			return l_retrow;
		} else {
			return 0;
		}
	} else {
		return -1;
	}
}
bool TBLGuilds::Disband(uLong gldid) {
	char sql[SQL_MAXLEN];
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set motto ='',passwd ='',leader_id =0,stat =0,money =0,exp =0,member_total =0,try_total =0\
					where guild_id =%d",
				gldid);

	SQLRETURN l_sqlret = Exec_sql_direct(sql, this);
	if (!DBOK(l_sqlret)) {
		if (DBNODATA(l_sqlret)) {
			LogLine l_line(g_LogGuild);
			l_line << newln << "dismiss guild SQL failed2! guild ID:" << gldid;
			return false;
		} else {
			LogLine l_line(g_LogGuild);
			l_line << newln << "dismiss guild SQL failed1! guild ID:" << gldid;
			return false; //普通SQL错误
		}
	}
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update character set guild_id =0 ,guild_stat =0,guild_permission =0\
					where guild_id =%d",
				gldid);

	l_sqlret = Exec_sql_direct(sql, this);
	if (!DBOK(l_sqlret)) {
		LogLine l_line(g_LogGuild);
		l_line << newln << "dismiss guild SQL failed3! guild ID:" << gldid;
		return false; //普通SQL错误
	}

	return true;
}
bool TBLGuilds::InitAllGuilds(char disband_days) {
	std::string sql_syntax = "";
	if (disband_days < 1) {
		return false;
	} else {
		/*
		sql_syntax =
			"	select g.guild_id, g.guild_name, g.motto, g.leader_id,g.type,g.stat,\
						g.money, g.exp, g.member_total, g.try_total,g.disband_date,\
						case when g.stat>0 then DATEDIFF(mi,g.disband_date,GETDATE()) else 0 end  解散考察累计分钟,\
						case when g.stat>0 then %d*24*60 -DATEDIFF(mi,g.disband_date,GETDATE()) else 0 end 解散考察剩余分钟\
					from guild As g\
					where (g.guild_id >0)\
			";
		*/
		sql_syntax =
			std::string("	select g.guild_id, g.guild_name, g.motto, g.leader_id,g.type,g.stat,\
						g.money, g.exp, g.member_total, g.try_total,g.disband_date,\
						case when g.stat>0 then DATEDIFF(mi,g.disband_date,GETDATE()) else 0 end TotalMins ") +
			std::string(", case when g.stat>0 then %d*24*60 -DATEDIFF(mi,g.disband_date,GETDATE()) else 0 end LeaveMins ") + std::string(" from guild As g where (g.guild_id >0) ");
	}

	bool l_ret = false;
	char sql[SQL_MAXLEN];
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, sql_syntax.c_str(), disband_days);

	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try {
		sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
			throw 1;
		}

		sqlret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
		if (sqlret != SQL_SUCCESS) {
			handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			if (sqlret != SQL_SUCCESS_WITH_INFO) {
				throw 2;
			}
		}

		sqlret = SQLNumResultCols(hstmt, &col_num);
		col_num = min(col_num, MAX_COL);
		col_num = min(col_num, _max_col);

		// Bind Column
		for (int i = 0; i < col_num; ++i) {
			SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
		}

		// Fetch each Row	int i; // 取出的行数
		for (int f_row = 1; (sqlret = SQLFetch(hstmt)) == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO; ++f_row) {
			if (sqlret != SQL_SUCCESS) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			}
			Guild* l_gld = Guild::Alloc();
			l_gld->m_id = atol((cChar*)_buf[0]);										 //公会ID
			strncpy_s(l_gld->m_name, sizeof(l_gld->m_name), (cChar*)_buf[1], _TRUNCATE); //公会名

			strncpy_s(l_gld->m_motto, sizeof(l_gld->m_motto), (cChar*)_buf[2], _TRUNCATE); //公会座右铭

			l_gld->m_leaderID = atol((cChar*)_buf[3]);		 //会长ID
			l_gld->m_type = atoi((cChar*)_buf[4]);			 //公会类型
			l_gld->m_stat = atoi((cChar*)_buf[5]);			 //公会状态
			l_gld->m_remain_minute = atol((cChar*)_buf[12]); //公会解散剩余分钟数
			l_gld->m_tick = GetTickCount();

			l_gld->BeginRun();
		}

		SQLFreeStmt(hstmt, SQL_UNBIND);
		l_ret = true;
	} catch (int& e) {
		LogLine l_line(g_LogGuild);
		l_line << newln << "init guild ODBC interface failed, InitAllGuilds() error:" << e;
	} catch (...) {
		LogLine l_line(g_LogGuild);
		l_line << newln << "Unknown Exception raised when InitAllGuilds()";
	}

	if (hstmt != SQL_NULL_HSTMT) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return l_ret;
}
bool TBLGuilds::SendGuildInfo(Player* ply) {
	WPacket l_togmSelf = g_gpsvr->GetWPacket();
	l_togmSelf.WriteCmd(CMD_PM_GUILDINFO);
	l_togmSelf.WriteLong(ply->m_chaid[ply->m_currcha]); // 角色DBID
	l_togmSelf.WriteChar(ply->GetGuild()->m_type);		//公会类型
	l_togmSelf.WriteLong(ply->m_guild[ply->m_currcha]); //公会ID
	l_togmSelf.WriteLong(ply->GetGuild()->m_leaderID);  //会长ID
	l_togmSelf.WriteString(ply->GetGuild()->m_name);	//公会name
	l_togmSelf.WriteString(ply->GetGuild()->m_motto);   //公会座佑名
	ply->m_gate->GetDataSock()->SendData(l_togmSelf);
	return true;
}
bool TBLGuilds::InitGuildMember(Player* ply, uLong chaid, uLong gldid, int mode) {
	bool l_ret = false;
	if (ply && gldid == 0) {
		WPacket l_toSelf = g_gpsvr->GetWPacket();
		l_toSelf.WriteCmd(CMD_PC_GUILD);
		l_toSelf.WriteChar(MSG_GUILD_START);
		l_toSelf.WriteLong(0);
		l_toSelf.WriteChar(0);
		g_gpsvr->SendToClient(ply, l_toSelf);
	} else {
		const char* sql_syntax =
			"	select c.mem_addr,c.cha_id, c.cha_name, c.motto, c.job, c.degree, c.icon, c.guild_permission\
					from character As c\
					where (c.guild_stat =0) and (c.guild_id =%d) and (c.delflag = 0)\
			";

		char sql[SQL_MAXLEN];
		_snprintf_s(sql, sizeof(sql), _TRUNCATE, sql_syntax, gldid);
		// 执行查询操作
		SQLRETURN sqlret;
		SQLHSTMT hstmt = SQL_NULL_HSTMT;
		SQLSMALLINT col_num = 0;
		bool found = true;

		try {
			sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
			if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
				handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
				throw 1;
			}

			sqlret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
			if (sqlret != SQL_SUCCESS) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				if (sqlret != SQL_SUCCESS_WITH_INFO) {
					throw 2;
				}
			}

			sqlret = SQLNumResultCols(hstmt, &col_num);
			col_num = min(col_num, MAX_COL);
			col_num = min(col_num, _max_col);

			// Bind Column
			for (int i = 0; i < col_num; ++i) {
				SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
			}
			WPacket l_toGuild = g_gpsvr->GetWPacket();
			l_toGuild.WriteCmd(CMD_PC_GUILD);
			if (mode) {
				l_toGuild.WriteChar(MSG_GUILD_ADD);
			} else {
				l_toGuild.WriteChar(MSG_GUILD_ONLINE);
				l_toGuild.WriteLong(chaid);
			}

			WPacket l_toSelf, wpk0;
			if (ply) {
				wpk0 = g_gpsvr->GetWPacket();
				wpk0.WriteCmd(CMD_PC_GUILD);
				wpk0.WriteChar(MSG_GUILD_START);
			}
			bool l_hrd = false;

			Player* playerlst[10240];
			short playernum = 0;

			long lPacketNum = 0;

			// Fetch each Row	int i; // 取出的行数
			int f_row = 1;
			for (f_row = 1; (sqlret = SQLFetch(hstmt)) == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO; ++f_row) {
				if (sqlret != SQL_SUCCESS) {
					handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				}
				if (ply && (f_row % 20) == 1) {
					l_toSelf = wpk0;
				}
				if (ply && !l_hrd) {
					l_hrd = true;
					l_toSelf.WriteLong(ply->m_guild[ply->m_currcha]); //公会ID
					l_toSelf.WriteString(ply->GetGuild()->m_name);	//公会name
					l_toSelf.WriteLong(ply->GetGuild()->m_leaderID);  //会长ID
				}
				unsigned long l_memaddr = atol((cChar*)_buf[0]);
				if (l_memaddr) {
					playerlst[playernum] = ToPointer<Player>(l_memaddr);
					playernum++;
				}
				if (mode && chaid == atol((cChar*)_buf[1])) {
					l_toGuild.WriteChar(l_memaddr ? 1 : 0);		//online
					l_toGuild.WriteLong(atol((cChar*)_buf[1])); //chaid
					l_toGuild.WriteString((cChar*)_buf[2]);		//chaname
					l_toGuild.WriteString((cChar*)_buf[3]);		//motto
					//l_toGuild.WriteString(	(cChar*)_buf[4]);	//job
					l_toGuild.WriteString(g_GetJobName(atoi((const char*)_buf[4]))); //job

					l_toGuild.WriteShort(atoi((cChar*)_buf[5])); //degree
					l_toGuild.WriteShort(atoi((cChar*)_buf[6])); //icon
					l_toGuild.WriteShort(atoi((cChar*)_buf[7])); //permission
				}
				if (ply) {
					l_toSelf.WriteChar(l_memaddr ? 1 : 0);	 //online
					l_toSelf.WriteLong(atol((cChar*)_buf[1])); //chaid
					l_toSelf.WriteString((cChar*)_buf[2]);	 //chaname
					l_toSelf.WriteString((cChar*)_buf[3]);	 //motto
					//l_toSelf.WriteString(	(cChar*)_buf[4]);	//job

					l_toSelf.WriteString(g_GetJobName(atoi((const char*)_buf[4]))); //job

					l_toSelf.WriteShort(atoi((cChar*)_buf[5])); //degree
					l_toSelf.WriteShort(atoi((cChar*)_buf[6])); //icon
					l_toSelf.WriteShort(atoi((cChar*)_buf[7])); //permission
				}
				if (ply && !(f_row % 20)) {
					l_toSelf.WriteLong(lPacketNum);
					lPacketNum++;
					l_toSelf.WriteChar(((f_row - 1) % 20) + 1); //本次包括的条数
					g_gpsvr->SendToClient(ply, l_toSelf);
				}
			}
			if (ply && (f_row % 20) == 1) {
				l_toSelf = wpk0;
			}
			if (ply && !l_hrd) {
				l_hrd = true;
				l_toSelf.WriteLong(ply->m_guild[ply->m_currcha]); //公会ID
				l_toSelf.WriteString(ply->GetGuild()->m_name);	//公会name
				l_toSelf.WriteLong(ply->GetGuild()->m_leaderID);  //会长ID
			}
			if (ply) {
				l_toSelf.WriteLong(lPacketNum);
				lPacketNum++;
				l_toSelf.WriteChar((f_row - 1) % 20);
				g_gpsvr->SendToClient(ply, l_toSelf);
			}
			LogLine l_line(g_LogGuild);
			l_line << newln << "online guild num:" << playernum << endln;
			g_gpsvr->SendToClient(playerlst, playernum, l_toGuild);

			SQLFreeStmt(hstmt, SQL_UNBIND);
			l_ret = true;
		} catch (int& e) {
			LogLine l_line(g_LogGuild);
			l_line << newln << "init guild ODBC interface failed, InitGuildMember() error:" << e;

			l_line << newln << sql;
		} catch (...) {
			LogLine l_line(g_LogGuild);
			l_line << newln << "Unknown Exception raised when InitGuildMember()";
		}

		if (hstmt != SQL_NULL_HSTMT) {
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			hstmt = SQL_NULL_HSTMT;
		}
	}

	return l_ret;
}
//查找公会成员数
bool TBLGuilds::GetGuildMemberNum(uLong gldid, std::atomic<long>& memberNum) {
	std::string buf[1];
	char filter[180];
	const char* param = "member_total";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, " guild_id = %d ", gldid);

	int l_retrow = 0;
	bool l_ret = false;
	l_ret = _get_row(m_buf, 1, param, filter, &l_retrow);

	if (l_ret && l_retrow > 0) {
		memberNum = atoi(m_buf[0].c_str());
		return true;
	}
	return false;
}

bool TBLParam::InitParam() {
	std::string strSQL = "select param1,param2,param3,param4,param5,param6,param7,param8,param9,param10 from param where id = 1";
	//	string strSQL = "select param1 from param where id = 1";

	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLLEN buf_len[MAXORDERNUM + MAXORDERNUM];
	//	SQLINTEGER		nID = 0,nlen;
	bool found = true;

	try {
		sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
			throw 1;
		}
		sqlret = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)1, 0);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
			throw 1;
		}
		SQLBindCol(hstmt, 1, SQL_C_LONG, &m_nOrder[0].nid, 0, &(buf_len[0]));
		SQLBindCol(hstmt, 2, SQL_C_LONG, &m_nOrder[1].nid, 0, &buf_len[1]);
		SQLBindCol(hstmt, 3, SQL_C_LONG, &m_nOrder[2].nid, 0, &buf_len[2]);
		SQLBindCol(hstmt, 4, SQL_C_LONG, &m_nOrder[3].nid, 0, &buf_len[3]);
		SQLBindCol(hstmt, 5, SQL_C_LONG, &m_nOrder[4].nid, 0, &buf_len[4]);

		SQLBindCol(hstmt, 6, SQL_C_LONG, &m_nOrder[0].nfightpoint, 0, &buf_len[5]);
		SQLBindCol(hstmt, 7, SQL_C_LONG, &m_nOrder[1].nfightpoint, 0, &buf_len[6]);
		SQLBindCol(hstmt, 8, SQL_C_LONG, &m_nOrder[2].nfightpoint, 0, &buf_len[7]);
		SQLBindCol(hstmt, 9, SQL_C_LONG, &m_nOrder[3].nfightpoint, 0, &buf_len[8]);
		SQLBindCol(hstmt, 10, SQL_C_LONG, &m_nOrder[4].nfightpoint, 0, &buf_len[9]);
		sqlret = SQLExecDirect(hstmt, (SQLCHAR*)const_cast<char*>(strSQL.c_str()), SQL_NTS);
		if (sqlret != SQL_SUCCESS) {
			handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			if (sqlret != SQL_SUCCESS_WITH_INFO) {
				throw 2;
			}
		}

		sqlret = SQLFetch(hstmt);
		if (sqlret != SQL_SUCCESS) {
			if (sqlret != SQL_NO_DATA) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			}
		}
		SQLFreeStmt(hstmt, SQL_CLOSE);
		SQLFreeStmt(hstmt, SQL_UNBIND);
	} catch (int& e) {
		LogLine l_line(g_LogGarner2);
		l_line << newln << "init guild ODBC interface failed, InitParam() error:" << e;
	} catch (...) {
		LogLine l_line(g_LogGarner2);
		l_line << newln << "Unknown Exception raised when InitParam()";
	}

	char buff[255];
	int nlev;
	try {
		sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);
			throw 1;
		}
		SQLBindCol(hstmt, 1, SQL_C_CHAR, _buf[0], MAX_DATALEN, &buf_len[0]);
		SQLBindCol(hstmt, 2, SQL_C_CHAR, _buf[1], MAX_DATALEN, &buf_len[1]);
		SQLBindCol(hstmt, 3, SQL_C_ULONG, &nlev, 0, &buf_len[2]);
		for (int n = 0; n < MAXORDERNUM; n++) {
			_snprintf_s(buff, sizeof(buff), _TRUNCATE, "select cha_name,job,degree from character where cha_id = %d ", m_nOrder[n].nid);

			sqlret = SQLExecDirect(hstmt, (SQLCHAR*)buff, SQL_NTS);
			if (sqlret != SQL_SUCCESS) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				if (sqlret != SQL_SUCCESS_WITH_INFO) {
					throw 2;
				}
			}
			int i = 0;
			if ((sqlret = SQLFetch(hstmt)) != SQL_NO_DATA) {
				if (sqlret == SQL_NO_DATA) {
					LogLine l_line(g_LogGarner2);
					l_line << newln << "cha name query failed .cha ID：" << m_nOrder[n].nid;
					continue;
				}
				if (sqlret != SQL_SUCCESS) {
					handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
				}
				if (buf_len[0] > 20) {
					LogLine l_line(g_LogGarner2);
					l_line << newln << "cha name query failed.";
					return false;
				}
				memcpy(m_nOrder[n].strname, _buf[0], buf_len[0]);
				m_nOrder[n].strname[buf_len[0]] = '\0';

				memcpy(m_nOrder[n].strjob, g_GetJobName(atoi((const char*)_buf[1])), buf_len[1]);
				m_nOrder[n].strjob[buf_len[1]] = '\0';

				m_nOrder[n].nlev = nlev;
			}
			SQLFreeStmt(hstmt, SQL_CLOSE);
		}
		SQLFreeStmt(hstmt, SQL_UNBIND);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	} catch (int& e) {
		LogLine l_line(g_LogGarner2);
		l_line << newln << "init guild ODBC interface failed, InitParam() erro :" << e;
	} catch (...) {
		LogLine l_line(g_LogGarner2);
		l_line << newln << "Unknown Exception raised when InitParam()";
	}
	return true;
}

bool TBLParam::SaveParam() {
	char buff[255];
	_snprintf_s(buff, sizeof(buff), _TRUNCATE, "update %s set param1 = %d,param2 = %d,param3 = %d,param4 = %d,param5 = %d,param6 = %d,param7 = %d,param8 = %d,param9 = %d,param10 = %d where id = 1", _get_table(), m_nOrder[0].nid, m_nOrder[1].nid, m_nOrder[2].nid, m_nOrder[3].nid, m_nOrder[4].nid, m_nOrder[0].nfightpoint, m_nOrder[1].nfightpoint, m_nOrder[2].nfightpoint, m_nOrder[3].nfightpoint, m_nOrder[4].nfightpoint);

	const SQLRETURN sqlret = _db->exec_sql_direct(buff);
	if (sqlret != SQL_SUCCESS) {
		LG("ParamErr", "Save Param Error SQL = %s", buff);
	}
	return true;
}

bool TBLParam::IsReady() {
	int l_retrow = 0;
	const char* param = "count(*)";
	if (_get_row(m_buf, 1, param, nullptr, &l_retrow)) {
		if (l_retrow == 1 && get_affected_rows() == 1 && atoi(m_buf[0].c_str()) >= 199) {
			return true;
		}
	}
	return false;
}

void TBLParam::UpdateOrder(ORDERINFO& Order) {
	ORDERINFO ordertemp[MAXORDERNUM];

	memcpy(ordertemp, m_nOrder, sizeof(ORDERINFO) * MAXORDERNUM);

	int i = 0;
	int oldid = 0;
	for (i = 0; i < MAXORDERNUM; i++) {
		if (ordertemp[i].nfightpoint >= Order.nfightpoint) {
			if (ordertemp[i].nid == Order.nid) {
				break;
			}
			continue;
		} else {
			oldid = i;
			if (ordertemp[i].nid == Order.nid) {
				m_nOrder[i].nfightpoint = Order.nfightpoint;
				break;
			}
			memcpy(&m_nOrder[i++], &Order, sizeof(ORDERINFO));

			int n = -1;
			for (int a = i; a < MAXORDERNUM;) {
				if (ordertemp[a + n].nid == Order.nid) {
					n++;
					continue;
				}

				if (a + n < MAXORDERNUM) {
					memcpy(&m_nOrder[a], &ordertemp[a + n], sizeof(ORDERINFO));
				} else {
					//strcpy(m_nOrder[a].strjob,"");
					strncpy_s(m_nOrder[a].strjob, sizeof(m_nOrder[a].strjob), "", _TRUNCATE);
					//strcpy(m_nOrder[a].strname,"");
					strncpy_s(m_nOrder[a].strname, sizeof(m_nOrder[a].strname), "", _TRUNCATE);
					m_nOrder[a].nid = -1;
					m_nOrder[a].nlev = 0;
					m_nOrder[a].nfightpoint = 0;
				}
				a++;
			}

			SaveParam();
			WPacket wpk = g_gpsvr->GetWPacket();
			wpk.WriteCmd(CMD_PM_GARNER2_UPDATE);
			for (i = 0; i < MAXORDERNUM; i++) {
				wpk.WriteLong(m_nOrder[i].nid);
			}
			wpk.WriteLong(oldid);
			wpk.WriteLong(0);

			for (auto& gate : g_gpsvr->m_gate) {
				auto pDatasock = gate.GetDataSock();
				if (pDatasock) {
					pDatasock->SendData(wpk);
				}
			}
			LogLine l_line(g_LogGarner2);
			l_line << newln << "order chaned";
			break;
		}
	}
}

#ifdef SHUI_JING
bool TBLCrystalTrade::IsHasBuyorSale(long cha_id, int type) {
	std::string buf[1];
	char filter[180];

	const char* param = "ishang";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, " cha_id = %d and tradetype = %d ", cha_id, type);

	int l_retrow = 0;
	bool l_ret = false;
	l_ret = _get_row(m_buf, 1, param, filter, &l_retrow);

	if (l_ret && l_retrow > 0) {
		const int isHang = atoi(m_buf[0].c_str());
		if (isHang == 1) {
			return true;
		}
	}
	return false;
}
#endif
