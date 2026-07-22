
/* * * * * * * * * * * * * * * * * * * *

    组合控件封装
    Jampe
    2006/4/12

 * * * * * * * * * * * * * * * * * * * */

#if !defined __MYCOMBOBOX_H__
#define __MYCOMBOBOX_H__

#include "MyControls.h"

class MyComboBox : public MyControls {
public:
	MyComboBox();
	MyComboBox(HWND parent, int ctlid);
	virtual ~MyComboBox();

	//  获取文本
	const char* GetText();

	//  加入文本
	void AddItem(const char* item);

	//  设置当前选中
	void SetCurSel(int index);

	//  设置文本
	void SetText(const char* txt);
};

#endif
