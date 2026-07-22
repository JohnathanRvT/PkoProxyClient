#include "StdAfx.h"
#include "uieditstrategy.h"
#include "uieditdata.h"

using namespace GUI;
//---------------------------------------------------------------------------
// class CEditRow
//---------------------------------------------------------------------------
CEditRow::CEditRow()
	: _dwWordCount(0), _dwHeight(0), _dwWidth(0) {
}

void CEditRow::PushUnit(CEditObj* pObj, CEditObj* pAtom) {
	int w, h;
	pObj->GetSize(w, h);
	_dwWidth += w;
	if ((DWORD)h > _dwHeight) {
		_dwHeight = (DWORD)h;
	}

	_units.push_back(pObj);
	_dwWordCount += pObj->GetWordCount();

	_atoms.push_back(pAtom);
}

void CEditRow::Clear() {
	for (auto* unit : _units) {
		SAFE_DELETE(unit);
	}
	_units.clear();
	_atoms.clear();

	_dwWordCount = 0;
	_dwHeight = 0;
	_dwWidth = 0;
}

void CEditRow::Render() {
	for (auto* unit : _units) {
		unit->Render();
	}
}

void CEditRow::SetPos(int x, int y) {
	int w, h;
	for (auto* unit : _units) {
		unit->SetPos(x, y);
		unit->GetSize(w, h);
		x += w;
	}
}

//---------------------------------------------------------------------------
// class CEditStrategy
//---------------------------------------------------------------------------
CEditStrategy::CEditStrategy(CEditArticle* pActicle)
	: _pActicle(pActicle) {
}

CEditStrategy::~CEditStrategy() {
	Clear();
}

bool CEditStrategy::Append(CEditObj* pChar) {
	pChar->Parse(this);
	return true;
}

void CEditStrategy::Clear() {
	for (auto* item : _items) {
		SAFE_DELETE(item);
	}
	_items.clear();
}

void CEditStrategy::Render() {
	for (auto* item : _items) {
		item->Render();
	}
}

void CEditStrategy::Init() {
}

void CEditStrategy::RefreshPos(int x, int y) {
	for (auto* item : _items) {
		item->SetPos(x, y);
		y += item->GetHeight();
	}
}

void CEditStrategy::ParseText(CEditTextObj* pText) {
	// 合并文本
	CEditRow* pRow = _AppendToBackRow(pText);

	CEditSentence* pSentence = nullptr; // 当前正在操作的句子
	if (pRow->GetObjNum() > 0) {
		pSentence = dynamic_cast<CEditSentence*>(pRow->GetObj(pRow->GetObjNum() - 1));
		if (pSentence && pSentence->IsSameType(pText)) {
			pSentence->AddCaption(pText->GetCaption());
			return;
		}
	}

	pSentence = new CEditSentence;
	pRow->PushUnit(pSentence, pText);
	pSentence->AddCaption(pText->GetCaption());
}

CEditRow* CEditStrategy::_AppendToBackRow(CEditObj* pChar) {
	return _items.empty() ? _items.emplace_back(new CEditRow) : _items.back();
}
