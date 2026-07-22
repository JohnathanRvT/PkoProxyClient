
#include "inistream.h"
#include <string.h>
#include <stdlib.h>

INIStream::INIStream(void) {
	memset(m_FilePath, 0, MAX_PATH);
}

INIStream::INIStream(const char* _filepath) {
	SetPath(_filepath);
}

INIStream::~INIStream(void) {
}

void INIStream::SetPath(const char* _filepath) {
	memset(m_FilePath, 0, MAX_PATH);
	strncpy(m_FilePath, _filepath, MAX_PATH);
}

unsigned int INIStream::ReadInt(const char* _appname, const char* _keyname, int _default) {
	return GetPrivateProfileInt(_appname, _keyname, _default, m_FilePath);
}

unsigned long INIStream::ReadString(char* _returnstring, int _len, const char* _appname, const char* _keyname, const char* _default) {
	return GetPrivateProfileString(_appname, _keyname, _default, _returnstring, _len, m_FilePath);
}

unsigned long INIStream::ReadSection(char* _returnstring, int _len, const char* _appname) {
	return GetPrivateProfileSection(_appname, _returnstring, _len, m_FilePath);
}

unsigned long INIStream::ReadSectionName(char* _returnstring, int _len) {
	return GetPrivateProfileSectionNames(_returnstring, _len, m_FilePath);
}

bool INIStream::WriteInt(const char* _appname, const char* _keyname, int _value) {
	char __buf[32] = {0};
	return WriteString(_appname, _keyname, itoa(_value, __buf, 10 /*十进制*/));
}

bool INIStream::WriteString(const char* _appname, const char* _keyname, const char* _value) {
	return TRUE == WritePrivateProfileString(_appname, _keyname, _value, m_FilePath);
}

bool INIStream::WriteSection(const char* _appname, const char* _value) {
	return TRUE == WritePrivateProfileSection(_appname, _value, m_FilePath);
}

bool INIStream::DeleteSection(const char* _appname) {
	return WriteSection(_appname, NULL);
}

bool INIStream::DeleteKey(const char* _appname, const char* _keyname) {
	return WriteString(_appname, _keyname, NULL);
}
