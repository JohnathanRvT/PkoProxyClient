//================================================================
// It must be permitted by Dabo.Zhang that this program is used for
// any purpose in any situation.
// Copyright (C) Dabo.Zhang 2000-2003
// All rights reserved by ZhangDabo.
// This program is written(created) by Zhang.Dabo in 2000.3
// This program is modified recently by Zhang.Dabo in 2003.7
//=================================================================
#ifndef DATASOCKET_H
#define DATASOCKET_H

#ifndef USING_TAO	 //使用Win32基本Platform SDK
#include <winsock2.h> //确保调用新的WinSock2.2版本
#include <windows.h>
#else
#include "TAOSpecial.h"
#endif

#include "DBCCommon.h"
#include "Packet.h"
#include "PreAlloc.h"
#include "RunBiDirectChain.h"

_DBC_BEGIN
#pragma pack(push)
#pragma pack(4)
//==============================DataSocket===================================
//
class RPCInfo;
class DataSocket : public PreAllocStru, public RunBiDirectItem<DataSocket> {
	friend class TcpCommApp;
	friend class TcpClientApp;
	friend class TcpServerApp;
	friend class RPCMGR;
	friend class Sender;
	friend class Receiver;
	friend class OnProcessData;

public:
	uLong GetRecvBuf() const { return m_recvbuf; }
	uLong GetSendBuf() const { return m_sendbuf; }
	TcpCommApp* GetTcpApp() const { return __tca; }
	cChar* GetLocalIP() const { return m_localip; }
	uShort GetLocalPort() const { return m_localport; }
	cChar* GetPeerIP() const { return m_peerip; }
	uShort GetPeerPort() const { return m_peerport; }
	int GetDisconnectReason() const { return m_delreason; }
	bool IsServer() const { return m_isServer; }

	WPacket GetWPacket();
	long SendData(WPacket sendbuf);
	void* GetPointer() const;
	bool SetPointer(void* appinfo);

	int SetSendBuf(uLong bytes);
	int SetRecvBuf(uLong bytes);

	DataSocket(size_t size);
	void Init(SOCKET socket, cChar* peerip, uShort peerport, TcpCommApp* tca, bool IsServer);
	void Free() override { PreAllocStru::Free(); }
	LLong volatile m_sendbytes, m_recvbytes;
	LLong volatile m_sendpkts, m_recvpkts;
	uLong volatile m_sendbyteps, m_recvbyteps;
	uLong volatile m_sendpktps, m_recvpktps;

private:
	virtual ~DataSocket();
	void Initially() override;
	void Finally() override;
	RPCInfo* GetRPCInfo() const { return m_rpcinfo; }

	std::atomic<long> m_sbts, m_rbts;
	std::atomic<long> m_spks, m_rpks;

	std::atomic<long> m_sendflag, m_recvflag, m_procflag, m_isProcess;
	std::atomic<long> m_sendtime, m_recvtime;
	std::atomic<long> m_deltime, m_delremain, m_delflag;
	int volatile m_delreason;

	TcpCommApp* __tca{nullptr};
	Sender& m_sender;
	Receiver& m_receiver;
	volatile uLong m_sendbuf, m_recvbuf;

	bool const m_isServer{false};
	SOCKET const m_socket{INVALID_SOCKET};
	char m_localip[16], m_peerip[16];
	uShort m_localport, m_peerport;

	RPCInfo* volatile m_rpcinfo{nullptr};
	void* volatile m_appinfo;
};
#pragma pack(pop)
_DBC_END

#endif