
#ifndef _ALGO_H_
#define _ALGO_H_

unsigned char const* sha1(unsigned char const* msg, unsigned int len, unsigned char* md);

bool dbpswd_out(char const* ctxt, int ctxt_len, std::string& pswd);
bool dbpswd_in(char const* pswd, int pswd_len, std::string& ctxt);

std::string md5string(std::string msg);

#include <string>

#define KCHAP_SESSION_KEY_LEN 6

class KCHAPc {
public:
	KCHAPc(char const* name, std::string password);
	~KCHAPc();

	std::string GetPwdDigest() const {
		return m_strPwdDig;
	}
	void SetChapString(std::string chap_string);
	void GetStep1Data(unsigned char* buf, int& out_len);
	void GetSessionClrKey(void* pdat, int dat_len,
						  unsigned char* buf, int& out_len);

private:
	std::string m_strName;
	std::string m_strPwd;
	std::string m_strChapString;

	char skey[KCHAP_SESSION_KEY_LEN];

public:
	std::string m_strPwdDig;
};

class KCHAPs {
public:
	KCHAPs(const char* passwd_digest = "");
	~KCHAPs();

	bool DoAuth(char const* name, char const* chap_string,
				char const* pdat, int dat_len, char const* passwd_digest);
	void NewSessionKey();
	void GetSessionClrKey(char* buf, int& out_len);
	void GetSessionEncKey(unsigned char* buf, int& out_len);

private:
	std::string m_strName;
	std::string m_strPwdDig;
	std::string m_strChapString;

	char skey[KCHAP_SESSION_KEY_LEN];
};

#endif
