
#include "infotype.h"
#include "logthread.h"
#include "dblogic.h"

LogThread::LogThread()
{
}


LogThread::~LogThread()
{
}


uint LogThread::Run()
{
    m_state = THREAD_INIT;   //  初始化状态
    unsigned int threadID = GetCurrentThreadId();
    LGPRT("System", "Begin Log Thread %li" __LN, threadID);

    const char* msgPtr;
    for(;;)
    {
__BEGIN_TRY
        m_state = THREAD_RESTING;        //  闲置状态
        if(INFO_STOPING == g_servInfo.serv_state)
        {
            m_state = THREAD_STOPPED;    //  停止状态
            break;
        }
        if(0 == g_listInfo.log_list.Size())   //  队列为空时
        {
            Sleep(1);   //  释放cpu资源
            continue;
        }
        m_state = THREAD_WORKING;        //  工作状态
        while(g_listInfo.log_list.Size())
        {
            msgPtr = 0;
            msgPtr = g_listInfo.log_list.Pop();
            if(msgPtr)
            {
                g_dbInfo.db_history.ExecuteInsert(msgPtr);
                SAFE_DELETE_ARRAY(msgPtr);
            }
        }
__END_TRY
    }

    LGPRT("System", "End Log Thread %li" __LN, threadID);
    return 0;
}


