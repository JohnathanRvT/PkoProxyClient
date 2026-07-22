//----------------------------------------------------------------------
// 名称:RichEdit
// 作者:lh 2005-07-11
//----------------------------------------------------------------------
#pragma once
#include "uicompent.h"
#include "uieditdata.h"

namespace GUI {

class CEditKey;
class CRichEdit : public CCompent {
public:
	CRichEdit(CForm& frmOwn);
	CRichEdit(const CRichEdit& rhs);
	CRichEdit& operator=(const CRichEdit& rhs);
	virtual ~CRichEdit();
	GUI_CLONE(CRichEdit)

	virtual void Init() override;
	virtual void Refresh() override;
	virtual void Render() override;
	virtual void OnActive() override;
	virtual void OnLost() override;

	bool OnKeyDown(int key) override;
	bool OnChar(char c) override;

private:
	void _SetSelf();
	void _Copy(const CRichEdit& rhs);

private:
	CEditArticle _cArticle;
	bool _IsReadyOnly; // 是否只读
	CEditKey* _pEditKey;
};

} // namespace GUI
