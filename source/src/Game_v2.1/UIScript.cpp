//----------------------------------------------------------------------
//	2005/4/13	Arcol	扩展修改表单风格接口：UI_SetFormStyleEx
//----------------------------------------------------------------------
#include "stdafx.h"
#ifdef CALUA_COMPAT
#include "caLua.h" //CHANGED: Removed CaLua dependency
#endif
#include "script.h"
#include "uiguidata.h"
#include "UIScript.h"
#include "UIEdit.h"
#include "UILabel.h"
#include "uiformmgr.h"
#include "uitextbutton.h"
#include "uilabel.h"
#include "uiprogressbar.h"
#include "uiscroll.h"
#include "uilist.h"
#include "uicombo.h"
#include "uiimage.h"
#include "UICheckBox.h"
#include "uiimeinput.h"
#include "uigrid.h"
#include "uilistview.h"
#include "uipage.h"
#include "uitreeview.h"
#include "sceneobjset.h"
#include "LuaInterface.h"
#include "ui3dcompent.h"
#include "UIPicList.h"
#include "uitextparse.h"
#include "uimemo.h"
#include "uigoodsgrid.h"
#include "uifastcommand.h"
#include "UIHeadSay.h"
#include "UISkillList.h"
#include "CharacterRecord.h"
#include "MapSet.h"
#include "EffectSet.h"
#include "uimenu.h"
#include "uiCozeform.h"
#include "UIChat.h"
#include "StringLib.h"
#include "uititle.h"
#include "uiequipform.h"
#include "uirichedit.h"
#include <Windows.h>			//Lark.li
#include "uiamphitheaterform.h" //add by sunny.sun20080718

#include "uiwebbrowser.h"

#ifndef CALUA_COMPAT
#include "lua_platform.h" //MAJOR CHANGED: Removed CaLua dependency
#endif

using namespace GUI;

static UIScript<CItemObj> g_ItemScript;

static CList* GetList(int list_id) {
	CGuiData* p = CGuiData::GetGui(list_id);
	if (!p)
		return NULL;

	CList* f = dynamic_cast<CList*>(p);
	if (f) {
		return f;
	}

	CListView* l = dynamic_cast<CListView*>(p);
	if (l) {
		return l->GetList();
	}

	return NULL;
}
//---------------------------------------------------------------------------
// UI_Script
//---------------------------------------------------------------------------
int UI_LoadScript(char* file) {
#ifdef CALUA_COMPAT
	CLU_LoadScript(file);
#else
	//extern  lua_State *L;

	luaL_handled_dofile(L, file);
	//lua_pcall(L, 0, 0, 0);
#endif

	return R_OK;
}

//安全释放内存 by Waiting 2009-06-18
void UI_ReleaseItemScript() {
	g_ItemScript.ReleaseAll();
}

enum eCompentType {
	LABEL_TYPE = 0,
	LABELEX_TYPE = 1,
	BUTTON_TYPE = 2,
	COMBO_TYPE = 3,
	EDIT_TYPE = 4,
	IMAGE_TYPE = 5,
	LIST_TYPE = 6,
	PROGRESS_TYPE = 7,
	CHECK_TYPE = 8,
	CHECK_GROUP_TYPE = 9,
	GRID_TYPE = 10,
	PAGE_TYPE = 11,
	FIX_LIST_TYPE = 12,
	CHECK_FIX_LIST_TYPE = 13,
	DRAG_TITLE_TYPE = 14,
	TREE_TYPE = 15,
	IMAGE_FRAME_TYPE = 16,
	UI3D_COMPENT_TYPE = 17,
	MEMO_TYPE = 18,
	MEMOEX_TYPE = 19,
	GOODS_GRID_TYPE = 20,
	FAST_COMMANG_TYPE = 21,
	COMMAND_ONE_TYPE = 22,
	IMAGE_FLASH_TYPE = 23,
	SCROLL_TYPE = 24,
	SKILL_LIST_TYPE = 25,

	LISTEX_TYPE = 26,
	MENU_TYPE = 27,
	RICHMEMO_TYPE = 28,
	TITLE_TYPE = 29,
	RICHEDIT_TYPE = 30,

	AMPHI_LIST_TYPE = 31, //ADD by sunny.sun20080718

	WEB_BROWSER_TYPE = 32,

	GUI_END,
};

enum eHotKey {
	ALT_KEY = 0,
	CTRL_KEY,
	SHIFT_KEY
};

// Add by lark.li 20090220 begin
int UI_SetFormInMianForm(int formId) {
	CForm* f = dynamic_cast<CForm*>(CGuiData::GetGui(formId));
	if (!f)
		return R_FAIL;

	f->SetInMainForm(true);
	return R_OK;
}
// End

int UI_CreateCompent(int formId, int type, char* pszName, int w, int h, int x, int y) {
	CForm* f = dynamic_cast<CForm*>(CGuiData::GetGui(formId));
	if (!f)
		return R_FAIL;

	CGuiData* g = NULL;
	switch (type) {
	case BUTTON_TYPE:
		g = new CTextButton(*f);
		break;
	case COMBO_TYPE:
		g = new CCombo(*f);
		break;
	case EDIT_TYPE:
		g = new CEdit(*f);
		break;
	case IMAGE_TYPE:
		g = new CImage(*f);
		break;
	case LABEL_TYPE:
		g = new CLabel(*f);
		break;
	case LABELEX_TYPE:
		g = new CLabelEx(*f);
		break;
	case LIST_TYPE:
		g = new CList(*f);
		break;
	case PROGRESS_TYPE:
		g = new CProgressBar(*f);
		break;
	case CHECK_TYPE:
		g = new CCheckBox(*f);
		break;
	case CHECK_GROUP_TYPE:
		g = new CCheckGroup(*f);
		break;
	case GRID_TYPE:
		g = new CGrid(*f);
		break;
	case PAGE_TYPE:
		g = new CPage(*f);
		break;
	case FIX_LIST_TYPE:
		g = new CFixList(*f);
		break;
	case CHECK_FIX_LIST_TYPE:
		g = new CCheckFixList(*f);
		break;
	case DRAG_TITLE_TYPE:
		g = new CDragTitle(*f);
		break;
	case TREE_TYPE:
		g = new CTreeView(*f);
		break;
	case IMAGE_FRAME_TYPE:
		g = new CFrameImage(*f);
		break;
	case UI3D_COMPENT_TYPE:
		g = new C3DCompent(*f);
		break;
	case MEMO_TYPE:
		g = new CMemo(*f);
		break;
	case MEMOEX_TYPE:
		g = new CMemoEx(*f);
		break;
	case RICHMEMO_TYPE:
		g = new CRichMemo(*f);
		break;
	case GOODS_GRID_TYPE:
		g = new CGoodsGrid(*f);
		break;
	case FAST_COMMANG_TYPE:
		g = new CFastCommand(*f);
		break;
	case COMMAND_ONE_TYPE:
		g = new COneCommand(*f);
		break;
	case IMAGE_FLASH_TYPE:
		g = new CFlashImage(*f);
		break;
	case SCROLL_TYPE:
		g = new CScroll(*f);
		break;
	case SKILL_LIST_TYPE:
		g = new CSkillList(*f);
		break;
	case MENU_TYPE:
		g = new CMenu(f);
		break;
	case TITLE_TYPE:
		g = new CTitle(*f);
		break;
	case RICHEDIT_TYPE:
		g = new CRichEdit(*f);
		break;
	case AMPHI_LIST_TYPE:
		g = new CAmphitheaterList(*f);
		break; //add by sunny.sun 20080718
	// Modify by lark.li 20081217 begin
	case WEB_BROWSER_TYPE:
		g = new CWebBrowser(*f);
		break;
	// End
	default:
		return R_FAIL;
	}

	g->SetName(pszName);
	g->SetPos(x, y);
	g->SetSize(w, h);
	return g->GetID();
}

int UI_SetFormTempleteMax(int max) {
	if (CFormMgr::s_Mgr.SetFormTempleteMax(max))
		return R_OK;

	return R_FAIL;
}

int UI_AddAllFormTemplete(int form_id) {
	CForm* p = dynamic_cast<CForm*>(CGuiData::GetGui(form_id));
	if (!p)
		return R_FAIL;

	int n = CFormMgr::s_Mgr.GetFormTempleteMax();
	for (int i = 0; i < n; ++i)
		CFormMgr::s_Mgr.AddForm(p, i);

	return R_OK;
}

int UI_AddFormToTemplete(int formid, int nTempleteNo) {
	CForm* p = dynamic_cast<CForm*>(CGuiData::GetGui(formid));
	if (!p)
		return R_FAIL;

	if (CFormMgr::s_Mgr.AddForm(p, nTempleteNo))
		return R_OK;

	return R_FAIL;
}

int UI_SwitchTemplete(int nTempleteNo) {
	if (CFormMgr::s_Mgr.SwitchTemplete(nTempleteNo))
		return R_OK;

	return R_FAIL;
}

int UI_CreateForm(char* pszName, int isModal, int w, int h, int x, int y, int isTitle, int isShowFrame) {
	CForm* f = new CForm();
	f->SetName(pszName);
	f->SetSize(w, h);
	f->SetPos(x, y);
	f->GetFrameImage()->SetIsTitle(isTitle != 0 ? true : false);
	f->GetFrameImage()->SetIsShowFrame(isShowFrame != 0 ? true : false);
	//#if _DEBUG
	//	char buffer[255];
	//	_snprintf_s(buffer, _TRUNCATE,"UI_CreateForm\t%s\r\n", pszName);
	//	::OutputDebugStr(buffer);
	//#endif
	return f->GetID();
}

int UI_FormSetIsEscClose(int nFormID, int IsEscClose) {
	CForm* p = dynamic_cast<CForm*>(CGuiData::GetGui(nFormID));
	if (!p)
		return R_FAIL;

	p->SetIsEscClose(IsEscClose ? true : false);
	return R_OK;
}

int UI_FormSetEnterButton(int nFormID, int nButtonID) {
	CForm* p = dynamic_cast<CForm*>(CGuiData::GetGui(nFormID));
	if (!p)
		return R_FAIL;

	CTextButton* b = dynamic_cast<CTextButton*>(CGuiData::GetGui(nButtonID));
	if (!b)
		return R_FAIL;

	if (b->GetForm() != p)
		return R_FAIL;

	p->SetEnterButton(b);
	return R_OK;
}

int UI_FormSetHotKey(int id, int control_key, int key) {
	CForm* p = dynamic_cast<CForm*>(CGuiData::GetGui(id));
	if (!p)
		return R_FAIL;

	p->SetHotKey(key);
	return R_OK;
}

int UI_SetIsDrag(int id, int isDrag) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	p->SetIsDrag(isDrag != 0);
	return R_OK;
}

int UI_LoadFormImage(int id, const char* client, int cw, int ch, int tx, int ty, const char* file, int w, int h) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CForm* f = dynamic_cast<CForm*>(p);
	if (!f)
		return R_FAIL;

	f->GetFrameImage()->LoadImage(client, cw, ch, tx, ty, file, w, h);
	return R_OK;
}

int UI_LoadFrameImage(int id, const char* client, int cw, int ch, int tx, int ty, const char* file, int w, int h) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CFrameImage* f = dynamic_cast<CFrameImage*>(p);
	if (!f)
		return R_FAIL;

	f->GetFrameImage()->LoadImage(client, cw, ch, tx, ty, file, w, h);
	return R_OK;
}

int UI_ShowForm(int id, int show) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CForm* f = dynamic_cast<CForm*>(p);
	if (!f)
		return R_FAIL;

	f->ScriptSetShow(show ? true : false);
	return R_OK;
}

int UI_CreateListView(int formId, char* pszName, int w, int h, int x, int y, int col, int style) {
	CForm* f = dynamic_cast<CForm*>(CGuiData::GetGui(formId));
	if (!f)
		return R_FAIL;

	CListView* g = new CListView(*f, col, (CListView::eStyle)style);
	g->SetName(pszName);
	g->SetPos(x, y);
	g->SetSize(w, h);

	return g->GetID();
}

int UI_ListViewSetTitle(int listviewid, int index, int width, const char* titleimage, int w, int h, int sx, int sy) {
	CListView* f = dynamic_cast<CListView*>(CGuiData::GetGui(listviewid));
	if (!f)
		return R_FAIL;

	f->GetTitle()->SetColumnWidth(index, width);
	CImage* p = dynamic_cast<CImage*>(f->GetColumnImage(index));
	if (p)
		p->GetImage()->LoadImage(titleimage, w, h, 0, sx, sy);

	CListItems* pItem = f->GetList()->GetItems();
	pItem->SetColumnWidth(index, width);
	return R_OK;
}

int UI_ListViewSetTitleHeight(int listviewid, int height) {
	CListView* f = dynamic_cast<CListView*>(CGuiData::GetGui(listviewid));
	if (!f)
		return R_FAIL;

	f->SetColumnHeight(height);
	return R_OK;
}

int UI_SetListIsMouseFollow(int list, int IsFollow) {
	CList* l = GetList(list);
	if (l) {
		l->GetItems()->SetIsMouseFollow(IsFollow ? true : false);
		return R_OK;
	}
	return R_FAIL;
}

int UI_SetListFontColor(int list, DWORD nBackColor, DWORD nSelectColor) //FIXED: Color not working (changed int to DWORD)
{
	CList* l = GetList(list);
	if (l) {
		l->SetFontColor(nBackColor);
		l->SetSelectColor(nSelectColor);
		return R_OK;
	}
	return R_FAIL;
}

int UI_LoadListFixSelect(int id, const char* imagefile, int w, int h, int sx, int sy) {
	CFixList* f = dynamic_cast<CFixList*>(CGuiData::GetGui(id));
	if (!f)
		return R_FAIL;

	f->GetSelectImage()->LoadImage(imagefile, w, h, 0, sx, sy);
	return R_OK;
}

// 装入CheckFixList的Check图象
int UI_LoadCheckFixListCheck(int id, const char* checkimage, int cw, int ch, int csx, int csy, const char* uncheckimage, int uw, int uh, int usx, int usy) {
	CCheckFixList* f = dynamic_cast<CCheckFixList*>(CGuiData::GetGui(id));
	if (!f)
		return R_FAIL;

	f->GetCheckImage()->LoadImage(checkimage, cw, ch, 0, csx, csy);
	f->GetUnCheckImage()->LoadImage(uncheckimage, uw, uh, 0, usx, usy);
	return R_OK;
}

int UI_SetSize(int id, int w, int h) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	p->SetSize(w, h);
	return R_OK;
}

int UI_SetPos(int id, int x, int y) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	p->SetPos(x, y);
	return R_OK;
}

int UI_GetScroll(int id) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CList* list = dynamic_cast<CList*>(p);
	if (list)
		return list->GetScroll()->GetID();

	CListView* view = dynamic_cast<CListView*>(p);
	if (view)
		return view->GetList()->GetScroll()->GetID();

	//	CListEx * lst = dynamic_cast<CListEx*>(p);
	//	if( lst ) return lst->GetScroll()->GetID();

	CCombo* combo = dynamic_cast<CCombo*>(p);
	if (combo)
		return combo->GetList()->GetScroll()->GetID();

	CTreeView* tree = dynamic_cast<CTreeView*>(p);
	if (tree)
		return tree->GetScroll()->GetID();

	CGoodsGrid* grid = dynamic_cast<CGoodsGrid*>(p);
	if (grid)
		return grid->GetScroll()->GetID();

	CMemo* memo = dynamic_cast<CMemo*>(p);
	if (memo)
		return memo->GetScroll()->GetID();

	CMemoEx* memoex = dynamic_cast<CMemoEx*>(p);
	if (memoex)
		return memoex->GetScroll()->GetID();

	CRichMemo* richmemo = dynamic_cast<CRichMemo*>(p);
	if (richmemo)
		return richmemo->GetScroll()->GetID();

	CSkillList* skill = dynamic_cast<CSkillList*>(p);
	if (skill)
		return skill->GetScroll()->GetID();
	//Add by sunny.sun20080721
	CAmphitheaterList* lpList = dynamic_cast<CAmphitheaterList*>(p);
	if (lpList)
		return lpList->GetScroll()->GetID();
	return R_FAIL;
}

int UI_SetScrollStyle(int id, int style) {
	CGuiData* t = CGuiData::GetGui(id);
	if (!t)
		return R_FAIL;

	CScroll* s = dynamic_cast<CScroll*>(t);
	if (!s)
		return R_FAIL;

	s->SetStyle((CScroll::eStyle)style);
	return R_OK;
}

int UI_GetList(int id) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CCombo* combo = dynamic_cast<CCombo*>(p);
	if (combo)
		return combo->GetList()->GetID();

	CListView* v = dynamic_cast<CListView*>(p);
	if (v)
		return v->GetList()->GetID();

	CList* list = dynamic_cast<CList*>(p);
	if (list)
		return list->GetID();

	return R_FAIL;
}

enum {
	SCROLL_UP = 0,
	SCROLL_DOWN,
	SCROLL_SCROLL,
	EXLIST_BUTTON,
};

int UI_GetScrollObj(int id, int scrolltype) {
	CScroll* p = dynamic_cast<CScroll*>(CGuiData::GetGui(id));
	if (!p)
		return R_FAIL;

	switch (scrolltype) {
	case SCROLL_UP:
		return p->GetUp()->GetID();
	case SCROLL_DOWN:
		return p->GetDown()->GetID();
	case SCROLL_SCROLL:
		return p->GetScroll()->GetID();
	}
	return R_FAIL;
}
// modify by sun 2008-7-19
//Begin
int UI_GetEXListObj(int id, int type) {
	CAmphitheaterList* p = dynamic_cast<CAmphitheaterList*>(CGuiData::GetGui(id));
	if (!p)
		return R_FAIL;
	switch (type) {
	case EXLIST_BUTTON:
		return p->GetButton()->GetID();
		break;
	default:
		break;
	}
	return R_FAIL;
}
//End

int UI_LoadComboImage(int id, char* edit, int ew, int eh, int ex, int ey, char* button, int bw, int bh, int bx, int by, int isHorizontal) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CCombo* combo = dynamic_cast<CCombo*>(p);
	if (!combo)
		return R_FAIL;

	combo->GetEdit()->GetImage()->LoadImage(edit, ew, eh, 0, ex, ey);
	combo->GetEdit()->SetSize(ew, eh);
	combo->GetButton()->LoadImage(button, bw, bh, bx, by, isHorizontal != 0);
	combo->GetButton()->SetSize(bw, bh);
	return R_OK;
}

int UI_ComboSetStyle(int id, int style) {
	CCombo* combo = dynamic_cast<CCombo*>(CGuiData::GetGui(id));
	if (!combo)
		return R_FAIL;

	combo->SetListStyle((CCombo::eListStyle)style);
	return R_OK;
}

int UI_ComboSetTextColor(int id, DWORD color) //FIXED: Color not working (changed int to DWORD)
{
	CCombo* combo = dynamic_cast<CCombo*>(CGuiData::GetGui(id));
	if (!combo)
		return R_FAIL;

	combo->GetEdit()->SetTextColor(color);
	return R_OK;
}

int UI_LoadButtonImage(int id, const char* file, int w, int h, int sx, int sy, int isHorizontal) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CTextButton* b = dynamic_cast<CTextButton*>(p);
	if (!b)
		return R_FAIL;

	b->LoadImage(file, w, h, sx, sy, isHorizontal != 0 ? true : false);
	return R_OK;
}

int UI_LoadImage(int id, char* file, int frame, int w, int h, int tx, int ty) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p) {
		if (id != 0) //id=0时，表示 UI_CreateCompent 失败，可能该 ui 已经被杠掉，不需要关注
			LG("gui", "UI_LoadImage( %d, %s, %d, %d, %d, %d, %d ) failed\n", id, file, frame, w, h, tx, ty);
		return R_FAIL;
	}

	CGuiPic* img = p->GetImage();
	if (!img)
		return R_FAIL;

	if (img->LoadImage(file, w, h, frame, tx, ty))
		return R_OK;
	return R_FAIL;
}

// 装载带比例的图象
int UI_LoadScaleImage(int id, char* file, int frame, int w, int h, int tx, int ty, float scalex, float scaley) {

	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CGuiPic* img = p->GetImage();
	if (!img)
		return R_FAIL;

	if (img->LoadImage(file, w, h, frame, tx, ty, scalex, scaley))
		return R_OK;

	return R_FAIL;
}
// 装载带比例的图象
int UI_LoadFlashScaleImage(int id, int flash, char* file, int frame, int w, int h, int tx, int ty, float scalex, float scaley) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CProgressBar* pro = dynamic_cast<CProgressBar*>(p);
	if (!pro)
		return R_FAIL;
	pro->SetFlashNum(flash);

	CGuiPic* img = p->GetImage();
	if (!img)
		return R_FAIL;

	if (img->LoadImage(file, w, h, frame, tx, ty, scalex, scaley))
		return R_OK;
	return R_FAIL;
}

int UI_SetIsEnabled(int id, int isEnabled) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	p->SetIsEnabled(isEnabled != 0 ? true : false);
	return R_OK;
}

int UI_SetTag(int id, int tag) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	p->nTag = tag;
	return R_OK;
}

int UI_CopyImage(int targetid, int sourceid) {
	CGuiData* t = CGuiData::GetGui(targetid);
	if (!t)
		return R_FAIL;

	CGuiPic* pt = t->GetImage();
	if (!pt)
		return R_FAIL;

	CGuiData* s = CGuiData::GetGui(sourceid);
	if (!s)
		return R_FAIL;

	CGuiPic* ps = s->GetImage();
	if (!ps)
		return R_FAIL;

	*ps = *pt;
	return R_OK;
}

int UI_CopyCompent(int targetid, int sourceid) {
	CGuiData* t = CGuiData::GetGui(targetid);
	if (!t)
		return R_FAIL;

	CGuiData* s = CGuiData::GetGui(sourceid);
	if (!s)
		return R_FAIL;

	t->Clone(s);

	// const type_info& p = typeid(t);

	return R_OK;
}

int UI_SetIsShow(int id, int isshow) {
	CGuiData* t = CGuiData::GetGui(id);
	if (!t)
		return R_FAIL;

	t->ScriptSetShow(isshow != 0 ? true : false);
	return R_OK;
}

int UI_SetAlign(int id, int align) {
	CCompent* s = dynamic_cast<CCompent*>(CGuiData::GetGui(id));
	if (!s)
		return R_FAIL;

	s->SetAlign((eCompentAlign)align);
	return R_OK;
}

int UI_SetIsKeyFocus(int id, int IsKeyFocus) {
	CCompent* s = dynamic_cast<CCompent*>(CGuiData::GetGui(id));
	if (!s)
		return R_FAIL;

	s->SetIsFocus(IsKeyFocus != FALSE);
	return R_OK;
}

int UI_SetButtonModalResult(int id, int modal) {
	CTextButton* g = dynamic_cast<CTextButton*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	g->SetFormModal((CForm::eModalResult)modal);
	return R_OK;
}

int UI_SetLabelExFont(int id, int nFontIndex, int IsShadow, DWORD dwShadowColor) //FIXED: Color not working (changed int to DWORD)
{
	CLabelEx* g = dynamic_cast<CLabelEx*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	g->SetIsShadow(IsShadow != 0);
	g->SetFont(nFontIndex);
	g->SetShadowColor((DWORD)dwShadowColor);
	return R_OK;
}

int UI_SetTitleFont(int id, int nFontIndex, DWORD color, int h) //FIXED: Color not working (changed int to DWORD)
{
	CTitle* g = dynamic_cast<CTitle*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	g->SetFont(nFontIndex);
	g->SetTextColor(color);
	g->SetFontH(h);
	return R_OK;
}

int UI_ButtonSetHint(int id, char* hint) {
	CTextButton* b = dynamic_cast<CTextButton*>(CGuiData::GetGui(id));
	if (!b)
		return R_FAIL;

	b->SetHint(hint);
	return R_OK;
}

int UI_SetMaxImage(int id, int max) {
	CGuiData* t = CGuiData::GetGui(id);
	if (!t)
		return R_FAIL;

	CGuiPic* p = t->GetImage();
	if (p) {
		p->SetMax(max);
	}
	return R_OK;
}

int UI_SetHint(int id, char* hint) {
	CGuiData* t = CGuiData::GetGui(id);
	if (!t)
		return R_FAIL;

	t->SetHint(hint);
	return R_OK;
}

int UI_SetProgressStyle(int id, int style) {
	CGuiData* t = CGuiData::GetGui(id);
	if (!t)
		return R_FAIL;

	CProgressBar* s = dynamic_cast<CProgressBar*>(t);
	if (!s)
		return R_FAIL;

	s->SetStyle((CProgressBar::eStyle)style);
	return R_OK;
}

int UI_AddListText(int id, char* text) {
	CList* s = GetList(id);
	if (!s)
		return R_FAIL;

	s->Add(text);
	return R_OK;
}

int UI_ListSetItemMargin(int id, int left, int top) {
	CList* s = GetList(id);
	if (!s)
		return R_FAIL;

	s->GetItems()->SetItemMargin(left, top);
	return R_OK;
}

int UI_ListSetItemImageMargin(int id, int left, int top) {
	CList* s = GetList(id);
	if (!s)
		return R_FAIL;

	s->GetItems()->SetImageMargin(left, top);
	return R_OK;
}

int UI_AddListBarText(int id, char* text, float prgress) {
	CList* s = GetList(id);
	if (!s)
		return R_FAIL;

	CItemRow* item = s->GetItems()->NewItem();
	CItemBar* bar = new CItemBar;
	bar->SetString(text);
	item->SetBegin(bar);
	bar->SetScale(prgress);
	return R_OK;
}

int UI_AddGroupBox(int id, int checkbox) {
	CGuiData* t = CGuiData::GetGui(id);
	if (!t)
		return R_FAIL;

	CGuiData* b = CGuiData::GetGui(checkbox);
	if (!b)
		return R_FAIL;

	CCheckBox* box = dynamic_cast<CCheckBox*>(b);
	if (!box)
		return R_FAIL;

	CCheckGroup* s = dynamic_cast<CCheckGroup*>(t);
	if (!s)
		return R_FAIL;

	s->AddCheckBox(box);
	return R_OK;
}

int UI_SetEditEnterButton(int nEditID, int nButtonID) {
	CEdit* t = dynamic_cast<CEdit*>(CGuiData::GetGui(nEditID));
	if (!t)
		return R_FAIL;

	CTextButton* b = dynamic_cast<CTextButton*>(CGuiData::GetGui(nButtonID));
	if (!b)
		return R_FAIL;

	t->SetEnterButton(b);
	return R_OK;
}

int UI_SetEditCursorColor(int nEditID, DWORD color) //FIXED: Color not working (changed int to DWORD)
{
	CEdit* t = dynamic_cast<CEdit*>(CGuiData::GetGui(nEditID));
	if (!t)
		return R_FAIL;

	t->SetCursorColor(color);
	return R_OK;
}

int UI_SetEditMaxNum(int id, int num) {
	CEdit* t = dynamic_cast<CEdit*>(CGuiData::GetGui(id));
	if (!t)
		return R_FAIL;

	t->SetMaxNum(num);
	return R_OK;
}

int UI_SetEditMaxNumVisible(int id, int num) {
	CEdit* t = dynamic_cast<CEdit*>(CGuiData::GetGui(id));
	if (!t)
		return R_FAIL;

	t->SetMaxNumVisible(num);
	return R_OK;
}

//TODO: Change definition of everything that uses int from lua to a 8-bit number or double.
int UI_SetTextColor(int id, DWORD color) //CHANGED: int to DWORD - number is not being converted properly
{
	CGuiData* t = CGuiData::GetGui(id);
	if (!t)
		return R_FAIL;

	t->SetTextColor(color);
	return R_OK;
}

int UI_SetListRowHeight(int id, int height) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CList* lst = dynamic_cast<CList*>(p);
	if (lst) {
		lst->SetRowHeight(height);
		return R_OK;
	}

	CListView* lsv = dynamic_cast<CListView*>(p);
	if (lsv) {
		lsv->GetList()->SetRowHeight(height);
		return R_OK;
	}

	CFixList* fls = dynamic_cast<CFixList*>(p);
	if (fls) {
		fls->SetRowHeight(height);
		return R_OK;
	}

	CSkillList* skill = dynamic_cast<CSkillList*>(p);
	if (skill) {
		skill->SetRowHeight(height);
		return R_OK;
	}
	return R_FAIL;
}

int UI_ListLoadSelectImage(int id, char* file, int w, int h, int sx, int sy) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CListView* pView = dynamic_cast<CListView*>(p);
	CList* lst = NULL;
	if (pView) {
		lst = pView->GetList();
	}

	if (!lst)
		lst = dynamic_cast<CList*>(p);
	if (lst) {
		lst->GetItems()->GetSelect()->GetImage()->LoadImage(file, w, h, 0, sx, sy, 1.0f, 1.0f);
		return R_OK;
	}

	CSkillList* skill = dynamic_cast<CSkillList*>(p);
	if (skill) {
		skill->GetSelect()->LoadImage(file, w, h, 0, sx, sy, 1.0f, 1.0f);
		return R_OK;
	}
	return R_FAIL;
}

int UI_LoadSkillListButtonImage(int id, char* file, int w, int h, int sx, int sy, int item_w, int item_h) {
	CSkillList* p = dynamic_cast<CSkillList*>(CGuiData::GetGui(id));
	if (!p)
		return R_FAIL;

	CGuiPic* pic = p->GetButtonImage();
	pic->LoadImage(file, w, h, 0, sx, sy, 1.0f, 1.0f);
	pic->LoadImage(file, w, h, 1, sx + w, sy, 1.0f, 1.0f);
	pic->LoadImage(file, w, h, 2, sx + 2 * w, sy, 1.0f, 1.0f);
	pic->LoadImage(file, w, h, 3, sx + 3 * w, sy, 1.0f, 1.0f);

	pic->SetScale(item_w, item_h);
	return R_OK;
}

int UI_LoadListItemImage(int id, char* file, int w, int h, int sx, int sy, int item_w, int item_h) {
	CList* lst = GetList(id);
	if (!lst)
		return R_FAIL;

	lst->GetItemImage()->LoadImage(file, w, h, 0, sx, sy);
	lst->GetItemImage()->SetScale(item_w, item_h);
	return R_OK;
}

int UI_FixListSetRowSpace(int id, int height) {
	CFixList* fls = dynamic_cast<CFixList*>(CGuiData::GetGui(id));
	if (fls) {
		fls->SetRowSpace(height);
		return R_OK;
	}

	return R_FAIL;
}

// 设置CheckFixList中Check的显示边距
int UI_CheckFixListSetCheckMargin(int id, int left, int top) {
	CCheckFixList* fls = dynamic_cast<CCheckFixList*>(CGuiData::GetGui(id));
	if (fls) {
		fls->SetCheckMargin(left, top);
		return R_OK;
	}

	return R_FAIL;
}

int UI_SetMargin(int id, int left, int top, int right, int bottom) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	p->SetMargin(left, top, right, bottom);
	return R_OK;
}

int UI_SetImageAlpha(int id, int alpha) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CGuiPic* pic = p->GetImage();
	if (pic)
		pic->SetAlpha(alpha);
	return R_OK;
}

int UI_SetAlpha(int id, int alpha) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	p->SetAlpha(alpha);
	return R_OK;
}

int UI_GridLoadSelectImage(int id, char* file, int w, int h, int tx, int ty) {
	CGrid* g = dynamic_cast<CGrid*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	g->GetSelectImage()->GetImage()->LoadImage(file, w, h, 0, tx, ty);
	return R_OK;
}

int UI_SetGridIsDragSize(int id, int IsEnabled) {
	CGrid* g = dynamic_cast<CGrid*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	g->SetIsDragSize(false);
	return R_OK;
}

int UI_SetGridSpace(int id, int x, int y) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CGoodsGrid* d = dynamic_cast<CGoodsGrid*>(p);
	if (d) {
		d->SetSpace(x, y);
		return R_OK;
	}

	CGrid* g = dynamic_cast<CGrid*>(p);
	if (g) {
		g->SetSpace(x, y);
		return R_OK;
	}

	return R_FAIL;
}

int UI_SetGridContent(int id, int nRow, int nCol) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CGoodsGrid* d = dynamic_cast<CGoodsGrid*>(p);
	if (d) {
		d->SetContent(nRow, nCol);
		return R_OK;
	}

	return R_FAIL;
}

int UI_SetGridUnitSize(int id, int w, int h) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CGrid* g = dynamic_cast<CGrid*>(p);
	if (g) {
		g->SetUnitSize(w, h);
		return R_OK;
	}

	CGoodsGrid* d = dynamic_cast<CGoodsGrid*>(p);
	if (d) {
		d->SetUnitSize(w, h);
		return R_OK;
	}

	return R_FAIL;
}

int UI_GoodGridLoadUnitImage(int id, char* file, int w, int h, int tx, int ty) {
	CGoodsGrid* g = dynamic_cast<CGoodsGrid*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	g->GetUnitImage()->LoadImage(file, w, h, 0, tx, ty);
	return R_OK;
}

int UI_AddFaceToGrid(int id, char* file, int w, int h, int sx, int sy, int frame, int nTag) {
	CGrid* g = dynamic_cast<CGrid*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	CGraph* p = new CGraph(file, w, h, sx, sy, frame);
	p->GetImage()->SetScale(g->GetUnitWidth(), g->GetUnitHeight());

	p->nTag = nTag;
	g->Add(p);
	return R_OK;
}

// 设置最大行数
int UI_FixListSetMaxNum(int id, int num) {
	CFixList* g = dynamic_cast<CFixList*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	g->SetMaxNum(num);
	return R_OK;
}

// 设置每一行对应的文本
int UI_FixListSetText(int id, int index, const char* text) {
	CFixList* g = dynamic_cast<CFixList*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	g->SetString(index, text);
	return R_OK;
}

enum eTreeImage {
	enumTreeAddImage = 0,
	enumTreeSubImage,
};
int UI_TreeLoadImage(int nTreeID, int nType, const char* imagefile, int w, int h, int sx, int sy, int itemw, int itemh) {
	CTreeView* f = dynamic_cast<CTreeView*>(CGuiData::GetGui(nTreeID));
	if (!f)
		return R_FAIL;

	CGuiPic* pic = NULL;
	switch (nType) {
	case enumTreeAddImage:
		pic = f->GetAddImage();
		break;
	case enumTreeSubImage:
		pic = f->GetSubImage();
		break;
	}
	if (pic) {
		pic->LoadImage(imagefile, w, h, 0, sx, sy);
		pic->SetScale(itemw, itemh);
		return R_OK;
	}
	return R_FAIL;
}

int UI_CreateTextItem(const char* text, DWORD color) //FIXED: Color not working (changed int to DWORD)
{
	CItem* item = new CItem(text, color);
	return g_ItemScript.AddObj(item);
}

int UI_CreateGraphItem(char* file, int w, int h, int sx, int sy, int frame) {
	CGraph* item = new CGraph(file, w, h, sx, sy, frame);
	return g_ItemScript.AddObj(item);
}

int UI_CreateGraphItemTex(int tx, int ty, int tw, int th, float scale_x, float scale_y, int nTextureID, int tag) {
	CHintGraph* item = new CHintGraph(1);
	MPTexRect* pImage = item->GetImage()->GetImage();
	pImage->nTexSX = tx;
	pImage->nTexSY = ty;
	pImage->nTexW = tw;
	pImage->nTexH = th;
	pImage->fScaleX = scale_x;
	pImage->fScaleY = scale_y;
	pImage->nTextureNo = nTextureID;
	item->nTag = tag;
	return g_ItemScript.AddObj(item);
}

int UI_CreateNoteGraphItem(char* file, int w, int h, int sx, int sy, int frame, const char* text, int TextX, int TextY) {
	CNoteGraph* item = new CNoteGraph(frame);
	item->GetImage()->LoadAllImage(file, w, h, sx, sy);
	item->GetImage()->SetScale(w, h);
	item->SetString(text);
	item->SetTextX(TextX);
	item->SetTextY(TextY);
	return g_ItemScript.AddObj(item);
}

int UI_CreateSingleNode(int treeid, int itemid, int nodeid_parent) {
	CTreeView* g = dynamic_cast<CTreeView*>(CGuiData::GetGui(treeid));
	if (!g)
		return R_FAIL;

	CItemObj* item = g_ItemScript.GetObj(itemid);
	if (!item)
		return R_FAIL;

	CTreeNodeObj* obj = new CTreeNode(g, item);

	CTreeNodeObj* parent = NULL;
	if (nodeid_parent <= 0)
		parent = g->GetRootNode();
	else
		parent = CTreeNodeObj::GetNode(nodeid_parent);

	if (parent)
		parent->AddNode(obj);

	return obj->GetID();
}

int UI_CreateGridNode(int treeid, int itemid, int maxcol, int uw, int uh, int nodeid_parent) {
	CTreeView* g = dynamic_cast<CTreeView*>(CGuiData::GetGui(treeid));
	if (!g)
		return R_FAIL;

	CItemObj* item = g_ItemScript.GetObj(itemid);
	if (!item)
		return R_FAIL;

	CTreeGridNode* obj = new CTreeGridNode(g, item);
	obj->SetIsExpand(false);
	obj->SetColMaxNum(maxcol);
	obj->SetUnitSize(uw, uh);

	CTreeNodeObj* parent = NULL;
	if (nodeid_parent < 0)
		parent = g->GetRootNode();
	else
		parent = CTreeNodeObj::GetNode(nodeid_parent);

	if (parent)
		parent->AddNode(obj);

	return obj->GetID();
}

int UI_GridNodeAddItem(int nodeid, int itemid) {
	CTreeGridNode* obj = dynamic_cast<CTreeGridNode*>(CTreeNodeObj::GetNode(nodeid));
	if (!obj)
		return R_FAIL;

	CItemObj* item = g_ItemScript.GetObj(itemid);
	if (!item)
		return R_FAIL;

	obj->AddItem(item);

	// 初始化提示
	CHintGraph* pHint = dynamic_cast<CHintGraph*>(item);
	if (pHint) {
		CTreeNode* pParent = dynamic_cast<CTreeNode*>(obj->GetParent());

		if (_stricmp(obj->GetItem()->GetString(), RES_STRING(CL_LANGUAGE_MATCH_528)) == 0) {
			CChaRecord* pInfo = GetChaRecordInfo(pHint->nTag);
			if (pInfo) {
				char szBuf[128] = {0};
				_snprintf_s(szBuf, _TRUNCATE, "%d.%s", pInfo->lID, pInfo->szName);
				pHint->SetHint(szBuf);
			}
		} else if (_stricmp(obj->GetItem()->GetString(), RES_STRING(CL_LANGUAGE_MATCH_532)) == 0) {
			CMapInfo* pInfo = GetMapInfo(pHint->nTag);
			if (pInfo) {
				pHint->SetHint(pInfo->szName);
			}
		} else if (_stricmp(obj->GetItem()->GetString(), RES_STRING(CL_LANGUAGE_MATCH_530)) == 0) {
			pHint->SetHint(g_GetAreaName(pHint->nTag));
		} else if (_stricmp(obj->GetItem()->GetString(), RES_STRING(CL_LANGUAGE_MATCH_529)) == 0) {
			CEffectInfo* pInfo = GetEffectInfo(pHint->nTag);
			if (pInfo) {
				char szBuf[128] = {0};
				_snprintf_s(szBuf, _TRUNCATE, "%d.%s", pInfo->nID, pInfo->szName);
				pHint->SetHint(szBuf);
			}
		} else if (pParent) {
			if (_stricmp(pParent->GetItem()->GetString(), RES_STRING(CL_LANGUAGE_MATCH_540)) == 0) {
				CSceneObjInfo* pInfo = GetSceneObjInfo(pHint->nTag);
				if (pInfo) {
					char szBuf[128] = {0};
					_snprintf_s(szBuf, _TRUNCATE, "%d.%s", pInfo->nID, pInfo->szName);
					pHint->SetHint(szBuf);
				}
			}
		}
	}
	return R_OK;
}

int UI_SetChatColor(DWORD world, DWORD road, DWORD team, DWORD guild, DWORD gm, DWORD system, DWORD trade, DWORD person) //FIXED: Color not working (changed int to DWORD)
{
	// 世界,路人,队聊,公会,GM,系统,交易,私聊
	//g_stUICoze.chatColor.road = road;
	//g_stUICoze.chatColor.person = person;
	//g_stUICoze.chatColor.team = team;
	//g_stUICoze.chatColor.guild = guild;
	//g_stUICoze.chatColor.world = world;
	//g_stUICoze.chatColor.system = system;
	//g_stUICoze.chatColor.trade = trade;
	//g_stUICoze.chatColor.gm = gm;

	CCharMsg::SetChannelColor(CCharMsg::CHANNEL_SIGHT, road);
	CCharMsg::SetChannelColor(CCharMsg::CHANNEL_PRIVATE, person);
	CCharMsg::SetChannelColor(CCharMsg::CHANNEL_TEAM, team);
	CCharMsg::SetChannelColor(CCharMsg::CHANNEL_GUILD, guild);
	CCharMsg::SetChannelColor(CCharMsg::CHANNEL_WORLD, world);
	CCharMsg::SetChannelColor(CCharMsg::CHANNEL_SYSTEM, system);
	CCharMsg::SetChannelColor(CCharMsg::CHANNEL_TRADE, trade);
	CCharMsg::SetChannelColor(CCharMsg::CHANNEL_PUBLISH, gm);
	return R_OK;
}

int UI_CreatePageItem(int page_id) {
	CPage* g = dynamic_cast<CPage*>(CGuiData::GetGui(page_id));
	if (!g)
		return R_FAIL;

	CPageItem* pitem = g->NewPage();
	return pitem->GetID();
}

int UI_SetPageButton(int page_id, int button_style, int bw, int bh) {
	CPage* g = dynamic_cast<CPage*>(CGuiData::GetGui(page_id));
	if (!g)
		return R_FAIL;

	g->SetButtonSize(bw, bh);
	g->SetButtonPutStyle((CPage::eButtonPos)button_style);
	return R_OK;
}

enum ePageItemObj {
	PAGE_ITEM_IMAGE = 0,
	PAGE_ITEM_TITLE,
};
int UI_GetPageItemObj(int page_item_id, int type) {
	CPageItem* g = dynamic_cast<CPageItem*>(CGuiData::GetGui(page_item_id));
	if (!g)
		return R_FAIL;

	if (type == PAGE_ITEM_IMAGE)
		return g->GetBkgImage()->GetID();
	else
		return g->GetTitle()->GetID();
}

int UI_AddCompent(int container_id, int compent_id) {
	CContainer* g = dynamic_cast<CContainer*>(CGuiData::GetGui(container_id));
	if (!g)
		return R_FAIL;

	CCompent* p = dynamic_cast<CCompent*>(CGuiData::GetGui(compent_id));
	if (!p)
		return R_FAIL;

	g->AddCompent(p);
	return R_OK;
}

int UI_CreateFont(char* font, int size800, int size1024, int nStyle) {
	return CGuiFont::s_Font.CreateFont(font, size800, size1024, (DWORD)nStyle);
}

int UI_SetLabelExShadowColor(int label_id, DWORD color) //FIXED: Color not working (changed int to DWORD)
{
	CLabelEx* g = dynamic_cast<CLabelEx*>(CGuiData::GetGui(label_id));
	if (!g)
		return R_FAIL;

	g->SetShadowColor(color);
	return R_OK;
}

int UI_ItemBarLoadImage(char* file, int w, int h, int tx, int ty) {
	CItemBar::LoadImage(file, w, h, tx, ty);
	return R_OK;
}

int UI_SetProgressHintStyle(int id, int style) {
	CProgressBar* g = dynamic_cast<CProgressBar*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	g->SetHintStyle((CProgressBar::eHintStyle)style);
	return R_OK;
}

int UI_SetProgressActiveMouse(int id, int style) {
	CProgressBar* g = dynamic_cast<CProgressBar*>(CGuiData::GetGui(id));
	if (!g)
		return R_FAIL;

	if (style > 0)
		g->SetActiveMouse(true);
	if (style == 0)
		g->SetActiveMouse(false);
	return R_OK;
}

//设定每行显示的英文字数
int UI_SetMemoMaxNumPerRow(int id, int num) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CMemo* memo = dynamic_cast<CMemo*>(p);
	if (memo) {
		memo->SetMaxNumPerRow(num);
		return R_OK;
	}

	CMemoEx* memoex = dynamic_cast<CMemoEx*>(p);
	if (memoex) {
		memoex->SetMaxNumPerRow(num);
		return R_OK;
	}

	return R_OK;
}

//设定Memo每页现实的行数
int UI_SetMemoPageShowNum(int id, int num) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CMemo* memo = dynamic_cast<CMemo*>(p);
	if (memo) {
		memo->SetPageShowNum(num);
		return R_OK;
	}
	CMemoEx* memoex = dynamic_cast<CMemoEx*>(p);
	if (memoex) {
		memoex->SetPageShowNum(num);
		return R_OK;
	}
	return R_OK;
}

//设定Memo每页现实的行数
int UI_SetMemoRowHeight(int id, int num) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CMemo* memo = dynamic_cast<CMemo*>(p);
	if (memo) {
		memo->SetRowHeight(num);
		return R_OK;
	}

	CMemoEx* memoex = dynamic_cast<CMemoEx*>(p);
	if (memoex) {
		memoex->SetRowHeight(num);
		return R_OK;
	}
	return R_OK;
}

int UI_RichSetMaxLine(int id, int line) {
	CRichMemo* s = dynamic_cast<CRichMemo*>(CGuiData::GetGui(id));
	if (!s)
		return R_FAIL;

	s->SetMaxLine((unsigned short)line);
	return R_OK;
}

int UI_RichSetClipRect(int id, int x0, int y0, int x1, int y1) {
	CRichMemo* s = dynamic_cast<CRichMemo*>(CGuiData::GetGui(id));
	if (!s)
		return R_FAIL;

	RECT rt = {x0, y0, x1, y1};
	s->SetClipRect(rt);
	return R_OK;
}

//---------------------------------------------------------------------------
// CHeadSay
//---------------------------------------------------------------------------
int UI_LoadHeadSayFaceImage(int num, int maxframe, int w, int h, char* file, int cw, int ch, int tx, int ty) {
	CGuiPic* p = CHeadSay::GetFacePic(num);
	if (!p)
		return R_FAIL;

	if (maxframe > 0) {
		p->SetMax(maxframe);
	} else {
		return R_FAIL;
	}

	p->LoadAllImage(file, cw, ch, tx, ty);
	p->SetScale(w, h);
	return R_OK;
}

int UI_LoadHeadSayShopImage(int num, int w, int h, char* file, int cw, int ch, int tx, int ty) {
	CGuiPic* p = CHeadSay::GetShopPic(num);
	if (!p)
		return R_FAIL;

	p->LoadImage(file, cw, ch, 0, tx, ty);
	p->SetScale(w, h);
	return R_OK;
}

int UI_LoadHeadSayLifeImage(int uw, int uh, char* file, int w, int h, int tx, int ty, int isHorizontal) {
	CGuiPic* p = CHeadSay::GetLifePic();
	if (!p)
		return R_FAIL;

	if (isHorizontal == FALSE) //FIXED: from "TRUE" - it used wrong orientation
	{
		p->LoadImage(file, w, h, 0, tx, ty);
		p->LoadImage(file, w, h, 1, tx + w, ty);
	} else {
		p->LoadImage(file, w, h, 0, tx, ty);
		p->LoadImage(file, w, h, 1, tx, ty + h);
	}
	p->SetScale(uw, uh);
	return R_OK;
}

int UI_SetDragSnapToGrid(int nGridWidth, int nGridHeight) {
	CDrag::SetSnapToGrid(nGridWidth, nGridHeight);
	return R_OK;
}

int UI_SetTextParse(int nIndex, char* file, int w, int h, int sx, int sy, int frame) //建立文字与图象的映射
{
	g_TextParse.AddFace(nIndex, file, w, h, sx, sy, frame);
	return R_OK;
}

int UI_SetFormStyle(int id, int index) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CForm* f = dynamic_cast<CForm*>(p);
	if (!f)
		return R_FAIL;

	f->SetStyle(CForm::eFormStyle(index));
	return R_OK;
}

int UI_SetFormStyleEx(int id, int index, int offWidth, int offHeight) {
	CGuiData* p = CGuiData::GetGui(id);
	if (!p)
		return R_FAIL;

	CForm* f = dynamic_cast<CForm*>(p);
	if (!f)
		return R_FAIL;

	f->SetStyle(CForm::eFormStyle(index), offWidth, offHeight);
	return R_OK;
}

// 菜单
int UI_MenuLoadSelect(int id, const char* imagefile, int w, int h, int sx, int sy) {
	CMenu* f = dynamic_cast<CMenu*>(CGuiData::GetGui(id));
	if (!f)
		return R_FAIL;

	f->GetSelectImage()->LoadImage(imagefile, w, h, 0, sx, sy);
	return R_OK;
}

int UI_MenuLoadImage(int id, int IsShowFrame, int IsTitle, const char* clientfile, int cw, int ch, int tx, int ty, const char* framefile, int w, int h) {
	CMenu* f = dynamic_cast<CMenu*>(CGuiData::GetGui(id));
	if (!f)
		return R_FAIL;

	CFramePic* frame = f->GetBkgImage();
	frame->SetIsTitle(IsTitle ? true : false);
	frame->SetIsShowFrame(IsShowFrame ? true : false);
	frame->LoadImage(clientfile, cw, ch, tx, ty, framefile, w, h);
	return R_OK;
}

int UI_MenuAddText(int id, const char* text) {
	CMenu* f = dynamic_cast<CMenu*>(CGuiData::GetGui(id));
	if (!f)
		return R_FAIL;

	f->AddMenu(text);
	return R_OK;
}

int UI_AddFilterTextToNameTable(const char* text) {
	return CTextFilter::Add(CTextFilter::NAME_TABLE, text);
}

int UI_AddFilterTextToDialogTable(const char* text) {
	return CTextFilter::Add(CTextFilter::DIALOG_TABLE, text);
}

int UI_SetHeadSayBkgColor(DWORD color) //FIXED: Color not working (changed int to DWORD)
{
	CHeadSay::SetBkgColor(color);
	return R_OK;
}

int UI_LoadSkillActiveImage(char* file, int maxframe, int w, int h, int sx, int sy) {
	CGuiPic* pic = CSkillCommand::GetActiveImage();
	pic->SetMax(maxframe);
	pic->LoadAllImage(file, w, h, sx, sy);
	return R_OK;
}

int UI_LoadChargeImage(int link, char* file, int maxframe, int w, int h, int sx, int sy) {
	CGuiPic* pic = CEquipMgr::GetChargePic(link);
	if (pic) {
		pic->SetMax(maxframe);
		pic->LoadAllImage(file, w, h, sx, sy);
		return R_OK;
	}
	return R_FAIL;
}

int UI_SetCaption(int id, char* caption) {
	CGuiData* p = CGuiData::GetGui(id);

	if (!p)
		return R_FAIL;

	p->SetCaption(caption);
	return R_OK;
}

// Add by lark.li
const char* UI_ResString(char* id) {
	const char* text = CResourceBundleManage::Instance()->LoadResString(id);

#if _DEBUG
	char buffer[255];
	_snprintf_s(buffer, _TRUNCATE, "UI_ResString\t%s\t%s\r\n", id, text);
	::OutputDebugStr(buffer);
#endif
	return text;
}

int UI_SetCaptionEx(int id, char* caption, int tail) {
	CGuiData* p = CGuiData::GetGui(id);

	if (!p)
		return R_FAIL;

	char buffer[255];
	_snprintf_s(buffer, _TRUNCATE, "%s%d", caption, tail);

	p->SetCaption(buffer);
	return R_OK;
}

//Added functions by deguix
int UI_MsgBox(char* text) {
	g_pGameApp->MsgBox(text);
	return R_OK;
}

// End

//---------------------------------------------------------------------------
// ScriptRegedit
//---------------------------------------------------------------------------
//MAJOR CHANGED: Removed CaLua dependency
#ifdef CALUA_COMPAT
void MPInitLua_Gui() {
	//TODO: Check if CaLua needs DWORD for colors
	CLU_RegisterFunction("GetChaPhotoTexID", "int", "int", CLU_CDECL, CLU_CAST(GetChaPhotoTexID));
	CLU_RegisterFunction("GetSceneObjPhotoTexID", "int", "int", CLU_CDECL, CLU_CAST(GetSceneObjPhotoTexID));
	CLU_RegisterFunction("GetEffectPhotoTexID", "int", "int", CLU_CDECL, CLU_CAST(GetEffectPhotoTexID));

	CLU_RegisterFunction("GetTextureID", "int", "char*", CLU_CDECL, CLU_CAST(GetTextureID));
	CLU_RegisterFunction("GetTerrainTextureID", "int", "int", CLU_CDECL, CLU_CAST(GetTerrainTextureID));
	CLU_RegisterFunction("GetTerrainTextureType", "int", "int", CLU_CDECL, CLU_CAST(GetTerrainTextureType));
	CLU_RegisterFunction("GetSceneObjPhotoTexType", "int", "int", CLU_CDECL, CLU_CAST(GetSceneObjPhotoTexType));

	// gui
	CLU_RegisterFunction("UI_LoadScript", "int", "char*", CLU_CDECL, CLU_CAST(UI_LoadScript));
	CLU_RegisterFunction("UI_SetDragSnapToGrid", "int", "int,int", CLU_CDECL, CLU_CAST(UI_SetDragSnapToGrid));

	CLU_RegisterFunction("UI_SetMaxImage", "int", "int,int", CLU_CDECL, CLU_CAST(UI_SetMaxImage));

	CLU_RegisterFunction("UI_SetFormTempleteMax", "int", "int", CLU_CDECL, CLU_CAST(UI_SetFormTempleteMax));
	CLU_RegisterFunction("UI_SwitchTemplete", "int", "int", CLU_CDECL, CLU_CAST(UI_SwitchTemplete));
	CLU_RegisterFunction("UI_AddAllFormTemplete", "int", "int", CLU_CDECL, CLU_CAST(UI_AddAllFormTemplete));
	CLU_RegisterFunction("UI_AddFormToTemplete", "int", "int,int", CLU_CDECL, CLU_CAST(UI_AddFormToTemplete));

	CLU_RegisterFunction("UI_CreateForm", "int", "char*, int, int, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_CreateForm));
	CLU_RegisterFunction("UI_FormSetIsEscClose", "int", "int, int", CLU_CDECL, CLU_CAST(UI_FormSetIsEscClose));
	CLU_RegisterFunction("UI_FormSetEnterButton", "int", "int, int", CLU_CDECL, CLU_CAST(UI_FormSetEnterButton));
	CLU_RegisterFunction("UI_FormSetHotKey", "int", "int, int, int", CLU_CDECL, CLU_CAST(UI_FormSetHotKey));
	CLU_RegisterFunction("UI_LoadFormImage", "int", "int, char*, int, int, int, int, char*, int, int", CLU_CDECL, CLU_CAST(UI_LoadFormImage));
	CLU_RegisterFunction("UI_ShowForm", "int", "int, int", CLU_CDECL, CLU_CAST(UI_ShowForm));
	CLU_RegisterFunction("UI_SetIsDrag", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetIsDrag));
	CLU_RegisterFunction("UI_SetIsKeyFocus", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetIsKeyFocus));
	CLU_RegisterFunction("UI_CreateCompent", "int", "int, int, char*, int, int, int, int", CLU_CDECL, CLU_CAST(UI_CreateCompent));

	// Add by lark.li 20090220 begin
	CLU_RegisterFunction("UI_SetFormInMianForm", "int", "int", CLU_CDECL, CLU_CAST(UI_SetFormInMianForm));
	// End

	CLU_RegisterFunction("UI_SetCaption", "int", "int, char*", CLU_CDECL, CLU_CAST(UI_SetCaption));
	CLU_RegisterFunction("UI_CopyImage", "int", "int, int", CLU_CDECL, CLU_CAST(UI_CopyImage));
	CLU_RegisterFunction("UI_SetSize", "int", "int, int, int", CLU_CDECL, CLU_CAST(UI_SetSize));
	CLU_RegisterFunction("UI_SetPos", "int", "int, int, int", CLU_CDECL, CLU_CAST(UI_SetPos));
	CLU_RegisterFunction("UI_SetTag", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetTag));
	CLU_RegisterFunction("UI_SetAlign", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetAlign));
	CLU_RegisterFunction("UI_LoadImage", "int", "int, char*, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_LoadImage));
	CLU_RegisterFunction("UI_LoadScaleImage", "int", "int, char*, int, int, int, int, int, float, float", CLU_CDECL, CLU_CAST(UI_LoadScaleImage));
	CLU_RegisterFunction("UI_SetHint", "int", "int, char*", CLU_CDECL, CLU_CAST(UI_SetHint));
	CLU_RegisterFunction("UI_LoadFlashScaleImage", "int", "int, int ,char*, int, int, int, int, int, float, float", CLU_CDECL, CLU_CAST(UI_LoadFlashScaleImage));
	CLU_RegisterFunction("UI_SetImageAlpha", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetImageAlpha));
	CLU_RegisterFunction("UI_ComboSetTextColor", "int", "int, int", CLU_CDECL, CLU_CAST(UI_ComboSetTextColor));
	CLU_RegisterFunction("UI_ComboSetStyle", "int", "int, int", CLU_CDECL, CLU_CAST(UI_ComboSetStyle));
	CLU_RegisterFunction("UI_LoadComboImage", "int", "int, char*, int, int, int, int, char*, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_LoadComboImage));
	CLU_RegisterFunction("UI_LoadButtonImage", "int", "int, char*, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_LoadButtonImage));
	CLU_RegisterFunction("UI_GetScroll", "int", "int", CLU_CDECL, CLU_CAST(UI_GetScroll));
	CLU_RegisterFunction("UI_CopyCompent", "int", "int, int", CLU_CDECL, CLU_CAST(UI_CopyCompent));
	CLU_RegisterFunction("UI_GetList", "int", "int", CLU_CDECL, CLU_CAST(UI_GetList));
	CLU_RegisterFunction("UI_AddListText", "int", "int, char*", CLU_CDECL, CLU_CAST(UI_AddListText));
	CLU_RegisterFunction("UI_ListLoadSelectImage", "int", "int, char*, int, int, int, int", CLU_CDECL, CLU_CAST(UI_ListLoadSelectImage));
	CLU_RegisterFunction("UI_LoadListItemImage", "int", "int, char*, int, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_LoadListItemImage));
	CLU_RegisterFunction("UI_ListSetItemMargin", "int", "int, int, int", CLU_CDECL, CLU_CAST(UI_ListSetItemMargin));
	CLU_RegisterFunction("UI_ListSetItemImageMargin", "int", "int, int, int", CLU_CDECL, CLU_CAST(UI_ListSetItemImageMargin));
	CLU_RegisterFunction("UI_SetListFontColor", "int", "int, int, int", CLU_CDECL, CLU_CAST(UI_SetListFontColor));
	CLU_RegisterFunction("UI_SetProgressStyle", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetProgressStyle));
	CLU_RegisterFunction("UI_SetScrollStyle", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetScrollStyle));

	CLU_RegisterFunction("UI_AddGroupBox", "int", "int, int", CLU_CDECL, CLU_CAST(UI_AddGroupBox));
	CLU_RegisterFunction("UI_CreateListView", "int", "int, char*, int, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_CreateListView));
	CLU_RegisterFunction("UI_ListViewSetTitle", "int", "int, int, int, char*, int, int, int, int", CLU_CDECL, CLU_CAST(UI_ListViewSetTitle));
	CLU_RegisterFunction("UI_ListViewSetTitleHeight", "int", "int, int", CLU_CDECL, CLU_CAST(UI_ListViewSetTitleHeight));
	CLU_RegisterFunction("UI_GoodGridLoadUnitImage", "int", "int, char*, int, int, int, int", CLU_CDECL, CLU_CAST(UI_GoodGridLoadUnitImage));

	CLU_RegisterFunction("UI_SetChatColor", "int", "int, int, int, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_SetChatColor));
	CLU_RegisterFunction("UI_SetListRowHeight", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetListRowHeight));
	CLU_RegisterFunction("UI_SetAlpha", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetAlpha));
	CLU_RegisterFunction("UI_GridLoadSelectImage", "int", "int, char*, int, int, int, int", CLU_CDECL, CLU_CAST(UI_GridLoadSelectImage));
	CLU_RegisterFunction("UI_SetGridIsDragSize", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetGridIsDragSize));
	CLU_RegisterFunction("UI_GetScrollObj", "int", "int, int", CLU_CDECL, CLU_CAST(UI_GetScrollObj));
	CLU_RegisterFunction("UI_GetEXListObj", "int", "int, int", CLU_CDECL, CLU_CAST(UI_GetEXListObj)); //Add by sunny.sun20080721
	CLU_RegisterFunction("UI_SetGridUnitSize", "int", "int, int, int", CLU_CDECL, CLU_CAST(UI_SetGridUnitSize));
	CLU_RegisterFunction("UI_AddFaceToGrid", "int", "int, char*, int, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_AddFaceToGrid));
	CLU_RegisterFunction("UI_SetIsShow", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetIsShow));
	CLU_RegisterFunction("UI_SetIsEnabled", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetIsEnabled));
	CLU_RegisterFunction("UI_SetMargin", "int", "int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_SetMargin));
	CLU_RegisterFunction("UI_LoadListFixSelect", "int", "int, char*, int, int, int, int", CLU_CDECL, CLU_CAST(UI_LoadListFixSelect));
	CLU_RegisterFunction("UI_FixListSetMaxNum", "int", "int, int", CLU_CDECL, CLU_CAST(UI_FixListSetMaxNum));
	CLU_RegisterFunction("UI_FixListSetText", "int", "int, int, char*", CLU_CDECL, CLU_CAST(UI_FixListSetText));
	CLU_RegisterFunction("UI_FixListSetRowSpace", "int", "int, int", CLU_CDECL, CLU_CAST(UI_FixListSetRowSpace));
	CLU_RegisterFunction("UI_SetListIsMouseFollow", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetListIsMouseFollow));

	CLU_RegisterFunction("UI_CheckFixListSetCheckMargin", "int", "int, int, int", CLU_CDECL, CLU_CAST(UI_CheckFixListSetCheckMargin));
	CLU_RegisterFunction("UI_LoadCheckFixListCheck", "int", "int, char*, int, int, int, int, char*, int, int, int, int", CLU_CDECL, CLU_CAST(UI_LoadCheckFixListCheck));

	CLU_RegisterFunction("UI_SetEditCursorColor", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetEditCursorColor));
	CLU_RegisterFunction("UI_SetEditMaxNum", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetEditMaxNum));
	CLU_RegisterFunction("UI_SetEditEnterButton", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetEditEnterButton));
	CLU_RegisterFunction("UI_SetEditMaxNumVisible", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetEditMaxNumVisible));
	CLU_RegisterFunction("UI_SetProgressHintStyle", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetProgressHintStyle));

	CLU_RegisterFunction("UI_SetButtonModalResult", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetButtonModalResult));
	CLU_RegisterFunction("UI_SetTextColor", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetTextColor));
	CLU_RegisterFunction("UI_LoadFrameImage", "int", "int, char*, int, int, int, int, char*, int, int", CLU_CDECL, CLU_CAST(UI_LoadFrameImage));
	CLU_RegisterFunction("UI_CreatePageItem", "int", "int", CLU_CDECL, CLU_CAST(UI_CreatePageItem));
	CLU_RegisterFunction("UI_GetPageItemObj", "int", "int, int", CLU_CDECL, CLU_CAST(UI_GetPageItemObj));

	CLU_RegisterFunction("UI_AddCompent", "int", "int, int", CLU_CDECL, CLU_CAST(UI_AddCompent));
	CLU_RegisterFunction("UI_SetLabelExShadowColor", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetLabelExShadowColor));
	CLU_RegisterFunction("UI_SetPageButton", "int", "int, int, int, int", CLU_CDECL, CLU_CAST(UI_SetPageButton));
	CLU_RegisterFunction("UI_ButtonSetHint", "int", "int, char*", CLU_CDECL, CLU_CAST(UI_ButtonSetHint));

	CLU_RegisterFunction("UI_TreeLoadImage", "int", "int, int, char*, int, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_TreeLoadImage));
	CLU_RegisterFunction("UI_CreateTextItem", "int", "char*, int", CLU_CDECL, CLU_CAST(UI_CreateTextItem));
	CLU_RegisterFunction("UI_CreateNoteGraphItem", "int", "char*, int, int, int, int, int, char*, int, int", CLU_CDECL, CLU_CAST(UI_CreateNoteGraphItem));
	CLU_RegisterFunction("UI_CreateGraphItem", "int", "char*, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_CreateGraphItem));
	CLU_RegisterFunction("UI_CreateSingleNode", "int", "int, int, int", CLU_CDECL, CLU_CAST(UI_CreateSingleNode));
	CLU_RegisterFunction("UI_CreateGridNode", "int", "int,int,int,int,int,int", CLU_CDECL, CLU_CAST(UI_CreateGridNode));
	CLU_RegisterFunction("UI_GridNodeAddItem", "int", "int,int", CLU_CDECL, CLU_CAST(UI_GridNodeAddItem));
	CLU_RegisterFunction("UI_CreateGraphItemTex", "int", "int,int,int,int,float,float,int,int", CLU_CDECL, CLU_CAST(UI_CreateGraphItemTex));
	CLU_RegisterFunction("UI_SetLabelExFont", "int", "int,int,int,int", CLU_CDECL, CLU_CAST(UI_SetLabelExFont));
	CLU_RegisterFunction("UI_SetGridSpace", "int", "int,int,int", CLU_CDECL, CLU_CAST(UI_SetGridSpace));
	CLU_RegisterFunction("UI_SetGridContent", "int", "int,int,int", CLU_CDECL, CLU_CAST(UI_SetGridContent));

	CLU_RegisterFunction("UI_ItemBarLoadImage", "int", "char*,int,int,int,int", CLU_CDECL, CLU_CAST(UI_ItemBarLoadImage));
	CLU_RegisterFunction("UI_AddListBarText", "int", "int,char*,float", CLU_CDECL, CLU_CAST(UI_AddListBarText));

	// headsay
	CLU_RegisterFunction("UI_LoadHeadSayFaceImage", "int", "int,int,int,int,char*,int,int,int,int", CLU_CDECL, CLU_CAST(UI_LoadHeadSayFaceImage));
	CLU_RegisterFunction("UI_LoadHeadSayShopImage", "int", "int,int,int,char*,int,int,int,int", CLU_CDECL, CLU_CAST(UI_LoadHeadSayShopImage));
	CLU_RegisterFunction("UI_LoadHeadSayLifeImage", "int", "int,int,char*,int,int,int,int,int", CLU_CDECL, CLU_CAST(UI_LoadHeadSayLifeImage));

	//from style
	CLU_RegisterFunction("UI_SetFormStyle", "int", "int,int", CLU_CDECL, CLU_CAST(UI_SetFormStyle));
	CLU_RegisterFunction("UI_SetFormStyleEx", "int", "int,int,int,int", CLU_CDECL, CLU_CAST(UI_SetFormStyleEx));

	// font
	CLU_RegisterFunction("UI_CreateFont", "int", "char*,int,int,int", CLU_CDECL, CLU_CAST(UI_CreateFont));

	//add graph to text
	CLU_RegisterFunction("UI_SetTextParse", "int", "int,  char*, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_SetTextParse));

	// memo
	CLU_RegisterFunction("UI_SetMemoMaxNumPerRow", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetMemoMaxNumPerRow));
	CLU_RegisterFunction("UI_SetMemoPageShowNum", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetMemoPageShowNum));
	CLU_RegisterFunction("UI_SetMemoRowHeight", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetMemoRowHeight));

	// Rich
	CLU_RegisterFunction("UI_RichSetClipRect", "int", "int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_RichSetClipRect));
	CLU_RegisterFunction("UI_RichSetMaxLine", "int", "int, int", CLU_CDECL, CLU_CAST(UI_RichSetMaxLine));

	CLU_RegisterFunction("UI_SetProgressActiveMouse", "int", "int, int", CLU_CDECL, CLU_CAST(UI_SetProgressActiveMouse));
	CLU_RegisterFunction("UI_LoadSkillListButtonImage", "int", "int, char*, int, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_LoadSkillListButtonImage));

	// 菜单
	CLU_RegisterFunction("UI_MenuLoadSelect", "int", "int, char*, int, int, int, int", CLU_CDECL, CLU_CAST(UI_MenuLoadSelect));
	CLU_RegisterFunction("UI_MenuLoadImage", "int", "int, int, int, char*, int, int, int, int, char*, int, int", CLU_CDECL, CLU_CAST(UI_MenuLoadImage));
	CLU_RegisterFunction("UI_MenuAddText", "int", "int, char*", CLU_CDECL, CLU_CAST(UI_MenuAddText));
	CLU_RegisterFunction("UI_AddFilterTextToNameTable", "int", "char*", CLU_CDECL, CLU_CAST(UI_AddFilterTextToNameTable));
	CLU_RegisterFunction("UI_AddFilterTextToDialogTable", "int", "char*", CLU_CDECL, CLU_CAST(UI_AddFilterTextToDialogTable));

	CLU_RegisterFunction("UI_SetHeadSayBkgColor", "int", "int", CLU_CDECL, CLU_CAST(UI_SetHeadSayBkgColor));

	CLU_RegisterFunction("UI_SetTitleFont", "int", "int, int, int, int", CLU_CDECL, CLU_CAST(UI_SetTitleFont));

	CLU_RegisterFunction("UI_LoadSkillActiveImage", "int", "char*, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_LoadSkillActiveImage));
	CLU_RegisterFunction("UI_LoadChargeImage", "int", "int, char*, int, int, int, int, int", CLU_CDECL, CLU_CAST(UI_LoadChargeImage));

	// Add by lark.li
	char ret[] = "char*";
	CLU_RegisterFunction("UI_ResString", ret, "char*", CLU_CDECL, CLU_CAST(UI_ResString));
	CLU_RegisterFunction("UI_SetCaptionEx", "int", "int, char*, int", CLU_CDECL, CLU_CAST(UI_SetCaptionEx));

	// Added by deguix
	CLU_RegisterFunction("UI_MsgBox", "int", "char*", CLU_CDECL, CLU_CAST(UI_MsgBox));
}
#else
LUA_FUNC_ARITY1(GetChaPhotoTexID,
				number, number,
				int, int,
				LUA_FUNC_NIL_HANDLING(0));
LUA_FUNC_ARITY1(GetSceneObjPhotoTexID,
				number, number,
				int, int,
				LUA_FUNC_NIL_HANDLING(0));
LUA_FUNC_ARITY1(GetEffectPhotoTexID,
				number, number,
				int, int,
				LUA_FUNC_NIL_HANDLING(0));

LUA_FUNC_ARITY1(GetTextureID,
				number, string,
				int, char*);
LUA_FUNC_ARITY1(GetTerrainTextureID,
				number, number,
				int, int,
				LUA_FUNC_NIL_HANDLING(0));
LUA_FUNC_ARITY1(GetTerrainTextureType,
				number, number,
				int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY1(GetSceneObjPhotoTexType,
				number, number,
				int, int,
				LUA_FUNC_NIL_HANDLING(-1));

// gui
LUA_FUNC_ARITY1(UI_LoadScript,
				number, string,
				int, char*);
LUA_FUNC_ARITY2(UI_SetDragSnapToGrid,
				number, number, number,
				int, int, int);

LUA_FUNC_ARITY2(UI_SetMaxImage,
				number, number, number,
				int, int, int);

LUA_FUNC_ARITY1(UI_SetFormTempleteMax,
				number, number,
				int, int);
LUA_FUNC_ARITY1(UI_SwitchTemplete,
				number, number,
				int, int);
LUA_FUNC_ARITY1(UI_AddAllFormTemplete,
				number, number,
				int, int);
LUA_FUNC_ARITY2(UI_AddFormToTemplete,
				number, number, number,
				int, int, int);

LUA_FUNC_ARITY8(UI_CreateForm,
				number, string, number, number, number, number, number, number, number,
				int, char*, int, int, int, int, int, int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY2(UI_FormSetIsEscClose,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_FormSetEnterButton,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY3(UI_FormSetHotKey,
				number, number, number, number,
				int, int, int, int);
LUA_FUNC_ARITY9(UI_LoadFormImage,
				number, number, string, number, number, number, number, string, number, number,
				int, int, char*, int, int, int, int, char*, int, int);
LUA_FUNC_ARITY2(UI_ShowForm,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetIsDrag,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetIsKeyFocus,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY7(UI_CreateCompent,
				number, number, number, string, number, number, number, number,
				int, int, int, char*, int, int, int, int,
				LUA_FUNC_NIL_HANDLING(-1));

// Add by lark.li 20090220 begin
LUA_FUNC_ARITY1(UI_SetFormInMianForm,
				number, number,
				int, int);
// End

LUA_FUNC_ARITY2(UI_SetCaption,
				number, number, string,
				int, int, char*);
LUA_FUNC_ARITY2(UI_CopyImage,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY3(UI_SetSize,
				number, number, number, number,
				int, int, int, int);
LUA_FUNC_ARITY3(UI_SetPos,
				number, number, number, number,
				int, int, int, int);
LUA_FUNC_ARITY2(UI_SetTag,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetAlign,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY7(UI_LoadImage,
				number, number, string, number, number, number, number, number,
				int, int, char*, int, int, int, int, int);
LUA_FUNC_ARITY9(UI_LoadScaleImage,
				number, number, string, number, number, number, number, number, number, number,
				int, int, char*, int, int, int, int, int, float, float);
LUA_FUNC_ARITY2(UI_SetHint,
				number, number, string,
				int, int, char*);
LUA_FUNC_ARITY10(UI_LoadFlashScaleImage,
				 number, number, number, string, number, number, number, number, number, number, number,
				 int, int, int, char*, int, int, int, int, int, float, float);
LUA_FUNC_ARITY2(UI_SetImageAlpha,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_ComboSetTextColor,
				number, number, number,
				int, int, DWORD); //FIXED: Color not working (changed int to DWORD)
LUA_FUNC_ARITY2(UI_ComboSetStyle,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY12(UI_LoadComboImage,
				 number, number, string, number, number, number, number, string, number, number, number, number, number,
				 int, int, char*, int, int, int, int, char*, int, int, int, int, int);
LUA_FUNC_ARITY7(UI_LoadButtonImage,
				number, number, string, number, number, number, number, number,
				int, int, char*, int, int, int, int, int);
LUA_FUNC_ARITY1(UI_GetScroll,
				number, number,
				int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY2(UI_CopyCompent,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY1(UI_GetList,
				number, number,
				int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY2(UI_AddListText,
				number, number, string,
				int, int, char*);
LUA_FUNC_ARITY6(UI_ListLoadSelectImage,
				number, number, string, number, number, number, number,
				int, int, char*, int, int, int, int);
LUA_FUNC_ARITY8(UI_LoadListItemImage,
				number, number, string, number, number, number, number, number, number,
				int, int, char*, int, int, int, int, int, int);
LUA_FUNC_ARITY3(UI_ListSetItemMargin,
				number, number, number, number,
				int, int, int, int);
LUA_FUNC_ARITY3(UI_ListSetItemImageMargin,
				number, number, number, number,
				int, int, int, int);
LUA_FUNC_ARITY3(UI_SetListFontColor,
				number, number, number, number,
				int, int, DWORD, DWORD); //FIXED: Color not working (changed int to DWORD)
LUA_FUNC_ARITY2(UI_SetProgressStyle,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetScrollStyle,
				number, number, number,
				int, int, int);

LUA_FUNC_ARITY2(UI_AddGroupBox,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY8(UI_CreateListView,
				number, number, string, number, number, number, number, number, number,
				int, int, char*, int, int, int, int, int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY8(UI_ListViewSetTitle,
				number, number, number, number, string, number, number, number, number,
				int, int, int, int, char*, int, int, int, int);
LUA_FUNC_ARITY2(UI_ListViewSetTitleHeight,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY6(UI_GoodGridLoadUnitImage,
				number, number, string, number, number, number, number,
				int, int, char*, int, int, int, int);

LUA_FUNC_ARITY8(UI_SetChatColor,
				number, number, number, number, number, number, number, number, number,
				DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD); //FIXED: Color not working (changed int to DWORD)
LUA_FUNC_ARITY2(UI_SetListRowHeight,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetAlpha,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY6(UI_GridLoadSelectImage,
				number, number, string, number, number, number, number,
				int, int, char*, int, int, int, int);
LUA_FUNC_ARITY2(UI_SetGridIsDragSize,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_GetScrollObj,
				number, number, number,
				int, int, int, LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY2(UI_GetEXListObj,
				number, number, number,
				int, int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY3(UI_SetGridUnitSize,
				number, number, number, number,
				int, int, int, int);
LUA_FUNC_ARITY8(UI_AddFaceToGrid,
				number, number, string, number, number, number, number, number, number,
				int, int, char*, int, int, int, int, int, int);
LUA_FUNC_ARITY2(UI_SetIsShow,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetIsEnabled,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY5(UI_SetMargin,
				number, number, number, number, number, number,
				int, int, int, int, int, int);
LUA_FUNC_ARITY6(UI_LoadListFixSelect,
				number, number, string, number, number, number, number,
				int, int, char*, int, int, int, int);
LUA_FUNC_ARITY2(UI_FixListSetMaxNum,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY3(UI_FixListSetText,
				number, number, number, string,
				int, int, int, char*);
LUA_FUNC_ARITY2(UI_FixListSetRowSpace,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetListIsMouseFollow,
				number, number, number,
				int, int, int);

LUA_FUNC_ARITY3(UI_CheckFixListSetCheckMargin,
				number, number, number, number,
				int, int, int, int);
LUA_FUNC_ARITY11(UI_LoadCheckFixListCheck,
				 number, number, string, number, number, number, number, string, number, number, number, number,
				 int, int, char*, int, int, int, int, char*, int, int, int, int);

LUA_FUNC_ARITY2(UI_SetEditCursorColor,
				number, number, number,
				int, int, DWORD); //FIXED: Color not working (changed int to DWORD)
LUA_FUNC_ARITY2(UI_SetEditMaxNum,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetEditEnterButton,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetEditMaxNumVisible,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetProgressHintStyle,
				number, number, number,
				int, int, int);

LUA_FUNC_ARITY2(UI_SetButtonModalResult,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetTextColor,
				number, number, number,
				int, int, DWORD); //FIXED: Color not working (changed int to DWORD)
LUA_FUNC_ARITY9(UI_LoadFrameImage,
				number, number, string, number, number, number, number, string, number, number,
				int, int, char*, int, int, int, int, char*, int, int);
LUA_FUNC_ARITY1(UI_CreatePageItem,
				number, number,
				int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY2(UI_GetPageItemObj,
				number, number, number,
				int, int, int,
				LUA_FUNC_NIL_HANDLING(-1));

LUA_FUNC_ARITY2(UI_AddCompent,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetLabelExShadowColor,
				number, number, number,
				int, int, DWORD); //FIXED: Color not working (changed int to DWORD)
LUA_FUNC_ARITY4(UI_SetPageButton,
				number, number, number, number, number,
				int, int, int, int, int);
LUA_FUNC_ARITY2(UI_ButtonSetHint,
				number, number, string,
				int, int, char*);

LUA_FUNC_ARITY9(UI_TreeLoadImage,
				number, number, number, string, number, number, number, number, number, number,
				int, int, int, char*, int, int, int, int, int, int);
LUA_FUNC_ARITY2(UI_CreateTextItem,
				number, string, number,
				int, char*, DWORD,
				LUA_FUNC_NIL_HANDLING(-1)); //FIXED: Color not working (changed int to DWORD)
LUA_FUNC_ARITY9(UI_CreateNoteGraphItem,
				number, string, number, number, number, number, number, string, number, number,
				int, char*, int, int, int, int, int, char*, int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY6(UI_CreateGraphItem,
				number, string, number, number, number, number, number,
				int, char*, int, int, int, int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY3(UI_CreateSingleNode,
				number, number, number, number,
				int, int, int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY6(UI_CreateGridNode,
				number, number, number, number, number, number, number,
				int, int, int, int, int, int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY2(UI_GridNodeAddItem,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY8(UI_CreateGraphItemTex,
				number, number, number, number, number, number, number, number, number,
				int, int, int, int, int, float, float, int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY4(UI_SetLabelExFont,
				number, number, number, number, number,
				int, int, int, int, int);
LUA_FUNC_ARITY3(UI_SetGridSpace,
				number, number, number, number,
				int, int, int, int);
LUA_FUNC_ARITY3(UI_SetGridContent,
				number, number, number, number,
				int, int, int, int);

LUA_FUNC_ARITY5(UI_ItemBarLoadImage,
				number, string, number, number, number, number,
				int, char*, int, int, int, int);
LUA_FUNC_ARITY3(UI_AddListBarText,
				number, number, string, number,
				int, int, char*, float);

// headsay
LUA_FUNC_ARITY9(UI_LoadHeadSayFaceImage,
				number, number, number, number, number, string, number, number, number, number,
				int, int, int, int, int, char*, int, int, int, int);
LUA_FUNC_ARITY8(UI_LoadHeadSayShopImage,
				number, number, number, number, string, number, number, number, number,
				int, int, int, int, char*, int, int, int, int);
LUA_FUNC_ARITY8(UI_LoadHeadSayLifeImage,
				number, number, number, string, number, number, number, number, number,
				int, int, int, char*, int, int, int, int, int);

//from style
LUA_FUNC_ARITY2(UI_SetFormStyle,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY4(UI_SetFormStyleEx,
				number, number, number, number, number,
				int, int, int, int, int);

// font
LUA_FUNC_ARITY4(UI_CreateFont,
				number, string, number, number, number,
				int, char*, int, int, int);

//add graph to text
LUA_FUNC_ARITY7(UI_SetTextParse,
				number, number, string, number, number, number, number, number,
				int, int, char*, int, int, int, int, int);

// memo
LUA_FUNC_ARITY2(UI_SetMemoMaxNumPerRow,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetMemoPageShowNum,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY2(UI_SetMemoRowHeight,
				number, number, number,
				int, int, int);

// Rich
LUA_FUNC_ARITY5(UI_RichSetClipRect,
				number, number, number, number, number, number,
				int, int, int, int, int, int);
LUA_FUNC_ARITY2(UI_RichSetMaxLine,
				number, number, number,
				int, int, int);

LUA_FUNC_ARITY2(UI_SetProgressActiveMouse,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY8(UI_LoadSkillListButtonImage,
				number, number, string, number, number, number, number, number, number,
				int, int, char*, int, int, int, int, int, int);

// 菜单
LUA_FUNC_ARITY6(UI_MenuLoadSelect,
				number, number, string, number, number, number, number,
				int, int, char*, int, int, int, int);
LUA_FUNC_ARITY11(UI_MenuLoadImage,
				 number, number, number, number, string, number, number, number, number, string, number, number,
				 int, int, int, int, char*, int, int, int, int, char*, int, int);
LUA_FUNC_ARITY2(UI_MenuAddText,
				number, number, string,
				int, int, char*);
LUA_FUNC_ARITY1(UI_AddFilterTextToNameTable,
				number, string,
				int, char*);
LUA_FUNC_ARITY1(UI_AddFilterTextToDialogTable,
				number, string,
				int, char*);

LUA_FUNC_ARITY1(UI_SetHeadSayBkgColor,
				number, number,
				int, DWORD); //FIXED: Color not working (changed int to DWORD)

LUA_FUNC_ARITY4(UI_SetTitleFont,
				number, number, number, number, number,
				int, int, int, DWORD, int); //FIXED: Color not working (changed int to DWORD)

LUA_FUNC_ARITY6(UI_LoadSkillActiveImage,
				number, string, number, number, number, number, number,
				int, char*, int, int, int, int, int);
LUA_FUNC_ARITY7(UI_LoadChargeImage,
				number, number, string, number, number, number, number, number,
				int, int, char*, int, int, int, int, int);

// Add by lark.li
LUA_FUNC_ARITY1(UI_ResString,
				string, string,
				char*, char*);
LUA_FUNC_ARITY3(UI_SetCaptionEx,
				number, number, string, number,
				int, int, char*, int);

// Added by deguix
LUA_FUNC_ARITY1(UI_MsgBox,
				number, string,
				int, char*);

void MPInitLua_Gui(lua_State* L) {
	LUA_FUNC_REG(GetChaPhotoTexID);
	LUA_FUNC_REG(GetSceneObjPhotoTexID);
	LUA_FUNC_REG(GetEffectPhotoTexID);

	LUA_FUNC_REG(GetTextureID);
	LUA_FUNC_REG(GetTerrainTextureID);
	LUA_FUNC_REG(GetTerrainTextureType);
	LUA_FUNC_REG(GetSceneObjPhotoTexType);

	// gui
	LUA_FUNC_REG(UI_LoadScript);
	LUA_FUNC_REG(UI_SetDragSnapToGrid);

	LUA_FUNC_REG(UI_SetMaxImage);

	LUA_FUNC_REG(UI_SetFormTempleteMax);
	LUA_FUNC_REG(UI_SwitchTemplete);
	LUA_FUNC_REG(UI_AddAllFormTemplete);
	LUA_FUNC_REG(UI_AddFormToTemplete);

	LUA_FUNC_REG(UI_CreateForm);
	LUA_FUNC_REG(UI_FormSetIsEscClose);
	LUA_FUNC_REG(UI_FormSetEnterButton);
	LUA_FUNC_REG(UI_FormSetHotKey);
	LUA_FUNC_REG(UI_LoadFormImage);
	LUA_FUNC_REG(UI_ShowForm);
	LUA_FUNC_REG(UI_SetIsDrag);
	LUA_FUNC_REG(UI_SetIsKeyFocus);
	LUA_FUNC_REG(UI_CreateCompent);

	// Add by lark.li 20090220 begin
	LUA_FUNC_REG(UI_SetFormInMianForm);
	// End

	LUA_FUNC_REG(UI_SetCaption);
	LUA_FUNC_REG(UI_CopyImage);
	LUA_FUNC_REG(UI_SetSize);
	LUA_FUNC_REG(UI_SetPos);
	LUA_FUNC_REG(UI_SetTag);
	LUA_FUNC_REG(UI_SetAlign);
	LUA_FUNC_REG(UI_LoadImage);
	LUA_FUNC_REG(UI_LoadScaleImage);
	LUA_FUNC_REG(UI_SetHint);
	LUA_FUNC_REG(UI_LoadFlashScaleImage);
	LUA_FUNC_REG(UI_SetImageAlpha);
	LUA_FUNC_REG(UI_ComboSetTextColor);
	LUA_FUNC_REG(UI_ComboSetStyle);
	LUA_FUNC_REG(UI_LoadComboImage);
	LUA_FUNC_REG(UI_LoadButtonImage);
	LUA_FUNC_REG(UI_GetScroll);
	LUA_FUNC_REG(UI_CopyCompent);
	LUA_FUNC_REG(UI_GetList);
	LUA_FUNC_REG(UI_AddListText);
	LUA_FUNC_REG(UI_ListLoadSelectImage);
	LUA_FUNC_REG(UI_LoadListItemImage);
	LUA_FUNC_REG(UI_ListSetItemMargin);
	LUA_FUNC_REG(UI_ListSetItemImageMargin);
	LUA_FUNC_REG(UI_SetListFontColor);
	LUA_FUNC_REG(UI_SetProgressStyle);
	LUA_FUNC_REG(UI_SetScrollStyle);

	LUA_FUNC_REG(UI_AddGroupBox);
	LUA_FUNC_REG(UI_CreateListView);
	LUA_FUNC_REG(UI_ListViewSetTitle);
	LUA_FUNC_REG(UI_ListViewSetTitleHeight);
	LUA_FUNC_REG(UI_GoodGridLoadUnitImage);

	LUA_FUNC_REG(UI_SetChatColor);
	LUA_FUNC_REG(UI_SetListRowHeight);
	LUA_FUNC_REG(UI_SetAlpha);
	LUA_FUNC_REG(UI_GridLoadSelectImage);
	LUA_FUNC_REG(UI_SetGridIsDragSize);
	LUA_FUNC_REG(UI_GetScrollObj);
	LUA_FUNC_REG(UI_GetEXListObj); //Add by sunny.sun20080721
	LUA_FUNC_REG(UI_SetGridUnitSize);
	LUA_FUNC_REG(UI_AddFaceToGrid);
	LUA_FUNC_REG(UI_SetIsShow);
	LUA_FUNC_REG(UI_SetIsEnabled);
	LUA_FUNC_REG(UI_SetMargin);
	LUA_FUNC_REG(UI_LoadListFixSelect);
	LUA_FUNC_REG(UI_FixListSetMaxNum);
	LUA_FUNC_REG(UI_FixListSetText);
	LUA_FUNC_REG(UI_FixListSetRowSpace);
	LUA_FUNC_REG(UI_SetListIsMouseFollow);

	LUA_FUNC_REG(UI_CheckFixListSetCheckMargin);
	LUA_FUNC_REG(UI_LoadCheckFixListCheck);

	LUA_FUNC_REG(UI_SetEditCursorColor);
	LUA_FUNC_REG(UI_SetEditMaxNum);
	LUA_FUNC_REG(UI_SetEditEnterButton);
	LUA_FUNC_REG(UI_SetEditMaxNumVisible);
	LUA_FUNC_REG(UI_SetProgressHintStyle);

	LUA_FUNC_REG(UI_SetButtonModalResult);
	LUA_FUNC_REG(UI_SetTextColor);
	LUA_FUNC_REG(UI_LoadFrameImage);
	LUA_FUNC_REG(UI_CreatePageItem);
	LUA_FUNC_REG(UI_GetPageItemObj);

	LUA_FUNC_REG(UI_AddCompent);
	LUA_FUNC_REG(UI_SetLabelExShadowColor);
	LUA_FUNC_REG(UI_SetPageButton);
	LUA_FUNC_REG(UI_ButtonSetHint);

	LUA_FUNC_REG(UI_TreeLoadImage);
	LUA_FUNC_REG(UI_CreateTextItem);
	LUA_FUNC_REG(UI_CreateNoteGraphItem);
	LUA_FUNC_REG(UI_CreateGraphItem);
	LUA_FUNC_REG(UI_CreateSingleNode);
	LUA_FUNC_REG(UI_CreateGridNode);
	LUA_FUNC_REG(UI_GridNodeAddItem);
	LUA_FUNC_REG(UI_CreateGraphItemTex);
	LUA_FUNC_REG(UI_SetLabelExFont);
	LUA_FUNC_REG(UI_SetGridSpace);
	LUA_FUNC_REG(UI_SetGridContent);

	LUA_FUNC_REG(UI_ItemBarLoadImage);
	LUA_FUNC_REG(UI_AddListBarText);

	// headsay
	LUA_FUNC_REG(UI_LoadHeadSayFaceImage);
	LUA_FUNC_REG(UI_LoadHeadSayShopImage);
	LUA_FUNC_REG(UI_LoadHeadSayLifeImage);

	//from style
	LUA_FUNC_REG(UI_SetFormStyle);
	LUA_FUNC_REG(UI_SetFormStyleEx);

	// font
	LUA_FUNC_REG(UI_CreateFont);

	//add graph to text
	LUA_FUNC_REG(UI_SetTextParse);

	// memo
	LUA_FUNC_REG(UI_SetMemoMaxNumPerRow);
	LUA_FUNC_REG(UI_SetMemoPageShowNum);
	LUA_FUNC_REG(UI_SetMemoRowHeight);

	// Rich
	LUA_FUNC_REG(UI_RichSetClipRect);
	LUA_FUNC_REG(UI_RichSetMaxLine);

	LUA_FUNC_REG(UI_SetProgressActiveMouse);
	LUA_FUNC_REG(UI_LoadSkillListButtonImage);

	// 菜单
	LUA_FUNC_REG(UI_MenuLoadSelect);
	LUA_FUNC_REG(UI_MenuLoadImage);
	LUA_FUNC_REG(UI_MenuAddText);
	LUA_FUNC_REG(UI_AddFilterTextToNameTable);
	LUA_FUNC_REG(UI_AddFilterTextToDialogTable);

	LUA_FUNC_REG(UI_SetHeadSayBkgColor);

	LUA_FUNC_REG(UI_SetTitleFont);

	LUA_FUNC_REG(UI_LoadSkillActiveImage);
	LUA_FUNC_REG(UI_LoadChargeImage);

	// Add by lark.li
	LUA_FUNC_REG(UI_ResString);
	LUA_FUNC_REG(UI_SetCaptionEx);

	// Added by deguix
	LUA_FUNC_REG(UI_MsgBox);
}
#endif