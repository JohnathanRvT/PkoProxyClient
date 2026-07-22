#pragma once

#include "atltime.h"

#include <map>

class CSQLDatabase;

//此类只允许主线程调用
class CDataBaseCtrl {
public:
	CDataBaseCtrl();
	~CDataBaseCtrl();
	bool CreateObject();
	void ReleaseObject();
	bool KickUser(std::string strUserName);

	void SetExpScale(std::string strUserName, long time);

	bool UserLogin(int nUserID, std::string strUserName, std::string strIP);
	bool UserLogout(int nUserID);
	bool UserLoginMap(std::string strUserName, std::string strPassport);
	bool UserLogoutMap(std::string strUserName);
	bool OperAccountBan(std::string strActName, int iban); //Add by sunny.sun 20090828
private:
	struct sPlayerData {
		CTime ctLoginTime;
	};
	using StringMap = std::map<std::string, CDataBaseCtrl::sPlayerData>;

private:
	bool Connect();
	bool IsConnect();
	void Disconnect();

private:
	std::string m_strServerIP;
	std::string m_strServerDB;
	std::string m_strUserID;
	std::string m_strUserPwd;
	CSQLDatabase* m_pDataBase;
	StringMap m_mapUsers;
};
