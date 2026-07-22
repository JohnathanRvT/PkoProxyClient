//------------------------------------------------------------------------
//	2005.3.25	Arcol	create this file
//	2005.4.7	Arcol	add function : bool GetNameFormString(const string &str,string &name)
//						add function : string CutFaceText(string &text,size_t cutLimitlen)
//	2005.4.18	Arcol	add function : bool GetNameFormString(const string &str,string &name)
//						add function : string CutFaceText(string &text,size_t cutLimitlen)
//	2005.4.19	Arcol	add function : void ChangeParseSymbol(string &text,int nMaxCount)
//	2005.4.28	Arcol	remove all filter system function and create a filter manager class in the common lib
//------------------------------------------------------------------------

#include "StringLib.h"
#include "i18n.h"

std::string StringLimit(const std::string& str, size_t len) {
	if (len <= 3 || len >= str.length())
		return str;
	char* buf = new char[len + 5];
	memcpy(buf, str.c_str(), len);
	buf[len] = 0;
	buf[len - 2] = buf[len - 1] = '.';
	std::string ret = buf;
	if (buf)
		delete[] buf;

	return ret;
}

//------------------------------------------------------------------------
//	浠庡瓧绗︿覆涓彇浜虹墿鍚嶅瓧锛屼汉鍚嶆渶澶ч暱搴︿负16涓瓧绗?
//------------------------------------------------------------------------
bool GetNameFormString(const std::string& str, std::string& name) {
	size_t n = str.find(":");
	if (n > 24)
		return false;
	name = str.substr(0, n);
	while (n && name[--n] == 0x20)
		name.erase(n);
	if (name.find("<From ") == 0 && name.length() > 7) {
		name = name.substr(6, name.length() - 7);
	} else if (name.find("<To ") == 0 && name.length() > 5) {
		name = name.substr(4, name.length() - 5);
	}
	if (name.length() <= 16)
		return true;
	return false;
}

//------------------------------------------------------------------------
//	鍒嗗壊鍚湁琛ㄦ儏鏍囪鐨勪腑鏂囨枃瀛楋紝鍒嗗壊鍚庡彲鐢ㄤ簬鍒嗚鏄剧ず
//------------------------------------------------------------------------
std::string CutFaceText(std::string& text, size_t cutLimitlen) {
	// modify by ning.yan 20081007 宸︿笅瑙掕亰澶╂ 绯荤粺鍏憡鍜屾父鎴忎笂闈㈢殑绯荤粺鍏憡 GM鍠婅瘽  鏂瓧澶勭悊 begin
	std::string retStr = text;

	try {
		size_t maxNum = cutLimitlen;
		const char* resName = CResourceBundleManage::Instance()->GetLocale().getName();
		if (strcmp(resName, "zh_Hans_CN") == 0 || strcmp(resName, "zh_Hant_TW") == 0 || strcmp(resName, "zh_Hant_HK") == 0)
			maxNum = cutLimitlen / 2;
		if (cutLimitlen <= 0 || text.length() <= maxNum) {
			text.clear();
			return retStr;
		}

		icu::UnicodeString str = text.c_str();
		int32_t lineStarts[10];
		int32_t lineEnds[10];

		int32_t maxLines = str.length() / (int32_t)maxNum + 1;
		if (maxLines > 10)
			maxLines = 10;

		int32_t lineNum = CBreakLine::WrapParagraph(str,
													CResourceBundleManage::Instance()->GetLocale(),
													(int32_t)maxNum,
													lineStarts,
													lineEnds,
													maxLines);
		char lineBuf[200];
		icu::UnicodeString string(str, lineStarts[0], lineEnds[0] - lineStarts[0]);
		string.extract(0, string.length(), lineBuf, sizeof(lineBuf) - 1, nullptr);
		lineBuf[sizeof(lineBuf) - 1] = 0;
		retStr = lineBuf;

		if (retStr.length() > 0) {
			if ((*--retStr.end()) == '#') {
				retStr = retStr.substr(0, retStr.length() - 1);
			}
		}
		if (retStr.length() > 1) {
			if ((*--retStr.end()) == '#') {
				retStr = retStr.substr(0, retStr.length() - 1);
			} else if ((*(retStr.end() - 2)) == '#') {
				retStr = retStr.substr(0, retStr.length() - 2);
			}
		}
		text = text.substr(retStr.length(), text.length() - retStr.length());
	} catch (...) {
		retStr = "";
	}

	return retStr;
}

//------------------------------------------------------------------------
//	瑙ｆ瀽绗﹁浆鎹?浠?璧?渚?
//	#00 -> </00>
//	#01 -> </01>
//	#02 -> </02>
//	...鍒皀MaxCount-1
//------------------------------------------------------------------------
void ChangeParseSymbol(std::string& text, int nMaxCount) {
	for (int i = 0; i < nMaxCount; i++) {
		char szSrc[100];
		char szRpl[100];
		//sprintf(szSrc,"#%.2d",i);
		_snprintf_s(szSrc, sizeof(szSrc), _TRUNCATE, "#%.2d", i);
		//sprintf(szRpl,"</%.2d>",i);
		_snprintf_s(szRpl, sizeof(szRpl), _TRUNCATE, "</%.2d>", i);

		size_t nPos = text.find(szSrc);

		while (nPos != std::string::npos) {
			text.replace(nPos, strlen(szSrc), szRpl);
			nPos = text.find(szSrc, nPos + strlen(szRpl));
		}
	}
}

// 灏咺nBuf涓殑瀛楃涓叉寜nWidth鎻掓斁鍥炶溅鏀惧湪OutBuf涓?
int StringNewLine(char* pOutBuf, int nOutBufSize, unsigned int nWidth, const char* pInBuf, unsigned int nInLen) {
	// modify by ning.yan 20081007 閬撳叿璇存槑鏂瓧澶勭悊 begin
	if (nWidth == 0 || nInLen == 0) {
		if (pOutBuf)
			pOutBuf[0] = '\0';
		return 0;
	}
	icu::UnicodeString str = pInBuf;
	icu::UnicodeString strDest;
	int32_t lineStarts[10];
	int32_t lineEnds[10];
	int maxNum = nWidth;
	const char* resName = CResourceBundleManage::Instance()->GetLocale().getName();
	if (strcmp(resName, "zh_Hans_CN") == 0 || strcmp(resName, "zh_Hant_TW") == 0 || strcmp(resName, "zh_Hant_HK") == 0)
		maxNum = maxNum / 2;

	//閬垮厤闄ら浂閿欒 -Waiting Add 2009-03-24
	if (maxNum == 0) {
		if (pOutBuf)
			pOutBuf[0] = '\0';
		return 0;
	}

	int32_t maxLines = str.length() / maxNum + 1;

	int32_t lineNum = CBreakLine::WrapParagraph(str,
												CResourceBundleManage::Instance()->GetLocale(),
												maxNum,
												lineStarts,
												lineEnds,
												maxLines);
	for (int row = 0; row < lineNum; row++) {
		icu::UnicodeString string(str, lineStarts[row], lineEnds[row] - lineStarts[row]);
		strDest += string + '\n'; //鍔犲叆鎹㈣绗?
	}

	//鍘绘帀澶氫綑鐨勫眬閮ㄥ彉閲忋€佸噺灏戜竴娆trcpy -Waiting Add 2009-05-25
	if (pOutBuf) {
		int ret = strDest.extract(0, strDest.length(), pOutBuf, nOutBufSize, nullptr);
		pOutBuf[ret] = '\0';
		return ret;
	}
	return 0;
}

const char* StringSplitNum(long nNumber, int nCount, char cSplit) {
	static char szBuf[256] = {0};
	static char szTmp[256] = {0};

	//sprintf( szBuf, "%d", nNumber );
	_snprintf_s(szBuf, sizeof(szBuf), _TRUNCATE, "%d", nNumber);
	//strcpy( szTmp, szBuf );
	strncpy_s(szTmp, sizeof(szTmp), szBuf, _TRUNCATE);

	int nLen = (int)strlen(szBuf);
	int nLoop = (nLen - 1) / nCount;
	if (nLoop < 0)
		return szBuf;

	if (nLoop + nLen > sizeof(szBuf))
		return szBuf;

	szBuf[nLen + nLoop] = '\0';

	int nStart = 0;
	int nTarget = 0;
	for (int i = 0; i < nLoop; i++) {
		nStart = nLen - nCount * (i + 1);
		nTarget = nStart + nLoop - i;
		memcpy(&szBuf[nTarget], &szTmp[nStart], nCount);
		szBuf[nTarget - 1] = cSplit;
	}
	return szBuf;
}
