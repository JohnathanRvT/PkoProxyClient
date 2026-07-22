

#include "iocp.h"



IOCP::IOCP()
{
    m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}


IOCP::~IOCP()
{
}


void IOCP::Post(Socket2 sock, ULONG_PTR phd)
{
    CreateIoCompletionPort((HANDLE)sock.m_socket, m_CompletionPort, phd, 0);
}


bool IOCP::GetIOCPQueue(LPDWORD rsbyte, LPOVERLAPPED* pid, PULONG_PTR phd)
{
    return TRUE == GetQueuedCompletionStatus(m_CompletionPort, rsbyte, phd, pid, INFINITE);
}


void IOCP::StopIOCP()
{
    PostQueuedCompletionStatus(m_CompletionPort, 0, 0, 0);
}


void IOCP::Close()
{
    CloseHandle(m_CompletionPort);
}
