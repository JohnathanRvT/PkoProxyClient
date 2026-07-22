#include "util.h"
#include "algo.h"
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include "rng.h"

//TODO: Accept an encryption from text file.
bool dbpswd_out(char const* ctxt, int ctxt_len, std::string& pswd) {
	pswd = ctxt;
	return true;
}

bool dbpswd_in(char const* pswd, int pswd_len, std::string& ctxt) {
	ctxt = pswd;
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
// Message Digest 5
//
std::string md5string(std::string input) {
	//Taken from: http://ivbel.blogspot.com/2012/02/how-to-calculate-md5-hash-value-using.html adapted to updated example in OpenSSL 1.1.0 manual.

	EVP_MD_CTX* mdctx;
	unsigned char md_value[EVP_MAX_MD_SIZE] = {0};
	unsigned int md_len, i;

	/* You can pass the name of another algorithm supported by your version of OpenSSL here */
	/* For instance, MD2, MD4, SHA1, RIPEMD160 etc. Check the OpenSSL documentation for details */
	const EVP_MD* md = EVP_md5();
	md_len = EVP_MD_size(md);

	if (!md) {
		return "";
	}

	mdctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(mdctx, md, nullptr);
	EVP_DigestUpdate(mdctx, input.c_str(), input.length());
	/* to add more data to hash, place additional calls to EVP_DigestUpdate here */
	EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	EVP_MD_CTX_free(mdctx);

	///* Now output contains the hash value, output_len contains length of output, which is 128 bit or 16 byte in case of MD5 */
	//Taken from: https://stackoverflow.com/questions/11181251/saving-hex-values-to-a-c-string
	std::ostringstream buffer;
	buffer << std::hex << std::setfill('0') << std::uppercase;
	for (i = 0; i < md_len; i++)
		buffer << std::setw(2) << static_cast<int>(md_value[i]);
	return buffer.str();
}

//////////////////////////////////////////////////////////////////////////
//
// KOP CHAP class (client and server)
//

KCHAPc::KCHAPc(char const* name, std::string password) {
	std::string dig = md5string(password);

	m_strName = name;
	m_strPwd = password;
	m_strPwdDig = dig;

	memset(skey, 0, sizeof skey);
}

KCHAPc::~KCHAPc() {
	m_strName = "";
	m_strPwd = "";
	m_strPwdDig = "";

	memset(skey, 0, sizeof skey);
}

void KCHAPc::SetChapString(std::string chap_string) {
	m_strChapString = chap_string;
}

void KCHAPc::GetStep1Data(unsigned char* buf, int& out_len) { //size is set to EVP_MAX_MD_SIZE
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	int md_len;

	unsigned char dig[EVP_MAX_MD_SIZE];
	unsigned char chapstring[EVP_MAX_MD_SIZE];
	unsigned char iv[EVP_MAX_MD_SIZE] = {"abcdefghijklmnopqrstuvwxyz"};

	std::copy(m_strPwdDig.begin(), m_strPwdDig.end(), dig);
	std::copy(m_strChapString.begin(), m_strChapString.end(), chapstring);

	EVP_EncryptInit_ex(ctx, EVP_des_ecb(), nullptr, dig, iv);
	EVP_EncryptUpdate(ctx, buf, &md_len, chapstring, m_strChapString.length());
	EVP_EncryptFinal_ex(ctx, buf + md_len, &out_len);

	EVP_CIPHER_CTX_cleanup(ctx);

	//std::ostringstream buffer;
	//buffer << std::hex << std::setfill('0') << std::uppercase;
	//for (i = 0; i < out_len; i++)
	//	buffer << std::setw(2) << static_cast<int>(buf[i]);
	//return buffer.str();
}

void KCHAPc::GetSessionClrKey(void* pdat, int dat_len,
							  unsigned char* buf, int& out_len) {
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	int md_len;

	unsigned char dig[EVP_MAX_MD_SIZE];
	unsigned char iv[EVP_MAX_MD_SIZE] = {"abcdefghijklmnopqrstuvwxyz"};

	std::copy(m_strPwdDig.begin(), m_strPwdDig.end(), dig);

	EVP_DecryptInit_ex(ctx, EVP_des_ecb(), nullptr, dig, iv);
	EVP_DecryptUpdate(ctx, buf, &md_len, (const unsigned char*)pdat, dat_len);
	EVP_DecryptFinal_ex(ctx, buf + md_len, &out_len);

	EVP_CIPHER_CTX_cleanup(ctx);

	//std::ostringstream buffer;
	//buffer << std::hex << std::setfill('0') << std::uppercase;
	//for (i = 0; i < out_len; i++)
	//	buffer << std::setw(2) << static_cast<int>(buf[i]);
	//return buffer.str();
}

KCHAPs::KCHAPs(const char* passwd_digest) {
	m_strName = "";
	m_strPwdDig = passwd_digest;
	m_strChapString = "";

	memset(skey, 0, sizeof skey);
	srand((unsigned)time(nullptr));
}

KCHAPs::~KCHAPs() {
	m_strName = "";
	m_strPwdDig = "";
	m_strChapString = "";

	memset(skey, 0, sizeof skey);
}

bool KCHAPs::DoAuth(char const* name, char const* chap_string,
					char const* pdat, int dat_len,
					char const* passwd_digest) {
	bool ret;

	// 保存相关数据
	m_strName = name;
	m_strPwdDig = passwd_digest;
	m_strChapString = chap_string;

	//std::string szDat;

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	unsigned char buf[1024] = {0};
	int md_len, i, out_len;

	unsigned char dig[EVP_MAX_MD_SIZE];
	unsigned char chapstring[EVP_MAX_MD_SIZE];
	unsigned char iv[EVP_MAX_MD_SIZE] = {"abcdefghijklmnopqrstuvwxyz"};

	std::copy(m_strPwdDig.begin(), m_strPwdDig.end(), dig);
	std::copy(m_strChapString.begin(), m_strChapString.end(), chapstring);

	EVP_EncryptInit_ex(ctx, EVP_des_ecb(), nullptr, dig, iv);
	EVP_EncryptUpdate(ctx, buf, &md_len, chapstring, m_strChapString.length());
	EVP_EncryptFinal_ex(ctx, buf + md_len, &out_len);

	EVP_CIPHER_CTX_cleanup(ctx);

	//std::ostringstream buffer;
	//buffer << std::hex << std::setfill('0') << std::uppercase;
	//for (i = 0; i < md_len; i++)
	//	buffer << std::setw(2) << static_cast<int>(buf[i]);
	//szDat = buffer.str();

	// compare
	ret = (memcmp(pdat, buf, out_len) == 0) ? true : false;
	if (ret) {
		// 认证成功，则立即产生会话密钥
		NewSessionKey();
	}

	return ret;
}

void KCHAPs::NewSessionKey() {
	
	for (int i = 0; i < sizeof skey; ++i) {
		skey[i] = rng.uniform(1, 127 - 1);
	}
}

void KCHAPs::GetSessionClrKey(char* buf, int& out_len) {
	int len = sizeof skey;
	memcpy((void*)buf, skey, len);
	out_len = len;
}

void KCHAPs::GetSessionEncKey(unsigned char* buf, int& out_len) {
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	int md_len;

	unsigned char dig[EVP_MAX_MD_SIZE];
	unsigned char chapstring[EVP_MAX_MD_SIZE];
	unsigned char iv[EVP_MAX_MD_SIZE] = {"abcdefghijklmnopqrstuvwxyz"};

	std::copy(m_strPwdDig.begin(), m_strPwdDig.end(), dig);
	std::copy(m_strChapString.begin(), m_strChapString.end(), chapstring);

	EVP_EncryptInit_ex(ctx, EVP_des_ecb(), nullptr, dig, iv);
	EVP_EncryptUpdate(ctx, buf, &md_len, chapstring, m_strChapString.length());
	EVP_EncryptFinal_ex(ctx, buf + md_len, &out_len);

	EVP_CIPHER_CTX_cleanup(ctx);

	//std::ostringstream buffer;
	//buffer << std::hex << std::setfill('0') << std::uppercase;
	//for (i = 0; i < md_len; i++)
	//	buffer << std::setw(2) << static_cast<int>(buf[i]);
	//return buffer.str();
}

// SHA-1
unsigned long func_S(char n, unsigned long num) {
	return (num << n) | (num >> (32 - n));
}

unsigned long func_F(unsigned int t, unsigned long B, unsigned long C, unsigned long D) {
	if (t < 20) {
		return ((B & C) | ((~B) & D));
	} else if (t < 40) {
		return (B ^ C ^ D);
	} else if (t < 60) {
		return ((B & C) | (B & D) | (C & D));
	} else if (t < 80) {
		return (B ^ C ^ D);
	} else
		return 0;
}

unsigned char const* sha1(unsigned char const* msg, unsigned int len, unsigned char* md) {
	unsigned long message_length;
	unsigned char i, ctemp, index;
	unsigned char hasAdd1 = 0;
	unsigned char hasAddLen = 0;
	unsigned long W[16];
	unsigned long H[5];
	unsigned long A, B, C, D, E, s;
	unsigned long MASK = 0x0000000F;
	unsigned long K[4] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};
	unsigned long TEMP;
	unsigned int t;

	message_length = len;
	H[0] = 0x67452301;
	H[1] = 0xEFCDAB89;
	H[2] = 0x98BADCFE;
	H[3] = 0x10325476;
	H[4] = 0xC3D2E1F0;
	index = 0;

	while (!hasAddLen) {
		for (t = 0; t < 16; ++t) {
			W[t] = 0;
			for (i = 0; i < 4; ++i) {
				if (!hasAdd1) {
					if (index >= len) {
						ctemp = 0x80;
						hasAdd1 = 1;
					} else
						ctemp = msg[index++];
				} else
					ctemp = 0x00;

				W[t] = W[t] * 256 + ctemp;
			}
		} // end for

		if (hasAdd1)
			if ((W[14] == 0x00) && (W[15] == 0x00)) {
				W[15] = message_length;
				for (t = 0; t < 3; ++t) {
					if (W[15] & 0x80000000)
						W[14] = W[14] * 2 + 1;
					W[15] = W[15] * 2;
				}

				hasAddLen = 1;
			}

		A = H[0];
		B = H[1];
		C = H[2];
		D = H[3];
		E = H[4];
		for (t = 0; t < 80; ++t) {
			s = t & MASK;
			if (t >= 16) {
				W[s] = W[(s + 13) & MASK] ^ W[(s + 8) & MASK] ^ W[(s + 2) & MASK] ^ W[s];
				W[s] = func_S(1, W[s]);
			}
			TEMP = func_S(5, A) + func_F(t, B, C, D) + E + W[s] + K[t / 20];
			E = D;
			D = C;
			C = func_S(30, B);
			B = A;
			A = TEMP;
		}

		H[0] += A;
		H[1] += B;
		H[2] += C;
		H[3] += D;
		H[4] += E;
	} // end while

	md[0] = (unsigned char)((H[0] >> 24) & 0xFF);
	md[1] = (unsigned char)((H[0] >> 16) & 0xFF);
	md[2] = (unsigned char)((H[0] >> 8) & 0xFF);
	md[3] = (unsigned char)((H[0]) & 0xFF);
	md[4 + 0] = (unsigned char)((H[1] >> 24) & 0xFF);
	md[4 + 1] = (unsigned char)((H[1] >> 16) & 0xFF);
	md[4 + 2] = (unsigned char)((H[1] >> 8) & 0xFF);
	md[4 + 3] = (unsigned char)((H[1]) & 0xFF);
	md[8 + 0] = (unsigned char)((H[2] >> 24) & 0xFF);
	md[8 + 1] = (unsigned char)((H[2] >> 16) & 0xFF);
	md[8 + 2] = (unsigned char)((H[2] >> 8) & 0xFF);
	md[8 + 3] = (unsigned char)((H[2]) & 0xFF);
	md[12 + 0] = (unsigned char)((H[3] >> 24) & 0xFF);
	md[12 + 1] = (unsigned char)((H[3] >> 16) & 0xFF);
	md[12 + 2] = (unsigned char)((H[3] >> 8) & 0xFF);
	md[12 + 3] = (unsigned char)((H[3]) & 0xFF);
	md[16 + 0] = (unsigned char)((H[4] >> 24) & 0xFF);
	md[16 + 1] = (unsigned char)((H[4] >> 16) & 0xFF);
	md[16 + 2] = (unsigned char)((H[4] >> 8) & 0xFF);
	md[16 + 3] = (unsigned char)((H[4]) & 0xFF);

	return md;
}