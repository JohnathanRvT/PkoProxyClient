

/* * * * * * * * * * * * * * * * * * * *

    控件基类封装
    Jampe
    2006/9/12

 * * * * * * * * * * * * * * * * * * * */

#if !defined __MYCONTROLS_H__
#define __MYCONTROLS_H__

#include "datatype.h"
#include <commctrl.h>

class MyControls {
public:
	MyControls();
	virtual ~MyControls();

	virtual void InitCommon();
	virtual void GetHandle(HWND parent, int resID);

public:
	HWND m_hWnd;
};

#endif
