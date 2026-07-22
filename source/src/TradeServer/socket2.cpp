
#include "socket2.h"
#include "log.h"


#pragma comment(lib, "ws2_32.lib")
#include "infotype.h"

//  初始化SOCKET
class InitNet
{
public:
    InitNet()
    {
        WSAStartup(0x0202, &wsaData);
    }

    ~InitNet()
    {
        WSACleanup();
    }

private:
    WSADATA wsaData;

}initNet;



Socket2::Socket2()
{
}


Socket2::Socket2(const Socket2& sock)
: m_socket(sock.m_socket)
{
    memcpy(&m_addr, &sock.m_addr, sizeof(SOCKADDR));
}


Socket2::~Socket2()
{
}


void Socket2::Create()
{
    m_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
}


void Socket2::Close()
{
    shutdown(m_socket, SD_BOTH);
    closesocket(m_socket);
}


void Socket2::Bind(ushort port)
{
    m_addr.sin_family         = AF_INET;
    m_addr.sin_addr.s_addr    = htonl(INADDR_ANY);
    m_addr.sin_port           = htons(port);

    bind(m_socket, (PSOCKADDR)&m_addr, sizeof(SOCKADDR));
}


void Socket2::Listen(int queue)
{
    listen(m_socket, queue);
}


void Socket2::Accept(Socket2& sock)
{
    int __size = sizeof(SOCKADDR);
    sock.m_socket = WSAAccept(m_socket, (PSOCKADDR)&sock.m_addr, &__size, NULL, 0);
}


bool Socket2::Connect(const char* host, ushort port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons((u_short)port);
    m_addr.sin_addr.s_addr = inet_addr(host);

    //  dns解析
    if (INADDR_NONE == m_addr.sin_addr.s_addr)
    {
        LPHOSTENT lphost;
        lphost = gethostbyname(host);

        if (lphost != NULL)
        {
            m_addr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
        }
    }
    return Connect();
}


bool Socket2::Connect(SOCKADDR_IN& addr)
{
    memcpy(&m_addr, &addr, sizeof(SOCKADDR));
    return Connect();
}


bool Socket2::Connect()
{
    return SOCKET_ERROR != WSAConnect(m_socket, (PSOCKADDR)&m_addr, sizeof(SOCKADDR), NULL, NULL, NULL, 0);
}


const char* Socket2::GetIPAddress()
{
    return inet_ntoa(m_addr.sin_addr);
}


ushort Socket2::GetPort()
{
    return m_addr.sin_port;
}


int Socket2::Send(LPWSABUF lpBuffers, LPDWORD lpNumberOfBytesSent, LPWSAOVERLAPPED lpOverlapped, DWORD dwBufferCount, DWORD dwFlags)
{
    return WSASend(m_socket, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, 0);
}


int Socket2::Recv(LPWSABUF lpBuffers, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, DWORD dwBufferCount)
{
    return WSARecv(m_socket, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, 0);
}


void Socket2::SetNoBuffer(bool nobuf)
{
    setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY,(const char*)&nobuf, sizeof(nobuf));
}


void Socket2::SetSendBufferSize(int size)
{
    setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size));
}


void Socket2::SetRecvBufferSize(int size)
{
    setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size));
}






IOCPSocket::IOCPSocket()
    : m_RcvPkt(0), m_SndPkt(0), m_tmpbuf(0), m_tmpsize(0), m_lServerID(0)
{
}


IOCPSocket::~IOCPSocket()
{
}


bool IOCPSocket::PostRecv()
{
__BEGIN_TRY
    memset(&m_pRcvIO, 0, sizeof(PER_IO_DATA));

    m_pRcvIO.OperationType = RECV;
    m_pRcvIO.DataBuf.buf = m_pRcvIO.Buffer;
    m_pRcvIO.DataBuf.len = BUFFER_SIZE;

    unsigned long lBytes = 0, lFlags = 0;

    if(SOCKET_ERROR == m_sock.Recv(&m_pRcvIO.DataBuf , &lBytes, &lFlags, &m_pRcvIO.Overlapped))
    {
        if (ERROR_IO_PENDING != WSAGetLastError())
        {
            m_check = false;
            return false;
        }
    }
    return true;
__END_TRY
    return false;
}


bool IOCPSocket::PostSend(const BYTE* buff, int size)
{
__BEGIN_TRY
    if(!buff)
    {
        return false;
    }

    m_sndLock.lock();

    memset(&m_pSndIO, 0, sizeof(PER_IO_DATA));
    m_pSndIO.OperationType = SEND;

    for(;;)
    {
        if(size > BUFFER_SIZE)
        {
            memset(&m_pSndIO.Overlapped, 0, sizeof(OVERLAPPED));
            memset(m_pSndIO.Buffer, 0, BUFFER_SIZE);
            memcpy(m_pSndIO.Buffer, buff, BUFFER_SIZE);

            m_pSndIO.DataBuf.buf = m_pSndIO.Buffer;
            m_pSndIO.DataBuf.len = BUFFER_SIZE;

            unsigned long lBytes = 0;

            if(SOCKET_ERROR == m_sock.Send(&m_pSndIO.DataBuf, &lBytes, &m_pSndIO.Overlapped))
            {
                if (ERROR_IO_PENDING != WSAGetLastError())
                {
                    m_sndLock.unlock();
                    return false;
                }
            }
            size -= BUFFER_SIZE;
            buff += BUFFER_SIZE;
        }
        else
        {
            memset(&m_pSndIO.Overlapped, 0, sizeof(OVERLAPPED));
            memset(m_pSndIO.Buffer, 0, BUFFER_SIZE);
            memcpy(m_pSndIO.Buffer, buff, size);

            m_pSndIO.DataBuf.buf = m_pSndIO.Buffer;
            m_pSndIO.DataBuf.len = size;

            unsigned long lBytes = 0;

            if(SOCKET_ERROR == m_sock.Send(&m_pSndIO.DataBuf, &lBytes, &m_pSndIO.Overlapped))
            {
                if (ERROR_IO_PENDING != WSAGetLastError())
                {
                    m_sndLock.unlock();
                    return false;
                }
            }
            m_sndLock.unlock();
            return true;
        }
    }
    m_sndLock.unlock();
    return true;
__END_TRY
    m_sndLock.unlock();
    return false;
}


void IOCPSocket::CountRecv(ulong bytes)
{
    m_RcvPkt += bytes;
    m_lastRcv = time(0);
}


void IOCPSocket::CountSend(ulong bytes)
{
    m_SndPkt += bytes;
    m_lastSnd = time(0);
}

