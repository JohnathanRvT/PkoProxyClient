#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include "PreAlloc.h"
#include "Packet.h"
class dbc::DataSocket;

_DBC_BEGIN
#pragma pack(push)
#pragma pack(4)

struct PKItem : public PreAllocStru {
	PKItem(size_t size)
		: PreAllocStru(size) {
		m_semWait.Create(0, 1, nullptr);
	}
	void Initially() override {}
	void Finally() override {
		m_inpk = nullptr;
		m_retpk = nullptr;
		m_next = nullptr;
		m_iscall = 0;
	}
	Sema m_semWait;
	volatile char m_iscall{0};
	DataSocket* volatile m_datasock{nullptr};
	RPacket m_inpk{nullptr};
	WPacket m_retpk{nullptr};
	PKItem* volatile m_next{nullptr};
	DWORD m_dwLastTime{0};
};
struct PKQueue {
public:
	PKQueue(bool mode = true);
	~PKQueue() {
		CloseQueue();
	}
	void PeekPacket(uLong sleep = 0);
	uLong GetPkTotal() {
		MutexArmor l(m_mut);
		return m_pktotal;
	}

	void CloseQueue();
	WPacket SyncPK(DataSocket* datasock, RPacket& in_para, uLong ulMilliseconds = 5 * 1000);
	void AddPK(DataSocket* datasock, RPacket& pk);

private:
	PKItem* GetPKItem(uLong end);

	virtual void ProcessData(DataSocket* datasock, RPacket& recvbuf) = 0;
	virtual WPacket ServeCall(DataSocket* datasock, RPacket& in_para) { return nullptr; };
	const bool m_mode;
	bool volatile m_isclose;
	uLong volatile m_pktotal;
	Mutex m_mut;
	Sema m_sem;
	PKItem* volatile m_head;
	PKItem* volatile m_tail;
	static PreAllocHeapPtr<PKItem> m_heap;
};

#pragma pack(pop)
_DBC_END

#endif //PACKETQUEUE_H