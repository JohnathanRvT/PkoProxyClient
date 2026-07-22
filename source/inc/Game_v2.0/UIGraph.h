//----------------------------------------------------------------------
// 名称:静态文字
// 作者:lh 2004-07-19
// 最后修改日期:2004-10-09
//----------------------------------------------------------------------
#pragma once
#include "uiguidata.h"

namespace GUI {

// 图元
class CGraph : public CItemObj {
public:
	CGraph(const char* file, int w, int h, int sx, int sy, int maxframe)
		: _pImage(new CGuiPic(nullptr, maxframe)) {
		_pImage->LoadAllImage(file, w, h, sx, sy);
	}
	CGraph() : _pImage(new CGuiPic(nullptr, 1)) {}
	CGraph(unsigned int frame) : _pImage(new CGuiPic(nullptr, frame)) {}
	CGraph(const CGuiPic& rhs) : _pImage(new CGuiPic(rhs)) {}
	CGraph(const CGraph& rhs) : CItemObj(rhs), _pImage(new CGuiPic(*rhs._pImage)), nTag(rhs.nTag) {}
	CGraph& operator=(const CGraph& rhs) {
		CItemObj::operator=(rhs);
		*_pImage = *rhs._pImage;
		nTag = rhs.nTag;
		return *this;
	}
	~CGraph() {}
	ITEM_CLONE(CGraph)

	// Grid接口
	virtual void Render(int x, int y) override { _pImage->Render(x, y); }
	virtual int GetWidth() const override { return _pImage->GetWidth(); }
	virtual int GetHeight() const override { return _pImage->GetHeight(); }

	void Next() { _pImage->Next(); }
	void Reset() { _pImage->SetFrame(0); }

	// 外部接口
	CGuiPic* GetImage() { return _pImage.get(); }

	int nTag{0};

protected:
	std::unique_ptr<CGuiPic> _pImage{nullptr};
};

// 带hint的图元,用于编辑器树中节点
class CHintGraph : public CGraph {
public:
	CHintGraph(char* file, int w, int h, int sx, int sy, int maxframe);
	CHintGraph(unsigned int maxframe);

	virtual bool HasHint() const override { return !_strHint.empty(); }
	virtual void RenderHint(int x, int y) override;

public:
	void SetHint(const char* str) { _strHint = str; }

protected:
	std::string _strHint{};
};

// 用于聊天的头像
class CTextGraph : public CHintGraph {
public:
	CTextGraph(char* file, int w, int h, int sx, int sy, int maxframe);
	CTextGraph(unsigned int maxframe);
	~CTextGraph();

	virtual void Render(int x, int y) override;
	virtual void SetString(const char* str) override { _strName = str; }

	const char* GetString() const override { return _strName.c_str(); }
	virtual void SetColor(DWORD color) override { _color = color; }

	void SetPointer(void* v) { _pVoid = v; }
	void* GetPointer() { return _pVoid; }

protected:
	std::string _strName{};
	DWORD _color{COLOR_WHITE};

	void* _pVoid{nullptr};
};

// 用于树型控件的表格节点
class CNoteGraph : public CHintGraph {
public:
	CNoteGraph(unsigned int maxframe);

	virtual void Render(int x, int y) override;

	virtual void SetString(const char* str) override { _strName = str; }
	const char* GetString() const override { return _strName.c_str(); }

	virtual void SetColor(DWORD color) override { _color = color; }

	void SetTextX(int v) { _nTextX = v; }
	void SetTextY(int v) { _nTextY = v; }

protected:
	std::string _strName{};
	DWORD _color{COLOR_WHITE};
	int _nTextX{0}, _nTextY{0}; // 文字的X,Y偏移
};

} // namespace GUI
