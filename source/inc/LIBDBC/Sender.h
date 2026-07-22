//================================================================
// It must be permitted by Dabo.Zhang that this program is used for
// any purpose in any situation.
// Copyright (C) Dabo.Zhang 2000-2003
// All rights reserved by ZhangDabo.
// This program is written(created) by Zhang.Dabo in 2000.3
// This program is modified recently by Zhang.Dabo in 2003.7
//=================================================================
#ifndef SENDER_H
#define SENDER_H

#include "DBCCommon.h"
#include "ThreadPool.h"
#include "Packet.h"

_DBC_BEGIN
class DataSocket;
class SNDPacket : public PreAllocStru, private rptr<rbuf> {
	friend class Sender;

public:
	SNDPacket(size_t size) : PreAllocStru(size) {}
	void Initially() override {
		m_pkts = 0;
		m_cpos = 0;
		m_next = nullptr;
	}
	void Finally() override {
		rptr<rbuf>::operator=(0);
		m_haspace = false;
	}
	void Init(DataSocket* datasock, size_t size);
	void Init(DataSocket* datasock, rbuf* buf);

	inline SNDPacket& operator<<(WPacket& wpk);

	operator bool() const { return rptr<rbuf>::operator bool(); }
	size_t SendSize() const { return (*this) ? (*this)->Size() : 0; }
	uLong HasSpace() const { return (m_haspace && bool(*this) && (*this)->Size() > m_cpos) ? (*this)->Size() - m_cpos : 0; }
	char* GetBufAddr() const { return (*this)->getbuf(); }

private:
	uLong volatile m_pkts{0};
	uLong volatile m_cpos{0};
	DataSocket* volatile m_datasock{nullptr};
	SNDPacket* volatile m_next{nullptr};
	bool volatile m_haspace{false};
};

//===================================================================================
class Sender : public Task {
public:
	Sender(DataSocket* datasock);
	void Initially();
	void Finally();
	void Init();
	Sender& operator<<(WPacket& wpk);
	bool PopSNDPacket(int instancy);
	bool HasData() { return m_send || m_head; }

private: //override
	long Process() override;
	Task* Lastly() override { return nullptr; }

private: //data
	static PreAllocHeapPtr<SNDPacket> m_SNDPackets;
	Sema m_semSndC;
	Mutex m_mtxarra;
	Mutex m_mtxcomb;
	Mutex m_mtxsend;
	SNDPacket* volatile m_send;
	SNDPacket* volatile m_head;
	SNDPacket* volatile m_tail;

	uLong volatile m_p;
	DataSocket* volatile m_datasock;
	TcpCommApp* volatile __tca;
};
//================================================================================

_DBC_END

#endif
