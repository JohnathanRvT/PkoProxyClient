#pragma once

class CChatRecord {
public:
	CChatRecord();
	~CChatRecord();
	static bool Save(const std::string name, DWORD number, const std::string chatData);
	static std::string GetLastSavePath();

private:
	static std::string m_strPath;
};
