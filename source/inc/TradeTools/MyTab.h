
/* * * * * * * * * * * * * * * * * * * *

    标签控件封装
    Jampe
    2006/9/12

 * * * * * * * * * * * * * * * * * * * */

#if !defined __MYTAB_H__
#define __MYTAB_H__

#include "MyControls.h"

class MyTab : public MyControls {
public:
	MyTab();
	MyTab(HWND parent, int ctlid);
	virtual ~MyTab();

	void InsertItem(int idx, const char* txt, int img = -1, int mask = TCIF_TEXT | TCIF_IMAGE);
	int GetCurrentSelect();
	int GetCurrentFocus();
	void SetCurrentSelect(int idx);
};

#endif
