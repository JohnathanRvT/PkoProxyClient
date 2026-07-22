


#if !defined __MSGQUEUE_H__
#define __MSGQUEUE_H__


#include "socket2.h"
#include "thread.h"
#include "lock.h"
#include <deque>
#include <vector>

using std::deque;
using std::vector;


template <class T>
class _AnyQueue
{
public:
    typedef deque<T>    _AnyType;
    _AnyQueue()
    {
    }
    virtual ~_AnyQueue()
    {
    }
    virtual T Pop()
    {
        m_lock.lock();
        T ty = (T)m_queue.front();
        m_queue.pop_front();
        m_lock.unlock();
        return ty;
    }
    virtual T Top()
    {
        m_lock.lock();
        T ty = (T)m_queue.front();
        m_lock.unlock();
        return ty;
    }
    virtual void Push(T val)
    {
        m_lock.lock();
        m_queue.push_back(val);
        m_lock.unlock();
    }
    virtual int Size()
    {
        return (int)m_queue.size();
    }
    virtual void Clear()
    {
        _AnyType::iterator it;

        m_lock.lock();
        for(it = m_queue.begin(); it != m_queue.end(); it++)
        {
            SAFE_DELETE(*it);
        }
        m_queue.clear();
        m_lock.unlock();
    }

public:
    _AnyType    m_queue;
    Lock        m_lock;
};


typedef vector<IOCPSocket*> __SockList;


class SockList
{
public:
    SockList();
    virtual ~SockList();

    virtual void Add(IOCPSocket* sock);
    virtual void Remove(IOCPSocket* sock);
    virtual int Size();
    virtual void Clear();
    virtual void Check();

public:
    __SockList  m_list;
    Lock        m_lock;
};



#endif

