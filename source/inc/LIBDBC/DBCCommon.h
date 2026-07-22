//================================================================
// It must be permitted by Dabo.Zhang that this program is used for
// any purpose in any situation.
// Copyright (C) Dabo.Zhang 2000-2003
// All rights reserved by ZhangDabo.
// This program is written(created) by Zhang.Dabo in 2000.3
// The last modification of this program is in 2003.7
//=================================================================
//include file;
#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x501
#endif

#ifndef USING_TAO	 //使用Win32基本Platform SDK
#include <winsock2.h> //确保调用新的WinSock2.2版本
#include <windows.h>
#else
#include "TAOSpecial.h"
#endif
#include <atomic>

//----------------------------------------------------------------
#define _DBC_BEGIN \
	namespace dbc {
#define _DBC_END \
	}            \
	;
#define _DBC_USING using namespace dbc;
extern "C" {
WINBASEAPI
_Must_inspect_result_
	BOOL
		WINAPI
		InitializeCriticalSectionAndSpinCount(
			_Out_ LPCRITICAL_SECTION lpCriticalSection,
			_In_ DWORD dwSpinCount);

WINBASEAPI
BOOL
	WINAPI
	TryEnterCriticalSection(
		_Inout_ LPCRITICAL_SECTION lpCriticalSection);
};

_DBC_BEGIN
#pragma pack(push)
#pragma pack(4)

//=================================================================
#define nil 0
//typedef type define
using Char = char; //带符号8个二进制位(=1个字节)整型/字符数据
using wChar = wchar_t;
using Short = short; //带符号2字节整型数据
using Long = long;   //带符号4字节整型数据
using Int = int;	 //带符号系统依赖整数数据(32位机实现为uLong)
using cChar = const char;
using cwChar = const wchar_t;
using cShort = const short;
using cLong = const long;
using cInt = const int;
using uChar = unsigned char;   //无符号8个二进制位(=1个字节)整型/字符数据
using uShort = unsigned short; //无符号2字节整型数据
using uLong = unsigned long;   //无符号4字节整型数据
using uInt = unsigned int;	 //无符号系统依赖整数数据(32位机实现为uLong)
using cuChar = const unsigned char;
using cuShort = const unsigned short;
using cuLong = const unsigned long;
using cuInt = const unsigned int;
using WChar = wchar_t; //2字节字符数据
using LLong = __int64; //带符号8字节整型数据
#define SIGN32 uLong(0x80000000)
#define SIGN16 uShort(0x8000)
#define SIGN8 uChar(0x80)

struct RefArmor {
	RefArmor(std::atomic<long>& count) : m_count(count) { ++m_count; }
	~RefArmor() { --m_count; }

private:
	std::atomic<long>& m_count;
};
//==============================BitMaskStatus===================================
//
class BitMaskStatus {
public:
	BitMaskStatus(uLong initmask = 0) : m_BitMask(initmask) {}
	int operator=(uLong mask) { return (m_BitMask = mask); }
	void ClearBit(uLong mask) { m_BitMask &= ~mask; }
	void SetBit(uLong mask) { m_BitMask |= mask; }
	bool IsTrue() { return (m_BitMask) ? true : false; }
	bool IsFalse() { return (m_BitMask) ? false : true; }
	bool IsTrue(uLong mask) { return (m_BitMask & mask) ? true : false; }
	bool IsFalse(uLong mask) { return (m_BitMask & mask) ? false : true; }

private:
	uLong m_BitMask;
};

//=================================================================
template <typename T>
inline T* ToPointer(uintptr_t address) {
	return reinterpret_cast<T*>(address);
}
inline uintptr_t ToAddress(void* pointer) {
	return reinterpret_cast<uintptr_t>(pointer);
}
//=================================================================
inline char* MemCpy(char* dest, cChar* src, size_t size) {
	return (char*)memcpy(dest, src, size);
}
inline char* MemSet(char* dest, int val, size_t size) {
	return (char*)memset(dest, val, size);
}
//=================================================================
inline int wctmbcpy(char* dst, cwChar* src) {
	return WideCharToMultiByte(936,
#if (WINVER >= 0x0500) /* WINVER >= 0x0500 */
							   WC_NO_BEST_FIT_CHARS |
#endif
								   WC_COMPOSITECHECK,
							   src, -1, dst, dst ? 0x7FFFFFFF : 0, nullptr, nullptr);
}
inline int wctmbcpy(uChar* dst, cwChar* src) { return wctmbcpy(reinterpret_cast<char*>(dst), src); }
//=================================================================
class Mutex {
public:
	Mutex(uLong ulSpinCount = 4000) : m_ulSpinCount(ulSpinCount), m_create(false) { Create(false); }
	~Mutex() {
		if (m_create) {
			m_create = false;
			lock();
			DeleteCriticalSection(&m_handle);
		}
	}

	bool Create(bool bInitialOwner) {
		if (!m_create) {
			InitializeCriticalSectionAndSpinCount(&m_handle, 0x80000000 + m_ulSpinCount);
			m_create = true;
		}
		if (bInitialOwner) {
			lock();
		}
		return m_create;
	}
	operator bool() const { return m_create; }
	void lock() const {
		if (m_create) {
			EnterCriticalSection(&m_handle);
		}
	}
	BOOL trylock() const { return m_create ? TryEnterCriticalSection(&m_handle) : 0; }
	void unlock() const {
		if (m_create) {
			LeaveCriticalSection(&m_handle);
		}
	}

private:
	bool m_create;
	uLong m_ulSpinCount;
	mutable CRITICAL_SECTION m_handle;
};
class MutexArmor {
public:
	MutexArmor(const Mutex& mtx) : m_mtx(mtx), m_locknum(0) { lock(); };
	~MutexArmor() {
		while (m_locknum) {
			unlock();
		}
	};
	operator bool() const { return m_mtx; };

	void lock() const {
		m_mtx.lock();
		m_locknum++;
	}
	BOOL trylock() const {
		BOOL l_ret = m_mtx.trylock();
		if (l_ret) {
			m_locknum++;
		}
		return l_ret;
	}
	void unlock() const {
		if (m_locknum) {
			m_locknum--;
			m_mtx.unlock();
		}
	}

private:
	mutable uLong m_locknum{0};
	const Mutex& m_mtx;
};

//=================================================================
class Sema {
public:
	Sema() : m_handle(nullptr), m_lMaximumCount(0) {}
	Sema(LONG lInitialCount, LONG lMaximumCount, LPCTSTR lpName = nullptr) : m_handle(nullptr), m_lMaximumCount(0) { Create(lInitialCount, lMaximumCount, lpName); }
	~Sema() { Destroy(); }
	void Destroy() {
		if (m_handle) {
			CloseHandle(m_handle);
			m_handle = nullptr;
		}
	}
	operator bool() { return m_handle != nullptr; };
	BOOL Create(LONG lInitialCount, LONG lMaximumCount, LPCTSTR lpName = nullptr) {
		if (!m_handle) {
			m_handle = CreateSemaphore(nullptr, lInitialCount, lMaximumCount, lpName);
			return TRUE;
		} else {
			return FALSE;
		}
	}

	DWORD lock() { return WaitForSingleObject(m_handle, INFINITE); }
	DWORD trylock() { return WaitForSingleObject(m_handle, 0); }
	DWORD timelock(DWORD dwMilliseconds) { return WaitForSingleObject(m_handle, dwMilliseconds); }
	BOOL unlock(Long relcount = 1) { return ReleaseSemaphore(m_handle, relcount, nullptr); }

private:
	HANDLE m_handle; //LockSemaphore
	std::atomic<long> m_ilCount;
	LONG m_lMaximumCount; //只是一个标志，没有控制最大计数的功能
};

//比较操作符定义
template <class T>
int operator<(const T& t1, const T& t2) { return (t2 > t1); }
template <class T>
int operator<=(const T& t1, const T& t2) { return !(t1 > t2); }
template <class T>
int operator>=(const T& t1, const T& t2) { return !(t2 > t1); }
template <class T>
int operator!=(const T& t1, const T& t2) { return !(t1 == t2); }

/*/=================================================================
class Mutex{
public:
	Mutex():m_handle(0){}
	~Mutex(){if(m_handle){CloseHandle(m_handle);m_handle=0;}}
	HANDLE Create(BOOL bInitialOwner,LPCTSTR lpName)
	{
		m_handle	=CreateMutex(0,bInitialOwner,lpName);
		return m_handle;
	};
	operator bool()const{return m_handle!=0;};

	DWORD	lock()const					{return WaitForSingleObject(m_handle,INFINITE);}
	DWORD	trylock()const				{return WaitForSingleObject(m_handle,0);}
	DWORD	timelock(DWORD time)const	{return WaitForSingleObject(m_handle,time);}
	BOOL	unlock()const				{return ReleaseMutex(m_handle);}
private:
	HANDLE m_handle;
};
class MutexArmor{
public:
	MutexArmor(Mutex& mtx):m_mtx(mtx),m_locknum(0){};
	~MutexArmor()
	{
		while(m_locknum >0)
		{
			unlock();
		}
	};
	operator bool()const{return m_mtx;};

	DWORD lock()const
	{
		DWORD l_ret =m_mtx.lock();
		if(l_ret ==WAIT_OBJECT_0)
		{
			m_locknum++;
		}else
		if(l_ret ==WAIT_ABANDONED)
		{
			m_locknum =1;
		}
		return l_ret;
	}
	DWORD trylock()const
	{
		DWORD l_ret =m_mtx.trylock();
		if(l_ret ==WAIT_OBJECT_0)
		{
			m_locknum++;
		}else
		if(l_ret ==WAIT_ABANDONED)
		{
			m_locknum =1;
		}
		return l_ret;
	}
	DWORD timelock(DWORD time)const
	{
		DWORD l_ret =m_mtx.timelock(time);
		if(l_ret ==WAIT_OBJECT_0)
		{
			m_locknum++;
		}else
		if(l_ret ==WAIT_ABANDONED)
		{
			m_locknum =1;
		}
		return l_ret;
	}
	BOOL unlock()const
	{
		BOOL l_ret =m_mtx.unlock();
		if(l_ret)
		{
			m_locknum--;
		}
		return l_ret;
	}
private:
	mutable uLong	m_locknum;
	Mutex		&	m_mtx;
};
*/

#pragma pack(pop)
_DBC_END