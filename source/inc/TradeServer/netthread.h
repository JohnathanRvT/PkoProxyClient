


#if !defined __NETTHREAD_H__
#define __NETTHREAD_H__

#include "thread.h"


class AcceptThread : public Thread
{
public:
    AcceptThread();
    virtual ~AcceptThread();
    uint Run();

public:
    volatile int m_state;
};


class IOCPWorkThread : public Thread
{
public:
    IOCPWorkThread();
    virtual ~IOCPWorkThread();
    uint Run();

public:
    volatile int m_state;
};


#endif
