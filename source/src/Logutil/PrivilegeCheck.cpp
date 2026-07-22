#include "PrivilegeCheck.h"

PrivilegeCheck* PrivilegeCheck::_instance = nullptr;

PrivilegeCheck::PrivilegeCheck() {
}

PrivilegeCheck::~PrivilegeCheck() {
}

PrivilegeCheck* PrivilegeCheck::Instance() {
	if (_instance == nullptr) {
		_instance = new PrivilegeCheck();
	}

	return _instance;
}

bool PrivilegeCheck::Init(const char* configFileName) {
	std::fstream fs(configFileName, std::ios_base::in);
	if (!fs.is_open()) {
		return false;
	}

	char buf[255];

	char* token;
	char* nexttoken{nullptr};

	while (!fs.eof()) {
		fs.getline(buf, 255);

		std::vector<std::string> item;
		CommandV commands;

		token = strtok_s(buf, "=", &nexttoken);
		while (token != nullptr) {
			item.emplace_back(token);

			token = strtok_s(nullptr, "=", &nexttoken);
		}

		if (item.size() == 2) {
			if (item[0].compare(0, 2, "//") == 0)
				continue;

			token = strtok_s(const_cast<char*>(item[1].c_str()), ",", &nexttoken);
			while (token != nullptr) {
				commands.emplace_back(token);

				token = strtok_s(nullptr, ",", &nexttoken);
			}

			m_mapPrivilege[atoi(item[0].c_str())] = commands;
		}
	}

	fs.close();

	return true;
}

bool PrivilegeCheck::IsAdmin(const char* accountName, const char* ip) {
#ifdef _DEBUG
	return true;
#else
	// 目前IP还取不到，以后追加公司IP限制
	//if(strcmp(accountName, "GameTest1") == 0 &&
	//	(strcmp(ip, "116.228.42.71") == 0 || strcmp(ip, "10.0.30.60") == 0))
	if (strcmp(accountName, "gametest1") == 0) {
		return true;
	}
#endif

	return false;
}

bool PrivilegeCheck::HasPrivilege(int level, const char* command, const char* accountName, const char* ip) {
	if (IsAdmin(accountName, ip))
		return true;

	for (auto it = m_mapPrivilege.begin(); it != m_mapPrivilege.end(); it++) {
		if (it->first <= level) {
			for (auto cit = it->second.begin(); cit != it->second.end(); cit++) {
				if (cit->compare(command) == 0)
					return true;
			}
		} else
			break;
	}

	return false;
}