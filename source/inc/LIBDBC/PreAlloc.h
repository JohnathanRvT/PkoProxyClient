//================================================================
// It must be permitted by Dabo.Zhang that this program is used for
// any purpose in any situation.
// Copyright (C) Dabo.Zhang 2000-2003
// All rights reserved by ZhangDabo.
// This program is written(created) by Zhang.Dabo in 2000.3
// This program is modified recently by Zhang.Dabo in 2003.7
//=================================================================
#pragma once

#ifndef USING_TAO	 //ʹ��Win32����Platform SDK
#include <winsock2.h> //ȷ�������µ�WinSock2.2�汾
#include <windows.h>
#else
#include "TAOSpecial.h"
#endif

#include "DBCCommon.h"
#include "excp.h"
#include "assert.h"
#include <new>
//===Ԥ����ṹ========

_DBC_BEGIN
#pragma pack(push)
#pragma pack(4)
//=======PreAllocStru=======================================================================
class PreAllocStru {
protected:
	PreAllocStru(size_t) {}
	virtual ~PreAllocStru() {}

public:
	bool IsFree() { return !__preAllocHeap; }
	virtual void Free();
	virtual size_t Size(); //������Override���ص�ǰ�ṹ����Ļ���ߴ�

	template <class T>
	friend class PreAllocHeap;
	template <class T>
	friend class PreAllocHeapPtr;

private:
	virtual void Initially() {}
	virtual void Finally() {}
	PreAllocHeap<PreAllocStru>* volatile __preAllocHeap{0};
	PreAllocHeapPtr<PreAllocStru>* volatile __preAllocHeapPtr{0};
	PreAllocStru* volatile __nextHeapItem{nullptr};
};
//==========PreAllocHeap====================================================================
struct __FIFOQu_ {
	PreAllocStru* volatile head;
	PreAllocStru* volatile tail;
	uLong volatile free; //������;
};
template <class T>
class PreAllocHeap {
	friend class PreAllocStru;
	template <class T>
	friend class PreAllocHeapPtr;

public:
	PreAllocHeap(size_t unitsize)
		: m_unitsize(unitsize ? unitsize : 1), m_initnum(0), m_unitflag(unitsize ? unitsize : 1) {
	}
	PreAllocHeap(size_t unitsize, size_t initnum)
		: m_unitsize(unitsize ? unitsize : 1), m_initnum(initnum ? initnum : 1), m_unitflag(unitsize ? unitsize : 1) {
	}
	virtual ~PreAllocHeap() {
		Finally();
		*const_cast<size_t*>(&m_unitflag) = 0;
	};

	void Init(size_t bufnumlist[]) //bufnumlistΪһ0��β����
	{
		MutexArmor l_lock(m_mtxfreeacc);
		if (bufnumlist) {
			for (size_t k = 0; bufnumlist[k]; k++) {
				InitLoop(bufnumlist[k], (k + 1));
			}
		} else {
			if (!m_numptr) {
				m_numptr = 1;
				m_freestru = new __FIFOQu_[m_numptr];
				if (!m_freestru) {
					THROW_EXCP(excpMem, "Ԥ������ڴ����ʧ��");
				}
				m_freestru[0].head = m_freestru[0].tail = 0;
				m_freestru[0].free = 0;
			}
			InitLoop(m_initnum, 1);
		}
		l_lock.unlock();
	};
	void Init() {
		Init({});
	};
	T* Get(size_t size = 0) {
		if (!m_unitsize) {
			return 0;
		}
		MutexArmor l_lock(m_mtxfreeacc);

		PreAllocStru* l_t = nullptr;
		if (!size || m_initnum)
			size = m_unitsize;

		size_t l_reqpos = (size + m_unitsize - 1) / m_unitsize;

		if ((l_reqpos <= m_numptr) && m_freestru[l_reqpos - 1].head) {
			l_t = m_freestru[l_reqpos - 1].head;
			m_freestru[l_reqpos - 1].head = l_t->__nextHeapItem;
			if (!m_freestru[l_reqpos - 1].head) {
				m_freestru[l_reqpos - 1].tail = 0;
			}
			m_freestru[l_reqpos - 1].free--;
		}
		l_lock.unlock();

		if (!l_t) {
			l_t = newT(m_unitsize * l_reqpos);
		}

		l_t->__preAllocHeap = reinterpret_cast<PreAllocHeap<PreAllocStru>*>(this);
		l_t->__nextHeapItem = nullptr;

		++m_usednum;
		try {
			l_t->Initially();
		} catch (...) {
			throw;
		}

		return static_cast<T*>(l_t);
	}
	size_t GetUsedNum() const { return m_usednum; }
	size_t GetUnitSize() const { return m_unitsize; }

private:
	void InitLoop(size_t num, size_t mult) {
		PreAllocStru* t;
		for (size_t i = 0; i < num; i++) {
			t = newT(m_unitsize * mult);
			t->__preAllocHeap = 0;
			InsertT(t, m_unitsize * mult);
		}
	}

	PreAllocHeap<T>& operator<<(PreAllocStru* t) {
		size_t l_size = t->Size();

		MutexArmor l_lock(m_mtxfreeacc);
		if (t->__preAllocHeap == reinterpret_cast<PreAllocHeap<PreAllocStru>*>(this)) {
			t->__preAllocHeap = 0;
			l_lock.unlock();

			try {
				t->Finally();
			} catch (...) {
				--m_usednum;

				InsertT(t, l_size);
				throw;
			}
			--m_usednum;

			InsertT(t, l_size);
		}
		return *this;
	}
	void InsertT(PreAllocStru* t, size_t size) {
		MutexArmor l_lock(m_mtxfreeacc);
		if (!m_unitflag) {
			l_lock.unlock();
			if (t)
				delete t;
			return;
		}
		size_t l_pos = (size + m_unitsize - 1) / m_unitsize;
		if (l_pos > m_numptr) {
			size_t l_num = (l_pos + 15) / 16;
			auto* l_ptr = new __FIFOQu_[l_num *= 16];
			if (!l_ptr) {
				THROW_EXCP(excpMem, "Ԥ������ڴ����ʧ��");
			}
			if (m_freestru) //�����쳣 by Waiting 2009-06-30
				MemCpy((char*)l_ptr, (char*)m_freestru, m_numptr * sizeof(__FIFOQu_));
			MemSet((char*)(l_ptr + m_numptr), 0, (l_num - m_numptr) * sizeof(__FIFOQu_)); //�¿��ٵĿռ���Ϊ0;
			if (m_freestru)
				delete[] m_freestru; //�����쳣 by Waiting 2009-06-30
			m_freestru = l_ptr;
			m_numptr = l_num;
		}
		assert(m_freestru);
		if (m_freestru) {
			if (m_freestru[l_pos - 1].tail) {
				m_freestru[l_pos - 1].tail->__nextHeapItem = t;
			} else {
				m_freestru[l_pos - 1].head = t;
			}
			m_freestru[l_pos - 1].tail = t;
			m_freestru[l_pos - 1].free++;
			assert(t);
			//			if( t )
			t->__nextHeapItem = nullptr;
		}
		// 		else
		// 		{
		// 			l_lock.unlock();
		// 			if( t )
		// 				delete t;
		// 			return;
		// 		}
		l_lock.unlock();
	};
	PreAllocStru* newT(size_t size) {
		PreAllocStru* l_t = nullptr;

		try {
			l_t = new T(size);
		} catch (...) {
			THROW_EXCP(excpMem, "Ԥ������ڴ����ʧ��");
		}

		if (l_t) {
			l_t->__preAllocHeap = reinterpret_cast<PreAllocHeap<PreAllocStru>*>(this);
			if (l_t->Size() < size) {
				delete l_t;
				THROW_EXCP(excpMem, "Ԥ������ڴ����ʧ��");
			}
		} else {
			THROW_EXCP(excpMem, "Ԥ������ڴ����ʧ��");
		}
		return l_t;
	}
	void Finally() {
		MutexArmor l_lock(m_mtxfreeacc);
		if (m_freestru) {
			PreAllocStru* l_struct;
			for (uLong i = 0; i < m_numptr; i++) {
				while (m_freestru[i].head) {
					l_struct = m_freestru[i].head;
					m_freestru[i].head = l_struct->__nextHeapItem;

					l_struct->__nextHeapItem = nullptr;
					try {
						delete l_struct;
					} catch (...) {
						throw; //��ǰû��
					}
				}
				m_freestru[i].tail = 0;
			}
			delete[] m_freestru;
			m_freestru = 0;
			m_numptr = 0;
		}
		l_lock.unlock();
	}

private:
	size_t m_initnum{0};
	size_t m_unitsize, m_unitflag;
	struct
	{
		Mutex m_mtxfreeacc;
		size_t volatile m_numptr{0};
		__FIFOQu_* volatile m_freestru{nullptr};
	};
	std::atomic<long> m_usednum{0};
};
//=======PreAllocHeapPtr=======================================================================
template <class T>
class PreAllocHeapPtr {
	friend class PreAllocStru;

public:
	PreAllocHeapPtr(uLong unitsize, uLong initnum) : m_unitsize(unitsize ? unitsize : 1), m_initnum(initnum ? initnum : 1) {
		if (!m_mtxptr.Create(false))
			THROW_EXCP(excpSync, "Ԥ�����ָ������ͬ��������ʧ��");
	};
	~PreAllocHeapPtr() {
		MutexArmor l_lockPtr(m_mtxptr);
		if (m_ptr) {
			delete m_ptr;
		}
		l_lockPtr.unlock();
	};
	T* Get(size_t size = 0) {
		if (!m_ptr) {
			if (m_initnum) {
				Init();
			} else {
				size_t l_bufnumlist[] = {1, 0};
				Init(l_bufnumlist);
			}
		}
		T* l_t = m_ptr->Get(size);
		l_t->__preAllocHeapPtr = reinterpret_cast<PreAllocHeapPtr<PreAllocStru>*>(this);
		return l_t;
	}
	bool Init(size_t bufnumlist[]) {
		if (m_ptr)
			return false;
		MutexArmor l_lockPtr(m_mtxptr);
		try {
			if (!m_ptr) {
				if (!bufnumlist) {
					auto* l_ptr = new PreAllocHeap<T>(m_unitsize, m_initnum);
					if (!l_ptr)
						THROW_EXCP(excpMem, "Ԥ������ڴ����ʧ��");
					l_ptr->Init();
					m_ptr = l_ptr;
				} else {
					auto* l_ptr = new PreAllocHeap<T>(m_unitsize);
					if (!l_ptr)
						THROW_EXCP(excpMem, "Ԥ������ڴ����ʧ��");
					l_ptr->Init(bufnumlist);
					m_ptr = l_ptr;
				}
			}
		} catch (...) {
			delete m_ptr;
			l_lockPtr.unlock();
			throw;
		}
		l_lockPtr.unlock();
		return true;
	}
	uLong GetUnitSize() const { return m_ptr ? m_ptr->GetUnitSize() : m_unitsize; }
	uLong GetUsedNum() const { return m_ptr ? m_ptr->GetUsedNum() : 0; }

	uLong GetBufNum() const { return m_ptr ? m_ptr->m_numptr : 0; }
	const __FIFOQu_* GetPerformance() const { return m_ptr ? m_ptr->m_freestru : 0; }

private:
	PreAllocHeapPtr<T>& operator<<(PreAllocStru* t) {
		if (t->__preAllocHeapPtr == reinterpret_cast<PreAllocHeapPtr<PreAllocStru>*>(this)) {
			MutexArmor l_lock(m_mtxptr);
			t->__preAllocHeapPtr = 0;
			if (m_ptr) {
				*m_ptr << t;
			} else {
				l_lock.unlock();
				try {
					t->__preAllocHeap = 0;
					t->Finally();
				} catch (...) {
					delete t;
					throw;
				}
				delete t;
			}
		}
		return *this;
	}
	void Init() {
		Init({});
	}
	uLong volatile m_initnum;
	uLong volatile m_unitsize;
	Mutex m_mtxptr;
	PreAllocHeap<T>* volatile m_ptr{nullptr};
};

#pragma pack(pop)
_DBC_END