
/* * * * * * * * * * * * * * * * * * * *

    组合控件封装
    Jampe
    2006/4/12

 * * * * * * * * * * * * * * * * * * * */


#if !defined __MYCOMBOBOX_H__
#define __MYCOMBOBOX_H__

#include <windows.h>

class MyComboBox
{
public:
    MyComboBox();
    MyComboBox(HWND parent, int ctlid);
    virtual ~MyComboBox();

    //  获取组合框控件句柄
    void GetComboBoxHandle(HWND parent, int ctlid);

    //  获取文本
    const char* GetText();

    //  加入文本
    void AddItem(const char* item);

    //  设置当前选中
    void SetCurSel(int index);

    //  设置文本
    void SetText(const char* txt);

public:
    HWND        m_hwnd;
};


#endif
