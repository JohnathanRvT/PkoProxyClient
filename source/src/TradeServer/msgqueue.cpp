

#include "msgqueue.h"



#define KICK_SPACE          1000000


SockList::SockList()
{
}


SockList::~SockList()
{
}


void SockList::Add(IOCPSocket* sock)
{
    m_lock.lock();
    m_list.push_back(sock);
    m_lock.unlock();
}


void SockList::Remove(IOCPSocket* sock)
{
    __SockList::iterator it;

    m_lock.lock();
    for(it = m_list.begin(); it != m_list.end(); it++)
    {
        if(sock->m_sock.m_socket == (*it)->m_sock.m_socket)
        {
            (*it)->m_sock.Close();
            SAFE_DELETE(*it);
            m_list.erase(it);
            break;
        }
    }
    m_lock.unlock();
}


int SockList::Size()
{
    m_lock.lock();
    int ret = (int)m_list.size();
    m_lock.unlock();
    return ret;
}


void SockList::Clear()
{
    __SockList::iterator it;

    m_lock.lock();
    for(it = m_list.begin(); it != m_list.end(); it++)
    {
        (*it)->m_sock.Close();
        SAFE_DELETE(*it);
    }
    m_list.clear();
    m_lock.unlock();
}


void SockList::Check()
{
    __SockList::iterator it, it2;

    int spRcv = 0;
    int spSnd = 0;
    m_lock.lock();
    for(it = m_list.begin(); it != m_list.end();)
    {
        spRcv = (int)(time(0) - (*it)->m_lastRcv);
        spSnd = (int)(time(0) - (*it)->m_lastSnd);

        if((spRcv > KICK_SPACE) && (spSnd > KICK_SPACE))
        {
            (*it)->m_sock.Close();
            SAFE_DELETE(*it);
            m_list.erase(it);
            if(it == m_list.end())
            {
                break;
            }
        }
        ++it;
    }
    m_lock.unlock();
}






