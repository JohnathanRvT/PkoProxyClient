//----------------------------------------------------------------------
// 名称:静态文字
// 作者:lh 2004-07-19
// 最后修改日期:2004-10-09
//----------------------------------------------------------------------
#pragma once
#include "uiguidata.h"

namespace GUI {
// 固定列数的ItemObj
class CItemRow {
public:
	CItemRow(unsigned int max);
	CItemRow(const CItemRow& rhs);
	CItemRow& operator=(const CItemRow& rhs);
	~CItemRow();

	void SetBegin(CItemObj* p) { SetIndex(0, p); }
	CItemObj* GetBegin() { return _items[0]; }

	void SetIndex(unsigned int v, CItemObj* p);
	CItemObj* GetIndex(unsigned int v) { return _items[v]; }

	unsigned int GetMax() const { return _nMax; }
	void SetPointer(void* p) { _pTag = p; }
	void* GetPointer() { return _pTag; }

	void SetColor(DWORD c);

private:
	void _Copy(const CItemRow& rhs);
	void _Clear();
	void _Init();
	CItemObj* _GetNullItem() { return &_NullObj; }

private:
	static CItemObj _NullObj;

	CItemObj** _items;
	unsigned int _nMax;
	void* _pTag;
};

// 文字
class CItem : public CItemObj {
public:
	CItem() {}
	CItem(const char* str, DWORD c = COLOR_WHITE) : _str(str), _dwColor(c) { CGuiFont::s_Font.GetSize(str, _nWidth, _nHeight); }
	CItem(const CItem& rhs) : _str(rhs._str), _dwColor(rhs._dwColor), _nWidth(rhs._nWidth), _nHeight(rhs._nHeight) {}
	CItem& operator=(const CItem& rhs) {
		_str = rhs._str;
		_dwColor = rhs._dwColor;
		return *this;
	}
	ITEM_CLONE(CItem)

	virtual void Render(int x, int y) override;
	virtual int GetWidth() const override { return _nWidth; }
	virtual int GetHeight() const override { return _nHeight; }
	virtual void SetHeight(int n) { _nHeight = n; }
	virtual void SetWidth(int v) override { _nWidth = v; }

	virtual void SetString(const char* str) override {
		_str = str;
		CGuiFont::s_Font.GetSize(str, _nWidth, _nHeight);
	}
	virtual const char* GetString() const override { return _str.c_str(); }

	void SetColor(DWORD c) override { _dwColor = c; }
	DWORD GetColor() { return _dwColor; }

protected:
	std::string _str{0};

	DWORD _dwColor{COLOR_RED};
	int _nWidth{0};
	int _nHeight{0};
};

class CColorItem : public CItem {
public:
	CColorItem() = default;

	virtual void Render(int x, int y) override;
	virtual void SetString(const char* str) override;
	virtual const char* GetString() const override { return _str.c_str(); }

private:
	struct ITEM_TEXT_DATA {
		ITEM_TEXT_DATA() {
			sxPos = 0;
			dwColor = 0xFF000000;
			strData = "";
		}
		USHORT sxPos;
		DWORD dwColor;
		std::string strData;
	};

	// 分段显示不通颜色的文字信息
	using ITEM_TEXT_ARRAY = std::vector<ITEM_TEXT_DATA>;
	ITEM_TEXT_ARRAY m_TextArray;

	void ParseScript(const char szScript[], USHORT sStartCom, DWORD dwDefColor);
};

// 带进度条的文字
class CItemBar : public CItem {
public:
	CItemBar();

	virtual void Render(int x, int y) override;

	void SetScale(float f) {
		if (f >= 0.0f && f <= 1.0f)
			_fScale = f;
	}
	static void LoadImage(const char* file, int w, int h, int tx = 0, int ty = 0);

protected:
	static CGuiPic* _pBar;
	float _fScale{0.3f};
};

#define CITEMEX_MAX_LINES 3
#define CITEMEX_MAX_LINE_LENGTH 32

// 可换行文字
class CItemEx : public CItemObj {
public:
	CItemEx() = default;
	CItemEx(std::string str, DWORD c) : _str(str), _dwColor(c), _bParseText(false), _bMultiLine(false), _nLineNum(1), m_Allign(eAlignTop) { CGuiFont::s_Font.GetSize(str.c_str(), _nWidth, _nHeight); }
	CItemEx(const CItemEx& rhs) : _str(rhs._str), _dwColor(rhs._dwColor), _nWidth(rhs._nWidth), _nHeight(rhs._nHeight), _bParseText(rhs._bParseText), _bMultiLine(rhs._bMultiLine), _nLineNum(rhs._nLineNum), m_Allign(rhs.m_Allign) {}
	CItemEx& operator=(const CItemEx& rhs) {
		_str = rhs._str;
		_dwColor = rhs._dwColor;
		m_Allign = rhs.m_Allign;
		return *this;
	}
	~CItemEx() = default;
	ITEM_CLONE(CItemEx)

	virtual void Render(int x, int y) override;
	void RenderEx(int x, int y, int height, float scale);
	virtual void SetHeight(int n) { _nHeight = n; }

public:
	virtual int GetWidth() const override;
	virtual int GetHeight() const override { return _nHeight; }

	virtual void SetString(const char* str) override {
		_str = str;
		CGuiFont::s_Font.GetSize(str, _nWidth, _nHeight);
	}
	virtual const char* GetString() const override { return _str.c_str(); }

	virtual void SetColor(DWORD c) override { _dwColor = c; }
	DWORD GetColor() const { return _dwColor; }

	void SetIsParseText(bool v) { _bParseText = v; }
	bool GetIsParseText() const { return _bParseText; }

	void SetHasHeadText(DWORD headLen, DWORD headColor);
	DWORD GetHeadLength() const;
	DWORD GetHeadColor() const;

	void SetIsMultiLine(bool v) { _bMultiLine = v; }
	bool GetIsMultiLine() const { return _bMultiLine; }

	int GetLineNum() const { return _nLineNum; }

	void SetAlignment(ALLIGN allign) { m_Allign = allign; }
	void SetItemName(const char* name) { _strItemName = name; }
	const char* GetItemName() const { return _strItemName.c_str(); }

	void ProcessString(size_t length); // 参数：角色名称的长度

protected:
	std::string _str{};
	std::string _strItemName{};
	bool _bParseText{false}; // 是否需要解析图元
	bool _bMultiLine{false}; //是否多行显示
	std::string _strLine[CITEMEX_MAX_LINES]{}; //最多3行
	DWORD _HeadLen{0};
	DWORD _HeadColor{0};

	DWORD _dwColor{COLOR_WHITE};
	int _nWidth{0};
	int _nHeight{0};
	int _nLineNum{0};  //显示在头像上的行数
	int _nMaxWidth{0}; //多行中最大的宽度
	ALLIGN m_Allign{eAlignTop};
};

// 内联函数
inline void CItemRow::SetIndex(unsigned int v, CItemObj* p) {
	if (_items[v] != _GetNullItem() && _items[v] != p) {
		//delete _items[v];
		SAFE_DELETE(_items[v]); // UI当机处理
	}
	_items[v] = p;
}

inline int CItemEx::GetWidth() const {
	if (!_bMultiLine)
		return _nWidth;
	else
		return _nMaxWidth;
}

} // namespace GUI
