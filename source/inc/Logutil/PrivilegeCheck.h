#pragma once

#include <utility>
#include <vector>
#include <map>
#include <fstream>


class PrivilegeCheck {
private:
	PrivilegeCheck();

public:
	~PrivilegeCheck();

public:
	static PrivilegeCheck* Instance(); // 唯一实例
	bool Init(const char* configFileName);
	bool HasPrivilege(int level, const char* command, const char* accountName = "", const char* ip = "");

	bool IsAdmin(const char* accountName, const char* ip = "");

private:
	using CommandV = std::vector<std::string>;		 // 命令数组
	using PrivilegeMap = std::map<int, CommandV>; // 权限集合

	using CommandIt = CommandV::iterator;		// 命令
	using PrivilegeIt = PrivilegeMap::iterator; // 权限

	static PrivilegeCheck* _instance;
	PrivilegeMap m_mapPrivilege;
};
