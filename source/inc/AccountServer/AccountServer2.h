
#ifndef ACCOUNTSERVER2_H_
#define ACCOUNTSERVER2_H_

#include "time.h"
#include "commrpc.h"
#include "threadpool.h"
#include "packetqueue.h"
#include "tlsindex.h"
#include "util.h"

#include "MyThread.h"

using namespace dbc;

class ToMMS;
class ToBilling;

struct GroupServer2 : PreAllocStru {
	GroupServer2(size_t size) : PreAllocStru(size) {}

	WPacket GetWPacket() { return m_datasock->GetWPacket(); }
	long SendData(WPacket wpkt) { return m_datasock->SendData(wpkt); }
	bool IsSame(std::string const& strGroup) { return (m_strName == strGroup); }
	char const* const GetName() const { return m_strName.c_str(); }
	char const* const GetAddr() const { return m_strAddr.c_str(); }

	std::string m_strName;
	std::string m_strAddr;
	DataSocket* m_datasock{nullptr};
	GroupServer2* m_next{nullptr};
};

class AccountServer2 : public TcpServerApp, public RPCMGR {
public:
	AccountServer2(ThreadPool* proc = nullptr, ThreadPool* comm = nullptr);
	virtual ~AccountServer2();

	GroupServer2* FindGroup(const std::string& szGroup);
	void DisplayGroup();

	void IncreaseMembers(long nCount = 1);
	void DecreaseMembers(long nCount = 1);
	void ResetMembersCount();
	long GetMembersCount();

protected:
	virtual bool OnConnect(DataSocket* datasock) override;
	virtual void OnProcessData(DataSocket* datasock, RPacket& rpkt) override;
	virtual WPacket OnServeCall(DataSocket* datasock, RPacket& rpkt) override;
	virtual void OnDisconnect(DataSocket* datasock, int reason) override;

private:
	static volatile long m_nMembersCount;

	// GroupServer 相关处理例程和数据
	void Gs_Init();
	GroupServer2* Gs_Find(const std::string& szGroupName);
	bool Gs_Auth(const std::string& szGroupName, const std::string& szGroupPwd);
	void Gs_ListAdd(GroupServer2* Gs);
	void Gs_ListDel(GroupServer2* Gs);
	void Gs_Exit();

	PreAllocHeap<GroupServer2> m_GsHeap;
	GroupServer2* m_GsList{nullptr};
	Mutex m_GsListLock;
	short m_GsNumber{0};

	WPacket
	Gs_Login(DataSocket* datasock, RPacket& rpkt);
	bool AddGroup(GroupServer2* pGs);
	bool DelGroup(DataSocket* datasock);
	void Gs_Logout(DataSocket* datasock);

	WPacket ProcessUnknownCmd(RPacket rpkt);
	//Add by sunny.sun 20090201
public:
	//ThreadPool* m_mmsproc, * m_mmscomm;	//同MMS的任务池
	//ToMMS* m_ToMMS;	//同MMS的连接对象（主动重连机制）

	ThreadPool *m_billproc, *m_billcomm; //同Billing的任务池
	ToBilling* m_ToBilling;				 // 同BillingSerena的连接对象
};

// 认证

class AuthQueue : public PKQueue {
public:
	AuthQueue();
	virtual ~AuthQueue();

protected:
	virtual void ProcessData(DataSocket* datasock, RPacket& rpkt) override;
	virtual WPacket ServeCall(DataSocket* datasock, RPacket& rpkt) override;
};

//  登陆临时队列        by Jampe
using TmpNameList = std::vector<std::string>;
class LoginTmpList {
public:
	LoginTmpList();
	virtual ~LoginTmpList();

	bool Insert(const std::string& name);
	bool Remove(const std::string& name);
	bool Query(const std::string& name, bool lock = true);

	void Lock();
	void UnLock();

private:
	CRITICAL_SECTION m_cs;
	TmpNameList m_list;
};

class AuthThread : public MyThread {
public:
	AuthThread(int nIndex);
	virtual ~AuthThread();

	void Reconnt();

	void QueryAccount(RPacket rpkt);
	bool IsValidName(char const* szName, unsigned short usNameLen);

	WPacket AccountLogin(DataSocket* datasock);
	void AccountLogout(RPacket rpkt);
	WPacket TomAccountLogin(DataSocket* datasock, RPacket& rpkt);
	void TomAccountLogout(RPacket& rpkt);

	void BeginBilling(RPacket& rpkt);
	void EndBilling(RPacket& rpkt);
	void ResetBilling();

	void LogoutGroup(DataSocket* datasock, RPacket rpkt);

	static void LoadConfig();

	CSQLDatabase* GetSQLDatabase();

protected:
	void Init();
	void Exit();
	bool Connect();
	void Disconn();

	virtual int Run() override;
	void SetRunLabel(int nRunLabel);
	void LogUserLogin(int nUserID, std::string strUserName, std::string strIP);
	void LogUserLogout(int nUserID);

	enum {
		ACCOUNT_OFFLINE,
		ACCOUNT_ONLINE,
		ACCOUNT_SAVING,
	};

	struct AccountInfo {
		bool bExist;
		int nId;
		bool bAllowLogin;
		std::string strName;
		std::string strPwdDigest;
		int nSid;
		int nStatus;
		int nEnableLoginTick;
		std::string strGroup;
		std::string strChapString;
		char const* pDat;
		unsigned short usDatLen;
		std::string strMAC;
		std::string strIP;
		// Modify by lark.li 20080825 begin
		//bool bBan;
		int nBan;
		// End

		int nProtectTime;
	};

	long GenSid(std::string szName);
	void ResetAccount();
	void KickAccount(std::string& strGroup, int nId);

private:
	enum {
		INVALID_SID = 0
	};
	enum {
		RELOGIN_TIME = 15
	};
	enum {
		SAVING_TIME = 15
	};
	CSQLDatabase* m_pAuth{nullptr};
	AccountInfo m_AcctInfo;
	int m_nIndex;

	static Sema m_Sema;
	static std::string m_strSrvip;
	static std::string m_strSrvdb;
	static std::string m_strUserId;
	static std::string m_strUserPwd;
	static std::string m_strAccountTableName;
};

class AuthThreadPool {
public:
	AuthThreadPool();
	~AuthThreadPool();

	void Launch();
	void NotifyToExit();
	void WaitForExit();

	enum {
		AT_MAXNUM = 10
	};
	static int volatile RunLabel[AT_MAXNUM];
	static DWORD volatile RunLast[AT_MAXNUM];
	static DWORD volatile RunConsume[AT_MAXNUM];
	static unsigned int volatile uiAuthCount;
	static void IncAuthCount();
	static unsigned int GetAuthCount();

protected:
	std::array<std::unique_ptr<AuthThread>, AT_MAXNUM> m_Pool;
	//AuthThread* m_Pool[AT_MAXNUM];
};

//Add by sunny.sun 20090201

class ConnectMMS : public Task {
public:
	ConnectMMS(ToMMS* tmms) {
		_tmms = tmms;
		_timeout = 3000;
	}

private:
	virtual long Process() override;
	virtual Task* Lastly() override;

	ToMMS* _tmms;
	DWORD _timeout;
};

class ToMMS : public TcpClientApp, public RPCMGR {
	friend class ConnectMMS;
	struct MMS {
		MMS() {}
		std::string ip;
		uShort port;
		std::string type;
		uShort key;

		DataSocket* datasock{nullptr};
	};

public:
	ToMMS(const char* configeName, ThreadPool* proc, ThreadPool* comm);
	~ToMMS();

	int GetCallTotal() const { return m_calltotal; }

	void Login(cChar* ip, uShort port, int accountID);
	void Logout(cChar* ip, uShort port, int accountID);

private:
	virtual bool OnConnect(DataSocket* datasock) override;
	virtual void OnDisconnect(DataSocket* datasock, int reason) override;
	virtual void OnProcessData(DataSocket* datasock, RPacket& recvbuf) override;
	virtual WPacket OnServeCall(DataSocket* datasock, RPacket& in_para) override;

	std::atomic<long> m_atexit, m_calltotal;
	bool volatile _connected;
	MMS m_MMS;
};
//End

// Add by lark.li 20090722 begin
class ConnectBilling : public Task {
public:
	ConnectBilling(ToBilling* billing)
		: _billing(billing) {}

private:
	virtual long Process() override;
	virtual Task* Lastly() override;

	ToBilling* _billing{nullptr};
	DWORD _timeout{3'000};
};

class ToBilling : public TcpClientApp, public RPCMGR {
	friend class ConnectBilling;
	struct Billing {
		Billing() {}
		std::string ip;
		uShort port;
		bool enable;   //是否启动billing系统，0为不启动 1为启动
		bool killuser; //是否允许account踢人

		DataSocket* datasock{nullptr};
	};

public:
	ToBilling(const char* configeName, ThreadPool* proc, ThreadPool* comm);
	~ToBilling();

	int GetCallTotal() const { return m_calltotal; }

	void UserLogin(std::string strUserName, std::string strPassport);
	void UserLogout(std::string strUserName);

private:
	virtual bool OnConnect(DataSocket* datasock) override;
	virtual void OnDisconnect(DataSocket* datasock, int reason) override;
	virtual void OnProcessData(DataSocket* datasock, RPacket& recvbuf) override;
	virtual WPacket OnServeCall(DataSocket* datasock, RPacket& in_para) override;

	std::atomic<long> m_atexit, m_calltotal;
	bool volatile _connected;
	Billing m_Billing;
};
// End

// 全局变量
extern AccountServer2* g_As2;
extern AuthQueue g_Auth;

#endif // ACCOUNTSERVER2_H_
