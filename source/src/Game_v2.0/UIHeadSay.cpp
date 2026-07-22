#include "StdAfx.h"
#include "uiheadsay.h"
#include "GameApp.h"
#include "Character.h"
#include "uiitem.h"
#include "chastate.h"
#include "uiboatform.h"
#include "uiMiniMapForm.h"

using namespace GUI;

bool g_IsShowShop = true;
//---------------------------------------------------------------------------
// calss CHeadSay
//---------------------------------------------------------------------------
int CHeadSay::_nMaxShowTime = 100;
std::unique_ptr<CGuiPic> CHeadSay::_pImgLife = std::make_unique<CGuiPic>(nullptr, 2);

DWORD CHeadSay::_dwBkgColor = 0x90000000;

unsigned int CHeadSay::_nFaceFrequency = 3;
std::unique_ptr<CGuiPic[]> CHeadSay::_pImgFace = std::make_unique<CGuiPic[]>(FACE_MAX);
std::unique_ptr<CGuiPic> CHeadSay::_pImgTeamLeaderFlag = std::make_unique<CGuiPic>();

CGuiPic CHeadSay::_ImgShop[3];
int CHeadSay::_nShopFrameWidth = 0;
int CHeadSay::_nShopFontYOff = 0;

//名字显示方式 (前缀) ＋ 角色名 + (座右铭) ＋ [船名]
char CHeadSay::s_sNamePart[NAME_PART_NUM][64] = {0};
//{
//	"",							//	(
//	"",							//	前缀名
//	"",							//	)
//	"",							//	角色名
//	"",							//	(
//	"",							//	座右铭
//	"",							//	)
//	"",							//	[
//	"",							//	船名
//	"",							//	]
//};

char CHeadSay::s_szName[1024] = {0};

DWORD CHeadSay::s_dwNamePartsColors[NAME_PART_NUM][2] = //	前一列是前景色,后一列是阴影颜色
	{
		COLOR_WHITE, COLOR_BLACK,	 //	(
		0, COLOR_BLACK,				  //	前缀名
		COLOR_WHITE, COLOR_BLACK,	 //	)
		0, COLOR_BLACK,				  //	角色名
		COLOR_BLACK, 0,				  //	(
		COLOR_BLACK, 0,				  //	座右铭
		COLOR_BLACK, 0,				  //	)
		COLOR_WHITE, COLOR_BLACK,	 //	[
		COLOR_SHIP_NAME, COLOR_BLACK, //	船名
		COLOR_WHITE, COLOR_BLACK,	 //	]
};

char CHeadSay::s_szConsortiaNamePart[4][64] = {0};
//{
//	"",							//公会名
//	"",							//"("
//	"",							//公会座右铭
//	"",							//")"
//};

char CHeadSay::s_szConsortiaName[256] = {0};

// Member Methods---------------------------------------------------------

bool CHeadSay::Init() {
	_pImgTeamLeaderFlag->LoadImage("texture/ui/flag.tga", 12, 12, 0, 0, 0, 1.0, 1.0);

	_nShopFrameWidth = _ImgShop[0].GetWidth() - 1;
	_nShopFontYOff = (_ImgShop[0].GetHeight() - CGuiFont::s_Font.GetHeight(RES_STRING(CL_LANGUAGE_MATCH_489))) / 2;
	return true;
}

bool CHeadSay::Clear() {
	_pImgLife.reset();
	_pImgFace.reset();
	_pImgTeamLeaderFlag.reset();
	return true;
}

CHeadSay::CHeadSay(CCharacter* p) : _pOwn(p) {}

void CHeadSay::AddItem(CItemEx* obj) {
	if (_pObj)
		delete _pObj;

	_pObj = obj;

	_nShowTime = _nMaxShowTime;
	_fScale = 0.1f;
}

void CHeadSay::SetName(const char* name) {
	_nChaNameOffX = 0 - CGuiFont::s_Font.GetWidth(name) / 2;
}

void CHeadSay::Render(D3DXVECTOR3& pos) {
	static int x = 0, y = 0;
	static int nSayTotalWidth = 32 * CGuiFont::s_Font.GetWidth("a"); //32个字符的长度
	g_Render.WorldToScreen(pos.x, pos.y, pos.z + _pOwn->GetDefaultChaInfo()->fHeight, &x, &y);

	if (_IsShowLife) //血条
	{
		static int x1 = 0, y1 = 0;
		g_Render.WorldToScreen(pos.x, pos.y, pos.z, &x1, &y1);

		int nLifeWidth = _pImgLife->GetWidth();
		_pImgLife->SetScaleW(1, _fLifeW);
		int nOffset = (int)((x - g_Render.GetScrWidth() / 2) * 0.02f);
		_pImgLife->RenderAll(x - nLifeWidth / 2 - nOffset, y1 + 20, (DWORD)0xa0ffffff);
	}

	if (g_stUIMap.IsPKSilver()) {
		// 在乱斗白银城里玩家头上不显示任何内容，仅显示玩家或怪的血条
		return;
	}

	if (_pOwn->GetIsFly())
		y -= 30;

	if (_pOwn->IsTeamLeader()) {
		y -= 20;
		GetLeaderPic()->Render(x - 6, y);
	}

	if (_pOwn->IsShop()) {
		_RenderShop(_pOwn->getShopName(), x, y);

		static int nImageHeight = _ImgShop[0].GetHeight();
		y -= nImageHeight;
	}

	if (_nShowTime <= 0) {
#ifdef _LOG_NAME_								//角色名称
		const int LINE_HEIGHT_STEP = 14;		//一行的高度
		int iNameHeightStep = LINE_HEIGHT_STEP; //角色名或船名的高度

		// 处理日志名
		if (CCharacter::IsShowLogName) {
			int nNameLength = 0 - CGuiFont::s_Font.GetWidth(_pOwn->getLogName()) / 2;
			CGuiFont::s_Font.BRender(_pOwn->getLogName(), x + nNameLength, y - iNameHeightStep, _dwNameColor, COLOR_BLACK);
		} else if (_IsShowName) {
			// 处理公会名
			if (*(_pOwn->getGuildName())) {
				iNameHeightStep += LINE_HEIGHT_STEP; //放在上面一行
				int iGuildNameHeightStep = LINE_HEIGHT_STEP;

				//公会名
				if (*(_pOwn->getGuildName())) {
					strncpy_s(s_szConsortiaNamePart[0], _pOwn->getGuildName(), _TRUNCATE);
				} else {
					strncpy_s(s_szConsortiaNamePart[0], "", _TRUNCATE);
				}
				//公会座右铭
				if (*(_pOwn->getGuildMotto())) {
					strncpy_s(s_szConsortiaNamePart[1], "(", _TRUNCATE);
					strncpy_s(s_szConsortiaNamePart[2], _pOwn->getGuildMotto(), _TRUNCATE);
					strncpy_s(s_szConsortiaNamePart[3], ")", _TRUNCATE);
				} else {
					strncpy_s(s_szConsortiaNamePart[1], "", _TRUNCATE);
					strncpy_s(s_szConsortiaNamePart[2], "", _TRUNCATE);
					strncpy_s(s_szConsortiaNamePart[3], "", _TRUNCATE);
				}

				//ZeroMemory(s_szConsortiaName, sizeof(s_szConsortiaName));
				s_szConsortiaName[0] = '\0';
				for (int i(0); i < 4; i++) {
					strncat_s(s_szConsortiaName, s_szConsortiaNamePart[i], _TRUNCATE);
				}
				int nNameLength = 0 - CGuiFont::s_Font.GetWidth(s_szConsortiaName) / 2;
				int iStartPosX = 0 - CGuiFont::s_Font.GetWidth(s_szConsortiaName) / 2;

				// render每一个部分
				for (int i(0); i < 4; i++) {
					CGuiFont::s_Font.BRender(s_szConsortiaNamePart[i], x + iStartPosX, y - iGuildNameHeightStep, _pOwn->getGuildColor(), COLOR_BLACK);
					iStartPosX += CGuiFont::s_Font.GetWidth(s_szConsortiaNamePart[i]);
				}
			}

			// 前缀名
			if (*(_pOwn->GetPreName())) {
				strncpy_s(s_sNamePart[PRENAME_SEP1_INDEX], "(", _TRUNCATE);
				strncpy_s(s_sNamePart[PRENAME_INDEX], _pOwn->GetPreName(), _TRUNCATE);
				strncpy_s(s_sNamePart[PRENAME_SEP2_INDEX], ")", _TRUNCATE);
				s_dwNamePartsColors[PRENAME_INDEX][0] = _pOwn->GetPreColor();
			} else {
				strncpy_s(s_sNamePart[PRENAME_SEP1_INDEX], "", _TRUNCATE);
				strncpy_s(s_sNamePart[PRENAME_INDEX], "", _TRUNCATE);
				strncpy_s(s_sNamePart[PRENAME_SEP2_INDEX], "", _TRUNCATE);
				s_dwNamePartsColors[PRENAME_INDEX][0] = 0;
			}

			//座右铭
			if (*(_pOwn->getSecondName())) {
				strncpy_s(s_sNamePart[MOTTO_NAME_SEP1_INDEX], "(", _TRUNCATE);
				strncpy_s(s_sNamePart[MOTTO_NAME_INDEX], _pOwn->getSecondName(), _TRUNCATE);
				strncpy_s(s_sNamePart[MOTTO_NAME_SEP2_INDEX], ")", _TRUNCATE);
			} else {
				strncpy_s(s_sNamePart[MOTTO_NAME_SEP1_INDEX], "", _TRUNCATE);
				strncpy_s(s_sNamePart[MOTTO_NAME_INDEX], "", _TRUNCATE);
				strncpy_s(s_sNamePart[MOTTO_NAME_SEP2_INDEX], "", _TRUNCATE);
			}

			//角色名或船名
			s_dwNamePartsColors[NAME_INDEX][0] = _dwNameColor;

			if (_pOwn->IsBoat()) {
				strncpy_s(s_sNamePart[NAME_INDEX], _pOwn->getHumanName(), _TRUNCATE);
				strncpy_s(s_sNamePart[BOAT_NAME_SEP1_INDEX], "[", _TRUNCATE);
				strncpy_s(s_sNamePart[BOAT_NAME_INDEX], _pOwn->getName(), _TRUNCATE);
				strncpy_s(s_sNamePart[BOAT_NAME_SEP2_INDEX], "]", _TRUNCATE);
			} else {
				strncpy_s(s_sNamePart[BOAT_NAME_SEP1_INDEX], "", _TRUNCATE);
				strncpy_s(s_sNamePart[BOAT_NAME_INDEX], "", _TRUNCATE);
				strncpy_s(s_sNamePart[BOAT_NAME_SEP2_INDEX], "", _TRUNCATE);
				strncpy_s(s_sNamePart[NAME_INDEX], _pOwn->getName(), _TRUNCATE);

				if (_pOwn->IsMonster()) { //如果是怪物，且等级高于自身10级的显示怪物等级

					// 自身等级
					static int nMainLevel(0);
					if (g_stUIBoat.GetHuman()) {
						nMainLevel = (int)g_stUIBoat.GetHuman()->getGameAttr()->get(ATTR_LV);
					}

					// 怪物等级
					static int nMonsterLevel(0);
					nMonsterLevel = (int)_pOwn->getGameAttr()->get(ATTR_LV);

					static char szBuf[NAME_LENGTH] = {0};
					if (nMonsterLevel - nMainLevel <= 10) { //显示等级
						_snprintf_s(szBuf, _TRUNCATE, "Lv%d %s", nMonsterLevel, _pOwn->getName());
					} else {
						_snprintf_s(szBuf, _TRUNCATE, "??? %s", _pOwn->getName());
					}
					strncpy_s(s_sNamePart[NAME_INDEX], szBuf, _TRUNCATE);
				}
			}

			// 得到名字的总长
			s_szName[0] = '\0';
			//ZeroMemory( s_szName, sizeof(s_szName) );
			for (int i(0); i < NAME_PART_NUM; i++) {
				strncat_s(s_szName, s_sNamePart[i], _TRUNCATE);
			}
			int nNameLength = 0 - CGuiFont::s_Font.GetWidth(s_szName) / 2;
			int iStartPosX = 0 - CGuiFont::s_Font.GetWidth(s_szName) / 2;

			// render每一个部分
			for (int i(0); i < NAME_PART_NUM; i++) {
				if (s_dwNamePartsColors[i][1]) {
					CGuiFont::s_Font.BRender(s_sNamePart[i], x + iStartPosX, y - iNameHeightStep, s_dwNamePartsColors[i][0], s_dwNamePartsColors[i][1]);
				} else {
					CGuiFont::s_Font.Render(s_sNamePart[i], x + iStartPosX, y - iNameHeightStep, s_dwNamePartsColors[i][0]);
				}
				iStartPosX += CGuiFont::s_Font.GetWidth(s_sNamePart[i]);
			}
		}
#else
		if (_IsShowName) {
			if (_pOwn->IsBoat()) //显示角色名+船名
			{
				static std::string sNameBuf;
				sNameBuf = _pOwn->getHumanName();
				sNameBuf += "--";
				sNameBuf += _pOwn->getName();

				int nNameLength = 0 - CGuiFont::s_Font.GetWidth(sNameBuf.c_str()) / 2;
				CGuiFont::s_Font.BRender(sNameBuf.c_str(), x + nNameLength, y - LINE_HEIGHT_STEP, _dwNameColor, COLOR_BLACK);
				if (_pOwn->IsShowSecondName()) {
					if (strlen(_pOwn->getSecondName()) > 0) {
						std::string strSec = "(";
						strSec += _pOwn->getSecondName();
						strSec += ")";
						CGuiFont::s_Font.Render(strSec.c_str(), x - nNameLength, y - LINE_HEIGHT_STEP, COLOR_BLACK);
					}
				}
			} else //显示角色名
			{
				int nNameLength = 0 - CGuiFont::s_Font.GetWidth(_pOwn->getName()) / 2;
				CGuiFont::s_Font.BRender(_pOwn->getName(), x + nNameLength, y - LINE_HEIGHT_STEP, _dwNameColor, COLOR_BLACK);
				if (_pOwn->IsShowSecondName()) {
					if (strlen(_pOwn->getSecondName()) > 0) {
						std::string strSec = "(";
						strSec += _pOwn->getSecondName();
						strSec += ")";
						CGuiFont::s_Font.Render(strSec.c_str(), x - nNameLength, y - LINE_HEIGHT_STEP, COLOR_BLACK);
					}
				}

			} //end of if (_pOwn->IsBoat())
		}
#endif
	} else if (_nShowTime > 0) //聊天
	{
		_nShowTime--;

		if (_pObj) {
			int left = x - (int)(_fScale * _pObj->GetWidth() / 2);
			int nLine = _pObj->GetLineNum();

			std::string str = _pObj->GetString();

			if (str.find("#") != -1) {
				if (nLine == 1) {
					GetRender().FillFrame(left - 2, y - 20 - 32, left + (int)(_fScale * _pObj->GetWidth()) + 4, y - 14, _dwBkgColor);
					_pObj->RenderEx(left, y - 18 - 32, 32, _fScale);
				} else if (nLine == 2) {
					GetRender().FillFrame(left - 2, y - 20 - 44, left + (int)(_fScale * _pObj->GetWidth()) + 4, y - 14, _dwBkgColor);
					_pObj->RenderEx(left, y - 18 - 44, 24, _fScale);
				} else if (nLine == 3) {
					GetRender().FillFrame(left - 2, y - 20 - 76, left + (int)(_fScale * _pObj->GetWidth()) + 4, y - 14, _dwBkgColor);
					_pObj->RenderEx(left, y - 18 - 76, 24, _fScale);
				}
			} else {
				if (nLine == 1) {
					GetRender().FillFrame(left - 2, y - 20 - 18, left + (int)(_fScale * _pObj->GetWidth()) + 4, y - 14, _dwBkgColor);
					_pObj->RenderEx(left, y - 18 - 18, 12, _fScale);
				} else if (nLine == 2) {
					GetRender().FillFrame(left - 2, y - 20 - 36, left + (int)(_fScale * _pObj->GetWidth()) + 4, y - 14, _dwBkgColor);
					_pObj->RenderEx(left, y - 18 - 36, 12, _fScale);
				} else if (nLine == 3) {
					GetRender().FillFrame(left - 2, y - 20 - 54, left + (int)(_fScale * _pObj->GetWidth()) + 4, y - 14, _dwBkgColor);
					_pObj->RenderEx(left, y - 18 - 54, 12, _fScale);
				}
			}
			_fScale += 0.2f;
			if (_fScale > 1.0f)
				_fScale = 1.0f;

			if (_nShowTime == 4)
				_fScale = 0.8f;
			else if (_nShowTime == 3)
				_fScale = 0.6f;
			else if (_nShowTime == 2)
				_fScale = 0.4f;
			else if (_nShowTime == 1)
				_fScale = 0.2f;
		}
	}

	if (_pCurFace && _nFaceTime > 0) //表情
	{
		_nFaceTime--;
		y -= 20;
		_nCurFaceCycle++;
		if (_nCurFaceCycle > _nFaceFrequency) {
			_nCurFaceCycle = 0;

			_nCurFaceFrame++;
			if (_nCurFaceFrame >= _pCurFace->GetMax()) {
				_nCurFaceFrame = 0;
			}
			_pCurFace->SetFrame(_nCurFaceFrame);
		}
		_pCurFace->Render(x - _pCurFace->GetWidth() / 2, y - 20);
	}

#ifdef _LOG_NAME_ //角色名称
	if (CCharacter::IsShowLogName) {
		CChaStateMgr* pState = _pOwn->GetStateMgr();
		int nCount = pState->GetSkillStateNum();
		for (int i = 0; i < nCount; i++) {
			y -= 20;
			CGuiFont::s_Font.Render(pState->GetSkillState(i)->szName, x, y, COLOR_WHITE);
		}
	}
#endif
}

bool CHeadSay::SetFaceID(unsigned int faceid) {
	if (faceid >= FACE_MAX)
		return false;

	_nCurFaceFrame = 0;
	_nCurFaceCycle = 0;
	_nFaceTime = _nMaxShowTime;
	_pCurFace = &_pImgFace[faceid];
	return true;
}

void CHeadSay::SetLifeNum(int num, int max) {
	if (max == 0)
		return;

	if (num < 0)
		num = 0;
	if (num > max)
		num = max;
	if (_pImgLife) {
		_fLifeW = (float)num / (float)max;
	}
}

void CHeadSay::_RenderShop(const char* szShopName, int x, int y) {
	static int nImageHeight = _ImgShop[0].GetHeight();
	static int nImageWidth = _ImgShop[0].GetWidth();
	y -= nImageHeight;

	static int nFontWidth = 0;
	nFontWidth = CGuiFont::s_Font.GetWidth(szShopName);
	const DWORD dwImgColor = 0xffffffff;

	if (nFontWidth < 40) {
		static int nFontTrueWidth = 0;
		nFontTrueWidth = nFontWidth;
		nFontWidth = 40;

		x -= _nShopFrameWidth + nFontWidth / 2;

		_nShopX0 = x;
		_nShopY0 = y;
		_nShopX1 = x + nFontWidth + nImageWidth;
		_nShopY1 = y + nImageHeight;

		_ImgShop[0].Render(x, y, dwImgColor);
		x += _nShopFrameWidth;

		_ImgShop[2].Render(x + nFontWidth, y, dwImgColor);

		_ImgShop[1].SetScaleW(nFontWidth);
		_ImgShop[1].Render(x, y, dwImgColor);
		//CGuiFont::s_Font.BRender( szShopName, x + (nFontWidth - nFontTrueWidth) / 2, y+_nShopFontYOff, COLOR_BLACK, COLOR_WHITE );
		CGuiFont::s_Font.Render(szShopName, x + (nFontWidth - nFontTrueWidth) / 2, y + _nShopFontYOff, COLOR_BLACK);
	} else {
		x -= _nShopFrameWidth + nFontWidth / 2;

		_nShopX0 = x;
		_nShopY0 = y;
		_nShopX1 = x + nFontWidth + nImageWidth;
		_nShopY1 = y + nImageHeight;

		_ImgShop[0].Render(x, y, dwImgColor);
		x += _nShopFrameWidth;

		_ImgShop[2].Render(x + nFontWidth, y, dwImgColor);

		_ImgShop[1].SetScaleW(nFontWidth);
		_ImgShop[1].Render(x, y, dwImgColor);
		//CGuiFont::s_Font.BRender( szShopName, x, y+_nShopFontYOff, COLOR_BLACK, COLOR_WHITE );
		CGuiFont::s_Font.Render(szShopName, x, y + _nShopFontYOff, COLOR_BLACK);
	}
}

void CHeadSay::RenderText(const char* szShopName, int x, int y) {
	static int nImageHeight = _ImgShop[0].GetHeight();
	static int nImageWidth = _ImgShop[0].GetWidth();
	// y -= nImageHeight;
	y -= 100;

	static int nFontWidth = 0;
	nFontWidth = CGuiFont::s_Font.GetWidth(szShopName);
	const DWORD dwImgColor = 0xffffffff;

	if (nFontWidth < 40) {
		static int nFontTrueWidth = 0;
		nFontTrueWidth = nFontWidth;
		nFontWidth = 40;

		x -= _nShopFrameWidth + nFontWidth / 2;

		_ImgShop[0].Render(x, y, dwImgColor);
		x += _nShopFrameWidth;

		_ImgShop[2].Render(x + nFontWidth, y, dwImgColor);

		_ImgShop[1].SetScaleW(nFontWidth);
		_ImgShop[1].Render(x, y, dwImgColor);
		CGuiFont::s_Font.BRender(szShopName, x + (nFontWidth - nFontTrueWidth) / 2, y + _nShopFontYOff, COLOR_BLACK, COLOR_WHITE);
	} else {
		x -= _nShopFrameWidth + nFontWidth / 2;

		_ImgShop[0].Render(x, y, dwImgColor);
		x += _nShopFrameWidth;

		_ImgShop[2].Render(x + nFontWidth, y, dwImgColor);

		_ImgShop[1].SetScaleW(nFontWidth);
		_ImgShop[1].Render(x, y, dwImgColor);
		CGuiFont::s_Font.BRender(szShopName, x, y + _nShopFontYOff, COLOR_BLACK, COLOR_WHITE);
	}
}
