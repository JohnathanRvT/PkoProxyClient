#ifndef NETIF_H
#define NETIF_H

#include "CommRPC.h"
#include "PacketQueue.h"
#include "Connection.h"

class CProCirculate;

_DBC_USING

// 网络协议 (数据包->游戏控制)
using LPRPACKET = RPacket&;
using LPWPACKET = WPacket&;

struct lua_State;

extern inline int lua_initNoise(lua_State* L);
extern inline int lua_encryptNoise(lua_State* L);
extern inline int lua_decryptNoise(lua_State* L);
extern inline int lua_HandleNetMessage(lua_State* L);

// 用于记录Log
class CLogName {
public:
	CLogName();
	void Init();

	const char* SetLogName(DWORD dwWorlID, const char* szName); // 设置Log名称
	const char* GetLogName(DWORD dwWorlID);					  // 要把ID得到log名称
	const char* GetMainLogName();								  // 得到主角的log名称

	bool IsMainCha(DWORD dwWorlID);

private:
	enum {
		LOG_NAME = 256,
		LOG_MAX = 1000,
	};

	DWORD _dwWorldArray[LOG_MAX]{};
	char _szLogName[LOG_MAX][LOG_NAME]{};
	char _szNoFind[LOG_NAME]{};
};

class NetIF : public TcpClientApp, public RPCMGR, public PKQueue {
public:
	// Packet消息处理函数 Server -> Client 消息入口总控
	BOOL HandlePacketMessage(dbc::DataSocket* datasock, LPRPACKET pk);
	// Packet发送函数     Client -> Server 消息发出总控
	void SendPacketMessage(LPWPACKET pk);
	dbc::RPacket SyncSendPacketMessage(LPWPACKET pk, unsigned long timeout = 30 * 1000);

	//===============================================================================================
	NetIF(dbc::ThreadPool* comm = nullptr);
	virtual ~NetIF();
	void OnProcessData(dbc::DataSocket* datasock, dbc::RPacket& recvbuf) override;
	bool OnConnect(dbc::DataSocket* datasock) override;				  //返回值:true-允许连接,false-不允许连接
	void OnDisconnect(dbc::DataSocket* datasock, int reason) override; //reason值:0-本地程序正常退出；-1-Socket错误；-3-网络被对方关闭；-5-包长度超过限制。

	bool IsConnected() { return m_connect.IsConnected(); }
	int GetConnStat() { return m_connect.GetConnStat(); }
	void ProcessData(dbc::DataSocket* datasock, dbc::RPacket& recvbuf) override;

	unsigned long GetAveragePing();
	CProCirculate* GetProCir() { return m_pCProCir.get(); }
	void SwitchNet(bool isConnected);

	Connection m_connect;
	struct
	{
		dbc::uLong m_pingid{0};
		dbc::uLong m_maxdelay{0}, m_curdelay{0}, m_mindelay{0};
		DWORD dwLatencyTime[20];

		// 取最近的几次平均ping值，用于client,server 的预移动 xuedong 2004.09.01
		dbc::uLong m_ulCurStatistic{0};
		std::array<dbc::uLong, 4> m_ulDelayTime{};
		// end
	};
	unsigned long m_ulPacketCount{1}; // 记录包的个数，用于测试。xuedong 2004.09.10
	long m_framedelay{40};			  // 帧延迟

	std::unique_ptr<CProCirculate> m_pCProCir{nullptr};
	dbc::Mutex m_mutmov;
	std::string m_chapstr = {""};
	char m_accounts[100];
	char m_passwd[100];

	bool _enc{false}; // 是否加密
	int _comm_enc{0}; // 加密算法索引
	char _key[12]{};
	int _key_len{0};
	void OnEncrypt(dbc::DataSocket* datasock, char* ciphertext, const char* text, unsigned long& len) override;
	void OnDecrypt(dbc::DataSocket* datasock, char* ciphertext, unsigned long& len) override;
	lua_State* g_rLvm;
	lua_State* g_sLvm;
};

extern std::unique_ptr<NetIF> g_NetIF;
extern CLogName g_LogName;

#endif
