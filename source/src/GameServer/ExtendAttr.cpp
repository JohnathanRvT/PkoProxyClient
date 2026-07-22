#include "stdafx.h"
#include <ctype.h>
#include "Character.h"
#include "ExtendAttr.h"

const int EXTEND_STORE_BEGIN = ATTR_BCOUNFIR;
const int EXTEND_STORE_END = ATTR_BCOUNCOMA;

bool Strin2ChaExtendAttr(CCharacter* pCCha, std::string& strAttr) {
	if (!pCCha || strAttr.length() == 0)
		return false;

	int attrStart = EXTEND_STORE_BEGIN;
	int attrEnd = EXTEND_STORE_END;

	int attrIndex = 0;
	int value = 0;
	bool bNeg = false;
	std::string::iterator& iter = strAttr.begin();

	while (attrIndex <= (attrEnd - attrStart + 1) && iter != strAttr.end()) {
		char ch = *iter;

		if (isdigit(ch)) {
			value = value * 10 + ch - '0';
		} else if (ch == ',') {

			if (!attrIndex)
				pCCha->setAttr(ATTR_DISTTEMP, bNeg ? -value : value);
			else
				pCCha->setAttr(attrIndex - 1 + EXTEND_STORE_BEGIN, bNeg ? -value : value);
			++attrIndex;
			value = 0;
			bNeg = false;
		} else if (ch == '-') {
			if (!bNeg)
				bNeg = true;
			else
				return false;
		} else
			return false;
		++iter;
	}

	if (!attrIndex)
		pCCha->setAttr(ATTR_DISTTEMP, bNeg ? -value : value);
	else if (attrIndex <= (attrEnd - attrStart + 1))
		pCCha->setAttr(attrIndex - 1 + EXTEND_STORE_BEGIN, bNeg ? -value : value);

	return true;
}

char* ChaExtendAttr2String(CCharacter* pCCha, char* szAttrBuf, int nLen) {
	if (!pCCha || !szAttrBuf || (nLen <= 0))
		return NULL;

	char temp[20];
	int pos = 0;
	int left = nLen;
	char* pt = szAttrBuf;
	char* pt2;
	int attrIndex = ATTR_DISTTEMP;
	int value = pCCha->getAttr(ATTR_DISTTEMP);

	if (value) {
		itoa(value, temp, 10);
		pt2 = temp;
		while (*pt2 && left) {
			*pt = *pt2;
			++pt;
			++pt2;
			--left;
		}

		if (!left)
			return szAttrBuf;
	}

	for (int attrIndex = EXTEND_STORE_BEGIN; attrIndex <= EXTEND_STORE_END; attrIndex++) {
		*pt = ',';
		++pt;
		--left;

		value = pCCha->getAttr(attrIndex);
		if (value) {
			itoa(value, temp, 10);
			pt2 = temp;
			while (*pt2 && left) {
				*pt = *pt2;
				++pt;
				++pt2;
				--left;
			}

			if (!left)
				return szAttrBuf;
		}
	}
	*pt = 0;

	return szAttrBuf;
}
