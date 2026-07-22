#include "StdAfx.h"
#include "uiitem.h"
#include "uitextparse.h"

using namespace GUI;
//---------------------------------------------------------------------------
// class CItem
//---------------------------------------------------------------------------
void CItem::Render(int x, int y) {
	CGuiFont::s_Font.Render((char*)_str.c_str(), x, y, _dwColor);
}

//---------------------------------------------------------------------------
// class CItemBar
//---------------------------------------------------------------------------
CGuiPic* CItemBar::_pBar = new CGuiPic(nullptr, 2);

CItemBar::CItemBar() : CItem() {}

void CItemBar::LoadImage(const char* file, int w, int h, int tx, int ty) {
	_pBar->LoadAllImage(file, w, h, tx, ty);
}

void CItemBar::Render(int x, int y) {
	_pBar->SetScaleW(1, _fScale);
	_pBar->RenderAll(x, y);

	CGuiFont::s_Font.Render((char*)_str.c_str(), x, y, _dwColor);
}

void CColorItem::SetString(const char* str) {
	_str = str;
	ParseScript(_str.c_str(), 0, 0xFF000000);
	_str = "";

	ITEM_TEXT_ARRAY::iterator pos;
	for (pos = m_TextArray.begin(); pos != m_TextArray.end(); pos++) {
		_str += (*pos).strData;
	}

	CGuiFont::s_Font.GetSize(_str.c_str(), _nWidth, _nHeight);
}

void CColorItem::Render(int x, int y) {
	USHORT sWidth = CGuiFont::s_Font.GetHeight("a") / 2;

	ITEM_TEXT_ARRAY::iterator pos;
	for (pos = m_TextArray.begin(); pos != m_TextArray.end(); pos++) {
		int v = (*pos).sxPos;
		int xPos = (*pos).sxPos * sWidth + x;
		CGuiFont::s_Font.Render((*pos).strData.c_str(), xPos, y, (*pos).dwColor);
	}
}

void CColorItem::ParseScript(const char szScript[], USHORT sStartCom, DWORD dwDefColor) {
	m_TextArray.clear();
	if (szScript == nullptr)
		return;

	USHORT sCom = sStartCom;
	USHORT sxPos = sCom;
	DWORD dwColor = dwDefColor;
	char szData[3];
	memset(szData, 0, 3);
	const char* pszTemp = szScript;
	std::string strDesp = "";
	std::string strCaption = "";
	while (pszTemp[0]) {
		szData[0] = 0;
		szData[1] = 0;

		if (pszTemp[0] == '<') {
			if (!strDesp.empty()) {
				// 添加文字显示数据到列表中
				ITEM_TEXT_DATA Data;
				Data.dwColor = dwColor;
				Data.sxPos = sxPos;
				Data.strData = strDesp;
				m_TextArray.push_back(Data);
				strDesp = "";
				sxPos = sCom;
			}

			pszTemp++;
			if (pszTemp[0] == 'p') {
				dwColor = 0xFFFF00FF; // 紫色
				if (pszTemp[1] == '>') {
					pszTemp++;
					continue;
				}
			} else if (pszTemp[0] == 'j') {
				dwColor = 0xFFFE0000;
				if (pszTemp[1] == '>') {
					pszTemp += 2;
					continue;
				}
			} else if (pszTemp[0] == 'r') {
				dwColor = 0xFFFF0000;
				if (pszTemp[1] == '>') {
					pszTemp++;
					continue;
				}
			} else if (pszTemp[0] == 'y') {
				dwColor = 0xFFFFFF00;
				if (pszTemp[1] == '>') {
					pszTemp++;
					continue;
				}
			} else if (pszTemp[0] == 'b') {
				dwColor = 0xFF0000FF;
				if (pszTemp[1] == '>') {
					pszTemp++;
					continue;
				}
			} else if (pszTemp[0] == 'g') {
				dwColor = 0xFF00C800;
				if (pszTemp[1] == '>') {
					pszTemp++;
					continue;
				}
			} else {
				// 未知类型报错！
				continue;
			}

			// 使用新的字符记录
			pszTemp++;
		} else if (pszTemp[0] == '>') {
			// 添加文字显示数据到列表中
			ITEM_TEXT_DATA Data;
			Data.dwColor = dwColor;
			Data.sxPos = sxPos;
			Data.strData = strDesp;
			m_TextArray.push_back(Data);
			strDesp = "";
			sxPos = sCom;

			dwColor = dwDefColor;
			pszTemp++;
			continue;
		}

		// 判断是否一个GBK
		BOOL bFlag1 = 0x81 <= (BYTE)pszTemp[0] && (BYTE)pszTemp[0] <= 0xFE;
		BOOL bFlag2 = (0x40 <= (BYTE)pszTemp[1] && (BYTE)pszTemp[1] <= 0x7E) || (0x7E <= (BYTE)pszTemp[1] && (BYTE)pszTemp[1] <= 0xFE);
		if (bFlag1 && bFlag2) {
			szData[0] = pszTemp[0];
			szData[1] = pszTemp[1];
			strDesp += szData;

			// 移动列和指向下一个字符
			sCom += 2;
			pszTemp += 2;
		} else {
			szData[0] = pszTemp[0];
			strDesp += szData;

			// 移动列和指向下一个字符
			sCom++;
			pszTemp++;
		}
	}
}

//---------------------------------------------------------------------------
// class CItemRow
//---------------------------------------------------------------------------
CItemObj CItemRow::_NullObj;

CItemRow::CItemRow(unsigned int max)
	: _nMax(max) {
	_pTag = nullptr;
	_Init();
}

CItemRow::CItemRow(const CItemRow& rhs) {
	_nMax = rhs._nMax;

	_Copy(rhs);
}

CItemRow& CItemRow::operator=(const CItemRow& rhs) {
	_Clear();

	_nMax = rhs._nMax;

	_Copy(rhs);
	return *this;
}

void CItemRow::_Copy(const CItemRow& rhs) {
	_pTag = rhs._pTag;
	_items = new CItemObj*[_nMax];
	for (unsigned int i = 0; i < _nMax; i++) {
		if (rhs._items[i] != _GetNullItem())
			_items[i] = rhs._items[i]->Clone();
		else
			_items[i] = _GetNullItem();
	}
}

void CItemRow::_Clear() {
	for (unsigned int i = 0; i < _nMax; i++) {
		if (_items[i] != _GetNullItem()) {
			//delete _items[i];
			SAFE_DELETE(_items[i]); // UI当机处理
		}
	}

	//delete [] _items;
	SAFE_DELETE_ARRAY(_items); // UI当机处理
	_pTag = nullptr;
}

void CItemRow::_Init() {
	_items = new CItemObj*[_nMax];
	for (unsigned int i = 0; i < _nMax; i++)
		_items[i] = _GetNullItem();
}

CItemRow::~CItemRow() {
	_Clear();
}

void CItemRow::SetColor(DWORD c) {
	for (unsigned int i = 0; i < _nMax; i++)
		_items[i]->SetColor(c);
}
//---------------------------------------------------------------------------
// class CItemEx
//---------------------------------------------------------------------------
void CItemEx::ProcessString(size_t player_name_string_length) // 参数：角色名称的长度 //NOTE: player_name_string_length is assumed to be under "CITEMEX_MAX_LINE_LENGTH"
{
	_nLineNum = -1;
	size_t current_char = 0;
	if (_str.find("#") != std::string::npos) { //Parse emoticons (somewhat)
		_bParseText = true;
	}
	while ((++_nLineNum < CITEMEX_MAX_LINES) && (_str.length() > current_char)) {
		size_t max_width = CITEMEX_MAX_LINE_LENGTH;
		if ((_nLineNum == 0) && (_str.length() > max_width)) { //Place text on second line if text doesn't fit
			max_width = player_name_string_length;
			_bMultiLine = true;
		}
		_strLine[_nLineNum] = _str.substr(current_char, max_width);
		current_char += max_width;
	}

	CGuiFont::s_Font.GetSize((char*)(
		(_bMultiLine) ? _strLine[1] : _strLine[0]).c_str(),
		(_bMultiLine) ? _nMaxWidth : _nWidth,
		_nHeight);
}

void CItemEx::SetHasHeadText(DWORD headLen, DWORD headColor) {
	_HeadLen = headLen;
	_HeadColor = headColor;
}

DWORD CItemEx::GetHeadLength() const {
	return _HeadLen;
}

DWORD CItemEx::GetHeadColor() const {
	return _HeadColor;
}

void CItemEx::Render(int x, int y) {
	int sy = 0;
	if (m_Allign == eAlignCenter) {
		sy = (_nHeight - CGuiFont::s_Font.GetHeight(RES_STRING(CL_LANGUAGE_MATCH_623))) / 2;
	} else if (m_Allign == eAlignBottom) {
		sy = _nHeight - CGuiFont::s_Font.GetHeight(RES_STRING(CL_LANGUAGE_MATCH_623));
	}
	if (_bParseText) {
		if (!_bMultiLine) {
			if (GetHeadLength() && GetHeadLength() <= _str.length()) {
				std::string strHead = _str.substr(0, GetHeadLength());
				std::string strTail = _str.substr(GetHeadLength(), _str.length() - GetHeadLength());
				g_TextParse.Render((char*)strHead.c_str(), x, y, GetHeadColor(), m_Allign, _nHeight);
				g_TextParse.Render((char*)strTail.c_str(), x + CGuiFont::s_Font.GetWidth(strHead.c_str()), y, _dwColor, m_Allign, _nHeight);
			} else {
				g_TextParse.Render((char*)_str.c_str(), x, y, _dwColor, m_Allign, _nHeight);
			}
		} else {
			g_TextParse.Render((char*)_strLine[0].c_str(), x, y, _dwColor, m_Allign, _nHeight);
			g_TextParse.Render((char*)_strLine[1].c_str(), x, y + _nHeight, _dwColor, m_Allign, _nHeight);
			g_TextParse.Render((char*)_strLine[2].c_str(), x, y + 2 * _nHeight, _dwColor, m_Allign, _nHeight);
		}
	} else {
		if (!_bMultiLine) {
			CGuiFont::s_Font.Render((char*)_str.c_str(), x, y + sy, _dwColor);
		} else {
			CGuiFont::s_Font.Render((char*)_strLine[0].c_str(), x, y + sy, _dwColor);
			CGuiFont::s_Font.Render((char*)_strLine[1].c_str(), x, y + _nHeight + sy, _dwColor);
			CGuiFont::s_Font.Render((char*)_strLine[2].c_str(), x, y + 2 * _nHeight + sy, _dwColor);
		}
	}
}

void CItemEx::RenderEx(int x, int y, int height, float scale) {
	if (_bParseText) {
		y = y + 5;
		if (!_bMultiLine) {
			g_TextParse.RenderEx((char*)_str.c_str(), x, y, _dwColor, scale);
			int p = (int)(_str.find(":"));
			if (p != -1)
				CGuiFont::s_Font.RenderScale((char*)_str.substr(0, p).c_str(), x, y, 0xFFFFFFFF, scale);
		} else {
			g_TextParse.RenderEx((char*)_strLine[0].c_str(), x, y, 0xFFFFFFFF, scale);
			g_TextParse.RenderEx((char*)_strLine[1].c_str(), x, y + height, _dwColor, scale);
			g_TextParse.RenderEx((char*)_strLine[2].c_str(), x, y + 2 * height, _dwColor, scale);
		}
	} else {
		y = y + 3;
		if (!_bMultiLine) {
			CGuiFont::s_Font.RenderScale((char*)_str.c_str(), x, y, _dwColor, scale);
			int p = (int)(_str.find(":"));
			if (p != -1) {
				CGuiFont::s_Font.RenderScale((char*)_str.substr(0, p).c_str(), x, y, 0xFFFFFFFF, scale);
			}
		} else {
			CGuiFont::s_Font.RenderScale((char*)_strLine[0].c_str(), x, y, 0xFFFFFFFF, scale);
			CGuiFont::s_Font.RenderScale((char*)_strLine[1].c_str(), x, y + height + 3, _dwColor, scale);
			CGuiFont::s_Font.RenderScale((char*)_strLine[2].c_str(), x, y + 2 * height + 6, _dwColor, scale);
		}
	}
}
