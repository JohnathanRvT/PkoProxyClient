
#pragma once

#include "uiForm.h"
#include "uiFormMgr.h"
#include "uiEdit.h"
#include "uiLabel.h"
#include "ui3DCompent.h"
#include "uiRender.h"
#include "uiGlobalVar.h"
#include "PacketCmd.h"

#include "uiStoreForm.h"

//
#define ENABLE_NUMANSWER 1

// 读取内存BMP图片，暂时只支持黑白图片
class CImageBMP {
public:
	CImageBMP();
	~CImageBMP();

	bool Load(BYTE* pData, int nSize);
	void Clear();

	void RandomPoint();

	int GetWidth();
	int GetHeight();
	DWORD GetColor(int x, int y);

private:
	BYTE* _pData;
	int _nDataSize;
	int _nBmpData;
	int _nWidth;
	int _nHeight;
};

namespace GUI {

class CNumAnswerMgr : public CUIInterface {
public:
	CNumAnswerMgr();
	~CNumAnswerMgr();

	void ShowForm(bool v = true);

	void SetBmp(int nSeq, BYTE* pData, int nSize);
	void Refresh();

	void RenderCheckCode();
	void SendCheckCode();

protected:
	virtual bool Init() override;
	virtual void CloseForm() override {}
	virtual void FrameMove(DWORD dwTime) override;

private:
	CForm* frmNumAnswer;
	CForm* frmFast; // 仅作为坐标参照，不作任何处理
	CEdit* edtUserInput;
	CLabelEx* labTimeLeft;
	C3DCompent* ui3dCheckCode;

	LPTEXTURE _pNumTexture;

	static constexpr int NUMBER_COUNT = 4;
	CImageBMP _imgBMP[NUMBER_COUNT];

	DWORD _dwTickCount;

	static void _evtCheckCodeMouseEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);
	static void _evtCheckCodeRenderEvent(C3DCompent* pSender, int x, int y);
};

} // namespace GUI
