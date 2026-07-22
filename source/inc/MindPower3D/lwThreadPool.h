//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwErrorCode.h"
#include "lwSyncObj.h"
#include "lwInterfaceExt.h"
#include <process.h>
#include <queue>
//////////////
//
LW_BEGIN

enum lwThreadTaskStateType {
	THREADTASKSTATE_INVALID = 0,
	THREADTASKSTATE_WAITING = 1,
	THREADTASKSTATE_RUNNING = 2,
};

struct lwThreadTaskInfo {
	lwThreadProc proc;
	void* param;
};

struct lwThreadInfo {
	uintptr_t handle;
	unsigned int id;
	DWORD state;
};

class lwThreadPool : public lwIThreadPool {
	LW_STD_DECLARATION();

	using lwQueueTaskInfo = std::deque<lwThreadTaskInfo>;
	using lwQueueTaskInfo_It = lwQueueTaskInfo::iterator;

private:
	lwQueueTaskInfo _queue_task;
	lwThreadTaskInfo* _task_seq;
	lwThreadInfo* _thread_seq;
	DWORD _thread_seq_size;
	DWORD _thread_num;
	int _priority;

	DWORD _task_seq_size;
	DWORD _exit_flag;
	HANDLE _event_signal;
	CRITICAL_SECTION _cs_lock;
	lwEvent _event_pool;

	static unsigned int __stdcall __thread_proc(void* param);

private:
	unsigned int _ThreadProc();
	inline void _Lock() { ::EnterCriticalSection(&_cs_lock); }
	inline void _Unlock() { ::LeaveCriticalSection(&_cs_lock); }
	inline BOOL _SetEvent() { return ::SetEvent(_event_signal); }
	inline BOOL _ResetEvent() { return ::ResetEvent(_event_signal); }
	LW_RESULT _FindTask(lwQueueTaskInfo_It* it, lwThreadProc proc, void* param);

public:
	lwThreadPool();
	~lwThreadPool();

	LW_RESULT Create(DWORD thread_seq_size, DWORD task_seq_size, DWORD suspend_flag) override;
	LW_RESULT Destroy() override;
	LW_RESULT RegisterTask(lwThreadProc proc, void* param) override;
	LW_RESULT RemoveTask(lwThreadProc proc, void* param) override;
	LW_RESULT FindTask(lwThreadProc proc, void* param) override;
	LW_RESULT SuspendThread() override;
	LW_RESULT ResumeThread() override;
	LW_RESULT SetPriority(int priority) override;
	LW_RESULT SetPoolEvent(BOOL lock_flag) override;
	int GetPriority() const override { return _priority; }
	DWORD GetCurrentWaitingTaskNum() const override { return (DWORD)_queue_task.size(); }
	DWORD GetCurrentRunningTaskNum() const override;
	DWORD GetCurrentIdleThreadNum() const override;
	DWORD GetThreadId(DWORD id) { return _thread_seq[id].id; }
};

LW_END