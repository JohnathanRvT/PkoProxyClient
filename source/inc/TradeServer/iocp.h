

/* * * * * * * * * * * * * * * * * * * *

    完成端口封装
    Jampe
    2006/3/28

 * * * * * * * * * * * * * * * * * * * */

#if !defined __IOCP_H__
#define __IOCP_H__

#include "socket2.h"


class IOCP
{
public:
    IOCP(void);
    virtual ~IOCP();
    virtual void Post(Socket2 sock, ULONG_PTR phd);
    virtual bool GetIOCPQueue(LPDWORD rsbyte, LPOVERLAPPED* pid, PULONG_PTR phd);
    virtual void StopIOCP();
    virtual void Close();

public:
    HANDLE  m_CompletionPort;
};


#endif
