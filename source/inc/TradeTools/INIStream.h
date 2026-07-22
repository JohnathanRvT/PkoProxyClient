
/* * * * * * * * * * * * * * * * * * * *

    INI 文件读写操作
    Jampe
    2006/3/28

 * * * * * * * * * * * * * * * * * * * */

#if !defined __INISTREAM_H__
#define __INISTREAM_H__

#pragma once

#include "datatype.h"

class INIStream {
public:
	INIStream(void);
	INIStream(const char* _filepath);
	virtual ~INIStream(void);

	virtual void SetPath(const char* _filepath); //设置配置文件路径

	virtual uint ReadInt(const char* _appname, const char* _keyname, int _default = 0);												//读取整数
	virtual ulong ReadString(char* _returnstring, int _len, const char* _appname, const char* _keyname, const char* _default = ""); //读取字符串
	virtual ulong ReadSection(char* _returnstring, int _len, const char* _appname);													//读取小节内所有的关键字和值。_returnstring形式：key1=value1\0key2=value2\0key3=value3\0\0
	virtual ulong ReadSectionName(char* _returnstring, int _len);																	//读取所有小节名称。_returnstring形式：name1\0name2\0name3\0\0

	virtual bool WriteInt(const char* _appname, const char* _keyname, int _value);			  //写入一个整数
	virtual bool WriteString(const char* _appname, const char* _keyname, const char* _value); //写入字符串
	virtual bool WriteSection(const char* _appname, const char* _value);					  //将关键字和值写入小节。_value格式要求：key1=value1\0key2=value2\0key3=value3\0

	virtual bool DeleteSection(const char* _appname);					//删除一个小节
	virtual bool DeleteKey(const char* _appname, const char* _keyname); //删除一个关键字和它的值

public:
	char m_FilePath[MAX_PATH];
};

#endif
