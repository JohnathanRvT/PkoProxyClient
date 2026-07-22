
/* * * * * * * * * * * * * * * * * * * *

    SOCKET 2.2 封装
    Jampe
    2006/3/28

 * * * * * * * * * * * * * * * * * * * */


#if !defined __SOCKET_H__
#define __SOCKET_H__

#include <winsock2.h>
#include "datatype.h"
#include <time.h>
#include "lock.h"


class Socket2
{
public:
    Socket2();
    Socket2(const Socket2& sock);
    virtual ~Socket2();
    virtual void Create();
    virtual void Close();
    virtual void Bind(ushort port);
    virtual void Listen(int queue = SOMAXCONN);
    virtual void Accept(Socket2& sock);
    virtual bool Connect(const char* host, ushort port);
    virtual bool Connect(SOCKADDR_IN& addr);
    virtual const char* GetIPAddress();
    virtual ushort GetPort();
    virtual int Send(LPWSABUF lpBuffers, LPDWORD lpNumberOfBytesSent, LPWSAOVERLAPPED lpOverlapped, DWORD dwBufferCount = 1, DWORD dwFlags = 0);
    virtual int Recv(LPWSABUF lpBuffers, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, DWORD dwBufferCount = 1);
    virtual void SetNoBuffer(bool nobuf = true);
    virtual void SetSendBufferSize(int size);
    virtual void SetRecvBufferSize(int size);

protected:
    virtual bool Connect();

public:
    SOCKET          m_socket;
    SOCKADDR_IN     m_addr;
};



#define BUFFER_SIZE     1024 * 8
#define RECV            0
#define SEND            1


//--------------------------------------------------
//  单IO数据
//  消息数据
//--------------------------------------------------
typedef struct _PER_IO_DATA_
{
    OVERLAPPED      Overlapped;
    WSABUF          DataBuf;
    char            Buffer[BUFFER_SIZE];
    int             OperationType;
} PER_IO_DATA, * pPER_IO_DATA;


class IOCPSocket
{
public:
    IOCPSocket();
    virtual ~IOCPSocket();

    virtual bool PostRecv();
    virtual bool PostSend(const BYTE* buff, int size);
    virtual void CountRecv(ulong bytes);
    virtual void CountSend(ulong bytes);

public:
    long                m_lServerID;
    PER_IO_DATA         m_pRcvIO;
    PER_IO_DATA         m_pSndIO;
    Lock                m_sndLock;
    Socket2             m_sock;
    ulong               m_RcvPkt;
    ulong               m_SndPkt;
    void*               m_tmpbuf;
    long                m_tmpsize;
    bool                m_check;
    time_t              m_lastRcv;
    time_t              m_lastSnd;
    int                 m_ui_idx;
};


extern void PostSendMessage(IOCPSocket* client, long msgID, long subID, long extend, long section, unsigned char* body, long size);

#endif
