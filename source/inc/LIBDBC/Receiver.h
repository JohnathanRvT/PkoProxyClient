//================================================================
// It must be permitted by Dabo.Zhang that this program is used for
// any purpose in any situation.
// Copyright (C) Dabo.Zhang 2000-2003
// All rights reserved by ZhangDabo.
// This program is written(created) by Zhang.Dabo in 2000.3
// This program is modified recently by Zhang.Dabo in 2003.7
//=================================================================
#ifndef RECEIVER_H
#define RECEIVER_H

#include "DBCCommon.h"
#include "ThreadPool.h"
#include "Comm.h"

_DBC_BEGIN

//==============================================================================
class OnProcessData : public PreAllocTask {
public:
	OnProcessData(size_t size) : PreAllocTask(size){};
	long Process() override;
	Task* Lastly() override { return PreAllocTask::Lastly(); }
	inline void Init(DataSocket* datasock, RPacket& rpk);

private:
	RPacket m_rpk{nullptr};
	TcpCommApp* volatile __tca{nullptr};
	DataSocket* volatile m_datasock{nullptr};
};
//==============================================================================
class Receiver : public Task {
public:
	Receiver(DataSocket* datasock);
	void Initially();
	void Finally();
	void Init();

private: //override
	long Process() override;
	Task* Lastly() override { return nullptr; }

private: //data
	uLong HasSpace() { return (m_rpk.Size() > (m_rpk.GetOffset() + m_p)) ? (m_rpk.Size() - m_rpk.GetOffset() - m_p) : 0; }
	static PreAllocHeapPtr<OnProcessData> m_HeapProcData;

	uLong volatile m_p{0};
	RPacket m_rpk;

	cuLong volatile _len_inc{0};
	DataSocket* const volatile m_datasock{nullptr};
	TcpCommApp* volatile __tca{nullptr};
};
//================================================================================

_DBC_END

#endif