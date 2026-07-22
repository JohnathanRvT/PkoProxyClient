#define _CRTDBG_MAP_ALLOC
#include "stdafx.h"
#include <stdlib.h>
#include <crtdbg.h>

#include "AccountServer2.h"
#include "gamecommon.h"
#include "inifile.h"
#include "util.h"
#include "GlobalVariable.h"

#pragma warning(disable : 4800)

//TLSIndex g_TlsIndex;
AuthQueue g_Auth; // 认证服务对象

LoginTmpList tmpLogin; //   登陆临时列表

size_t NetBuffer[] = {100, 10, 0};
bool g_logautobak = true;

volatile long AccountServer2::m_nMembersCount = 0;

// Tcp Server
AccountServer2::AccountServer2(ThreadPool* proc, ThreadPool* comm)
	: TcpServerApp(this, proc, comm, false),
	  RPCMGR(this), m_GsHeap(1, 100) {
	//Add by sunny.sun 20090201
	//m_mmsproc = ThreadPool::CreatePool(4, 8, 1024, THREAD_PRIORITY_ABOVE_NORMAL);
	//m_mmscomm = ThreadPool::CreatePool(12, 24, 2048, THREAD_PRIORITY_ABOVE_NORMAL);

	m_billproc = ThreadPool::CreatePool(4, 8, 1024, THREAD_PRIORITY_ABOVE_NORMAL);
	m_billcomm = ThreadPool::CreatePool(12, 24, 2048, THREAD_PRIORITY_ABOVE_NORMAL);

	m_GsHeap.Init();

	IniFile inf(g_strCfgFile.data());
	IniSection& is = inf["net"];
	const char* ip = is["listen_ip"];
	const unsigned short port = atoi(is["listen_port"]);
	//Add by sunny.sun 20090201
	try {
		//m_ToMMS = new ToMMS(g_strCfgFile.c_str(), m_mmsproc, m_mmscomm);
		//m_mmsproc->AddTask(new ConnectMMS(m_ToMMS));

		m_ToBilling = new ToBilling(g_strCfgFile.data(), m_billproc, m_billcomm);
		m_billproc->AddTask(new ConnectBilling(m_ToBilling));
	} catch (...) {
		//m_mmsproc->DestroyPool();
		//m_mmscomm->DestroyPool();

		//if(m_ToMMS)
		//{
		//	delete m_ToMMS;
		//	m_ToMMS = NULL;
		//}

		m_billproc->DestroyPool();
		m_billcomm->DestroyPool();

		SAFE_DELETE(m_ToBilling);
	}

	// the offset of "packet length" is 0
	// the field of "packet length" is 2 bytes
	// the max length of packet is 4K bytes
	// the max length of send queue is 100
	SetPKParse(0, 2, 4 * 1024, 100);

#ifdef _DEBUG
	BeginWork(200);
#else
	BeginWork(30);
#endif

	OpenListenSocket(port, ip);

	ResetMembersCount();
}
AccountServer2::~AccountServer2() {
	ShutDown(12 * 1000);
	//Add by sunny.sun 20090201
	//m_mmsproc->DestroyPool();
	//m_mmscomm->DestroyPool();

	//delete m_ToMMS;

	m_billproc->DestroyPool();
	m_billcomm->DestroyPool();

	delete m_ToBilling;
}

void AccountServer2::IncreaseMembers(long nCount) {
	InterlockedExchangeAdd(&m_nMembersCount, nCount);
}

void AccountServer2::DecreaseMembers(long nCount) {
	InterlockedExchangeAdd(&m_nMembersCount, -nCount);
}

void AccountServer2::ResetMembersCount() {
	InterlockedExchange(&m_nMembersCount, 0);
}

long AccountServer2::GetMembersCount() {
	return m_nMembersCount;
}

bool AccountServer2::OnConnect(DataSocket* datasock) {
	return true;
}
void AccountServer2::OnProcessData(DataSocket* datasock, RPacket& rpkt) {
	const unsigned short usCmd = rpkt.ReadCmd();

	switch (usCmd) {
		// GroupServer协议
	case CMD_PA_LOGOUT: {
		g_Auth.AddPK(datasock, rpkt);
		break;
	}

		// 认证类协议
	case CMD_PA_USER_LOGOUT: {
		g_Auth.AddPK(datasock, rpkt);
		break;
	}
		// 计费类协议-用户开始计费
	case CMD_PA_USER_BILLBGN: {
		IncreaseMembers();
		//sUserLog *pUserLog = new sUserLog;
		//pUserLog->bLogin=true;
		//pUserLog->strUserName=rpkt.ReadString();
		//pUserLog->strPassport=rpkt.ReadString();

		//m_ToBilling->UserLogin(pUserLog->strUserName, pUserLog->strPassport);
		//if (!PostMessage(g_hMainWnd, WM_USER_LOG_MAP, 0, (LPARAM)pUserLog))
		//{
		//	LG("AccountServer", "AccountServer2::OnProcessData, CMD_PA_USER_BILLBGN: PostMessage WM_USER_LOG_MAP failed!\n");
		//	delete pUserLog;
		//}
		std::string userName{rpkt.ReadString()};
		std::string passport{rpkt.ReadString()};
		m_ToBilling->UserLogin(userName, passport);
		break;
	}

		// 计费类协议-用户停止计费
	case CMD_PA_USER_BILLEND: {
		DecreaseMembers();
		//sUserLog *pUserLog = new sUserLog;
		//pUserLog->bLogin=false;
		//pUserLog->strUserName=rpkt.ReadString();

		//m_ToBilling->UserLogout(pUserLog->strUserName);

		//if (!PostMessage(g_hMainWnd, WM_USER_LOG_MAP, 0, (LPARAM)pUserLog))
		//{
		//	LG("AccountServer", "AccountServer2::OnProcessData, CMD_PA_USER_BILLEND: PostMessage WM_USER_LOG_MAP failed!\n");
		//	delete pUserLog;
		//}
		const std::string& userName{rpkt.ReadString()};
		m_ToBilling->UserLogout(userName);
		break;
	}

		// 计费类协议-所有用户结束计费
	case CMD_PA_GROUP_BILLEND_AND_LOGOUT: {
		ResetMembersCount();
		break;
	}
	case CMD_PA_GMBANACCOUNT: {
		std::string actName = rpkt.ReadString();
		g_MainDBHandle.OperAccountBan(actName, 3);
		break;
	}
		// 其它未知协议
	default:
		LG("As2Excp", "Unknown usCmd=[%d]\n", usCmd);
	}
}
WPacket AccountServer2::OnServeCall(DataSocket* datasock, RPacket& rpkt) {
	const unsigned short usCmd = rpkt.ReadCmd();

	switch (usCmd) {
		// GroupServer协议
	case CMD_PA_LOGIN: {
		return Gs_Login(datasock, rpkt);
	}

		// 认证类协议
	case CMD_PA_USER_LOGIN: {
		return g_Auth.SyncPK(datasock, rpkt, 20 * 1000);
	}

		// 其它未知协议
	default: {
		LG("As2Excp", "Unknown usCmd=[%d]\n", usCmd);
		return ProcessUnknownCmd(rpkt);
	}
	}
}
WPacket AccountServer2::ProcessUnknownCmd(RPacket rpkt) {
	WPacket wpkt = GetWPacket();
	wpkt.WriteShort(ERR_AP_UNKNOWNCMD);
	return wpkt;
}
void AccountServer2::OnDisconnect(DataSocket* datasock, int reason) {
	Gs_Logout(datasock);
}

// GroupServer相关方法
void AccountServer2::Gs_Init() {
	m_GsNumber = 0;
	m_GsList = nullptr;
	m_GsHeap.Init();
	m_GsListLock.Create(false);
}
GroupServer2* AccountServer2::Gs_Find(const std::string& szGroupName) {
	GroupServer2* curr = m_GsList;
	GroupServer2* prev{nullptr};

	while (curr != nullptr) {
		if (curr->m_strName == szGroupName) {
			break;
		}
		prev = curr;
		curr = curr->m_next;
	}
	return curr;
}
bool AccountServer2::Gs_Auth(const std::string& szGroupName, const std::string& szGroupPwd) {
	std::string pwd{};
	IniFile inf(g_strCfgFile.data());

	try {
		pwd = inf["gs"][szGroupName.c_str()];
	} catch (...) {
		return false;
	}

	return (pwd == szGroupPwd);
}
void AccountServer2::Gs_ListAdd(GroupServer2* Gs) {
	Gs->m_next = m_GsList;
	m_GsList = Gs;
	++m_GsNumber;
}
void AccountServer2::Gs_ListDel(GroupServer2* Gs) {
	GroupServer2* curr = m_GsList;
	GroupServer2* prev{nullptr};

	while (curr) {
		if (curr == Gs) {
			break;
		}
		prev = curr;
		curr = curr->m_next;
	}

	if (curr) {
		if (prev) {
			prev->m_next = curr->m_next;
		} else {
			m_GsList = curr->m_next;
		}
		--m_GsNumber;
	}
}
void AccountServer2::Gs_Exit() {}
WPacket AccountServer2::Gs_Login(DataSocket* datasock, RPacket& rpkt) {
	/*
2005-4-14 add by Arcol: 
发现此函数会产生多个线程调用,自身需要一个线程同步锁
发现此函数与Gs_Logout可能存在多线程同步运行的情况,此函数需要与Gs_Logout建立一个线程同步锁
若此函数能高速返回(不包含数据库操作),出现线程同步问题机率很低
使用大范围线程同步锁可以解决同一函数内部资源冲突问题,但无法解决交叉函数调用的冲突(虽然此情况出现几率更加低)
最有效的方法是把Gs_Login和Gs_Logout之间的和自身的线程调用转换成队列消息访问,因结构改动比较大,目前未采用此方法
*/

	bool bAuthSuccess = false;
	bool bAlreadyLogin = false;
	std::string szGroupName{rpkt.ReadString()};
	std::string szGroupPwd{rpkt.ReadString()};

	if (FindGroup(szGroupName) != nullptr) {
		bAlreadyLogin = true;
	} else {
		bAuthSuccess = Gs_Auth(szGroupName, szGroupPwd);
	}

	WPacket wpkt = GetWPacket();
	if (bAlreadyLogin) {
		wpkt.WriteShort(ERR_AP_GPSLOGGED);
	} else {
		if (bAuthSuccess) {
			GroupServer2* pGs = m_GsHeap.Get();
			pGs->m_strName = szGroupName;
			pGs->m_strAddr = datasock->GetPeerIP();
			pGs->m_datasock = datasock;

			if (AddGroup(pGs)) {
				// 加入到List成功
				datasock->SetPointer(pGs);
				wpkt.WriteShort(ERR_SUCCESS);
				LG("GroupServer", "[%s] Add Successfully!\n", szGroupName);
			} else {
				// 加入到List失败，说明有同名GroupServer刚刚登录，或存在异常
				pGs->Free();
				bAlreadyLogin = true;
				wpkt.WriteShort(ERR_AP_GPSAUTHFAIL);
			}
		} else {
			wpkt.WriteShort(ERR_AP_GPSAUTHFAIL);
		}
	}

	if (bAlreadyLogin) {
		Disconnect(datasock, 1000);
	}
	return wpkt;
}
void AccountServer2::Gs_Logout(DataSocket* datasock) {
	/*
2005-4-14 add by Arcol: 
发现此函数会产生多个线程调用,自身需要一个线程同步锁
发现此函数与Gs_Login可能存在多线程同步运行的情况,此函数需要与Gs_Login建立一个线程同步锁
若此函数能高速返回(不包含数据库操作),出现线程同步问题机率很低
使用大范围线程同步锁可以解决同一函数内部资源冲突问题,但无法解决交叉函数调用的冲突(虽然此情况出现几率更加低)
最有效的方法是把Gs_Login和Gs_Logout之间的和自身的线程调用转换成队列消息访问,因结构改动比较大,目前未采用此方法
由于GroupServer断开连接后不清空状态,所以AccountServer也不应该清空用户状态,在等待自动重连接后便能恢复正常,这要求GroupServer重启后AccountServer也必须重启
*/
	std::string strGroupName;

	GroupServer2* pGs = (GroupServer2*)datasock->GetPointer();
	if ((pGs == nullptr) || (pGs->m_datasock != datasock)) {
		return;
	}

	strGroupName = pGs->m_strName;
	LG("GroupServer", "[%s] disconnected!\n", strGroupName.c_str());

	if (DelGroup(datasock)) {
		WPacket wpkt = GetWPacket();
		wpkt.WriteCmd(CMD_PA_LOGOUT);
		wpkt.WriteString(strGroupName.c_str());
		RPacket rpkt = RPacket(wpkt);
		OnProcessData(datasock, rpkt);
	}
}
GroupServer2* AccountServer2::FindGroup(const std::string& szGroup) {
	GroupServer2* pGs{nullptr};

	m_GsListLock.lock();
	try {
		pGs = Gs_Find(szGroup);
	} catch (...) {
		LG("As2Excp", "Exception raised from KickAccount when find GroupServer: [%s]\n", szGroup);
	}
	m_GsListLock.unlock();

	return pGs;
}
void AccountServer2::DisplayGroup() {
	extern void ClearGroupList();
	extern BOOL AddGroupToList(char const* strName, char const* strAddr, char const* strStatus);

	GroupServer2* pGs = m_GsList;
	ClearGroupList();
	m_GsListLock.lock();
	try {
		while (pGs != nullptr) {
			AddGroupToList(pGs->GetName(), pGs->GetAddr(), RES_STRING(AS_ACCOUNTSERVER2_CPP_00001));
			pGs = pGs->m_next;
		}
	} catch (...) {
	}
	m_GsListLock.unlock();
}
bool AccountServer2::AddGroup(GroupServer2* pGs) {
	bool bAlreadyLogin = false;

	m_GsListLock.lock();
	try {
		if (Gs_Find(pGs->m_strName.c_str()) != nullptr) {
			bAlreadyLogin = true;
		}

		if (!bAlreadyLogin) {
			Gs_ListAdd(pGs);
		}
	} catch (...) {
		LG("As2Excp", "Exception raised from AddGroup() when add [%s]\n", pGs->m_strName.c_str());
		bAlreadyLogin = true; // 不允许此GroupServer登录
	}
	m_GsListLock.unlock();

	return !bAlreadyLogin;
}
bool AccountServer2::DelGroup(DataSocket* datasock) {
	bool bDel = false;
	GroupServer2* pGs{nullptr};

	m_GsListLock.lock();
	try {
		// 再作一次检测!
		pGs = (GroupServer2*)datasock->GetPointer();
		if ((pGs != nullptr) && (pGs->m_datasock == datasock)) {
			Gs_ListDel(pGs);
			datasock->SetPointer(nullptr);
			bDel = true;
			pGs->Free();
		}
	} catch (...) {
		LG("As2Excp", "Exception raised from AddGroup() when add [%s]\n", pGs->m_strName.c_str());
	}
	m_GsListLock.unlock();

	return bDel;
}

// Auth
AuthQueue::AuthQueue() : PKQueue(false) {}
AuthQueue::~AuthQueue() {}
void AuthQueue::ProcessData(DataSocket* datasock, RPacket& rpkt) {
	bool bRetry = true;
	const unsigned short usCmd = rpkt.ReadCmd();

	// 得到当前线程对象
	auto pThis = static_cast<AuthThread*>(g_TlsIndex.GetPointer());
	if (!pThis) {
		return;
	}

	while (bRetry) {
		try {
			switch (usCmd) {
			case CMD_PA_LOGOUT: {
				pThis->LogoutGroup(datasock, rpkt);
			} break;

			case CMD_PA_USER_LOGOUT: {
				pThis->AccountLogout(rpkt);
			} break;

			case CMD_PA_USER_BILLBGN: {
				pThis->BeginBilling(rpkt);
			} break;

			case CMD_PA_USER_BILLEND: {
				pThis->EndBilling(rpkt);
			} break;

			case CMD_PA_GROUP_BILLEND_AND_LOGOUT: {
				pThis->ResetBilling();
			} break;
			default:
				LG("AuthProcessData", "Unknown usCmd=[%d], Skipped...\n", usCmd);
				break;
			}
			bRetry = false;
		} catch (CSQLException* se) {
			LG("AuthProcessDataExcp", "SQL Exception: %s\n", se->m_strError.c_str());

			// 重连
			pThis->Reconnt();
		} catch (...) {
			LG("AuthProcessDataExcp", "unknown exception raised from AuthQueue::ProcessData()\n");
			bRetry = false; // 非数据库造成的异常放过
		}
	}
}

WPacket AuthQueue::ServeCall(DataSocket* datasock, RPacket& rpkt) {
	bool bRetry = true;
	const unsigned short usCmd = rpkt.ReadCmd();
	WPacket wpkt = datasock->GetWPacket();

	// 得到当前线程对象
	auto pThis = static_cast<AuthThread*>(g_TlsIndex.GetPointer());
	if (!pThis) {
		LG("AuthExcp", "pThis = NULL\n");
		wpkt.WriteShort(ERR_AP_TLSWRONG);
		return wpkt;
	}

	while (bRetry) {
		try {
			switch (usCmd) {
			case CMD_PA_USER_LOGIN: {
				pThis->QueryAccount(rpkt);
				return pThis->AccountLogin(datasock);
			}

			default: {
				LG("AuthServeCall", "Unknown usCmd=[%d], Skipped...\n", usCmd);
				wpkt.WriteShort(ERR_AP_UNKNOWNCMD);
				return wpkt;
			}
			}
		} catch (CSQLException* se) {
			LG("AuthServeCallExcp", "SQL Exception: %s\n", se->m_strError.c_str());

			// 重连
			pThis->Reconnt();
		} catch (...) {
			LG("AuthServeCallExcp", "unknown exception raised from AuthQueue::ServerCall()\n");
			bRetry = false; // 非数据库造成的异常放过
		}
	}

	wpkt.WriteShort(ERR_AP_UNKNOWN);
	return wpkt;
}

//  LoginTmpList
LoginTmpList::LoginTmpList() {
	InitializeCriticalSection(&m_cs);
}

LoginTmpList::~LoginTmpList() {
	DeleteCriticalSection(&m_cs);
}

bool LoginTmpList::Insert(const std::string& name) {
	bool ret = false;
	Lock();
	if (!Query(name, false)) {
		m_list.push_back(name);
		ret = true;
	}
	UnLock();
	return ret;
}

bool LoginTmpList::Remove(const std::string& name) {
	bool ret = false;
	Lock();

	auto it = std::find(m_list.begin(), m_list.end(), name);
	if (it != m_list.end()) {
		ret = true;
		m_list.erase(it);
	}

	UnLock();
	return ret;
}

bool LoginTmpList::Query(const std::string& name, bool lock /* = true*/) {
	bool ret = false;
	if (lock) {
		Lock();
	}

	auto it = std::find(m_list.begin(), m_list.end(), name);
	if (it != m_list.end()) {
		ret = true;
	}

	if (lock) {
		UnLock();
	}
	return ret;
}

void LoginTmpList::Lock() {
	EnterCriticalSection(&m_cs);
}

void LoginTmpList::UnLock() {
	LeaveCriticalSection(&m_cs);
}

// AuthThread
Sema AuthThread::m_Sema(0, AuthThreadPool::AT_MAXNUM);
std::string AuthThread::m_strSrvip = "";
std::string AuthThread::m_strSrvdb = "";
std::string AuthThread::m_strUserId = "";
std::string AuthThread::m_strUserPwd = "";
std::string AuthThread::m_strAccountTableName = "account_login";
AuthThread::AuthThread(int nIndex) : m_nIndex(nIndex) {}
AuthThread::~AuthThread() { Exit(); }
void AuthThread::Init() {
	g_TlsIndex.SetPointer(nullptr);
	m_pAuth = nullptr;

	SetRunLabel(-1);

	while (!Connect()) {
		if (GetExitFlag()) {
			return;
		}
		Sleep(5000);
	}
	g_TlsIndex.SetPointer(this);
	ResetAccount();

	SetRunLabel(0);
}
void AuthThread::Exit() {
	Disconn();
	g_TlsIndex.SetPointer(nullptr);
}
bool AuthThread::Connect() {
	bool ret = true;
	if (m_pAuth != nullptr) {
		return true;
	}

	// 创建对象
	try {
		m_pAuth = new CSQLDatabase();
	} catch (std::bad_alloc& ba) {
		LG("AuthDBExcp", "AuthThread::Connect() new failed : %s\n", ba.what());
		m_pAuth = nullptr;
		return false;
	} catch (...) {
		LG("AuthDBExcp", "AuthThread::Connect() unknown exception\n");
		m_pAuth = nullptr;
		return false;
	}

	// 连接database
	char conn_str[512] = {0};
	char const* conn_fmt = "DRIVER={SQL Server};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s";
	//sprintf(conn_str, conn_fmt, m_strSrvip.c_str(), m_strUserId.c_str(),
	_snprintf_s(conn_str, sizeof(conn_str), _TRUNCATE, conn_fmt, m_strSrvip.c_str(), m_strUserId.c_str(),
				m_strUserPwd.c_str(), m_strSrvdb.c_str());
	ret = m_pAuth->Open(conn_str);
	if (ret) {
		m_pAuth->SetAutoCommit(true);
	} else {
		delete m_pAuth;
		m_pAuth = nullptr;
	}

	return ret;
}
void AuthThread::Disconn() {
	if (m_pAuth != nullptr) {
		try {
			m_pAuth->Close();
			delete m_pAuth;
		} catch (...) {
			LG("AuthExcp", "Exception raised when AuthThread::Disconn()\n");
		}
		m_pAuth = nullptr;
	}
}
void AuthThread::Reconnt() {
	Disconn();
	while (!Connect()) {
		LG("As2", RES_STRING(AS_ACCOUNTSERVER2_CPP_00002));
		if (GetExitFlag()) {
			return;
		}

		Sleep(5000);
	}
}
void AuthThread::LoadConfig() {
	char buf[80];
	IniFile inf(g_strCfgFile.data());
	IniSection& is = inf["db"];
	std::string strTmp = "";

	try {
		_snprintf_s(buf, sizeof(buf), "dbserver", _TRUNCATE);
		m_strSrvip = is[buf].c_str();
		_snprintf_s(buf, sizeof(buf), "db", _TRUNCATE);
		m_strSrvdb = is[buf].c_str();
		_snprintf_s(buf, sizeof(buf), "userid", _TRUNCATE);
		m_strUserId = is[buf].c_str();
		_snprintf_s(buf, sizeof(buf), "passwd", _TRUNCATE);
		strTmp = is[buf].c_str();
		//m_strUserPwd = strTmp;
	} catch (excp& e) {
		std::cout << e.what() << std::endl;
		getchar();
		ExitProcess(-1);
	}
	dbpswd_out(strTmp.c_str(), (int)strTmp.length(), m_strUserPwd);
}

void AuthThread::LogUserLogin(int nUserID, std::string strUserName, std::string strIP) {
	auto* pUserLog = new sUserLog;
	pUserLog->bLogin = true;
	pUserLog->nUserID = nUserID;
	pUserLog->strUserName = strUserName;
	pUserLog->strLoginIP = strIP;
	if (!PostMessage(g_hMainWnd, WM_USER_LOG, 0, (LPARAM)pUserLog)) {
		LG("AccountServer", "AuthThread::LogUserLogin: PostMessage WM_USER_LOG failed!\n");
		delete pUserLog;
	}
}

void AuthThread::LogUserLogout(int nUserID) {
	auto* pUserLog = new sUserLog;
	pUserLog->bLogin = false;
	pUserLog->nUserID = nUserID;
	if (!PostMessage(g_hMainWnd, WM_USER_LOG, 0, (LPARAM)pUserLog)) {
		LG("AccountServer", "AuthThread::LogUserLogout: PostMessage WM_USER_LOG failed!\n");
		delete pUserLog;
	}
}

void AuthThread::QueryAccount(RPacket rpkt) {
	unsigned short usNameLen;
	char szSql[512] = {0};
	SetRunLabel(11);

	// 取包中的信息，第一个是无用信息，丢弃(shit!)
	//LG("AuthThread", ",%d\n", ::GetCurrentThreadId());

	//Add by sunny.sun 20081223
	std::string szLocale{rpkt.ReadString()};
	std::string pName{rpkt.ReadString()};

	// 第二个是帐号名信息
	pName = rpkt.ReadString(&usNameLen);
	LG("PASSWD", "From GroupServer [%s] = [%d]\n", pName.c_str(), pName.length());
	if ((pName.empty()) || (!IsValidName(pName.c_str(), usNameLen))) {
		LG("AuthExcp", "NULL or INVALID Name field\n");
		m_AcctInfo.bExist = false;
		return;
	}

	m_AcctInfo.strName = pName;
	m_AcctInfo.pDat = rpkt.ReadSequence(m_AcctInfo.usDatLen); // 加密信息
	m_AcctInfo.strMAC = rpkt.ReadString();
	m_AcctInfo.strChapString = rpkt.ReadString();
	in_addr ipAddr;
	ipAddr.S_un.S_addr = rpkt.ReadLong();
	m_AcctInfo.strIP = inet_ntoa(ipAddr);

	// 查询对象
	SetRunLabel(12);
	CSQLRecordset rs(*m_pAuth);

	// 组织SQL查询语句
	_snprintf_s(szSql, sizeof(szSql), _TRUNCATE, "select id, password, sid, login_status, login_group, ban, datediff(s, enable_login_time, getdate()) as protect_time from account_login where name='%s'",
				m_AcctInfo.strName.c_str());
	rs << szSql; //account_login表中name字段一定要做唯一约束

	// 执行查询
	rs.SQLExecDirect();
	SetRunLabel(13);

	// 产生查询结果
	if (rs.SQLFetch()) {
		int n = 1;
		m_AcctInfo.bExist = true;
		m_AcctInfo.nId = rs.nSQLGetData(n++);
		m_AcctInfo.strPwdDigest = rs.SQLGetData(n++);
		m_AcctInfo.nSid = rs.nSQLGetData(n++);
		m_AcctInfo.nStatus = rs.nSQLGetData(n++);
		m_AcctInfo.strGroup = rs.SQLGetData(n++);
		// Modify by lark.li 20080825 begin
		//m_AcctInfo.bBan=rs.bSQLGetData(n++);
		m_AcctInfo.nBan = rs.nSQLGetData(n++);
		// End
		m_AcctInfo.nProtectTime = rs.nSQLGetData(n++);

		//static bool flag = true; // 测试重复登陆用代码
		//if(flag)
		//{
		//	::Sleep(5000);
		//}
		//flag = !flag;

		if (!tmpLogin.Insert(m_AcctInfo.strName)) {
			m_AcctInfo.nStatus = ACCOUNT_ONLINE;
			LG("AuthExcp", "Account %s multilogin at same times.\n", m_AcctInfo.strName.c_str());
		}
	} else {
		m_AcctInfo.bExist = false;
	}
}
bool AuthThread::IsValidName(char const* szName, unsigned short usNameLen) {
	//Tom的版本帐号允许有"_" "-" "."3种字符
	if (usNameLen > 32) {
		return false;
	}

	auto l_name = reinterpret_cast<const unsigned char*>(szName);
	bool l_ishan = false;
	for (unsigned short i = 0; i < usNameLen; i++) {
		if (!l_name[i]) {
			return false;
		} else if (l_ishan) {
			if (l_name[i - 1] == 0xA1 && l_name[i] == 0xA1) { //过滤全角空格
				return false;
			}
			if (l_name[i] > 0x3F && l_name[i] < 0xFF && l_name[i] != 0x7F) {
				l_ishan = false;
			} else {
				return false;
			}
		} else if (l_name[i] > 0x80 && l_name[i] < 0xFF) {
			l_ishan = true;
		} else if ((l_name[i] >= 'A' && l_name[i] <= 'Z') || (l_name[i] >= 'a' && l_name[i] <= 'z') || (l_name[i] >= '0' && l_name[i] <= '9') || (l_name[i] == '@') || (l_name[i] == '.')) {
		} else {
			return false;
		}
	}
	return !l_ishan;

	//if (usNameLen > 20) return false;
	//if (strstr(szName, "'") != NULL) return false;
	//if (strstr(szName, "-") != NULL) return false;
	//return true;
}

WPacket AuthThread::AccountLogin(DataSocket* datasock) {
	const DWORD dwStartCount = GetTickCount();

	SetRunLabel(14);
	WPacket wpkt = datasock->GetWPacket();
	auto pFromGroupServer = static_cast<GroupServer2*>(datasock->GetPointer());
	if (!pFromGroupServer) {
		// 此GroupServer已断掉
		LG("AuthExcp", "pFromGroupServer = NULL\n");
		wpkt.WriteShort(ERR_AP_DISCONN);
		return wpkt;
	}
	SetRunLabel(15);

	wpkt.WriteCmd(0); //added by andor.zhang,避免缓存重用原因GroupServer读到CMD_AP_KICKUSER命令码

	if (!m_AcctInfo.bExist) {
		// 不存在此用户
		wpkt.WriteShort(ERR_AP_INVALIDUSER);
		return wpkt;
	}
	if (m_AcctInfo.nBan == 2) {
		tmpLogin.Remove(m_AcctInfo.strName);
		wpkt.WriteShort(ERR_AP_BANUSER);
		return wpkt;
	} else if (m_AcctInfo.nBan == 1) {
		tmpLogin.Remove(m_AcctInfo.strName);
		wpkt.WriteShort(ERR_AP_PBANUSER);
		return wpkt;
	} else if (m_AcctInfo.nBan == 3) {
		tmpLogin.Remove(m_AcctInfo.strName);
		wpkt.WriteShort(ERR_AP_SBANUSER);
		return wpkt;
	}

	SetRunLabel(16);

	KCHAPs ChapSvr;
	bool bVerify = false;
	ChapSvr.NewSessionKey();
	bVerify = ChapSvr.DoAuth(m_AcctInfo.strName.c_str(), m_AcctInfo.strChapString.c_str(), m_AcctInfo.pDat, m_AcctInfo.usDatLen, m_AcctInfo.strPwdDigest.c_str());
	ChapSvr.NewSessionKey();
	if (!bVerify) { // 认证失败
		wpkt.WriteShort(ERR_AP_INVALIDPWD);
		LG("AccountAuth", "Thread#%d Auth [%s] (id=%d) failed: invalid password!\n", m_nIndex, m_AcctInfo.strName.c_str(), m_AcctInfo.nId);
		tmpLogin.Remove(m_AcctInfo.strName);
		return wpkt;
	}
	SetRunLabel(17);

	int nKeyLen = 0;
	char szKey[1024] = {0};
	char lpszSQL[2048] = {0};
	int nSid = GenSid(m_AcctInfo.strName.c_str());
	if (m_AcctInfo.nStatus == ACCOUNT_OFFLINE) {
		//登陆前状态是不在线(正常登陆)
		_snprintf_s(lpszSQL, sizeof(lpszSQL), _TRUNCATE, "update account_login set login_status=%d, login_group='%s', enable_login_time=getdate(), last_login_time=getdate(), last_login_mac='%s', last_login_ip='%s' where id=%d",
					ACCOUNT_ONLINE, pFromGroupServer->GetName(), m_AcctInfo.strMAC.c_str(), m_AcctInfo.strIP.c_str(), m_AcctInfo.nId);
		if (m_pAuth->ExecuteSQL(lpszSQL)) {
			//Add by sunny.sun 20090205 for album
			//Win32Guid id;
			//id.Generate();
			//std::string SessionKey = id.AsString();
			//char albumSQL[2048] = {0};
			//_snprintf_s( albumSQL, sizeof(albumSQL), _TRUNCATE,"IF EXISTS(SELECT act_name FROM act_album WHERE act_name= '%s')UPDATE act_album SET SessionKey='%s', update_time = getdate() WHERE act_name= '%s' ELSE INSERT INTO act_album(act_name,SessionKey,act_id,create_time,update_time) VALUES('%s','%s',%d,getdate(),getdate())"
			//	,m_AcctInfo.strName.c_str(), SessionKey.c_str(), m_AcctInfo.strName.c_str(), m_AcctInfo.strName.c_str(), SessionKey.c_str(), m_AcctInfo.nId);
			//if(m_pAuth->ExecuteSQL(albumSQL))
			{
				SetRunLabel(18);
				wpkt.WriteShort(ERR_SUCCESS);
				wpkt.WriteLong(m_AcctInfo.nId);
				ChapSvr.GetSessionEncKey((unsigned char*)szKey, nKeyLen);
				wpkt.WriteSequence(szKey, nKeyLen);
				ChapSvr.GetSessionClrKey(szKey, nKeyLen);
				wpkt.WriteSequence(szKey, nKeyLen);
				wpkt.WriteLong(nSid);
				//Add by sunny.sun 20090205
				//wpkt.WriteString( SessionKey.c_str() );
				wpkt.WriteString("temp");
				LogUserLogin(m_AcctInfo.nId, m_AcctInfo.strName.c_str(), m_AcctInfo.strIP.c_str());
			}
			//else
			{
				//	wpkt.WriteShort(ERR_AP_UNKNOWN);
				//	LG("AccountAuth", "Thread#%d Auth [%s] (id=%d) failed: update database error where normal login!\n", m_nIndex, m_AcctInfo.strName.c_str(), m_AcctInfo.nId);
				//	SetRunLabel(19);
				//	goto login_over;
			}
		} else {
			wpkt.WriteShort(ERR_AP_UNKNOWN);
			LG("AccountAuth", "Thread#%d Auth [%s] (id=%d) failed: update database error where normal login!\n", m_nIndex, m_AcctInfo.strName.c_str(), m_AcctInfo.nId);
			SetRunLabel(19);
			goto login_over;
		}
	} else if (m_AcctInfo.nStatus == ACCOUNT_ONLINE) {
		//登陆前状态是已在线(重复登陆)
		_snprintf_s(lpszSQL, sizeof(lpszSQL), _TRUNCATE, "update account_login set login_status=%d, login_group='%s', last_login_time=getdate(), last_login_mac='%s', last_login_ip='%s' where id=%d",
					ACCOUNT_SAVING, pFromGroupServer->GetName(), m_AcctInfo.strMAC.c_str(), m_AcctInfo.strIP.c_str(), m_AcctInfo.nId);
		if (m_pAuth->ExecuteSQL(lpszSQL)) {
			if (pFromGroupServer->IsSame(m_AcctInfo.strGroup)) {
				wpkt.WriteCmd(CMD_AP_KICKUSER);
				wpkt.WriteShort(ERR_AP_ONLINE);
				wpkt.WriteLong(m_AcctInfo.nId);
			} else {
				KickAccount(m_AcctInfo.strGroup, m_AcctInfo.nId);
			}
			SetRunLabel(20);
			goto login_over;
		} else {
			wpkt.WriteShort(ERR_AP_UNKNOWN);
			LG("AccountAuth", "Thread#%d Auth [%s] (id=%d) failed: update database error when relogin!\n", m_nIndex, m_AcctInfo.strName.c_str(), m_AcctInfo.nId);
			SetRunLabel(21);
			goto login_over;
		}
	} else if (m_AcctInfo.nStatus == ACCOUNT_SAVING) {
		//帐号在锁定状态(保存角色)
		if (m_AcctInfo.nProtectTime >= 0 && m_AcctInfo.nProtectTime < SAVING_TIME) {
			wpkt.WriteShort(ERR_AP_SAVING);
			SetRunLabel(22);
			goto login_over;
		} else {
			//sprintf(lpszSQL, "update account_login set login_status=%d, login_group='%s', enable_login_time=getdate(), last_login_time=getdate(), last_login_mac='%s', last_login_ip='%s' where id=%d",
			_snprintf_s(lpszSQL, sizeof(lpszSQL), _TRUNCATE, "update account_login set login_status=%d, login_group='%s', enable_login_time=getdate(), last_login_time=getdate(), last_login_mac='%s', last_login_ip='%s' where id=%d",
						ACCOUNT_ONLINE, pFromGroupServer->GetName(), m_AcctInfo.strMAC.c_str(), m_AcctInfo.strIP.c_str(), m_AcctInfo.nId);
			if (m_pAuth->ExecuteSQL(lpszSQL)) {
				//Win32Guid id;
				//id.Generate();
				//std::string SessionKey = id.AsString();
				//char albumSQL[2048] = {0};
				//_snprintf_s( albumSQL, sizeof(albumSQL), _TRUNCATE,"IF EXISTS(SELECT act_name FROM act_album WHERE act_name= '%s')UPDATE act_album SET SessionKey='%s', update_time = getdate() WHERE act_name= '%s' ELSE INSERT INTO act_album(act_name,SessionKey,act_id,create_time,update_time) VALUES('%s','%s',%d,getdate(),getdate())"
				//	,m_AcctInfo.strName.c_str(), SessionKey.c_str(), m_AcctInfo.strName.c_str(), m_AcctInfo.strName.c_str(), SessionKey.c_str(), m_AcctInfo.nId);
				//if(m_pAuth->ExecuteSQL(albumSQL))
				{
					SetRunLabel(23);
					wpkt.WriteShort(ERR_SUCCESS);
					wpkt.WriteLong(m_AcctInfo.nId);
					ChapSvr.GetSessionEncKey((unsigned char*)szKey, nKeyLen);
					wpkt.WriteSequence(szKey, nKeyLen);
					ChapSvr.GetSessionClrKey(szKey, nKeyLen);
					wpkt.WriteSequence(szKey, nKeyLen);
					wpkt.WriteLong(nSid);
					//wpkt.WriteString( SessionKey.c_str() );
					wpkt.WriteString("temp");
					LogUserLogin(m_AcctInfo.nId, m_AcctInfo.strName.c_str(), m_AcctInfo.strIP.c_str());
				}
				//else
				//{
				//	wpkt.WriteShort(ERR_AP_UNKNOWN);
				//	LG("AccountAuth", "Thread#%d Auth [%s] (id=%d) failed: update database error when login without locked!\n", m_nIndex, m_AcctInfo.strName.c_str(), m_AcctInfo.nId);
				//	SetRunLabel(24);
				//	goto login_over;
				//}
			} else {
				wpkt.WriteShort(ERR_AP_UNKNOWN);
				LG("AccountAuth", "Thread#%d Auth [%s] (id=%d) failed: update database error when login without locked!\n", m_nIndex, m_AcctInfo.strName.c_str(), m_AcctInfo.nId);
				SetRunLabel(24);
				goto login_over;
			}
		}
	} else {
		//登陆前状态是不确定
		wpkt.WriteShort(ERR_AP_UNKNOWN);
		LG("AccountAuth", "Thread#%d Auth [%s] (id=%d) failed: unknown last login status!\n", m_nIndex, m_AcctInfo.strName.c_str(), m_AcctInfo.nId);
		SetRunLabel(25);
		goto login_over;
	}
	SetRunLabel(0);

login_over:

	const DWORD dwEndCount = GetTickCount() - dwStartCount;
	AuthThreadPool::RunLast[m_nIndex] = dwEndCount;
	if (dwEndCount > AuthThreadPool::RunConsume[m_nIndex]) {
		AuthThreadPool::RunConsume[m_nIndex] = dwEndCount;
	}
	tmpLogin.Remove(m_AcctInfo.strName);
	return wpkt;
}

void AuthThread::AccountLogout(RPacket rpkt) {
	SetRunLabel(99);
	char lpszSQL[2048] = {0};
	const int nID = rpkt.ReadLong();
	_snprintf_s(lpszSQL, sizeof(lpszSQL), _TRUNCATE, "update account_login set login_status=%d, login_group='', enable_login_time=getdate(), last_logout_time=getdate(), total_live_time=total_live_time+datediff(second, last_login_time, getdate()) where id=%d", ACCOUNT_OFFLINE, nID); //  增加在线时间 by jampe
	if (m_pAuth->ExecuteSQL(lpszSQL)) {
		SetRunLabel(0);
	}
	LogUserLogout(nID);
}

void AuthThread::BeginBilling(RPacket& rpkt) {
}

void AuthThread::EndBilling(RPacket& rpkt) {
}

void AuthThread::ResetBilling() {
}

void AuthThread::LogoutGroup(DataSocket* datasock, RPacket rpkt) {
	/*
2005-4-14 added by arcol:
服务器断开后会立即自动重连,因此不清除帐号状态,要求GroupServer重启后AccountServer也需要重启
*/

	SetRunLabel(0);
}

CSQLDatabase* AuthThread::GetSQLDatabase() {
	return m_pAuth;
}

long AuthThread::GenSid(std::string szName) {
#define SHA1_DIGEST_LEN 20
	char md[SHA1_DIGEST_LEN];

	// 产生信息源
	char buf[256];
	const int buf_len = _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%s%d", szName.c_str(), GetTickCount());
	if (buf_len >= sizeof(buf)) {
		throw std::out_of_range("buffer overflow in GenSid()\n");
	}

	// 生成摘要
	sha1((unsigned char*)buf, buf_len, (unsigned char*)md);

	// 取出前4个字节
	long* ptr = (long*)md;
	return ptr[0];
}
void AuthThread::ResetAccount() {
	/*
2005-4-17 added by Arcol:
这里代码看得我头晕,如有能力建议重写,目前外部主线程有CDataBaseCtrl对象可以初始化及后期化数据库
注意:这里的m_nIndex为0时不代表一定是首次执行,因此,可能出现一种情况,当其它线程已经开展验证服务工作,而此时0号索引线程才进来初始化表格会造成表格资源访问冲突,导致初始化失败
*/
	//放弃使用后面的代码 2005-4-17
}
void AuthThread::KickAccount(std::string& strGroup, int nId) {
	GroupServer2* pGs = g_As2->FindGroup(strGroup.c_str());
	if (pGs) {
		WPacket wpkt = pGs->GetWPacket();
		wpkt.WriteCmd(CMD_AP_KICKUSER);
		wpkt.WriteShort(ERR_AP_LOGINTWICE);
		wpkt.WriteLong(nId);
		pGs->SendData(wpkt);
	}
}
int AuthThread::Run() {
	Init();
	while (!GetExitFlag()) {
		g_Auth.PeekPacket(1000); // 给于1秒的时间来采集队列中的网络包
	}
	Exit();

	ExitThread();
	return 0;
}
void AuthThread::SetRunLabel(int nRunLabel) {
	AuthThreadPool::RunLabel[m_nIndex] = nRunLabel;
}

//
int volatile AuthThreadPool::RunLabel[AT_MAXNUM] = {0};
DWORD volatile AuthThreadPool::RunLast[AT_MAXNUM] = {0};
DWORD volatile AuthThreadPool::RunConsume[AT_MAXNUM] = {0};
unsigned int volatile AuthThreadPool::uiAuthCount = 0;
void AuthThreadPool::IncAuthCount() { ++uiAuthCount; }
unsigned int AuthThreadPool::GetAuthCount() { return uiAuthCount; }
AuthThreadPool::AuthThreadPool() {

	for (size_t i = 0; i < m_Pool.size(); ++i) {
		m_Pool[i] = std::make_unique<AuthThread>((int)i);
	}
	AuthThread::LoadConfig();
}
AuthThreadPool::~AuthThreadPool() {
}
void AuthThreadPool::Launch() {
	for (auto& pool : m_Pool) {
		pool->Launch();
	}
}
void AuthThreadPool::NotifyToExit() {
	for (auto& pool : m_Pool) {
		pool->NotifyToExit();
	}
}
void AuthThreadPool::WaitForExit() {
	printf(RES_STRING(AS_ACCOUNTSERVER2_CPP_00006));
	for (auto& pool : m_Pool) {
		pool->WaitForExit(-1);
	}
}

// 网络框架对象
AccountServer2* g_As2 = nullptr;
