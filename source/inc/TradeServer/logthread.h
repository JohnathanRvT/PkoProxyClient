

#if !defined __LOGTHREAD_H__
#define __LOGTHREAD_H__

#include "thread.h"



class LogThread : public Thread
{
public:
    LogThread();
    virtual ~LogThread();
    uint Run();

public:
    volatile int m_state;
};


#endif
