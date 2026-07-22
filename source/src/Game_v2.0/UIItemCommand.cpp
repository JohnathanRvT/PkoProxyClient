#include "StdAfx.h"
#include "uiitemcommand.h"
#include "Character.h"
#include "GameApp.h"
#include "uifastcommand.h"
#include "PacketCmd.h"
#include "uiequipform.h"
#include "StringLib.h"
#include "uiboatform.h"
#include "stpose.h"
#include "stnpctalk.h"
#include "STAttack.h"
using namespace GUI;
//---------------------------------------------------------------------------
// class CItemCommand
//---------------------------------------------------------------------------
static char buf[256] = {0};

const DWORD VALID_COLOR = COLOR_RED;
const DWORD GENERIC_COLOR = COLOR_WHITE;
const DWORD ADVANCED_COLOR = 0xFF9CCFFF;
const DWORD GLOD_COLOR = 0xFFFFFF00;

const unsigned int ITEM_HEIGHT = 32;
const unsigned int ITEM_WIDTH = 32;

std::map<int, DWORD> CItemCommand::_mapCoolDown; // 保存上一次放的道具技能的时间

CItemCommand::CItemCommand(CItemRecord* pItem)
	: _pItem(pItem), _dwColor(COLOR_WHITE), _pImage(new CGuiPic) {
	if (!_pItem) {
		LG("error", "msgCItemCommand::CItemCommand(CItemRecord* pItem) pItem is NULL");
	}

	const char* file = pItem->GetIconFile();

	// 判断文件是否存在
	HANDLE hFile = CreateFile(file, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == hFile) {
		_pImage->LoadImage("texture/icon/error.tga", ITEM_WIDTH, ITEM_HEIGHT, 0);
	} else {
		CloseHandle(hFile);

		_pImage->LoadImage(file, ITEM_WIDTH, ITEM_HEIGHT, 0);
	}

	_ItemData = {};
	_ItemData.sID = (short)pItem->lID;
	_ItemData.SetValid();

	_nPrice = _pItem->lPrice;
}

CItemCommand::CItemCommand(const CItemCommand& rhs)
	: _pImage(new CGuiPic(*rhs._pImage)) {
	_Copy(rhs);
}

CItemCommand& CItemCommand::operator=(const CItemCommand& rhs) {
	*_pImage = *rhs._pImage;

	_Copy(rhs);
	return *this;
}

void CItemCommand::_Copy(const CItemCommand& rhs) {
	_ItemData = rhs._ItemData;
	SetBoatHint(rhs._pBoatHint.get());

	_pItem = rhs._pItem;
	_dwColor = rhs._dwColor;
	_nPrice = rhs._nPrice;
}

void CItemCommand::PUSH_HINT(const char* str, int value, DWORD color) {
	if (value == 0)
		return;

	_snprintf_s(buf, _TRUNCATE, str, value);
	PushHint(buf, color);
}

void CItemCommand::SaleRender(int x, int y, int nWidth, int nHeight) {
	static int nX, nY;
	static int w, h;
	nX = x + (nWidth - ITEM_WIDTH) / 2;
	nY = y + (nHeight - ITEM_HEIGHT) / 2;

	_pImage->Render(nX, nY, _ItemData.IsValid() ? _dwColor : (DWORD)0xff757575);

	if (_ItemData.sNum >= 0) {
		static int xNum, yNum;
		_snprintf_s(buf, _TRUNCATE, "%d", _ItemData.sNum);
		CGuiFont::s_Font.GetSize(buf, w, h);

		xNum = nX + ITEM_WIDTH - w;
		yNum = nY + ITEM_HEIGHT - h;
		GetRender().FillFrame(xNum, yNum, xNum + w, yNum + h, 0xE0ADF6F7);
		CGuiFont::s_Font.Render(buf, xNum, yNum, COLOR_BLACK);
	}

	CGuiFont::s_Font.GetSize(_pItem->szName, w, h);
	if (w > nWidth) {
		static char szBuf1[128] = {0};
		static char szBuf2[128] = {0};
		static int nEnter = 0;
		strncpy_s(szBuf1, sizeof(szBuf1), _pItem->szName, _TRUNCATE);
		nEnter = (int)strlen(szBuf1);
		if (nEnter < 0)
			return;

		nEnter++;
		szBuf1[nEnter] = '\0';
		strncpy_s(szBuf2, sizeof(szBuf2), &_pItem->szName[nEnter], _TRUNCATE);

		CGuiFont::s_Font.GetSize(szBuf1, w, h);
		CGuiFont::s_Font.Render(szBuf1, x + (nWidth - w) / 2, nY - h - h - 2, COLOR_BLACK);

		CGuiFont::s_Font.GetSize(szBuf2, w, h);
		CGuiFont::s_Font.Render(szBuf2, x + (nWidth - w) / 2, nY - h - 2, COLOR_BLACK);
	} else {
		CGuiFont::s_Font.Render(_pItem->szName, x + (nWidth - w) / 2, nY - h - 2, COLOR_BLACK);
	}

	_snprintf_s(buf, _TRUNCATE, "$%s", StringSplitNum(_nPrice));
	CGuiFont::s_Font.GetSize(buf, w, h);
	CGuiFont::s_Font.Render(buf, x + (nWidth - w) / 2, nY + ITEM_HEIGHT + 2, COLOR_BLACK);
}

void CItemCommand::Render(int x, int y) {
	_pImage->Render(x, y, _ItemData.IsValid() ? _dwColor : (DWORD)0xff757575);

	auto RenderItemIconLabel = [&](const char* str, DWORD color = COLOR_BLACK) {
		static int w, h;
		CGuiFont::s_Font.GetSize(str, w, h);

		x += ITEM_WIDTH - w;
		y += ITEM_HEIGHT - h;
		GetRender().FillFrame(x, y, x + w, y + h, 0xE0ADF6F7);
		CGuiFont::s_Font.Render(str, x, y, color);
	};

	// Item stack count
	if (_ItemData.sNum > 1) {
		_snprintf_s(buf, _TRUNCATE, "%d", _ItemData.sNum);
		RenderItemIconLabel(buf);
	}

	const auto type = _pItem->sType;

	// Gemstone level
	if (type == EItemType::RefiningStone || type == EItemType::GemStone) {
		_snprintf_s(buf, _TRUNCATE, "Lv%d", _ItemData.sEnergy[1]);
		RenderItemIconLabel(buf);
	}

	// Pet level
	if (type == EItemType::Pet) {
		const int nLevel = [&] {
			int result{0};
			for (const auto& [type, value] : _ItemData.sInstAttr) {
				result += value;
			}
			return result;
		}();
		_snprintf_s(buf, _TRUNCATE, "Lv%d", nLevel);
		RenderItemIconLabel(buf);
	}

	// Forge level
	if (const auto& forgeInfo = GetForgeInfo(); forgeInfo.nLevel > 0) {
		_snprintf_s(buf, _TRUNCATE, "+%d", forgeInfo.nLevel);
		RenderItemIconLabel(buf);
	}
}

void CItemCommand::OwnDefRender(int x, int y, int nWidth, int nHeight) {
	static int nX, nY;
	static int w, h;
	nX = x + (nWidth - ITEM_WIDTH) / 2;
	nY = y + (nHeight - ITEM_HEIGHT) / 2;

	_pImage->Render(nX, nY, _ItemData.IsValid() ? _dwColor : (DWORD)0xff757575);

	if (_ItemData.sNum >= 0) {
		static int xNum, yNum;
		_snprintf_s(buf, _TRUNCATE, "%d", _ItemData.sNum);
		CGuiFont::s_Font.GetSize(buf, w, h);

		xNum = nX + ITEM_WIDTH - w;
		yNum = nY + ITEM_HEIGHT - h;
		GetRender().FillFrame(xNum, yNum, xNum + w, yNum + h, 0xE0ADF6F7);
		CGuiFont::s_Font.Render(buf, xNum, yNum, COLOR_BLACK);
	}

	CGuiFont::s_Font.GetSize(_pItem->szName, w, h);
	if (w > nWidth) {
		static char szBuf1[128] = {0};
		static char szBuf2[128] = {0};
		static int nEnter = 0;
		strncpy_s(szBuf1, sizeof(szBuf1), _pItem->szName, _TRUNCATE);
		nEnter = (int)strlen(szBuf1);
		if (nEnter < 0)
			return;

		nEnter++;
		szBuf1[nEnter] = '\0';
		strncpy_s(szBuf2, sizeof(szBuf2), &_pItem->szName[nEnter], _TRUNCATE);

		CGuiFont::s_Font.GetSize(szBuf1, w, h);
		CGuiFont::s_Font.Render(szBuf1, x + (nWidth - w) / 2, nY - h - h - 2, COLOR_BLACK);

		CGuiFont::s_Font.GetSize(szBuf2, w, h);
		CGuiFont::s_Font.Render(szBuf2, x + (nWidth - w) / 2, nY - h - 2, COLOR_BLACK);
	} else {
		CGuiFont::s_Font.Render(_pItem->szName, x + (nWidth - w) / 2, nY - h - 2, COLOR_BLACK);
	}

	//_snprintf_s( buf, _TRUNCATE, "$%s", StringSplitNum(_nPrice) );
	CGuiFont::s_Font.GetSize(_OwnDefText.c_str(), w, h);
	CGuiFont::s_Font.Render(_OwnDefText.c_str(), x + (nWidth - w) / 2, nY + ITEM_HEIGHT + 2, COLOR_BLACK);
}

void CItemCommand::RenderEnergy(int x, int y) {
	if (_pItem->sType == EItemType::Conch && _ItemData.sEnergy[1] != 0) {
		float fLen = (float)_ItemData.sEnergy[0] / (float)_ItemData.sEnergy[1] * (float)ITEM_HEIGHT;
		int yb = y + ITEM_HEIGHT;
		GetRender().FillFrame(x, y, x + 2, yb, COLOR_BLUE);
		GetRender().FillFrame(x, yb - (int)fLen, x + 2, yb, COLOR_RED);
	}
}

void CItemCommand::_AddDescriptor() {
	StringNewLine(buf, sizeof(buf), 40, _pItem->szDescriptor, (unsigned int)strlen(_pItem->szDescriptor));
	PushHint(buf);
}

void CItemCommand::AddHint(int x, int y) {
	bool isMain = false; // "Determine if you want to divide two (selling price)"
	if (GetParent()) {
		std::string name = GetParent()->GetForm()->GetName();
		if (name == "frmPlayertrade" || name == "frmItem" || name == "frmNPCstorage" || name == "frmTempBag" ||
			name == "frmBreak" || name == "frmCooking" || name == "frmFound" || name == "frmBreak" ||
			name == "frmStore" || name == "frmViewAll" ||
			name == "frmSpiritMarry" || name == "frmSpiritErnie" || name == "frmEquipPurify") {
			isMain = true;
		}
	}

	bool isStore = false; // "Determine if it is a mall"
	if (GetParent()) {
		std::string name = GetParent()->GetForm()->GetName();
		if (name == "frmStore") {
			isStore = true;
		}
	}

	SGameAttr* pAttr = nullptr;
	if (g_stUIBoat.GetHuman()) {
		pAttr = g_stUIBoat.GetHuman()->getGameAttr();
	}
	if (!pAttr)
		return;

	SetHintIsCenter(true);

	static SItemHint item;
	memset(&item, 0, sizeof(SItemHint));
	CItemRecord* pEquipItem(nullptr);

	if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
		pEquipItem = GetItemRecordInfo(_ItemData.GetFusionItemID());
		if (pEquipItem) {
			item.Convert(_ItemData, pEquipItem);
		}
	} else {
		item.Convert(_ItemData, _pItem);
	}

	if (_ItemData.dwDBID) {
		PushHint(RES_STRING(CL_LANGUAGE_MATCH_960), COLOR_RED, 5, 0);
		AddHintHeight();
	}

	if (_pItem->isWeapon()) {
		if (_pItem->sType == EItemType::Glave) {
			if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
				_snprintf_s(buf, _TRUNCATE, "Lv%d %s", _ItemData.GetItemLevel(), GetName());
			} else {
				_snprintf_s(buf, _TRUNCATE, "%s", GetName());
			}

			PushHint(buf, COLOR_WHITE, 5, 1);
			PushHint(RES_STRING(CL_LANGUAGE_MATCH_624), COLOR_WHITE, 5, 1);
		} else {
			if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
				_snprintf_s(buf, _TRUNCATE, "Lv%d %s", _ItemData.GetItemLevel(), GetName());
				PushHint(buf, COLOR_WHITE, 5, 1);
			} else {
				PushHint(GetName(), COLOR_WHITE, 5, 1);
			}
		}

		// "Basic class: attack power, durability"
		AddHintHeight();

		if (_pItem->lID != 3669) { // "Special handling of props, romantic fireworks, does not show attack power"
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_625), _GetValue(ITEMATTR_VAL_MNATK, item), _GetValue(ITEMATTR_VAL_MXATK, item));
			PushHint(buf, GENERIC_COLOR);
		}

		if (!isStore) // Durability
		{
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_626), item.sEndure[0], item.sEndure[1]);
			PushHint(buf, GENERIC_COLOR);
		}

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_627), _ItemData.GetItemLevel() * 2 + 80); // Apparel Effectiveness
			PushHint(buf, GENERIC_COLOR);
		}

		//"Demand category: strength, agility, physique, spirit, luck, focus, rank, occupation"
		AddHintHeight();

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			if (_pItem->sNeedLv > pEquipItem->sNeedLv) {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			} else {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), pEquipItem->sNeedLv, pAttr->get(ATTR_LV) >= pEquipItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			}
		} else {
			PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
		}

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_ShowFusionBody(pEquipItem);
			_ShowFusionWork(_pItem, pEquipItem, pAttr);
		} else {
			_ShowBody();
			_ShowWork(_pItem, pAttr);
		}
	} else if (_pItem->isArmor()) {
		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_snprintf_s(buf, _TRUNCATE, "Lv%d %s", _ItemData.GetItemLevel(), GetName());
			PushHint(buf, COLOR_WHITE, 5, 1);
		} else {
			PushHint(GetName(), COLOR_WHITE, 5, 1);
		}

		// "Basic class: defense, durability"
		AddHintHeight();

		_PushValue(RES_STRING(CL_LANGUAGE_MATCH_629), ITEMATTR_VAL_DEF, item);

		if (!isStore) // Durability
		{
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_626), item.sEndure[0], item.sEndure[1]);
			PushHint(buf);
		}

		_PushValue(RES_STRING(CL_LANGUAGE_MATCH_630), ITEMATTR_VAL_PDEF, item);

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_627), _ItemData.GetItemLevel() * 2 + 80); // Apparel Effectivenss
			PushHint(buf, GENERIC_COLOR);
		}

		AddHintHeight();

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			if (_pItem->sNeedLv > pEquipItem->sNeedLv) {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			} else {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), pEquipItem->sNeedLv, pAttr->get(ATTR_LV) >= pEquipItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			}
		} else {
			PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
		}

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_ShowFusionBody(pEquipItem);
			_ShowFusionWork(_pItem, pEquipItem, pAttr);
		} else {
			_ShowBody();
			_ShowWork(_pItem, pAttr);
		}
	} else if (_pItem->IsNecklace()) {
		PushHint(GetName(), COLOR_WHITE, 5, 1);

		//AddHintHeight();

		//_PushItemAttr( ITEMATTR_VAL_MXHP, item );
		//_PushItemAttr( ITEMATTR_VAL_MXSP, item );
		//_PushItemAttr( ITEMATTR_VAL_HREC, item );
		//_PushItemAttr( ITEMATTR_VAL_SREC, item );
		//_PushItemAttr( ITEMATTR_VAL_PDEF, item );

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			if (_pItem->sNeedLv > pEquipItem->sNeedLv) {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			} else {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), pEquipItem->sNeedLv, pAttr->get(ATTR_LV) >= pEquipItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			}
		} else {
			PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
		}

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_ShowFusionBody(pEquipItem);
			_ShowFusionWork(_pItem, pEquipItem, pAttr);
		} else {
			_ShowBody();
			_ShowWork(_pItem, pAttr);
		}
		if (_pItem->lID == 6386 || _pItem->lID == 6387 ||
			_pItem->lID == 6388 || _pItem->lID == 6389 ||
			_pItem->lID == 6390 || _pItem->lID == 6391 ||
			_pItem->lID == 6434 || _pItem->lID == 6428 ||
			_pItem->lID == 6429 || _pItem->lID == 6430 ||
			_pItem->lID == 6431 || _pItem->lID == 6432) {
			_AddDescriptor();
		}
	} else if (_pItem->IsRing()) {
		PushHint(GetName(), COLOR_WHITE, 5, 1);

		//      AddHintHeight();

		//_PushItemAttr( ITEMATTR_VAL_MXATK, item );
		//_PushItemAttr( ITEMATTR_VAL_DEF, item );
		//_PushItemAttr( ITEMATTR_VAL_FLEE, item );
		//_PushItemAttr( ITEMATTR_VAL_HIT, item );
		//_PushItemAttr( ITEMATTR_VAL_CRT, item );

		if (_pItem->lID == 1034) // Star of Unity
		{
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_862), _ItemData.sEndure[0] * 10 - 1000, _ItemData.sEndure[1] * 10 - 1000); // "Specific calculation method"
			PushHint(buf, COLOR_WHITE, 5, 1);
			return;
		} else {

			if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
				if (_pItem->sNeedLv > pEquipItem->sNeedLv) {
					PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
				} else {
					PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), pEquipItem->sNeedLv, pAttr->get(ATTR_LV) >= pEquipItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
				}
			} else {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			}

			if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
				_ShowFusionBody(pEquipItem);
				_ShowFusionWork(_pItem, pEquipItem, pAttr);
			} else {
				_ShowBody();
				_ShowWork(_pItem, pAttr);
			}
		}
	} else if (_pItem->IsGlove()) {

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_snprintf_s(buf, _TRUNCATE, "Lv%d %s", _ItemData.GetItemLevel(), GetName());
			PushHint(buf, COLOR_WHITE, 5, 1);
		} else {
			PushHint(GetName(), COLOR_WHITE, 5, 1);
		}

		// "Basic class: defense, durability"
		AddHintHeight();

		_PushValue(RES_STRING(CL_LANGUAGE_MATCH_629), ITEMATTR_VAL_DEF, item);

		if (!isStore) { // Durability
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_626), item.sEndure[0], item.sEndure[1]);
			PushHint(buf);
		}

		_PushValue(RES_STRING(CL_LANGUAGE_MATCH_631), ITEMATTR_VAL_HIT, item);

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_627), _ItemData.GetItemLevel() * 2 + 80); // Apparel Effectiveness
			PushHint(buf, GENERIC_COLOR);
		}

		AddHintHeight();

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			if (_pItem->sNeedLv > pEquipItem->sNeedLv) {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			} else {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), pEquipItem->sNeedLv, pAttr->get(ATTR_LV) >= pEquipItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			}
		} else {
			PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
		}

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_ShowFusionBody(pEquipItem);
			_ShowFusionWork(_pItem, pEquipItem, pAttr);
		} else {
			_ShowBody();
			_ShowWork(_pItem, pAttr);
		}
	} else if (_pItem->IsBoot()) {
		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) // end
		{
			_snprintf_s(buf, _TRUNCATE, "Lv%d %s", _ItemData.GetItemLevel(), GetName());
			PushHint(buf, COLOR_WHITE, 5, 1);
		} else {
			PushHint(GetName(), COLOR_WHITE, 5, 1);
		}

		// "Basic class: defense, durability"
		AddHintHeight();
		_PushValue(RES_STRING(CL_LANGUAGE_MATCH_629), ITEMATTR_VAL_DEF, item);

		if (!isStore) // Durability
		{
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_626), item.sEndure[0], item.sEndure[1]);
			PushHint(buf);
		}

		_PushValue(RES_STRING(CL_LANGUAGE_MATCH_632), ITEMATTR_VAL_FLEE, item);

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_627), _ItemData.GetItemLevel() * 2 + 80); // Apparel Effectivness
			PushHint(buf, GENERIC_COLOR);
		}

		AddHintHeight();

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			if (_pItem->sNeedLv > pEquipItem->sNeedLv) {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			} else {
				PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), pEquipItem->sNeedLv, pAttr->get(ATTR_LV) >= pEquipItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
			}
		} else {
			PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);
		}

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_ShowFusionBody(pEquipItem);
			_ShowFusionWork(_pItem, pEquipItem, pAttr);
		} else {
			_ShowBody();
			_ShowWork(_pItem, pAttr);
		}
	} else if (_pItem->IsHair()) {
		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_snprintf_s(buf, _TRUNCATE, "Lv%d %s", _ItemData.GetItemLevel(), GetName());
			PushHint(buf, COLOR_WHITE, 5, 1);
		} else {
			PushHint(GetName(), COLOR_WHITE, 5, 1);
		}

		// Basic class: defense, durability
		AddHintHeight();
		_PushValue(RES_STRING(CL_LANGUAGE_MATCH_629), ITEMATTR_VAL_DEF, item);

		if (!isStore) // Durability
		{
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_626), item.sEndure[0], item.sEndure[1]);
			PushHint(buf);
		}

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_627), _ItemData.GetItemLevel() * 2 + 80); // Apparel effectivness
			PushHint(buf, GENERIC_COLOR);
		}

		AddHintHeight();

		// "Has been fused => Take the highest level of the two pieces of equipment before the fusion, the intersection of the two pieces of equipment, the occupation of the two pieces of equipment"
		// "no fusion => directly take the current equipment level, role, occupation"
		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			short nFusionItemLevel = _pItem->sNeedLv > pEquipItem->sNeedLv ? _pItem->sNeedLv : pEquipItem->sNeedLv;
			PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), nFusionItemLevel, pAttr->get(ATTR_LV) >= nFusionItemLevel ? GENERIC_COLOR : VALID_COLOR);

			_ShowFusionBody(pEquipItem);
			_ShowFusionWork(pEquipItem, _pItem, pAttr);
		} else {
			PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);

			_ShowBody();
			_ShowWork(_pItem, pAttr);
		}
	} else if (_pItem->IsConsumable()) { // "Use products: recovery class, bonus class, special effects class"

		SetHintIsCenter(false);

		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		if (5786 == _pItem->lID || 5787 == _pItem->lID || 5788 == _pItem->lID || 5789 == _pItem->lID) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_644), item.sEndure[0], item.sEndure[1]);
			PushHint(buf);
		}

		if (_ItemData.sNum > 0) {
			AddHintHeight();
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_633), _ItemData.sNum);
			PushHint(buf);
		}

		_AddDescriptor();
	}

	else if (_pItem->IsMission()) {
		PushHint(_pItem->szName, COLOR_BLUE, 5, 1);

		SetHintIsCenter(false);
		_AddDescriptor();
	} else if (_pItem->sType == EItemType::LifeSkillAxe || _pItem->sType == EItemType::LifeSkillPickAxe) {
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		if (_pItem->nID == 3908 || _pItem->nID == 3108) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_626), item.sEndure[0], item.sEndure[1]);
			PushHint(buf, GENERIC_COLOR);
		}

		if (CItemRecord::IsVaildFusionID(_pItem) && _ItemData.GetFusionItemID() > 0) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_627), _ItemData.GetItemLevel() * 2 + 80); // Apparel effectinvess
			PushHint(buf, GENERIC_COLOR);
		}

		PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);

		_AddDescriptor();
	} else if (_pItem->sType == EItemType::Boat) {
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		if (_pBoatHint) {
			PushHint(_pBoatHint->szName);

			xShipInfo* pInfo = GetShipInfo(_pBoatHint->sBoatID);

			if (pInfo) {
				int nNeedLv = pInfo->sLvLimit;
				if (nNeedLv > 0) {
					_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_628), nNeedLv);
					PushHint(buf, g_stUIBoat.GetHuman()->getGameAttr()->get(ATTR_LV) >= nNeedLv ? GENERIC_COLOR : VALID_COLOR);
				}
			}

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_634), _pBoatHint->sLevel);
			PushHint(buf);

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_635), _pBoatHint->dwExp);
			PushHint(buf);

			AddHintHeight();

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_636), _pBoatHint->dwHp, (int)(_pBoatHint->dwMaxHp));
			PushHint(buf);

			if (pInfo) {
				_ShowWork(pInfo, g_stUIBoat.GetHuman()->getGameAttr());
			}

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_637), _pBoatHint->dwSp, (int)(_pBoatHint->dwMaxSp));
			PushHint(buf);

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_638), _pBoatHint->dwMinAttack, (int)(_pBoatHint->dwMaxAttack));
			PushHint(buf);

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_639), _pBoatHint->dwDef);
			PushHint(buf);

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_640), _pBoatHint->dwSpeed);
			PushHint(buf);

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_641), _pBoatHint->dwShootSpeed);
			PushHint(buf);

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_642), _pBoatHint->byHasItem, _pBoatHint->byCapacity);
			PushHint(buf);

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_643), StringSplitNum(_pBoatHint->dwPrice / 2));
			PushHint(buf);
		} else {
			CBoat* pBoat = g_stUIBoat.FindBoat(_ItemData.GetDBParam(enumITEMDBP_INST_ID));
			if (pBoat) {
				CCharacter* pCha = pBoat->GetCha();
				PushHint(pCha->getName());

				int nNeedLv = pCha->GetShipInfo()->sLvLimit;
				if (nNeedLv > 0) {
					_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_628), nNeedLv);
					PushHint(buf, g_stUIBoat.GetHuman()->getGameAttr()->get(ATTR_LV) >= nNeedLv ? GENERIC_COLOR : VALID_COLOR);
				}

				SGameAttr* pAttr = pCha->getGameAttr();
				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_634), pAttr->get(ATTR_LV));
				PushHint(buf);

				char buff[32];
				_i64toa_s(pAttr->get(ATTR_CEXP), buff, _countof(buff), 10);
				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_635), buff);
				PushHint(buf);

				AddHintHeight();

				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_636), (int)pAttr->get(ATTR_HP), (int)pAttr->get(ATTR_MXHP));
				PushHint(buf);

				_ShowWork(pCha->GetShipInfo(), g_stUIBoat.GetHuman()->getGameAttr());

				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_637), (int)pAttr->get(ATTR_SP), (int)pAttr->get(ATTR_MXSP));
				PushHint(buf);

				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_638), (int)pAttr->get(ATTR_BMNATK), (int)pAttr->get(ATTR_BMXATK));
				PushHint(buf);

				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_639), pAttr->get(ATTR_BDEF));
				PushHint(buf);

				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_640), pAttr->get(ATTR_BMSPD));
				PushHint(buf);

				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_641), pAttr->get(ATTR_BASPD));
				PushHint(buf);

				CGoodsGrid* pGoods = pBoat->GetGoodsGrid();
				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_642), pGoods->GetCurNum(), pGoods->GetMaxNum());
				PushHint(buf);

				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_643), StringSplitNum(pAttr->get(ATTR_BOAT_PRICE) / 2));
				PushHint(buf);
			}
		}

		_AddDescriptor();
		return;
	} else if (_pItem->sType == EItemType::Conch) {
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_628), _pItem->sNeedLv, pAttr->get(ATTR_LV) >= _pItem->sNeedLv ? GENERIC_COLOR : VALID_COLOR);

		_ShowWork(_pItem, pAttr);

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_644), _ItemData.sEnergy[0], _ItemData.sEnergy[1]);
		PushHint(buf);

		_AddDescriptor();
	} else if (_pItem->sType == EItemType::Trade) { // "Trade certificate"
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_634), _ItemData.sEnergy[0]);

		float fB320 = (float)_ItemData.sEnergy[1];
		float fRate = 0.0f;
		if (_ItemData.sEnergy[1] == 0) {
			fRate = 30.0f;
		} else {
			fRate = max(0.0f, 30 - pow(fB320, 0.5f)) + pow(fB320, -0.5f);

			if (fRate > 30.0f)
				fRate = 30.0f;
			if (fRate < 0.0f)
				fRate = 0.0f;
		}
		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_645), fRate);
		PushHint(buf);
	} else if (_pItem->sType == EItemType::SkillBook) {
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		CSkillRecord* pSkill = GetSkillRecordInfo(_pItem->szName);
		if (pSkill) {
			if (pSkill->chJobSelect[0][0] >= 0) {
				std::ostrstream str;
				str << RES_STRING(CL_LANGUAGE_MATCH_646);

				for (char i = 0; i < defSKILL_JOB_SELECT_NUM; i++) {
					if (pSkill->chJobSelect[i][0] < 0)
						break;

					if (i > 0 && (i % 2) == 0) {
						str << RES_STRING(CL_LANGUAGE_MATCH_647);
					}
					str << g_GetJobName(pSkill->chJobSelect[i][0]);
					str << " ";
				}
				str << '\0';

				PushHint(str.str(), pSkill->IsJobAllow(pAttr->get(ATTR_JOB)) ? GENERIC_COLOR : VALID_COLOR);
			}

			if (pSkill->sLevelDemand != -1) {
				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_648), pSkill->sLevelDemand);
				PushHint(buf, pAttr->get(ATTR_LV) >= pSkill->sLevelDemand ? GENERIC_COLOR : VALID_COLOR);
			}

			CSkillRecord* p = nullptr;
			CSkillRecord* pSelfSkill = nullptr;
			for (int i = 0; i < defSKILL_PRE_SKILL_NUM; i++) {
				if (pSkill->sPremissSkill[i][0] < 0)
					break;

				p = GetSkillRecordInfo(pSkill->sPremissSkill[i][0]);
				if (p) {
					pSelfSkill = g_stUIEquip.FindSkill(p->nID);
					_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_649), p->szName, pSkill->sPremissSkill[i][1]);
					if (pSelfSkill && pSelfSkill->GetSkillGrid().chLv >= pSkill->sPremissSkill[i][1])
						PushHint(buf);
					else
						PushHint(buf, VALID_COLOR);
				}
			}
		} else {
			_AddDescriptor();
		}
	} else if (_pItem->sType == EItemType::Bravery) {
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		int show_attr[] = {ITEMATTR_VAL_STR, ITEMATTR_VAL_AGI, ITEMATTR_VAL_DEX, ITEMATTR_VAL_CON, ITEMATTR_VAL_STA};
		std::string show_text[] = {RES_STRING(CL_LANGUAGE_MATCH_650), RES_STRING(CL_LANGUAGE_MATCH_651), RES_STRING(CL_LANGUAGE_MATCH_652), RES_STRING(CL_LANGUAGE_MATCH_653), RES_STRING(CL_LANGUAGE_MATCH_654)};
		int value = 0;
		const int count = sizeof(show_attr) / sizeof(show_attr[0]);
		for (int i = 0; i < count; i++) {
			value = item.sInstAttr[show_attr[i]];
			item.sInstAttr[show_attr[i]] = 0;
			_snprintf_s(buf, _TRUNCATE, "%s:%d", show_text[i].c_str(), value);
			PushHint(buf, GENERIC_COLOR);
		}

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_655), _ItemData.sEndure[1]);
		PushHint(buf, GENERIC_COLOR);

		_snprintf_s(buf, _TRUNCATE, "%s:%d", RES_STRING(CL_LANGUAGE_MATCH_848), _ItemData.sEnergy[1]); // "Fighting points"
		PushHint(buf, GENERIC_COLOR);

		_AddDescriptor();
	} else if (_pItem->sType == EItemType::GemStone) {
		//TODO: Enable this on Chinese systems.
		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_656), ConvertNumToChinese(item.sEnergy[1]).c_str(), _pItem->szName);
		PushHint(buf, COLOR_WHITE, 5, 1);
		PushHint(GetStoneHint(1).c_str()); // 单个宝石都显示1级的属性
		_AddDescriptor();
	} else if (_pItem->sType == EItemType::RefiningStone) {
		//TODO: Enable this on Chinese systems.
		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_656), ConvertNumToChinese(item.sEnergy[1]).c_str(), _pItem->szName);
		PushHint(buf, COLOR_WHITE, 5, 1);
		_AddDescriptor();
	}
	//	else if( EItemType::Pet==_pItem->sType )
	else if (_pItem->sType == EItemType::Pet || _pItem->sType == EItemType::PetEgg) {
		const int nLevel = item.sInstAttr[ITEMATTR_VAL_STR] + item.sInstAttr[ITEMATTR_VAL_AGI] +
						   item.sInstAttr[ITEMATTR_VAL_DEX] + item.sInstAttr[ITEMATTR_VAL_CON] +
						   item.sInstAttr[ITEMATTR_VAL_STA] + item.sInstAttr[ITEMATTR_VAL_LUK];

		_snprintf_s(buf, _TRUNCATE, "Lv%d %s", nLevel, GetName());
		PushHint(buf, COLOR_WHITE, 5, 1);

		AddHintHeight();

		PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_657), item.sInstAttr[ITEMATTR_VAL_STR]);
		PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_658), item.sInstAttr[ITEMATTR_VAL_AGI]);
		PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_659), item.sInstAttr[ITEMATTR_VAL_CON]);
		PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_660), item.sInstAttr[ITEMATTR_VAL_DEX]);
		PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_661), item.sInstAttr[ITEMATTR_VAL_STA]);
		PUSH_HINT(RES_STRING(CL_LANGUAGE_MATCH_1085), item.sInstAttr[ITEMATTR_VAL_STA]);

		item.sInstAttr[ITEMATTR_VAL_STR] = 0;
		item.sInstAttr[ITEMATTR_VAL_AGI] = 0;
		item.sInstAttr[ITEMATTR_VAL_DEX] = 0;
		item.sInstAttr[ITEMATTR_VAL_CON] = 0;
		item.sInstAttr[ITEMATTR_VAL_STA] = 0;
		item.sInstAttr[ITEMATTR_VAL_LUK] = 0;

		AddHintHeight();

		if (!isStore) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_662), _ItemData.sEndure[0] / 50, _ItemData.sEndure[1] / 50);
			PushHint(buf);

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_663), _ItemData.sEnergy[0], _ItemData.sEnergy[1]);
			PushHint(buf);
		}

		AddHintHeight();

		AddHintHeight();
		for (int i = 0; i < ITEMATTR_CLIENT_MAX; i++) {
			if (item.sInstAttr[i] != 0) {
				_PushItemAttr(i, item, ADVANCED_COLOR);
			}
		}

		int array[3][2] = {0};
		g_pGameApp->GetScriptMgr()->DoString("GetElfSkill", "u-dddddd", _ItemData.lDBParam[0], &array[0][0], &array[0][1], &array[1][0], &array[1][1], &array[2][0], &array[2][1]);

		CElfSkillInfo* pInfo = nullptr;
		for (int i = 0; i < 3; i++) {
			pInfo = GetElfSkillInfo(array[i][0], array[i][1]);
			if (pInfo) {
				PushHint(pInfo->szDataName);
			}
		}
		_AddDescriptor();
		return;
	} else if (_pItem->sType == 65) { // "Black Dragon Altar" - used for more than altar

		if (5724 == _pItem->lID) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_UIITEMCOMMAND_CPP_00001), item.sEnergy[0] / 60);
			PushHint(buf);

			SetHintIsCenter(false);
			_AddDescriptor();

			return;
		}

		if (3279 <= _pItem->lID && _pItem->lID <= 3282 || _pItem->lID == 6370 || _pItem->lID == 6371 || _pItem->lID == 6376 ||
			_pItem->lID == 6377 || _pItem->lID == 6378 || (_pItem->lID >= 5882 && _pItem->lID <= 5893) || _pItem->lID == 5895 ||
			_pItem->lID == 5897 || (_pItem->lID >= 6383 && _pItem->lID <= 6385)) // "Reading system special treatment, nausea"
		{
			PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

			SetHintIsCenter(false);
			_AddDescriptor();

			return;
		}

		SetHintIsCenter(true);

		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);
		//_AddDescriptor();
		AddHintHeight();

		if (_pItem->lID == 2911 || _pItem->lID == 2912 || _pItem->lID == 2952 ||
			_pItem->lID == 3066 || _pItem->lID == 3078) { // "Task prop special treatment"
			int nMonth = 0;
			int nDay = 0;
			int nHour = 0;
			int nMinute = 0;
			int nSecond = 0;

			for (int i = 0; i < 5; ++i) {
				switch (GetData().sInstAttr[i][0]) {
				case ITEMATTR_VAL_STA:
					nMonth = GetData().sInstAttr[i][1];
					break;
				case ITEMATTR_VAL_STR:
					nDay = GetData().sInstAttr[i][1];
					break;
				case ITEMATTR_VAL_CON:
					nHour = GetData().sInstAttr[i][1];
					break;
				case ITEMATTR_VAL_DEX:
					nMinute = GetData().sInstAttr[i][1];
					break;
				case ITEMATTR_VAL_AGI:
					nSecond = GetData().sInstAttr[i][1];
					break;
				}
			}

			if (_pItem->lID == 2911 || _pItem->lID == 2952 || _pItem->lID == 3066 || _pItem->lID == 3078) {
				_snprintf_s(buf, _TRUNCATE, "%s: %d", RES_STRING(CL_LANGUAGE_MATCH_916), nMonth);
				PushHint(buf);

				_snprintf_s(buf, _TRUNCATE, "%s: %d", RES_STRING(CL_LANGUAGE_MATCH_917), nDay);
				PushHint(buf);

				_snprintf_s(buf, _TRUNCATE, "%s: %d", RES_STRING(CL_LANGUAGE_MATCH_918), nHour);
				PushHint(buf);

				_snprintf_s(buf, _TRUNCATE, "%s: %d", RES_STRING(CL_LANGUAGE_MATCH_919), nMinute);
				PushHint(buf);
			}

			if (_pItem->lID != 3066 && _pItem->lID != 3078) {
				_snprintf_s(buf, _TRUNCATE, "%s: %d", RES_STRING(CL_LANGUAGE_MATCH_920), nSecond);
				PushHint(buf);
			}

			AddHintHeight();
			_AddDescriptor();
			AddHintHeight();

			return;
		} else if (_pItem->lID == 2954) { // "Special item handling (death certificate)"
			int nCount = 0;
			for (int i = 0; i < 5; ++i) {
				if (GetData().sInstAttr[i][0] == ITEMATTR_VAL_STR) {
					nCount = GetData().sInstAttr[i][1];
					break;
				}
			}

			_snprintf_s(buf, _TRUNCATE, "%s: %d", RES_STRING(CL_LANGUAGE_MATCH_933), nCount); // "Number of deaths"
			PushHint(buf);

			AddHintHeight();
			_AddDescriptor();
			AddHintHeight();

			return;
		} else if (_pItem->lID == 579) { // "Admission certificate Display special treatment as ordinary props"
			SetHintIsCenter(false);
			_AddDescriptor();

			return;
		} else if (_pItem->nID == 5803 || _pItem->nID == 6373) {
			if (_pItem->nID == 5803) {
				_snprintf_s(buf, _TRUNCATE, "%s:%d", RES_STRING(CL_LANGUAGE_MATCH_651), item.sInstAttr[ITEMATTR_VAL_STR]);
			}
			if (_pItem->nID == 6373) {
				int nCount = 0;
				for (int i = 0; i < 5; ++i) {
					if (GetData().sInstAttr[i][0] == ITEMATTR_VAL_STR) {
						nCount = GetData().sInstAttr[i][1];
						break;
					}
				}

				_snprintf_s(buf, _TRUNCATE, "存储时间为：%d", nCount);
			}

			PushHint(buf, GENERIC_COLOR);
			_AddDescriptor();

			return;
		}

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_664), 5 - item.sInstAttr[ITEMATTR_VAL_AGI]);
		PushHint(buf);

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_665), 5 - item.sInstAttr[ITEMATTR_VAL_STR]);
		PushHint(buf);

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_666), 5 - item.sInstAttr[ITEMATTR_VAL_DEX]);
		PushHint(buf);

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_667), 5 - item.sInstAttr[ITEMATTR_VAL_CON]);
		PushHint(buf);

		AddHintHeight();

		switch (item.sInstAttr[ITEMATTR_VAL_STA]) {
		case 1: // item.sID = 866
			PushHint(RES_STRING(CL_LANGUAGE_MATCH_668), COLOR_RED);
			break;

		case 2: // item.sID = 865
			PushHint(RES_STRING(CL_LANGUAGE_MATCH_669), COLOR_RED);
			break;

		case 3: // item.sID = 864
			PushHint(RES_STRING(CL_LANGUAGE_MATCH_670), COLOR_RED);
			break;

		default:
			PushHint(RES_STRING(CL_LANGUAGE_MATCH_671), COLOR_RED);
			break;
		}

		AddHintHeight();

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_672), _ItemData.sEnergy[0]);
		PushHint(buf);

		return;
	} else if (_pItem->sType == EItemType::Blueprint) {
		int iItem = 0;
		long lForge = 0;
		CItemRecord* pCItemRec = nullptr;

		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_869), _ItemData.sEndure[0]);
		PushHint(buf, GENERIC_COLOR);

		iItem = item.sInstAttr[ITEMATTR_VAL_AGI];
		if (iItem) {
			pCItemRec = GetItemRecordInfo(iItem);
			if (pCItemRec) {
				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_870), pCItemRec->szName);
				PushHint(buf, GENERIC_COLOR);
			}
		}

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_871), _ItemData.sEnergy[1] - 100);
		PushHint(buf, GENERIC_COLOR);

		AddHintHeight();

		lForge = _ItemData.GetForgeParam();

		iItem = item.sInstAttr[ITEMATTR_VAL_STR];
		if (iItem) {
			pCItemRec = GetItemRecordInfo(iItem);
			if (pCItemRec) {
				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_872), pCItemRec->szName, (lForge / 10000000));
				PushHint(buf, GENERIC_COLOR);
			}
		}

		lForge %= 10000000;
		iItem = item.sInstAttr[ITEMATTR_VAL_CON];
		if (iItem) {
			pCItemRec = GetItemRecordInfo(iItem);
			if (pCItemRec) {
				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_873), pCItemRec->szName, (lForge / 10000));
				PushHint(buf, GENERIC_COLOR);
			}
		}

		lForge %= 1000;
		iItem = item.sInstAttr[ITEMATTR_VAL_DEX];
		if (iItem) {
			pCItemRec = GetItemRecordInfo(iItem);
			if (pCItemRec) {
				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_874), pCItemRec->szName, (lForge / 10));
				PushHint(buf, GENERIC_COLOR);
			}
		}

		AddHintHeight();

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_875), item.sInstAttr[ITEMATTR_VAL_STA]);
		PushHint(buf, GENERIC_COLOR);

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_876), _ItemData.sEnergy[0] * 10);
		PushHint(buf, GENERIC_COLOR);

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_877), _ItemData.sEndure[1]);
		PushHint(buf, GENERIC_COLOR);

		AddHintHeight();

		if (_nPrice != 0) {
			AddHintHeight();
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_674), StringSplitNum(isMain ? _nPrice / 2 : _nPrice));
			PushHint(buf, COLOR_WHITE);

			//TODO: Enable this on Chinese systems.
			//_snprintf_s( buf, _TRUNCATE, "%s", ConvertNumToChinese( isMain ? _nPrice / 2 : _nPrice ).c_str() );
			//PushHint( buf, COLOR_WHITE );
		}

		return;
	} else if (_pItem->sType == EItemType::LifeSkillTool) {
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		if (_pItem->lID == 2902 || _pItem->lID == 2903) //"Princess love, prince love"
		{
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_909), item.sInstAttr[ITEMATTR_VAL_STR]); // ""Love Code: %d""
			PushHint(buf, GENERIC_COLOR);

			AddHintHeight();
			_AddDescriptor();
			AddHintHeight();

			return;
		}

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_869), item.sInstAttr[ITEMATTR_VAL_STR]);
		PushHint(buf, GENERIC_COLOR);

		if (_pItem->lID != 2236) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_878), _ItemData.sEndure[0] / 50);
			PushHint(buf, GENERIC_COLOR);

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_897), _ItemData.sEnergy[0]); // ""Tool experience: %i""
			PushHint(buf, GENERIC_COLOR);
		}

		AddHintHeight();

		_AddDescriptor();

		AddHintHeight();

		if (_nPrice != 0) {
			AddHintHeight();
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_674), StringSplitNum(isMain ? _nPrice / 2 : _nPrice));
			PushHint(buf, COLOR_WHITE);

			//TODO: Enable this on Chinese systems.
			//_snprintf_s( buf, _TRUNCATE, "%s", ConvertNumToChinese( isMain ? _nPrice / 2 : _nPrice ).c_str() );
			//PushHint( buf, COLOR_WHITE );
		}

		return;
	} else if (_pItem->sType == 71 && _pItem->lID == 3010) {
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_644), _ItemData.sEnergy[0], _ItemData.sEnergy[1]);
		PushHint(buf);

		SetHintIsCenter(true);
		_AddDescriptor();
	} else if (_pItem->sType == 71 && _pItem->lID == 3289) { //"Student card processing"
		// "Student card special treatment (extremely disgusting!!)"
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		int nLevel = item.chForgeLv;
		const char* arShowName[5] = {RES_STRING(CL_LANGUAGE_MATCH_944), RES_STRING(CL_LANGUAGE_MATCH_945), RES_STRING(CL_LANGUAGE_MATCH_946), RES_STRING(CL_LANGUAGE_MATCH_947), RES_STRING(CL_LANGUAGE_MATCH_948)}; // ""Kindergarten", "Primary School", "Junior High School", "High School", "University" };"
		if (0 <= nLevel && nLevel <= 4) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_949), arShowName[nLevel]); // "Education: %s"
			PushHint(buf, COLOR_WHITE, 5, 1);
		}

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_950), item.sEndure[0], item.sEndure[1]); // ""Credits (%d/%d)""
		PushHint(buf, COLOR_WHITE, 5, 1);

		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_951), item.sEnergy[0] * 1000, item.sEnergy[1] * 1000); // ""Experience (%d/%d)""
		PushHint(buf, COLOR_WHITE, 5, 1);

		return;
	} else if (_pItem->sType == 71 && _pItem->lID == 6377) { // "Cursed doll treatment."

		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		SetHintIsCenter(false);
		_AddDescriptor();

		return;
	} else if (_pItem->sType == 41) {
		if (_pItem->lID == 58) { //"Crazy woman asks for special treatment, crab seedlings"
			PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

			//_snprintf_s( buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_626), item.sEndure[0] * 1000, item.sEndure[1] * 1000 );
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_626), item.sEnergy[0], item.sEnergy[1]);
			PushHint(buf, GENERIC_COLOR);

			SetHintIsCenter(true);
			_AddDescriptor();

			return;
		} else if (_pItem->lID == 171) { // "Bragi requires special handling"
			PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

			static const char* pszText[5] = {RES_STRING(CL_LANGUAGE_MATCH_952),
											 RES_STRING(CL_LANGUAGE_MATCH_953),
											 RES_STRING(CL_LANGUAGE_MATCH_954),
											 RES_STRING(CL_LANGUAGE_MATCH_955),
											 RES_STRING(CL_LANGUAGE_MATCH_956)};

			int nIndex = item.sEndure[0];

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_957), 0 <= nIndex && nIndex <= 4 ? pszText[nIndex] : "Not Valid");
			PushHint(buf, GENERIC_COLOR);

			SetHintIsCenter(true);
			_AddDescriptor();

			return;
		} else if (_pItem->lID == 2967) { // "Stables require special treatment, red wine"
			PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_626), item.sEnergy[0], item.sEnergy[1]);
			PushHint(buf, GENERIC_COLOR);

			SetHintIsCenter(true);
			_AddDescriptor();

			return;
		} else if (_pItem->lID == 6066) {
			PushHint(_pItem->szName, COLOR_WHITE, 5, 1);
			_snprintf_s(buf, _TRUNCATE, "%s:%d", RES_STRING(CL_UIITEMCOMMAND_CPP_00002), item.sInstAttr[ITEMATTR_VAL_STR]);
			PushHint(buf, GENERIC_COLOR);
			_snprintf_s(buf, _TRUNCATE, "%s:%d", RES_STRING(CL_UIITEMCOMMAND_CPP_00003), item.sInstAttr[ITEMATTR_VAL_AGI]);
			PushHint(buf, GENERIC_COLOR);
			_snprintf_s(buf, _TRUNCATE, "%s:%d", RES_STRING(CL_UIITEMCOMMAND_CPP_00004), item.sInstAttr[ITEMATTR_VAL_DEX]);
			PushHint(buf, GENERIC_COLOR);
			////_snprintf_s(buf, _TRUNCATE,"%s:%d","场次编号",item.sInstAttr[ITEMATTR_VAL_AGI]);
			//_snprintf_s(buf, _TRUNCATE,"%s:%d",RES_STRING(CL_UIITEMCOMMAND_CPP_00005),item.sInstAttr[ITEMATTR_VAL_CON]);
			//PushHint( buf, GENERIC_COLOR );
			return;
		} else { // "general props"
			PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

			SetHintIsCenter(false);
			_AddDescriptor();
		}
	} else if (_pItem->sType == 75) { // "Lottery Ticket"
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		SetHintIsCenter(false);

		//int nLevel = item.sInstAttr[ITEMATTR_VAL_STR]
		//			+ item.sInstAttr[ITEMATTR_VAL_AGI]
		//			+ item.sInstAttr[ITEMATTR_VAL_DEX]
		//			+ item.sInstAttr[ITEMATTR_VAL_CON]
		//			+ item.sInstAttr[ITEMATTR_VAL_STA];
		if (floor((float)item.sInstAttr[ITEMATTR_VAL_STR] / 1000) > 0) {
			//_snprintf_s( buf, _TRUNCATE, "期号：%03d", item.sInstAttr[ITEMATTR_VAL_STR] % 1000 );
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_UIITEMCOMMAND_CPP_00006), item.sInstAttr[ITEMATTR_VAL_STR] % 1000);
			PushHint(buf, GENERIC_COLOR);

			//_snprintf_s( buf, _TRUNCATE, "填写日期：%02d号%02d点", (short)floor( (float)item.sInstAttr[ITEMATTR_VAL_AGI] / 100),  item.sInstAttr[ITEMATTR_VAL_AGI] % 100);
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_UIITEMCOMMAND_CPP_00007), (short)floor((float)item.sInstAttr[ITEMATTR_VAL_AGI] / 100), item.sInstAttr[ITEMATTR_VAL_AGI] % 100);
			PushHint(buf, GENERIC_COLOR);

			short c1 = (short)floor((float)item.sInstAttr[ITEMATTR_VAL_DEX] / 100);
			short c2 = item.sInstAttr[ITEMATTR_VAL_DEX] % 100;

			short c3 = (short)floor((float)item.sInstAttr[ITEMATTR_VAL_CON] / 100);
			short c4 = item.sInstAttr[ITEMATTR_VAL_CON] % 100;

			short c5 = (short)floor((float)item.sInstAttr[ITEMATTR_VAL_STA] / 100);
			short c6 = item.sInstAttr[ITEMATTR_VAL_STA] % 100;

			//_snprintf_s( buf, _TRUNCATE, "彩球号：%c %c %c %c %c %c", c1, c2, c3, c4, c5, c6);
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_UIITEMCOMMAND_CPP_00008), c1, c2, c3, c4, c5, c6);
			PushHint(buf, GENERIC_COLOR);
		} else
			//PushHint( "未填写完毕" );
			PushHint(RES_STRING(CL_UIITEMCOMMAND_CPP_00009));

		_AddDescriptor();

		return;
	} else // "General props"
	{
		PushHint(_pItem->szName, COLOR_WHITE, 5, 1);

		SetHintIsCenter(false);
		_AddDescriptor();
	}

	if (_pItem->sType != 75) {
		//"Strengthen, special reinforcement"
		AddHintHeight();
		for (int i = 0; i < ITEMATTR_CLIENT_MAX; i++) {
			if (item.sInstAttr[i] != 0) {
				_PushItemAttr(i, item, ADVANCED_COLOR);
			}
		}
	}

	if (_hints.GetCount() > 0 && _pItem->sType <= 27 &&
		_pItem->sType != EItemType::LifeSkillAxe &&
		_pItem->sType != EItemType::LifeSkillPickAxe &&
		_pItem->sType != EItemType::Face) {
		// "The hundredth of the energy maximum represents the color, and the ten digits and below represent the name prefix."
		char szBuf[16] = {0};
		_snprintf_s(szBuf, _TRUNCATE, "%09d", _ItemData.sEnergy[1] / 10); // "Except for ten, because the value of the server is 4 digits, wait for later to change to three digits."
		char szHundred[2] = {szBuf[6], 0};
		int nHundred = atoi(szHundred);
		int nTen = atoi(&szBuf[7]);

		DWORD dwNameColor = COLOR_BLACK;
		switch (nHundred) {
		case 0:
			dwNameColor = 0xffC1C1C1;
			break;
		case 1:
			dwNameColor = 0xffFFFFFF;
			break;
		case 2:
			dwNameColor = 0xffFFFFFF;
			break;
		case 3:
			dwNameColor = 0xffA2E13E;
			break;
		case 4:
			dwNameColor = 0xffA2E13E;
			break;
		case 5:
			dwNameColor = 0xffd68aff;
			break;
		case 6:
			dwNameColor = 0xffd68aff;
			break;
		case 7:
			dwNameColor = 0xffff6440;
			break;
		case 8:
			dwNameColor = 0xffff6440;
			break;
		case 9:
			dwNameColor = 0xffffcc12;
			break;
		}

		CItemPreInfo* pInfo = GetItemPreInfo(nTen);
		if (pInfo && strcmp(pInfo->szDataName, "0") != 0) {
			if (CItemRecord::IsVaildFusionID(_pItem)) {
				_snprintf_s(buf, _TRUNCATE, "Lv%d %s%s", _ItemData.GetItemLevel(), pInfo->szDataName, GetName());

				//	"Increase the handling of item locks."
				if (_ItemData.dwDBID) {
					_hints.GetHint(1)->hint = buf;
				} else {
					_hints.GetHint(0)->hint = buf;
				}
			} else {
				//	"Increase the handling of item locks."
				if (_ItemData.dwDBID) {
					_hints.GetHint(1)->hint = pInfo->szDataName + _hints.GetHint(1)->hint;
				} else {
					_hints.GetHint(0)->hint = pInfo->szDataName + _hints.GetHint(0)->hint;
				}
			}
		}

		//"Increase the handling of item locks."
		if (_ItemData.dwDBID) {
			_hints.GetHint(1)->color = dwNameColor;
		} else {
			_hints.GetHint(0)->color = dwNameColor;
		}
	}

	// "Processing refining"
	SItemForge& Forge = GetForgeInfo();
	if (_hints.GetCount() > 0 && Forge.IsForge) {
		if (Forge.nHoleNum > 0) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_673), Forge.nHoleNum);
			PushHint(buf, ADVANCED_COLOR);
		}

		//TODO: Enable this on Chinese systems.
		for (int i = 0; i < Forge.nStoneNum && i < Forge.nHoleNum; i++) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_656), ConvertNumToChinese(Forge.nStoneLevel[i]).c_str(), Forge.pStoneInfo[i]->szDataName);
			PushHint(buf, ADVANCED_COLOR);
		}

		if (Forge.nStoneNum > 0) {
			AddHintHeight();
			for (int i = 0; i < Forge.nStoneNum && i < Forge.nHoleNum; i++) {
				PushHint(Forge.szStoneHint[i], ADVANCED_COLOR);
			}
		}

		if (Forge.nLevel > 0) {
			if (_ItemData.dwDBID) {
				_snprintf_s(buf, _TRUNCATE, "%s +%d", _hints.GetHint(1)->hint.c_str(), Forge.nLevel);
				_hints.GetHint(1)->hint = buf;
			} else {
				_snprintf_s(buf, _TRUNCATE, "%s +%d", _hints.GetHint(0)->hint.c_str(), Forge.nLevel);
				_hints.GetHint(0)->hint = buf;
			}
		}
	}

	if (_nPrice != 0) {
		AddHintHeight();
		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_674), StringSplitNum(isMain ? _nPrice / 2 : _nPrice));
		PushHint(buf, COLOR_WHITE);

		//TODO: Enable this on Chinese systems.
		//_snprintf_s( buf, _TRUNCATE, "%s", ConvertNumToChinese( isMain ? _nPrice / 2 : _nPrice ).c_str() );
		//PushHint( buf, COLOR_WHITE );
	}

	if (_ItemData.dwDBID) {
		_hints.GetHint(0)->color = 0xff888888;
	}
}

std::string CItemCommand::GetStoneHint(int nLevel) {
	std::string hint = "error";
	if (_pItem->sType == EItemType::GemStone) {
		CStoneInfo* pInfo = nullptr;
		int nCount = CStoneSet::I()->GetLastID() + 1;
		for (int i = 0; i < nCount; i++) {
			pInfo = ::GetStoneInfo(i);
			if (!pInfo)
				continue;

			if (pInfo->nItemID != _pItem->nID)
				continue;

			if (nLevel < 0)
				g_pGameApp->GetScriptMgr()->DoString(pInfo->szHintFunc, "u-s", _ItemData.sEnergy[1], &hint);
			else
				g_pGameApp->GetScriptMgr()->DoString(pInfo->szHintFunc, "u-s", 1, &hint);
			return hint;
		}
	}
	return hint;
}

//void CItemCommand::_PushItemAttr( int attr, SItemGrid& item, DWORD color )
//{
//    for( int i=0; i<defITEM_INSTANCE_ATTR_NUM; i++ )
//    {
//        if( item.sInstAttr[i][0]==attr )
//        {
//			if ( item.sInstAttr[i][1]==0 )
//			{
//				return;
//			}
//			else
//			{
//				if( attr <= ITEMATTR_COE_PDEF )
//				{
//					if( !(item.sInstAttr[i][1] % 10) )
//					{
//						_snprintf_s( buf, _TRUNCATE, "%s:%+d%%", g_GetItemAttrExplain( item.sInstAttr[i][0]), item.sInstAttr[i][1] / 10 );
//					}
//					else
//					{
//						float f = (float)item.sInstAttr[i][1] / 10.0f;
//						_snprintf_s( buf, _TRUNCATE, "%s:%+.1f%%", g_GetItemAttrExplain( item.sInstAttr[i][0]), f );
//					}
//				}
//				else
//				{
//					_snprintf_s( buf, _TRUNCATE, "%s:%+d", g_GetItemAttrExplain( item.sInstAttr[i][0]), item.sInstAttr[i][1] );
//				}
//				PushHint( buf, color );
//
//				item.sInstAttr[i][0] = 0;
//				return;
//			}
//        }
//    }
//}

void CItemCommand::_PushValue(const char* szFormat, int attr, SItemHint& item, DWORD color) {
	if (attr <= 0 || attr >= ITEMATTR_CLIENT_MAX)
		return;

	if (item.sInstAttr[attr] == 0)
		return;

	PUSH_HINT(szFormat, item.sInstAttr[attr], color);
	item.sInstAttr[attr] = 0;
}

void CItemCommand::_PushItemAttr(int attr, SItemHint& item, DWORD color) {
	if (attr <= 0 || attr >= ITEMATTR_CLIENT_MAX)
		return;

	if (item.sInstAttr[attr] == 0)
		return;

	if (attr <= ITEMATTR_COE_PDEF) {
		if (!(item.sInstAttr[attr] % 10)) {
			_snprintf_s(buf, _TRUNCATE, "%s:%+d%%", g_GetItemAttrExplain(attr), item.sInstAttr[attr] / 10);
		} else {
			float f = (float)item.sInstAttr[attr] / 10.0f;
			_snprintf_s(buf, _TRUNCATE, "%s:%+.1f%%", g_GetItemAttrExplain(attr), f);
		}
	} else {
		_snprintf_s(buf, _TRUNCATE, "%s:%+d", g_GetItemAttrExplain(attr), item.sInstAttr[attr]);
	}
	PushHint(buf, color);

	item.sInstAttr[attr] = 0;
}

bool CItemCommand::IsDragFast() const {
	return true;  //return (_pItem->sType >= 31 && _pItem->sType <= 33) || _pItem->sType == 71; //Kept it here to remind of old constraints
}

void CItemCommand::SetTotalNum(int num) {
	if (_pItem->GetIsPile()) {
		if (num >= 0) {
			_ItemData.sNum = num;
		}
	} else {
		_ItemData.sNum = 1;
	}
}

bool CItemCommand::IsAllowThrow() const {
	return _pItem->chIsThrow != 0;
}

//	2008-9-17	yangyinyu	add	begin!
bool _wait_select_lock_item_state_ = false; //	用户已经使用锁道具，正在等待选择被锁定的道具。
static char _lock_pos_;
static long _lock_item_id_;
static long _lock_grid_id_;
static long _lock_fusion_item_id_;

extern bool g_yyy_add_lock_item_wait_return_state; //	已经发送锁定消息，等待服务器返回锁定结果状态。

static void _evtSelectYesNoEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	//	关闭选择框。
	pSender->GetForm()->Close();

	//	取组件名字。
	std::string name = pSender->GetName();

	if (!strcmp(name.c_str(), "btnYes")) {
		//	发送加锁信息。
		CS_DropLock(_lock_pos_, _lock_item_id_, _lock_grid_id_, _lock_fusion_item_id_);
		//	设置等待光标。
		CCursor::I()->SetCursor(CCursor::stWait);
		//	设置等待服务器返回锁定结果状态。
		g_yyy_add_lock_item_wait_return_state = true;
	}

	//pBox->frmDialog->SetParent(	NULL );
};

#include "uiboxform.h"
extern CBoxMgr g_stUIBox;
//	2008-9-17	yangyinyu	add	end!

bool CItemCommand::MouseDown() {
	CCharacter* pCha = CGameScene::GetMainCha();
	if (!pCha)
		return false;

	//	2008-9-17	yangyinyu	add	begin!

	//	如果是等待锁定消息返回状态，则直接跳过。
	if (g_yyy_add_lock_item_wait_return_state && _ItemData.sID != 5939) {
		return false;
	}
	//	如果是等待被锁定状态。
	else if (_wait_select_lock_item_state_ && _ItemData.sID != 5939) {
		//	发送加锁消息。
		auto* pOwn = dynamic_cast<CGoodsGrid*>(GetParent());

		if (pOwn) {
			if (pOwn == g_stUIEquip.GetGoodsGrid()) {
				//	准备参数。
				_lock_grid_id_ = pOwn->FindCommand(this);
				_lock_fusion_item_id_ = _ItemData.GetFusionItemID();

				//	弹出选择框。
				stSelectBox* pBox = g_stUIBox.ShowSelectBox(_evtSelectYesNoEvent, "道具加锁后将变成不可交易\n、不可丢弃、不可精练、不\n可融合、是否锁定此道具？");
				//	pBox->dwTag = 0;
				//	pBox->dwParam = 0;
				pBox->frmDialog->SetPos(100, 250);
				pBox->frmDialog->SetParent(g_stUIEquip.GetItemForm());
				pBox->frmDialog->Show();
				pBox->frmDialog->Refresh();

				CCursor::I()->SetCursor(CCursor::stNormal);

				/*
				CS_DropLock(	_lock_pos_,	_lock_item_id_,	GridID,	_ItemData.GetFusionItemID()	);
				*/

				//	取消等待被锁定状态。
				_wait_select_lock_item_state_ = false;
			}
		}
	}
	//	2008-9-17	yangyinyu	add	end!

	if (auto* pState = dynamic_cast<CRepairState*>(pCha->GetActor()->GetCurState())) {
		// 如果处于修理道具状态
		if (_pItem->sType >= 31) {
			g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_675), _pItem->szName);
			return false;
		}

		auto* pOwn = dynamic_cast<CGoodsGrid*>(GetParent());
		if (pOwn) {
			if (pOwn == g_stUIEquip.GetGoodsGrid()) {
				int GridID = pOwn->FindCommand(this);
				if (GridID == -1)
					return false;

				CS_ItemRepairAsk(pCha->getAttachID(), pCha->lTag, 2, GridID);
				return true;
			}
		}

		auto* pOne = dynamic_cast<COneCommand*>(GetParent());
		if (pOne) {
			CS_ItemRepairAsk(pCha->getAttachID(), pCha->lTag, 1, pOne->nTag);
			return true;
		}
		return false;
	} else if (auto* pState = dynamic_cast<CFeedState*>(pCha->GetActor()->GetCurState())) {
		// 喂食状态
		if (_pItem->sType != EItemType::Pet) {
			g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_676));
			return false;
		}

		auto* pOwn = dynamic_cast<CGoodsGrid*>(GetParent());
		if (pOwn) {
			if (pOwn == g_stUIEquip.GetGoodsGrid()) {
				int GridID = pOwn->FindCommand(this);
				if (GridID == -1)
					return false;

				stNetUseItem param;
				param.sGridID = pState->GetFeedGridID();
				param.sTarGridID = GridID;
				CS_BeginAction(g_stUIBoat.GetHuman(), enumACTION_ITEM_USE, (void*)&param);

				pState->PopState();
				return true;
			}
		}
		return false;
	} else if (auto* pState = dynamic_cast<CFeteState*>(pCha->GetActor()->GetCurState())) {
		// 祭祀状态  add by Philip.Wu  2006-06-20
		if (_pItem->sType != 65) {
			g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_677));
			return false;
		}

		auto* pOwn = dynamic_cast<CGoodsGrid*>(GetParent());
		if (pOwn) {
			if (pOwn == g_stUIEquip.GetGoodsGrid()) {
				int GridID = pOwn->FindCommand(this);
				if (GridID == -1)
					return false;

				stNetUseItem param;
				param.sGridID = pState->GetFeteGridID();
				param.sTarGridID = GridID;
				CS_BeginAction(g_stUIBoat.GetHuman(), enumACTION_ITEM_USE, (void*)&param);

				pState->PopState();
				return true;
			}
		}
		return false;
	}
	return false;
}

bool CItemCommand::UseCommand() {
	static DWORD dwTime = 0;
	if (CGameApp::GetCurTick() < dwTime) {
		return false;
	}
	dwTime = CGameApp::GetCurTick() + 1000;

	if (!GetIsValid()) {
		return false;
	}

	CCharacter* pCha = CGameScene::GetMainCha();
	if (!pCha) {
		return false;
	}

	if (pCha->GetChaState()->IsFalse(enumChaStateUseItem)) {
		g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_678));
		return false;
	}

	auto pOwn = dynamic_cast<CGoodsGrid*>(GetParent());
	if (pOwn) {
		CCharacter* pCha = g_stUIBoat.FindCha(pOwn);
		if (!pCha) {
			return false;
		}

		const int GridID = pOwn->FindCommand(this);
		if (GridID == -1) {
			return false;
		}

		if (_pItem->sType == EItemType::PetFodder || _pItem->sType == EItemType::PetSock) {
			CActor* pActor = CGameScene::GetMainCha()->GetActor();
			auto pState = std::make_unique<CFeedState>(pActor);
			pState->SetFeedGridID(GridID);
			pActor->SwitchState(std::move(pState));
			return false;
		} else if (_pItem->sType == 66 || (864 <= _pItem->lID && _pItem->lID <= 866)) { // 添加祭祀状态  add by Philip.Wu  2006-06-20
			CActor* pActor = CGameScene::GetMainCha()->GetActor();
			auto pState = std::make_unique<CFeteState>(pActor);
			pState->SetFeteGridID(GridID);
			pActor->SwitchState(std::move(pState));
			return false;
		} else if (_pItem->sType == 71) { // 技能道具使用
			CCharacter* pCha = CGameScene::GetMainCha();
			if (!pCha) {
				return false;
			}

			const int nSkillID = atoi(GetItemInfo()->szAttrEffect);
			CSkillRecord* pSkill = GetSkillRecordInfo(nSkillID);

			// 被动技能，不执行
			if (!pSkill || !pSkill->GetIsUse()) {
				return false;
			}

			// 判断技能能否在海上施放
			if (pCha->IsBoat() && pSkill->chSrcType != 2) {
				g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_879));
				return false;
			}

			// 判断技能能否在陆地上施放
			if (!pCha->IsBoat() && pSkill->chSrcType != 1) {
				g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_880));
				return false;
			}

			if (const auto nCoolDownTime = atoi(pSkill->szFireSpeed); nCoolDownTime > 0) { // "Have cooldown time"
				const DWORD nCurTickCount = g_pGameApp->GetCurTick() - 500;				   // "500 millisecond delay processing"
				if (auto it = _mapCoolDown.find(nSkillID); it != _mapCoolDown.end() &&
														   it->second + nCoolDownTime >= nCurTickCount) {
					g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_898),
										(it->second + nCoolDownTime - nCurTickCount) / 1000 + 1);
					return false;
				}
				_mapCoolDown[nSkillID] = g_pGameApp->GetCurTick(); // "Save the skill usage time of the item"
			}

			// "Free the object or scope without clicking on the skill"
			if (pSkill->GetDistance() <= 0) {
				auto attack = std::make_unique<CAttackState>(pCha->GetActor());
				attack->SetSkill(pSkill);
				attack->SetTarget(pCha);
				attack->SetCommand(this);
				return pCha->GetActor()->SwitchState(std::move(attack));
			}

			//pCha->GetActor()->Stop();
			pCha->ChangeReadySkill(nSkillID);
			return false;
		}

		stNetUseItem param;
		param.sGridID = GridID;
		if ((_pItem->sType < 31 || _ItemData.sID == 5939) && pCha == CGameScene::GetMainCha() && g_stUIBoat.GetHuman() == pCha) {
			//	yyy	add	begin!
			//	如果使用锁道具，则设置选择被锁定道具状态。。
			if (_ItemData.sID == 5939) {
				//	记录锁道具的位置。
				_lock_pos_ = GridID;
				_lock_item_id_ = _ItemData.GetFusionItemID();

				//	设置光标为锁头。
				CCursor::I()->SetCursor(CCursor::stPick);

				//	设置状态。
				_wait_select_lock_item_state_ = true;

				//	返回，继续选择。
				return false;
			};

			//	yyy	add	end!

			CActor* pActor = g_stUIBoat.GetHuman()->GetActor();
			auto pState = std::make_unique<CEquipState>(pActor);
			pState->SetUseItemData(param);
			pActor->SwitchState(std::move(pState));
		} else {
			CS_BeginAction(pCha, enumACTION_ITEM_USE, (void*)&param);
		}
		return false;
	}

	auto pOne = dynamic_cast<COneCommand*>(GetParent());
	if (pOne) {
		g_stUIEquip.UnfixToGrid(this, -1, pOne->nTag);
	}
	return false;
}

//int CItemCommand::_GetValue( int nItemAttrType, SItemGrid& item )
//{
//    for( int i=0; i<defITEM_INSTANCE_ATTR_NUM; i++ )
//    {
//        if( item.sInstAttr[i][0]==nItemAttrType )
//        {
//            item.sInstAttr[i][0] = 0;
//            return item.sInstAttr[i][1];
//        }
//    }
//
//    return -1;
//}

int CItemCommand::_GetValue(int nItemAttrType, SItemHint& item) {
	if (nItemAttrType <= 0 || nItemAttrType >= ITEMATTR_CLIENT_MAX)
		return -1;

	int nValue = 0;
	if (item.sInstAttr[nItemAttrType] != 0) {
		nValue = item.sInstAttr[nItemAttrType];
		item.sInstAttr[nItemAttrType] = 0;
		return nValue;
	}

	return -1;
}

bool CItemCommand::GetIsPile() const {
	return _pItem->GetIsPile();
}

int CItemCommand::GetPrice() const {
	return _nPrice;
}

void CItemCommand::SetData(const SItemGrid& item) {
	memcpy(&_ItemData, &item, sizeof(_ItemData));
	int start = 0;
	for (; start < defITEM_INSTANCE_ATTR_NUM; start++) {
		if (item.sInstAttr[start][0] == 0) {
			break;
		}
	}
	for (int i = start; i < defITEM_INSTANCE_ATTR_NUM; i++) {
		_ItemData.sInstAttr[i][0] = 0;
		_ItemData.sInstAttr[i][1] = 0;
	}
}

int CItemCommand::GetTotalNum() const {
	return _ItemData.sNum;
}

const char* CItemCommand::GetName() const {
	if (_ItemData.chForgeLv == 0) {
		return _pItem->szName;
	} else {
		static char szBuf[128] = {0};
		_snprintf_s(szBuf, _TRUNCATE, "%+d %s", _ItemData.chForgeLv, _pItem->szName);
		return szBuf;
	}
}

void CItemCommand::_ShowWork(CItemRecord* pItem, SGameAttr* pAttr) {
	bool isFind = false;
	bool isSame = false;

	for (int i = 0; i < MAX_JOB_TYPE; i++) {
		if (pItem->szWork[i] < 0)
			break;

		if (!isFind) {
			/*_snprintf_s( buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_679), g_GetJobName(pItem->szWork[i]) );
			isFind = true;*/
			//	Add by alfred.shi 20080928
			std::string name(RES_STRING(CL_UIITEMCOMMAND_CPP_00012));
			g_GetJobName(pItem->szWork[i]);
			if (name.compare(g_GetJobName(pItem->szWork[i])) == 0) {
				_snprintf_s(buf, _TRUNCATE, "%s", RES_STRING(CL_UIITEMCOMMAND_CPP_00013));
			} else {
				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_679), g_GetJobName(pItem->szWork[i]));
			}

			isFind = true;
			//	End.
		}

		if (!isSame) {
			if (pAttr->get(ATTR_JOB) == pItem->szWork[i]) {
				isSame = true;
			}
		}
	}

	if (isFind) {
		PushHint(buf, isSame ? GENERIC_COLOR : VALID_COLOR);
	}
	//else
	//{
	//	PushHint( "职业限制:无", GENERIC_COLOR );
	//}
}

void CItemCommand::_ShowFusionWork(CItemRecord* pAppearItem, CItemRecord* pEquipItem, SGameAttr* pAttr) // 用于显示熔合后道具的职业限制
{
	bool isFind = false;
	int iAppearIndex = -1;
	int iEquipIndex = -1;
	bool isSame = false;
	CItemRecord* pItem(nullptr);

	if (pAppearItem->szWork[0] > pEquipItem->szWork[0]) {
		pItem = pAppearItem;
	} else {
		pItem = pEquipItem;
	}

	for (int i = 0; i < MAX_JOB_TYPE; i++) {
		if (pItem->szWork[i] < 0)
			break;

		if (!isFind) {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_679), g_GetJobName(pItem->szWork[i]));
			isFind = true;
		}

		if (!isSame) {
			if (pAttr->get(ATTR_JOB) == pItem->szWork[i]) {
				isSame = true;
			}
		}
	}

	if (isFind) {
		PushHint(buf, isSame ? GENERIC_COLOR : VALID_COLOR);
	}
	//else
	//{
	//	PushHint( "职业限制:无", GENERIC_COLOR );
	//}
}

void CItemCommand::_ShowWork(xShipInfo* pInfo, SGameAttr* pAttr) {
	bool isFind = false;
	bool isSame = false;

	for (int i = 0; i < MAX_JOB_TYPE; i++) {
		if (pInfo->sPfLimit[i] == (USHORT)-1)
			break;

		/*if( !isFind ) 
		{
			_snprintf_s( buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_679), g_GetJobName(pInfo->sPfLimit[i]) );
			isFind = true;
		}*/
		//add by alfred.shi 20080714	begin
		/*	修改他人代码请加注释，暴发户问题已经修改，后来又出现，代码被人改动，字符串资源也没找到。*/
		if (!isFind) {
			std::string name(RES_STRING(CL_UIITEMCOMMAND_CPP_00012));
			g_GetJobName(pInfo->sPfLimit[i]);
			if (name.compare(g_GetJobName(pInfo->sPfLimit[i])) == 0) {
				_snprintf_s(buf, _TRUNCATE, "%s", RES_STRING(CL_UIITEMCOMMAND_CPP_00013));
				//PushHint( buf, VALID_COLOR );
			} else {
				_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_679), g_GetJobName(pInfo->sPfLimit[i]));
			}

			isFind = true;
		}
		//	end

		if (!isSame) {
			if (pAttr->get(ATTR_JOB) == pInfo->sPfLimit[i]) {
				isSame = true;
			}
		}
	}

	if (isFind) {
		PushHint(buf, isSame ? GENERIC_COLOR : VALID_COLOR);
	}
	//else
	//{
	//	PushHint( "职业限制:无", GENERIC_COLOR );
	//}
}

void CItemCommand::_ShowBody() {
	if (_pItem->IsAllEquip())
		return;

	if (!g_stUIBoat.GetHuman() || !g_stUIBoat.GetHuman()->GetDefaultChaInfo())
		return;

	std::ostrstream str;
	str << RES_STRING(CL_LANGUAGE_MATCH_680);
	for (int i = 1; i < 5; i++) {
		if (!_pItem->IsAllowEquip(i))
			continue;

		switch (i) {
		case 1:
			str << RES_STRING(CL_LANGUAGE_MATCH_681);
			break;
		case 2:
			str << RES_STRING(CL_LANGUAGE_MATCH_682);
			break;
		case 3:
			str << RES_STRING(CL_LANGUAGE_MATCH_683);
			break;
		case 4:
			str << RES_STRING(CL_LANGUAGE_MATCH_684);
			break;
		}
	}
	str << '\0';

	int nBodyType = g_stUIBoat.GetHuman()->GetDefaultChaInfo()->lID;
	PushHint(str.str(), _pItem->IsAllowEquip(nBodyType) ? GENERIC_COLOR : VALID_COLOR);
}

void CItemCommand::_ShowFusionBody(CItemRecord* pEquipItem) {
	if (_pItem->IsAllEquip() && pEquipItem->IsAllEquip())
		return;

	if (!g_stUIBoat.GetHuman() || !g_stUIBoat.GetHuman()->GetDefaultChaInfo())
		return;

	std::ostrstream str;
	str << RES_STRING(CL_LANGUAGE_MATCH_680);
	for (int i = 1; i < 5; i++) {
		if (_pItem->IsAllowEquip(i) && pEquipItem->IsAllowEquip(i)) {
			switch (i) {
			case 1:
				str << RES_STRING(CL_LANGUAGE_MATCH_681);
				break;
			case 2:
				str << RES_STRING(CL_LANGUAGE_MATCH_682);
				break;
			case 3:
				str << RES_STRING(CL_LANGUAGE_MATCH_683);
				break;
			case 4:
				str << RES_STRING(CL_LANGUAGE_MATCH_684);
				break;
			}
		}
	}
	str << '\0';

	int nBodyType = g_stUIBoat.GetHuman()->GetDefaultChaInfo()->lID;
	PushHint(str.str(), _pItem->IsAllowEquip(nBodyType) ? GENERIC_COLOR : VALID_COLOR);
}

void CItemCommand::SetBoatHint(const NET_CHARTRADE_BOATDATA* const pBoat) {
	if (pBoat) {
		if (!_pBoatHint) {
			_pBoatHint = std::make_unique<NET_CHARTRADE_BOATDATA>();
		}
		*_pBoatHint = *pBoat;
	} else {
		_pBoatHint.reset();
	}
}

SItemForge& CItemCommand::GetForgeInfo() {
	return SItemForge::Convert(_ItemData.lDBParam[0]);
}

//---------------------------------------------------------------------------
// class SItemHint
//---------------------------------------------------------------------------
void SItemHint::Convert(SItemGrid& ItemGrid, CItemRecord* pInfo) {
	sID = ItemGrid.sID;
	sNum = ItemGrid.sNum;
	sEndure[0] = ItemGrid.sEndure[0] / 50;
	sEndure[1] = ItemGrid.sEndure[1] / 50;
	sEnergy[0] = ItemGrid.sEnergy[0];
	sEnergy[1] = ItemGrid.sEnergy[1];
	chForgeLv = ItemGrid.chForgeLv;
	memcpy(lDBParam, ItemGrid.lDBParam, sizeof(lDBParam));

	memset(sInstAttr, 0, sizeof(sInstAttr));

	for (int i = 0; i < ITEMATTR_CLIENT_MAX; i++) {
		sInstAttr[i] = pInfo->GetTypeValue(i);
	}

	// 读取属性，如果网络有属性，使用网络属性，否则从表格中读取
	int nAttr = 0;
	for (int i = 0; i < defITEM_INSTANCE_ATTR_NUM; i++) {
		nAttr = ItemGrid.sInstAttr[i][0];
		if (nAttr <= 0 || nAttr >= ITEMATTR_CLIENT_MAX)
			continue;

		sInstAttr[nAttr] = ItemGrid.sInstAttr[i][1];
	}
}

//---------------------------------------------------------------------------
// class SItemForge
//---------------------------------------------------------------------------
SItemForge& SItemForge::Convert(DWORD v, int nItemID) {
	static SItemForge forge;
	memset(&forge, 0, sizeof(forge));

	DWORD dwForgeValue = v;
	if (dwForgeValue == 0)
		return forge;

	forge.IsForge = true;
	forge.nHoleNum = v / 1000000000; // 槽数

	int nStoneData;
	CStoneInfo* pStoneInfo = nullptr;
	for (int i = 0; i < 3; ++i) {
		nStoneData = (v / (int)(pow(1000, 2 - i))) % 1000; // 三位一取

		pStoneInfo = GetStoneInfo(nStoneData / 10);
		if (!pStoneInfo)
			continue;

		forge.pStoneInfo[forge.nStoneNum] = pStoneInfo;
		forge.nStoneLevel[forge.nStoneNum] = nStoneData % 10;

		forge.nLevel += forge.nStoneLevel[forge.nStoneNum];

		std::string strHint = "";
		if (g_pGameApp->GetScriptMgr()->DoString(pStoneInfo->szHintFunc, "u-s", forge.nStoneLevel[forge.nStoneNum], &strHint)) {
			strncpy_s(forge.szStoneHint[forge.nStoneNum], sizeof(forge.szStoneHint[forge.nStoneNum]), strHint.c_str(), _TRUNCATE);
		}

		forge.nStoneNum += 1;
	}

	if (nItemID > 0) {
		forge.Refresh(nItemID);
	}

	return forge;

	//////////////////////////////////////////////////////////////////////////////////
	// 下面的可能会有问题，不执行

	//int Num = 0;
	//if( g_pGameApp->GetScriptMgr()->DoString( "Get_HoleNum", "u-d", dwForgeValue, &Num ) )
	//{
	//	if( Num>0 )
	//	{
	//		forge.nHoleNum = Num;
	//	}
	//}

	//// 得到三颗宝石
	//int nStone;
	//int nStoneLv;
	//CStoneInfo* pStone = NULL;
	//int StoneNum = 0;
	//string hint;
	//for( int i=0; i<3; i++ )
	//{
	//	_snprintf_s( buf, _TRUNCATE, "Get_Stone_%d", i+1 );
	//	nStone = 0;
	//	if( !g_pGameApp->GetScriptMgr()->DoString( buf, "u-d", dwForgeValue, &nStone ) )
	//		continue;

	//	pStone = GetStoneInfo( nStone );
	//	if( !pStone ) continue;

	//	forge.pStoneInfo[ StoneNum ] = pStone;

	//	nStoneLv = 0;
	//	_snprintf_s( buf, _TRUNCATE, "Get_StoneLv_%d", i+1 );
	//	if( g_pGameApp->GetScriptMgr()->DoString( buf, "u-d", dwForgeValue, &nStoneLv ) )
	//	{
	//		forge.nStoneLevel[ StoneNum ] = nStoneLv;
	//		forge.nLevel += nStoneLv;

	//		hint = "";
	//		if( g_pGameApp->GetScriptMgr()->DoString( pStone->szHintFunc, "u-s", nStoneLv, &hint ) )
	//		{
	//		strncpy_s( forge.szStoneHint[StoneNum], sizeof(forge.szStoneHint[StoneNum]) , hint.c_str(),_TRUNCATE);
	//		}
	//	}
	//	StoneNum++;
	//}

	//forge.nStoneNum = StoneNum;

	//if( nItemID>0 )
	//{
	//	forge.Refresh( nItemID );
	//}
	//return forge;
}

void SItemForge::Refresh(int nItemID) {
	for (int i = 0; i < 3; i++) {
		if (pStoneInfo[i])
			nStoneType[i] = pStoneInfo[i]->nType;
		else
			nStoneType[i] = -1;
	}

	int nEffectID = 0;
	if (!g_pGameApp->GetScriptMgr()->DoString("Item_Stoneeffect", "ddd-d", nStoneType[0], nStoneType[1], nStoneType[2], &nEffectID))
		return;

	nEffectID--;
	if (nEffectID < 0 || nEffectID >= ITEM_REFINE_NUM)
		return;

	pRefineInfo = GetItemRefineInfo(nItemID);
	if (!pRefineInfo)
		return;

	pEffectInfo = GetItemRefineEffectInfo(pRefineInfo->Value[nEffectID]);
	if (!pEffectInfo)
		return;

	if (nLevel >= 1) {
		nEffectLevel = (nLevel - 1) / 4;
		if (nEffectLevel > 3)
			nEffectLevel = 3;
	}
}

float SItemForge::GetAlpha(int nTotalLevel) {
	//static float fLevelAlpha[4] = { 150.0f, 180.0f, 220.0f, 255.0f };
	static float fLevelAlpha[4] = {80.0f, 140.0f, 200.0f, 255.0f};
	static float fLevelBase[4] = {fLevelAlpha[1] - fLevelAlpha[0], fLevelAlpha[2] - fLevelAlpha[1], fLevelAlpha[3] - fLevelAlpha[2], 0.0f};

	if (nTotalLevel <= 1)
		return fLevelAlpha[0] / 255.0f;
	if (nTotalLevel >= 13)
		return 1.0f;

	--nTotalLevel;
	int nLevel = nTotalLevel / 4;
	return (fLevelAlpha[nLevel] + (float)(nTotalLevel % 4) / 4.0f * fLevelBase[nLevel]) / 255.0f;
}
