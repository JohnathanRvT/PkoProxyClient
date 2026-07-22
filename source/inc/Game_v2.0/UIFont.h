//----------------------------------------------------------------------
// 名称:用于隔离字体显示底层
// 作者:lh 2004-07-15
// 最后修改日期:2004-10-20
//----------------------------------------------------------------------
#pragma once
#include "UIRender.h"

#include "MPFont.h"

namespace GUI {
const DWORD COLOR_BLACK = 0xff000000;
const DWORD COLOR_WHITE = 0xffffffff;
const DWORD COLOR_RED = 0xffff0000;
const DWORD COLOR_GREEN = 0xff00ff00;
const DWORD COLOR_BLUE = 0xff0000ff;
const DWORD COLOR_GRAY = 0xff808080;
const DWORD COLOR_CHAR_NAME = COLOR_WHITE;
const DWORD COLOR_SHIP_NAME = 0xff03fdbd;
const DWORD COLOR_SHIP_SEPARATOR = 0xff16074f;
#define SCROLLNEEDSPACE 60

enum ALLIGN {
	eAlignTop,
	eAlignCenter,
	eAlignBottom
};

class CGuiFont {
public:
	CGuiFont() = default;
	~CGuiFont();

	// 参数nStyle: 0x0001-粗体,0x0002-斜体,0x0004-带下划线
	int CreateFont(char* font, int size800, int size1024, DWORD dwStyle = 0);
	bool Init() { return !_stfonts.empty(); }
	bool Clear();

	void SetScreen(int nScrWidth, int nScrHeight);
	void SetIndex(unsigned int n);
	void Render(const char* str, int x, int y, DWORD color);				   // 正常显示
	void RenderScale(const char* str, int x, int y, DWORD color, float scale); //比例显示
	void BRender(const char* str, int x, int y, DWORD color, DWORD shadow);	// 阴影显示
	void Render3d(const char* str, D3DXVECTOR3& pos, DWORD color = COLOR_RED);

	void FrameRender(const char* str, int x, int y);
	void TipRender(const char* str, int x, int y);

	int GetWidth(const char*) const;
	int GetHeight(const char*) const;
	void GetSize(const char* str, int& w, int& h) const; // 已经自动进行了屏幕转换
	bool GetSize(unsigned int dwIndex, const char* str, int& w, int& h) const;

	void Begin();
	void Draw(const char* str, int x, int y, DWORD color);
	void End();

public:
	CMPFont* GetFont(unsigned int index = 0) const;
	void Render(unsigned int nIndex, const char* str, int x, int y, DWORD color);
	void BRender(unsigned int nIndex, const char* str, int x, int y, DWORD color, DWORD shadow); // 阴影显示

public:
	static CGuiFont s_Font;

private:
	typedef std::vector<CMPFont*> fonts;
	fonts _fonts;
	CMPFont* _pFont{nullptr};
	unsigned int _nMaxFont;

	struct stFont {
		std::string strFont;
		DWORD dwStyle;
		int size800;
		int size1024;
	};
	typedef std::vector<stFont> stfont;
	stfont _stfonts;

private:
	mutable SIZE _size{};
};

// 显示可居中的hint list
// 使用方法:
// 1.Clear清除原有的文字列表
// 2.PushHint,AddHintHeight,填加新列表
// 3.ReadyForHint( x, y ) - 准备在那儿显示
// 4.Render - 显示hint
class CTextHint {
public:
	CTextHint() = default;
	void Clear();
	void ReadyForHint(int x, int y);
	void ReadyForHintGM(int x, int y); //Add by sunny.sun 20080912
	void Render();

	void PushHint(const char* str, DWORD color = COLOR_WHITE, int height = 5, int font = 0); // height:扣除字高后的高度
	void AddHintHeight(int height = 6);														 // 填加一个空行

	int WriteText(const char* file); // 将当前hint写入文本，返回写入的行数

public:
	struct stHint {
		stHint(const char* str, DWORD c, int h, int f) : hint(str), color(c), height(h), offx(0), font(f), width(0) {}

		std::string hint;
		DWORD color;
		int height;
		int offx;
		int width;
		int font;
	};

	// 设置整个Hint是否居中
	void SetHintIsCenter(bool v) { _IsCenter = v; }
	bool GetHintIsCenter() { return _IsCenter; }

	void SetIsHeadShadow(bool v) { _IsHeadShadow = v; }
	bool GetIsHeadShadow() { return _IsHeadShadow; }

	stHint* GetHint(int v) { return _hint[v]; }
	int GetCount() { return (int)_hint.size(); }

	void SetFixWidth(int w);
	int GetFixWidth() { return _nFixWidth; }

	void SetBkgColor(DWORD color) { _dwBkgColor = color; }
	DWORD GetBkgColor() { return _dwBkgColor; }

	void SetIsShowFrame(bool v) { _IsShowFrame = v; }
	bool GetIsShowFrame() { return _IsShowFrame; }

private:
	using hints = std::vector<stHint*>;
	hints _hint;

private:
	enum eStyle {
		enumFontSize,
		enumFixWidth,
	};
	eStyle _eStyle{enumFontSize};
	int _nFixWidth{0};

	bool _IsCenter{true};
	bool _IsHeadShadow{true};
	bool _IsShowFrame{true};

	int _nHintW{0}, _nHintH{0};
	int _nStartX{0}, _nStartY{0};
	DWORD _dwBkgColor{0x90000000};
};

//Add by sunny.sun20080804
//Begin
class CTextScrollHint {
public:
	CTextScrollHint() = default;
	void Clear();
	void ReadyForHint(int x, int y, int SetNum);
	void Render();

	void PushHint(const char* str, DWORD color = COLOR_WHITE, int height = 5, int font = 0); // height:扣除字高后的高度
public:
	struct stHint {
		stHint(const char* str, DWORD c, int h, int f) : hint(str), color(c), height(h), offx(0), font(f), width(0) {}

		std::string hint;
		DWORD color;
		int height;
		int offx;
		int width;
		int font;
	};

	void SetFixWidth(int w);
	void SetBkgColor(DWORD color) { _dwBkgColor = color; }

private:
	using hints = std::vector<stHint*>;
	hints _hint;
	int index{0};
	std::string CopyHints;
	std::string SpaceLength;
	int num{0};
	int RenderScrollNum;
	int SetScrollNum = 1;
	int m{0};

private:
	enum eStyle {
		enumFontSize,
		enumFixWidth,
	};
	eStyle _eStyle{enumFontSize};
	int _nFixWidth{0};

	bool _IsCenter{false};
	bool _IsHeadShadow{true};
	bool _IsShowFrame{true};

	int _nHintW{0}, _nHintH{0};
	int _nStartX{0}, _nStartY{0};
	DWORD _dwBkgColor{0x90000000};
};
//End
inline void CGuiFont::Begin() {
	_pFont->Begin();
}
inline void CGuiFont::Draw(const char* str, int x, int y, DWORD color) {

	GetRender().DrawConvert(x, y);
	_pFont->Draw(const_cast<char*>(str), x, y, color);
}
inline void CGuiFont::End() {
	_pFont->End();
}

inline void CGuiFont::Render(unsigned int nIndex, const char* str, int x, int y, DWORD color) {
	if (nIndex < _nMaxFont) {
		GetRender().DrawConvert(x, y);
		_fonts[nIndex]->DrawText(const_cast<char*>(str), x, y, color, 1.0f);
	}
}

inline void CGuiFont::BRender(unsigned int nIndex, const char* str, int x, int y, DWORD color, DWORD shadow) {
	if (nIndex < _nMaxFont) {
		GetRender().DrawConvert(x, y);
		_fonts[nIndex]->DrawTextShadow(const_cast<char*>(str), x + 1, y + 1, x, y, shadow, color);
	}
}

inline void CGuiFont::BRender(const char* str, int x, int y, DWORD color, DWORD shadow) {
	GetRender().DrawConvert(x, y);
	_pFont->DrawTextShadow(const_cast<char*>(str), x + 1, y + 1, x, y, shadow, color);
}

inline void CGuiFont::Render(const char* str, int x, int y, DWORD color) {
	GetRender().DrawConvert(x, y);

	_pFont->DrawText(const_cast<char*>(str), x, y, color, 1.0f);
}

inline void CGuiFont::RenderScale(const char* str, int x, int y, DWORD color, float scale) {
	GetRender().DrawConvert(x, y);
	_pFont->DrawText(const_cast<char*>(str), x, y, color, scale);
}

inline void CGuiFont::Render3d(const char* str, D3DXVECTOR3& pos, DWORD color) {
	_pFont->Draw3DText(const_cast<char*>(str), pos, color);
}

inline int CGuiFont::GetWidth(const char* str) const {
	_pFont->GetTextSize(const_cast<char*>(str), &_size);
	return _size.cx;
}

inline int CGuiFont::GetHeight(const char* str) const {
	_pFont->GetTextSize(const_cast<char*>(str), &_size);
	return _size.cy;
}

inline void CGuiFont::GetSize(const char* str, int& w, int& h) const {
	_pFont->GetTextSize(const_cast<char*>(str), &_size);

	h = (int)(_size.cy);
	w = (int)(_size.cx);
}

inline bool CGuiFont::GetSize(unsigned int dwIndex, const char* str, int& w, int& h) const {
	if (dwIndex >= _nMaxFont) {
		return false;
	}

	_fonts[dwIndex]->GetTextSize(const_cast<char*>(str), &_size);

	h = (int)(_size.cy);
	w = (int)(_size.cx);
	return true;
}

inline void CGuiFont::SetIndex(unsigned int n) {
	if (n < _nMaxFont) {
		_pFont = _fonts[n];
	}
}

inline CMPFont* CGuiFont::GetFont(unsigned int index) const {
	if (index < _nMaxFont) {
		return _fonts[index];
	}
	return nullptr;
}

// 内联函数
inline void CTextHint::PushHint(const char* str, DWORD color, int height, int font) {
	if (str && strlen(str) > 0) {
		_hint.push_back(new stHint(str, color, height, font));
	}
}

inline void CTextHint::AddHintHeight(int height) {
	if (!_hint.empty()) {
		_hint.back()->height += height;
	}
}

inline void CTextHint::SetFixWidth(int w) {
	if (w <= 0) {
		_eStyle = enumFontSize;
	} else {
		_nFixWidth = w;
		_eStyle = enumFixWidth;
	}
}
// 内联函数//Add by sunny.sun20080804
//Begin
inline void CTextScrollHint::PushHint(const char* str, DWORD color, int height, int font) {
	if (str && strlen(str) > 0) {
		_hint.push_back(new stHint(str, color, height, font));
	}
}

inline void CTextScrollHint::SetFixWidth(int w) {
	if (w <= 0) {
		_eStyle = enumFontSize;
	} else {
		_nFixWidth = w;
		_eStyle = enumFixWidth;
	}
}
//End
} // namespace GUI
