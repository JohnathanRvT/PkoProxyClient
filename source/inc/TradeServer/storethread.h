

#if !defined __STORETHREAD_H__
#define __STORETHREAD_H__

#include "thread.h"


class StoreThread : public Thread
{
public:
    StoreThread();
    virtual ~StoreThread();
    uint Run();

public:
    int m_state;
};


#endif
