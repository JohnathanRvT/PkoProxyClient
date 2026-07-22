//================================================================
// It must be permitted by Dabo.Zhang that this program is used for
// any purpose in any situation.
// Copyright (C) Dabo.Zhang 2000-2003
// All rights reserved by ZhangDabo.
// This program is written(created) by Zhang.Dabo in 2000.3
// This program is modified recently by Zhang.Dabo in 2003.7
//=================================================================
#pragma once

#include "PreAlloc.h"
#include "robject.h"
#include "MyAlloc.h" //调试内存 Add by Waiting 2009-06-18

_DBC_BEGIN
#pragma pack(push)
#pragma pack(4)

//基础的数据缓存类
class BaseBuf : public PreAllocStru {
public:
	BaseBuf(size_t size) : PreAllocStru(size), m_size(size), m_buf((char*)_ALLOC(size)) {}
	virtual size_t Size() override { return m_buf ? m_size : 0; }
	char* getbuf() { return m_buf; }

protected:
	~BaseBuf() {
		if (m_buf) {
#ifdef __MEM_DEBUG__
			RemoveTrack(m_buf); //调试内存 Add by Waiting 2009-06-18
#endif
			free(m_buf);
		}
	}

private:
	char* const volatile m_buf;
	size_t m_size;
};
//基础的基于引用计数内存管理得数据缓存类
class rbuf : public BaseBuf, public robject<true> {
public:
	rbuf(size_t size) : BaseBuf(size) {}

protected:
	void Initially() override { InitRCount(0); }
	void Finally() override { InitRCount(1); }
	void Free() override { BaseBuf::Free(); }

private:
};

#pragma pack(pop)
_DBC_END