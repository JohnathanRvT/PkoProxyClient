
#include "NetThread.h"
#include <stdio.h>
#include "infotype.h"
#include "DBLogic.h"


#if TEST_SERVER == 1
    #include <Mmsystem.h>
    #pragma comment(lib, "Winmm.lib")

    extern DWORD nettotaltime;
    extern DWORD nettotalcount;
#endif

AcceptThread::AcceptThread()
{
}


AcceptThread::~AcceptThread()
{
}


uint AcceptThread::Run()
{
    m_state = THREAD_INIT;     //  更新状态初始化
    unsigned int threadID = GetCurrentThreadId();
    LGPRT("System", "Begin IOCP Accept Thread %i" __LN, threadID);

    IOCPSocket* client;

    for(;;)
    {
__BEGIN_TRY
        m_state = THREAD_RESTING;      //  更新状态为闲置
        if(INFO_STOPING == g_servInfo.serv_state)
        {
            m_state = THREAD_STOPPED;
            break;
        }
        client = 0;
        client = new IOCPSocket;
        g_servInfo.serv_sock.Accept(client->m_sock);    //  监听套接字
        m_state = THREAD_WORKING;      //  更新状态为工作
        client->m_check = false;
        g_servInfo.serv_iocp.Post(client->m_sock, (ULONG_PTR)client);   //  投递到IOCP
        LGPRT("Connection", "Client Connect: %s:%i" __LN, client->m_sock.GetIPAddress(), client->m_sock.GetPort());
        if(g_dbInfo.db_login.CheckClient(client))
        {
            client->CountRecv(0);   // 刷新接收时间
            client->CountSend(0);   // 刷新发送时间
            g_listInfo.sock_list.Add(client);   //  加入SOCKET列表
            client->PostRecv();             //  投递接收
        }
        else
        {
            LGPRT("Connection", "Cilent %s:%i Not In Allow List." __LN, client->m_sock.GetIPAddress(), client->m_sock.GetPort());
            BYTE kick = 0;
            client->PostSend(&kick, 1);
        }
__END_TRY
    }

    LGPRT("System", "End IOCP Accept Thread %i" __LN, threadID);
    return 0;
}



extern void OnNetMessage(pNetMessage msg, IOCPSocket* client);


IOCPWorkThread::IOCPWorkThread()
{
}


IOCPWorkThread::~IOCPWorkThread()
{
}


uint IOCPWorkThread::Run()
{
    m_state = THREAD_INIT;     //  初始化状态
    unsigned int threadID = GetCurrentThreadId();
    LGPRT("System", "Begin IOCP Work Thread %i" __LN, threadID);

    DWORD               rsbyte;
    pPER_IO_DATA        pid;
    IOCPSocket*         phd;
    pInfoMessage        im;
    bool ret;

    for(;;)
    {
__BEGIN_TRY
        m_state = THREAD_RESTING;      //  闲置状态
        pid = 0, phd = 0, rsbyte = 0, im = 0; //  初始化
        ret = g_servInfo.serv_iocp.GetIOCPQueue(&rsbyte, (LPOVERLAPPED*)&pid, (PULONG_PTR)&phd);

        m_state = THREAD_WORKING;      //  工作状态

        if(NULL == phd)     //  退出IOCP
        {
            if(INFO_STOPING == g_servInfo.serv_state)
            {
                m_state = THREAD_STOPPED;  //  停止状态
                LGPRT("System", "IOCP Work Thread %li Set State: STOP" __LN, threadID);
                break;
            }
            continue;
        }

        if(0 == rsbyte || (!ret)) //  断开连接
        {
            try
            {
                //  有时会重复收到断开消息
                LGPRT("Connection", "Cilent Disconnect: %s:%i, Recv Bytes: %li, Send Bytes: %li" __LN, phd->m_sock.GetIPAddress(), phd->m_sock.GetPort(), phd->m_RcvPkt, phd->m_SndPkt);
                g_listInfo.sock_list.Remove(phd); //  移出的时，同时处理关闭和释放资源的工作。
            }
            catch(...)
            {
                //LG("Exception", "Client Disconnect Exception." __LN);
            }
            continue;
        }

        switch(pid->OperationType)
        {
        case RECV:
            {
                phd->CountRecv(rsbyte);

                BYTE*   bufPtr = (BYTE*)phd->m_pRcvIO.Buffer;
                long    bufSize = rsbyte;
                pNetMessage nm = 0;
                int ret = 0;

                for(;;)
                {
                    ret = PeekNetMessage(&nm, &bufPtr, bufSize, &phd->m_tmpbuf, phd->m_tmpsize);

                    if(PNM_SUCCESS == ret)             //  成功
                    {
#if TEST_SERVER == 1
                        DWORD b = 0, e = 0;
                        b = timeGetTime();
#endif
                        OnNetMessage(nm, phd);
#if TEST_SERVER == 1
                        e = timeGetTime();

                        //printf("Store Begin: %li, Store End: %li, End - Begin: %li, ThreadID: %li\n", b, e, (e - b), ::GetCurrentThreadId());
                        nettotaltime += (e - b);
                        ++nettotalcount;
#endif
                    }
                    else if(PNM_FAILED == ret)         //  被拆包
                    {
                        break;
                    }
                    else if(PNM_UNKNOWN == ret)        //  解析消息头失败
                    {
                        LGPRT("Connection", "Cilent %s:%i Bad Message Head." __LN, phd->m_sock.GetIPAddress(), phd->m_sock.GetPort());
                        //phd->m_check = false;
                        if(phd->m_tmpbuf)
                        {
                            SAFE_DELETE(phd->m_tmpbuf);
                            phd->m_tmpsize = 0;
                        }
                        break;
                    }
                    else if(PNM_EXCEPTION ==ret)      //  异常（多数情况是因空指针引起）
                    {
                        LGPRT("Connection", "Receive Cilent %s:%i Exception." __LN, phd->m_sock.GetIPAddress(), phd->m_sock.GetPort());
                        if(phd->m_tmpbuf)
                        {
                            SAFE_DELETE(phd->m_tmpbuf);
                            phd->m_tmpsize = 0;
                        }
                        break;
                    }
                    else    //  未知情况
                    {
                        LGPRT("Connection", "Receive Cilent %s:%i Unknown Exception." __LN, phd->m_sock.GetIPAddress(), phd->m_sock.GetPort());
                        if(phd->m_tmpbuf)
                        {
                            SAFE_DELETE(phd->m_tmpbuf);
                            phd->m_tmpsize = 0;
                        }
                        break;
                    }
                }

                if(phd->m_check)
                {
                    phd->PostRecv();  //  继续投递接收
                }
                else
                {
                    //  认证未通过，断开连接
                    LGPRT("Connection", "Cilent %s:%i To Fall Check(RCV)." __LN, phd->m_sock.GetIPAddress(), phd->m_sock.GetPort());
                    g_listInfo.sock_list.Remove(phd); //  移出的时，同时处理关闭和释放资源的工作。
                }
            }  break;
        case SEND:
            {
                phd->CountSend(rsbyte);
                if(!phd->m_check)
                {
                    //  认证未通过，断开连接
                    LGPRT("Connection", "Cilent %s:%i To Fall Check(SND)." __LN, phd->m_sock.GetIPAddress(), phd->m_sock.GetPort());
                    g_listInfo.sock_list.Remove(phd); //  移出的时，同时处理关闭和释放资源的工作。
                }
            }  break;
        }
__END_TRY
    }

    LGPRT("System", "End IOCP Work Thread %i" __LN, threadID);
    return 0;
}
