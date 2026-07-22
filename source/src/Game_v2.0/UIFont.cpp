#include "StdAfx.h"
#include "uifont.h"

using namespace GUI;
//---------------------------------------------------------------------------
// class CGuiFont
//---------------------------------------------------------------------------
CGuiFont CGuiFont::s_Font;

int CGuiFont::CreateFont(char* font, int size800, int size1024, DWORD dwStyle) {
	stFont ft;
	ft.strFont = font;
	ft.size800 = size800;
	ft.size1024 = size1024;
	ft.dwStyle = dwStyle;

	_stfonts.push_back(ft);
	return (int)_stfonts.size() - 1;
}

void CGuiFont::FrameRender(const char* str, int x, int y) {
	static int w, h, offx;

	GetSize(str, w, h);
	offx = GetRender().GetScreenWidth() - x - w - 10;
	if (offx < 0) {
		x += offx;
	}
	GetRender().FillFrame(x - 5, y - 3, x + w + 10, y + h + 5, 0x90000000);
	CGuiFont::s_Font.Render(str, x - 1, y - 1, 0xffffffff);
}

void CGuiFont::TipRender(const char* str, int x, int y) {
	static int w, h;

	GetSize(str, w, h);
	x -= w / 2;
	GetRender().FillFrame(x - 2, y - 1, x + w + 2, y + h + 1, 0x90000000);
	CGuiFont::s_Font.Render(str, x - 1, y - 1, 0xffffffff);
}

//---------------------------------------------------------------------------
// class CGuiFont
//---------------------------------------------------------------------------
void CTextHint::Render() {
	if (_hint.empty()) {
		return;
	}
	static RECT rt;
	rt.left = _nStartX - 5;
	rt.top = _nStartY - 7;
	rt.right = _nStartX + _nHintW + 5;
	rt.bottom = _nStartY + _nHintH + 2;
	GetRender().FillFrame(rt.left, rt.top, rt.right, rt.bottom, _dwBkgColor);

	int yy = _nStartY;

	hints::iterator it = _hint.begin();
	if (_IsHeadShadow) {
		DWORD color = (*it)->color;
		color = (0xff000000 & color) | ~color;
		CGuiFont::s_Font.BRender((*it)->font, (*it)->hint.c_str(), _nStartX + (*it)->offx, yy, (*it)->color, color);
		yy += (*it)->height;
		++it;
	}
	for (; it != _hint.end(); ++it) {
		CGuiFont::s_Font.Render((*it)->font, (*it)->hint.c_str(), _nStartX + (*it)->offx, yy, (*it)->color);
		yy += (*it)->height;
	}

	if (_IsShowFrame) {
		extern bool RenderHintFrame(const RECT* rc, DWORD color);
		RenderHintFrame(&rt, D3DCOLOR_ARGB(255, 123, 218, 229));
		rt.left--;
		rt.top--;
		rt.right++;
		rt.bottom++;
		RenderHintFrame(&rt, COLOR_BLACK);
	}
}

void CTextHint::ReadyForHint(int x, int y) {
	if (_hint.empty()) {
		return;
	}

	// 计算开始显示位置，以及背景框等
	_nStartX = x + 40;
	_nStartY = y;
	_nHintW = 0;
	_nHintH = 0;
	static int w, h;

	for (auto& it : _hint) {
		CGuiFont::s_Font.GetSize(it->font, it->hint.c_str(), w, h);
		if (_nHintW < w) {
			_nHintW = w;
		}
		it->width = w;
		it->height += h;
		_nHintH += it->height;
	}

	if (_eStyle == enumFixWidth) {
		_nHintW = _nFixWidth;
	}

	if (!_hint.empty() && _IsCenter) {
		for (auto& it : _hint) {
			it->offx = (_nHintW - it->width) / 2;
		}
	}

	// 检查hint是否越界（右，下）
	int off = GetRender().GetScreenWidth() - _nStartX - _nHintW - 10;
	if (off < 0) {
		_nStartX += off;
	}

	off = GetRender().GetScreenHeight() - _nStartY - _nHintH - 10;
	if (off < 0) {
		_nStartY += off;
	}
}

//Add by sunny.sun 20080912为区分GM公告和道具说明显示
//Begin
void CTextHint::ReadyForHintGM(int x, int y) {
	if (_hint.empty()) {
		return;
	}

	// 计算开始显示位置，以及背景框等
	_nStartX = x + 40;
	_nStartY = y;
	_nHintW = 0;
	_nHintH = 0;
	static int w, h;

	for (auto& it : _hint) {
		CGuiFont::s_Font.GetSize(it->font, it->hint.c_str(), w, h);
		if (_nHintW < w) {
			_nHintW = w;
		}
		it->width = w;
		it->height += h;
		_nHintH += it->height;
	}

	if (_eStyle == enumFixWidth) {
		if (GetRender().GetScreenWidth() == 800) {
			_nHintW = _nFixWidth - 50;
		} else if (GetRender().GetScreenWidth() == 1024) {
			_nHintW = _nFixWidth + 170;
		} else {
			_nHintW = _nFixWidth + 420;
		}
	}

	if (!_hint.empty() && _IsCenter) {
		for (auto& it : _hint) {
			it->offx = (_nHintW - it->width) / 2;
		}
	}

	// 检查hint是否越界（右，下）
	int off = GetRender().GetScreenWidth() - _nStartX - _nHintW - 10;
	if (off < 0) {
		_nStartX += off;
	}

	off = GetRender().GetScreenHeight() - _nStartY - _nHintH - 10;
	if (off < 0) {
		_nStartY += off;
	}
}
//End

void CTextHint::Clear() {
	for (auto& it : _hint) {
		SAFE_DELETE(it); // UI当机处理
	}
	_hint.clear();
}

int CTextHint::WriteText(const char* file) {
	std::ofstream out(file, std::ios::app);

	int nLines = 0;
	for (const auto& it : _hint) {
		nLines++;
		out << it->hint << std::endl;
	}
	if (nLines > 0)
		out << "\n======================================\n"
			<< std::endl;
	return nLines;
}

void CTextScrollHint::Render() {
	if (_hint.empty()) {
		return;
	}
	try {
		int scrollneedspace = 0;
		static RECT rt;
		int yy = _nStartY;
		if (GetRender().GetScreenWidth() == 1024) {
			rt.left = _nStartX - 5;
			rt.top = _nStartY - 7;
			rt.right = _nStartX + _nHintW + 160;
			rt.bottom = _nStartY + _nHintH + 2;
			scrollneedspace = SCROLLNEEDSPACE + 39;
		} else {
			rt.left = _nStartX - 5;
			rt.top = _nStartY - 7;
			rt.right = _nStartX + _nHintW - 55;
			rt.bottom = _nStartY + _nHintH + 2;
			scrollneedspace = SCROLLNEEDSPACE + 3;
		}

		if (RenderScrollNum < SetScrollNum) {
			hints::iterator it = _hint.begin();
			GetRender().FillFrame(rt.left, rt.top, rt.right, rt.bottom, _dwBkgColor);
			SpaceLength = "";
			if (num == 0) {
				SpaceLength = "";
				for (int n = 0; n < scrollneedspace; n++) {
					SpaceLength = SpaceLength + " ";
				}
				CopyHints = SpaceLength + (*it)->hint;
				(*it)->hint = SpaceLength;
				m = 0;
			}
			if ((index % 10) == 0) {
				(*it)->hint = CopyHints;

				if (m == CopyHints.length()) {
					m = 0;
					(*it)->hint = "";
					RenderScrollNum++;
				} else if (m > (int)(CopyHints.length() - scrollneedspace - 1)) {
					char c = (*it)->hint.at(m);
					if ((::byte)c >= (::byte)0x80) {
						if (m >= (int)CopyHints.length())
							m = 0;
						(*it)->hint = (*it)->hint.erase(0, m);
						m += 1;
					} else {
						(*it)->hint = (*it)->hint.substr(m, CopyHints.length() - m);
					}
				} else {
					char c = (*it)->hint.at(m);
					char d = (*it)->hint.at(m + scrollneedspace - 1);
					if ((::byte)c >= (::byte)0x80 || (::byte)d >= (::byte)0x80) {
						(*it)->hint = (*it)->hint.substr(m, scrollneedspace - 1);
						m += 1;
					} else {
						(*it)->hint = (*it)->hint.substr(m, scrollneedspace - 1);
					}
				}
				++m;
				index = 0;
			}
			num++;
			CGuiFont::s_Font.Render((*it)->font, (*it)->hint.c_str(), _nStartX + (*it)->offx, yy, (*it)->color);
			index++;

			if (_IsShowFrame) {
				extern bool RenderHintFrame(const RECT* rc, DWORD color);
				RenderHintFrame(&rt, D3DCOLOR_ARGB(255, 123, 218, 229));
				rt.left--;
				rt.top--;
				rt.right++;
				rt.bottom++;
				RenderHintFrame(&rt, COLOR_BLACK);
			}
		}
	} catch (...) {
		try {
			for (stHint* st : _hint) {
				LG("sunny.txt", "hint error !%s\n", st->hint.c_str());
			}
			_hint.clear();
		} catch (...) {
		}
	}
}
void CTextScrollHint::ReadyForHint(int x, int y, int SetNum) {
	if (_hint.empty()) {
		return;
	}

	// "Calculate the starting display position, as well as the background frame, etc."
	_nStartX = x + 40;
	_nStartY = y;
	_nHintW = 0;
	_nHintH = 0;
	static int w, h;

	for (const auto& it : _hint) {
		CGuiFont::s_Font.GetSize(it->font, it->hint.c_str(), w, h);
		if (_nHintW < w) {
			_nHintW = w;
		}
		it->width = w;
		it->height += h;
		_nHintH += it->height;
	}

	if (_eStyle == enumFixWidth) {
		_nHintW = _nFixWidth;
	}

	if (!_hint.empty() && _IsCenter) {
		for (auto& it : _hint) {
			it->offx = (_nHintW - it->width) / 2;
		}
	}

	// "Check if the hint is out of bounds (right, bottom)"
	int off = GetRender().GetScreenWidth() - _nStartX - _nHintW - 10;
	if (off < 0) {
		_nStartX += off;
	}

	off = GetRender().GetScreenHeight() - _nStartY - _nHintH - 10;
	if (off < 0) {
		_nStartY += off;
	}
	num = 0;
	RenderScrollNum = 0;
	SetScrollNum = SetNum;
	CopyHints.clear();
}

void CTextScrollHint::Clear() {
	for (auto& it : _hint) {
		SAFE_DELETE(it); 
	}
	_hint.clear();
}
