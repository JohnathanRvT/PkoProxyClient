

#if !defined __MANAGETHREAD_H__
#define __MANAGETHREAD_H__

#include "thread.h"



class ManageThread : public Thread
{
public:
    ManageThread();
    virtual ~ManageThread();
    uint Run();

public:
    int m_state;
};



#endif
