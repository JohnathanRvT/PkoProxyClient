#pragma once

#include <exception>
#include <string>
#include "DBCCommon.h"

#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable : 4996)
_DBC_BEGIN
#pragma pack(push)
#pragma pack(4)

//------------------------------------------------------------------------------------------------------------------
//common exception define
class excp : public std::exception //基异常类
{
public:
	excp(cChar * desc) : std::exception((_mPtr = new char[strlen(desc) + 1]) ? strcpy(_mPtr, desc) : "") {}
	excp(excp & e) : std::exception(_mPtr = e._mPtr) { e.setZero(); }
	virtual ~excp() { delete[] _mPtr; }
	virtual cChar* what() const override { return _mPtr; }

private:
	excp& operator=(excp& e) {
		delete[] _mPtr;
		exception::operator=(e);
		_mPtr = e._mPtr;
		e.setZero();
		return *this;
	};
	void setZero() {
		exception::operator=(exception());
		_mPtr = nullptr;
	}

	char* _mPtr;
};
//------------------------------------------------------------------------------------------------------------------
class excpMem : public excp //内存分配或释放异常
{
public:
	excpMem(cChar* desc) : excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpArr : public excp //数组越界或其他数组相关错误异常
{
public:
	excpArr(cChar* desc) : excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpSync : public excp //操作系统同步对象操作异常
{
public:
	excpSync(cChar* desc) : excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpThrd : public excp //操作系统线程操作异常
{
public:
	excpThrd(cChar* desc) : excp(desc){};
};
class excpSock : public excp //操作系统线程操作异常
{
public:
	excpSock(cChar* desc) : excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpCOM : public excp //COM操作异常
{
public:
	excpCOM(cChar* desc) : excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpDB : public excp //数据库操作异常
{
public:
	excpDB(cChar* desc) : excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpIniF : public excp //文件操作异常
{
public:
	excpIniF(cChar* desc) : excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpFile : public excp //文件操作异常
{
public:
	excpFile(cChar* desc) : excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpXML : public excp //文件操作异常
{
public:
	excpXML(cChar* desc) : excp(desc){};
};

#pragma pack(pop)
_DBC_END

//===================================================================================================================================

//From: https://stackoverflow.com/questions/2670816/how-can-i-use-the-compile-time-constant-line-in-a-string
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

#define THROW_EXCP(EXCP, DESC) throw EXCP((std::string("File:") + __FILE__ + " Line:" + LINE_STRING + " desc:" + DESC).c_str()); //程序中抛异常所用的宏定义