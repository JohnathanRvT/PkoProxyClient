#ifndef _GATESERVER_H_
#define _GATESERVER_H_

// C++ STL
#include <iostream>
#include <map>
#include <mutex>
#include <array>
// C
#include <time.h>
#include <stdlib.h>

//
#include "dstring.h"
#include "datasocket.h"
#include "threadpool.h"
#include "commrpc.h"
#include "point.h"
#include "inifile.h"
#include "gamecommon.h"
#include "prealloc.h"
#include "ntservice.h"
#include "LogStream.h"
#include "algo.h"
#include "i18n.h" //Add by lark.li 20080130

// Add by lark.li 20080731 begin
#include "pi_Memory.h"
#include "pi_Alloc.h"
// End

#include "util.h"

using namespace dbc;

// 计时退出机制宏切换
//#define CHAEXIT_ONTIME

enum PlayStatus {

};

// 关于 Client 连接部分＝＝＝＝＝＝＝＝＝＝
struct Player;
class ToClient : public TcpServerApp, public RPCMGR {
	friend class TransmitCall;

public:
	ToClient(char const* fname, ThreadPool* proc, ThreadPool* comm);
	~ToClient();

	void post_mapcrash_msg(Player* ply);
	std::string GetDisconnectErrText(int reason) const;
	void SetMaxCon(uShort maxcon) { m_maxcon = maxcon; }
	uShort GetMaxCon() const { return m_maxcon; }
	void CM_LOGIN(DataSocket* datasock, RPacket& recvbuf);
	WPacket CM_LOGOUT(DataSocket* datasock, RPacket& recvbuf);
	WPacket CM_DISCONN(DataSocket* datasock, RPacket& recvbuf);
	void CM_BGNPLAY(DataSocket* datasock, RPacket& recvbuf);
	void CM_ENDPLAY(DataSocket* datasock, RPacket& recvbuf);
	void CM_NEWCHA(DataSocket* datasock, RPacket& recvbuf);
	void CM_DELCHA(DataSocket* datasock, RPacket& recvbuf);
	void CM_CREATE_PASSWORD2(DataSocket* datasock, RPacket& recvbuf);
	void CM_UPDATE_PASSWORD2(DataSocket* datasock, RPacket& recvbuf);
	uShort GetVersion() const { return m_version; }
	int GetCallTotal() const { return m_calltotal; }

#ifdef NET_CAL
	uShort GetCheckSpan() const {
		return m_checkSpan;
	}
	uShort GetCheckWaring() const { return m_checkWaring; }
	uShort GetCheckError() const { return m_checkError; }

	void SetCheckSpan(uShort checkSpan);
	void SetCheckWaring(uShort checkWaring);
	void SetCheckError(uShort checkError);
#endif

private:
	bool DoCommand(DataSocket* datasock, cChar* cmdline);
	virtual bool OnConnect(DataSocket* datasock) override; // 返回值:true-允许连接,false-不允许连接
	virtual void OnConnected(DataSocket* datasock) override;
	virtual void OnDisconnect(DataSocket* datasock, int reason) override;
	virtual void OnProcessData(DataSocket* datasock, RPacket& recvbuf) override;
	virtual WPacket OnServeCall(DataSocket* datasock, RPacket& in_para) override;
	virtual bool OnSendBlock(DataSocket* datasock) override { return false; }

	// communication encryption
	virtual void OnEncrypt(DataSocket* datasock, char* ciphertext, const char* text, uLong& len) override;
	virtual void OnDecrypt(DataSocket* datasock, char* ciphertext, uLong& len) override;

	std::atomic<long> m_atexit = {0}, m_calltotal = {0};
	volatile uShort m_maxcon = {500};
	uShort m_version = {0};
	int _comm_enc; // 加密算法索引

#ifdef NET_CAL

	volatile uShort m_checkSpan;
	volatile uShort m_checkWaring;
	volatile uShort m_checkError;

	//volatile uShort	m_checkIPNum;	// 检查重复IP个数
	//volatile uShort	m_checkIPSpan;	// 检查重复IP间隔

#endif

	IMPLEMENT_CDELETE(ToClient)
};

// 关于 GameServer 连接部分＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
class ToGameServer;

constexpr int MAX_MAP = {100};
constexpr int MAX_GAM = {100};
struct GameServer : public PreAllocStru {
	friend class PreAllocHeap<GameServer>;

private:
	GameServer(size_t Size) : PreAllocStru(Size) {}
	~GameServer() {
		if (m_datasock != nullptr) {
			m_datasock->SetPointer(nullptr);
			m_datasock = nullptr;
		}
	}
	GameServer(GameServer const&) : PreAllocStru(1) {}
	GameServer& operator=(GameServer const&) {}
	void Initially() override;
	void Finally() override;

public:
	void EnterMap(Player* ply, uLong actid, uLong dbid, uLong worldid, cChar* map, Long lMapCpyNO, uLong x, uLong y, char entertype, short swiner); //角色chaid进入本服务器的地图map中的位置(x,y) winer指定角色是否是乱斗之王。

	// Add by lark.li 20080921 begin
	void DeleteGameServer(int reason); //遇到异常关闭GameServer
									   // End

public:
	std::atomic<long> m_plynum = {0};
	std::string gamename = {};
	std::string ip = {};
	uShort port = {0};
	DataSocket* m_datasock = {nullptr};
	GameServer* next = {nullptr};
	std::array<std::string, MAX_MAP> maplist = {};
	uShort mapcnt = {0};
};

class ToGameServer : public TcpServerApp, public RPCMGR {
	friend class ToGroupServer;

public:
	ToGameServer(char const* fname, ThreadPool* proc, ThreadPool* comm);
	~ToGameServer();

	GameServer* find(cChar* mapname);

	void SendAllGameServer(WPacket& in_para);

private:
	virtual bool OnConnect(DataSocket* datasock) override; //返回值:true-允许连接,false-不允许连接
	virtual void OnDisconnect(DataSocket* datasock, int reason) override;
	virtual WPacket OnServeCall(DataSocket* datasock, RPacket& in_para) override;
	virtual void OnProcessData(DataSocket* datasock, RPacket& recvbuf) override;

	PreAllocHeap<GameServer> _game_heap;				   // GameServer 描述对象堆
	void MT_LOGIN(DataSocket* datasock, RPacket& recvbuf); // GameServer 登录 GateServer

	GameServer* _game_list = {nullptr}; // 存储 GameServer 描述对象的链表
	short _game_num = {0};
	void _add_game(GameServer* game);
	bool _exist_game(char const* game);
	void _del_game(GameServer* game);
	std::map<std::string, GameServer*> _map_game; // 从地图名对应 GameServer 描述对象
	std::mutex _mut_game;

	IMPLEMENT_CDELETE(ToGameServer)
};

// 关于 GroupServer 连接部分＝＝＝＝＝＝＝＝＝＝
class ToGroupServer;
class ConnectGroupServer : public Task {
public:
	ConnectGroupServer(ToGroupServer* tgts) : _toGroupServer(tgts) {}

private:
	virtual long Process() override;
	virtual Task* Lastly() override;

	ToGroupServer* _toGroupServer = {nullptr};
	DWORD _timeout = {3000};
};

struct GroupServer {
	std::string ip = {};
	uShort port = {0};
	DataSocket* datasock = {nullptr};
	GroupServer* next = {nullptr};
};

class ToGroupServer : public TcpClientApp, public RPCMGR {
	friend class ConnectGroupServer;
	friend void ToGameServer::MT_LOGIN(DataSocket* datasock, RPacket& rpk);

public:
	ToGroupServer(char const* fname, ThreadPool* proc, ThreadPool* comm);
	~ToGroupServer();

	DataSocket* get_datasock() const { return _gs.datasock; }
	RPacket get_playerlist();

	int GetCallTotal() const { return m_calltotal; }

	// Add by lark.li 20081119 begin
	bool IsSync() const { return m_bSync; }
	void SetSync(bool sync = true) { m_bSync = sync; }
	// End

	// 准备好
	bool IsReady() const { return (!m_bSync && _connected); }

	DataSocket* Connect();
	void Disconnect(int reason = DS_DISCONN, uLong remain = 0);
	RPacket SyncCall(WPacket wpk, uLong milliseconds = 30 * 1000);

private:
	virtual bool OnConnect(DataSocket* datasock) override; // 返回值:true-允许连接,false-不允许连接
	virtual void OnDisconnect(DataSocket* datasock, int reason) override;
	virtual void OnProcessData(DataSocket* datasock, RPacket& recvbuf) override;
	virtual WPacket OnServeCall(DataSocket* datasock, RPacket& in_para) override;

	std::atomic<long> m_atexit = {0}, m_calltotal = {0};

	std::string _myself = {}; // GateServer自己的名字
	GroupServer _gs = {};
	bool volatile _connected = {false};

	// Add by lark.li 20081119 begin
	bool volatile m_bSync = {false};
	// End

	IMPLEMENT_CDELETE(ToGroupServer)
};

class ToMMS;
class ConnectMMS : public Task {
public:
	ConnectMMS(ToMMS* tmms) : _tmms(tmms) {}

private:
	virtual long Process() override;
	virtual Task* Lastly() override;

	ToMMS* _tmms = {nullptr};
	DWORD _timeout = {3000};
};

class ToMMS : public TcpClientApp, public RPCMGR {
	friend class ConnectMMS;
	struct MMS {
		MMS() : datasock(nullptr) {}
		std::string ip;
		uShort port;
		std::string type;
		uShort key;

		DataSocket* datasock;
	};

public:
	ToMMS(const char* configeName, ThreadPool* proc, ThreadPool* comm);
	~ToMMS();

	int GetCallTotal() { return m_calltotal; }

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

// GateServer 自身部分＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
struct Player : public PreAllocStru, public RunBiDirectItem<Player> {
	friend class PreAllocHeap<Player>;
	friend class DelayLogout;

public:
	bool InitReference(DataSocket* datasock);
	void Free() override { PreAllocStru::Free(); }

	// Add by lark.li 20081119 begin
	bool BeginRun();
	bool EndRun();
	// End

	std::string m_chapstr = {""};
	char m_password[ROLE_MAXSIZE_PASSWORD2 + 1] = {""};
	char m_szSendKey[4] = {""};
	char m_szRecvKey[4] = {""};

	uLong volatile m_actid = {0};
	uLong volatile m_loginID = {0};
	uLong volatile m_dbid = {0};	// 当前角色的数据库ID
	uLong volatile m_worldid = {0}; // 当前角色的内存唯一ID
	uLong volatile m_pingtime = {0};
	uInt volatile comm_key_len = {0};			 // 通讯密钥长度
	char comm_textkey[12] = {""};				 // GateServer 与 Client 之间加密通讯的密钥
	std::atomic<uintptr_t> gm_addr = {0};				 // GameServer 上 Player 对象的指针
	std::atomic<uintptr_t> gp_addr = {0};				 // GroupServer 上 Player 对象的指针
	DataSocket* volatile m_datasock = {nullptr}; // 此 Player 的 GateServer <-> Client 连接
	GameServer* volatile game = {nullptr};		 // 此 Player 当前所在的 GameServer 描述对象
	volatile bool enc = {false};				 // 是否加密通信数据

	// 是否禁言
	uLong volatile m_lestoptick = {0};
	bool volatile m_estop = {false};
	short volatile m_sGarnerWiner{0};

	struct
	{
		std::mutex m_mtxstat;		  //"0: Lock m_status;"
		volatile char m_status = {0}; //"0: Invalid; 1. During the role selection; 2. Playing the game."
		volatile char m_switch = {0}; //"0: Invalid; 1. Switch map (to prevent map switching, because the server is slow, causing two players to exist on both maps at the same time)"
#ifdef NET_CAL
		volatile uLong m_cmdNum = {0};   // 记录一秒钟内命令的个数，分析网络状况
		volatile uLong m_lashTick = {0}; // 上一次记录时间

		volatile uLong m_waringNum = {0}; // 记录一秒钟内命令的个数，分析网络状况
#endif
	};

private:
	Player(size_t Size) : PreAllocStru(Size) {}
	~Player() {
		if (m_datasock != nullptr) {
			m_datasock->SetPointer(nullptr);
			m_datasock = nullptr;
		}
	}
	Player(Player const&) : PreAllocStru(1) {}
	Player& operator=(Player const&) {}
	void Initially() override;
	void Finally() override;
};
class TransmitCall : public PreAllocTask {
public:
	TransmitCall(size_t size) : PreAllocTask(size){};
	void Init(DataSocket* datasock, RPacket& recvbuf) {
		m_datasock = datasock;
		m_recvbuf = recvbuf;
	}
	long Process() override;

	DataSocket* m_datasock;
	RPacket m_recvbuf;
};
class GateServer {
public:
	GateServer(char const* fname);
	~GateServer();

	void RunLoop(); // 主循环

	ThreadPool *m_clproc{nullptr}, *m_clcomm{nullptr},
		*m_gpcomm{nullptr}, *m_gpproc{nullptr}, *m_gmcomm{nullptr};

	//ThreadPool* m_mmsproc, * m_mmscomm;	//同MMS的任务池
	//ToMMS* m_ToMMS;	//同MMS的连接对象（主动重连机制）

	std::unique_ptr<ToGroupServer> toGroupServer{nullptr}; // 同GroupServer的连接对象（主动重连机制）
	std::unique_ptr<ToGameServer> toGameServer{nullptr};		   // 同GameServer的连接对象（被动）
	std::unique_ptr<ToClient> toClient{nullptr};		   // 同Client的连接对象（被动）
	std::mutex _mtxother;

	PreAllocHeap<Player> player_heap; // 玩家对象堆

	// Add by lark.li 20081119 begin
	RunBiDirectChain<Player> m_playerlist; // 玩家连表
	// End

	PreAllocHeap<TransmitCall> m_tch;

	IMPLEMENT_CDELETE(GateServer)
};
extern std::unique_ptr<GateServer> g_GateServer;
extern bool volatile g_appexit;

class CUdpManage;
class CUdpServer;
class GateServerApp : public NTService {
public:
	GateServerApp();

	void ServiceStart() override;
	void ServiceStop() override;
	virtual cChar* SetSvcName() const override { return "GateServer"; }
	virtual cChar* SetDispName() const override { return "[Kop Online]Gate Server"; } //显示名缺省等于服务名
	virtual bool CanPaused() const override { return true; };						 //缺省支持暂停和继续操作

	//int		GetShowMin()				{ return _nShowMin;		}
	//int		GetShowMax()				{ return _nShowMax;		}
	//void	SetShowRange( int min, int max )	{ _nShowMin=min; _nShowMax=max;		}

private: // 用于在未连接前，客户端通过udp获得游戏统计信息Jerry 2006-1-10
		 //CUdpManage*	_pUdpManage;
		 //static void _NotifySocketNumEvent( CUdpManage* pManage, CUdpServer* pUdpServer, const char* szClientIP, unsigned int nClientPort, const char* pData, int len );
		 //static int	_nShowMin;
		 //static int	_nShowMax;
};

extern LogStream g_gateerr;
extern LogStream g_gatelog;
extern LogStream g_chkattack;
extern LogStream g_gateconnect;
//extern LogStream g_gatepacket;
extern GateServerApp* g_app;
extern LogStream g_Loginlog;

#endif // _GATESERVER_H_
