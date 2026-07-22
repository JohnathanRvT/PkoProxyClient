#include "StdAfx.h"
#include "uigraph.h"

using namespace GUI;
//---------------------------------------------------------------------------
// class CHintGraph
//---------------------------------------------------------------------------
CHintGraph::CHintGraph(char* file, int w, int h, int sx, int sy, int maxframe)
	: CGraph(file, w, h, sx, sy, maxframe) {
}

CHintGraph::CHintGraph(unsigned int maxframe)
	: CGraph(maxframe) {
}

void CHintGraph::RenderHint(int x, int y) {
	CGuiFont::s_Font.FrameRender(_strHint.c_str(), x + 32, y);
}

//---------------------------------------------------------------------------
// class CTextGraph
//---------------------------------------------------------------------------
CTextGraph::CTextGraph(char* file, int w, int h, int sx, int sy, int maxframe)
	: CHintGraph(file, w, h, sx, sy, maxframe) {
}

CTextGraph::CTextGraph(unsigned int maxframe)
	: CHintGraph(maxframe) {
}

CTextGraph::~CTextGraph() {}

void CTextGraph::Render(int x, int y) {
	_pImage->Render(x, y);
	static int nOffY = (16 - CGuiFont::s_Font.GetHeight(RES_STRING(CL_LANGUAGE_MATCH_489))) / 2;
	CGuiFont::s_Font.Render(_strName.c_str(), x + 20, y + nOffY, _color);
}

//---------------------------------------------------------------------------
// class CNoteGraph
//---------------------------------------------------------------------------
CNoteGraph::CNoteGraph(unsigned int maxframe)
	: CHintGraph(maxframe) {
}

void CNoteGraph::Render(int x, int y) {
	_pImage->Render(x, y);
	CGuiFont::s_Font.Render(_strName.c_str(), x + _nTextX, y + _nTextY, _color);
}
