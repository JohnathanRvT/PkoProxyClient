#ifndef _EXTENDATTR_H_
#define _EXTENDATTR_H_

//说明: 原来的代码采用固定格式存取储扩展属性,当属性增减时,代码要相应修改,而且效率很低,故重写

// 字符串转换成扩展属性
bool Strin2ChaExtendAttr(CCharacter* pCCha, std::string& strAttr);

// 扩展属性转换成字符串
char* ChaExtendAttr2String(CCharacter* pCCha, char* szAttrBuf, int nLen);

#endif