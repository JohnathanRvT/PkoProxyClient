#pragma once

#include "GuildData.h"

class CGuildListData;
class CGuildListMgr {
public:
	CGuildListMgr();
	~CGuildListMgr();
	static void AddGuild(CGuildListData* pGuildMember);
	static bool DelGuild(CGuildListData* pGuildMember);
	static bool DelGuildByID(DWORD dwID);
	static bool DelGuildByName(std::string strName);
	static CGuildListData* FindGuildByID(DWORD dwID);
	static CGuildListData* FindGuildByName(std::string strName);
	static CGuildListData* FindGuildByIndex(DWORD dwIndex);
	static void SortGuildsByName(bool bInc = true);
	static void SortGuildsByMemberCount(bool bInc = true);
	static void SortGuildsByExperience(bool bInc = true);
	static DWORD GetTotalGuilds();
	static void ResetAll();
	static void SetType(CGuildData::eType type);
	static CGuildData::eType GetType();

private:
	static std::vector<CGuildListData*> m_pGuildList;
	static CGuildData::eType m_eType;
};
