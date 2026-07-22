
#if !defined __MAILTHREAD_H__
#define __MAILTHREAD_H__

#include "thread.h"



class MailThread : public Thread
{
public:
    MailThread();
    virtual ~MailThread();
    uint Run();

public:
    volatile int m_state;
};


#endif
