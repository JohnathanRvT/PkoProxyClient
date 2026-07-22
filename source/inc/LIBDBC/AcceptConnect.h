#ifndef ACCEPTCONNECT_H
#define ACCEPTCONNECT_H

#include "DBCCommon.h"
#include "ThreadPool.h"

_DBC_BEGIN

class TcpCommApp;
class TcpServerApp;
class DataSocket;
//=====OnConnect==============================================================================
class OnConnect : public PreAllocTask {
public:
	OnConnect(size_t size) : PreAllocTask(size) {}
	inline void Init(DataSocket* datasock);
	virtual long Process() override;
	virtual Task* Lastly() override;

private:
	DataSocket* m_datasock{nullptr};
	TcpCommApp* __tca{nullptr};
};

//=====AcceptConnect==============================================================================
class AcceptConnect : public PreAllocTask {
public:
	AcceptConnect(size_t size) : PreAllocTask(size) {
	}
	void Init(TcpServerApp* tsa) { __tsa = tsa; };
	static PreAllocHeapPtr<AcceptConnect> m_acceptheap;

private:
	virtual long Process() override;
	virtual Task* Lastly() override;

	static PreAllocHeapPtr<OnConnect> m_connectheap;
	TcpServerApp* volatile __tsa{nullptr};
};
//=====DelConnect==============================================================================
class DelConnect : public PreAllocTask {
public:
	DelConnect(size_t size) : PreAllocTask(size) {}
	void Init(DataSocket* datasock, int reason) {
		m_datasock = datasock;
		m_reason = reason;
		__tca = m_datasock->GetTcpApp();
	};

	virtual long Process() override;
	virtual Task* Lastly() override;

	static PreAllocHeapPtr<DelConnect> m_delHeap;

	int m_reason{0};
	DataSocket* m_datasock{nullptr};
	TcpCommApp* __tca{nullptr};
};

_DBC_END

#endif
