#pragma once
#include "uicompent.h"
#include "uitextbutton.h"
#include "uigraph.h"

#include "uiscroll.h"
#include "netprotocol.h"
namespace GUI {
#define MEMO_MAX_LINE 512
#define MEMO_MAX_ITEM 128
#define MEMO_MAX_MISSION 16
#define MEMO_MAX_MISNEED 12
#define MEMOEX_MAX_PHRASE 16

//
enum MEMO_LINE_TYPE {
	MEMO_LINE_TITLE,	// 文字标题
	MEMO_LINE_DESP,		// 文字描述
	MEMO_LINE_INTERVAL, // 间隔行
	MEMO_LINE_ICON,		// 图标
};

class CMemo : public CCompent {
public:
	CMemo(CForm& frmOwn);
	CMemo(const CMemo& rhs);
	CMemo& operator=(const CMemo& rhs);
	virtual ~CMemo();
	GUI_CLONE(CMemo)

	virtual void Render() override;
	virtual void Refresh() override;
	virtual bool MouseRun(int x, int y, MouseClickState key) override;
	virtual bool IsHandleMouse() override { return true; }

	const char* GetCaption() const { return _strCaption.c_str(); }
	void SetCaption(const char* str) override { _strCaption = str; }
	void ProcessCaption();

	virtual void SetAlpha(BYTE alpha) override { _color = (_color & 0x00ffffff) & (alpha << 24); }
	virtual void SetTextColor(DWORD color) override { _color = color; }
	DWORD GetTextColor() const { return _color; }

	CScroll* GetScroll() const { return _pScroll.get(); }
	void Init() override;
	void AddIcon(int index, CGraph* pGraph);
	bool MouseScroll(int nScroll) override;

public:
	GuiEvent evtSelectChange = {nullptr}; // 选择发生了变化
	void SetMaxNumPerRow(int n) { _nMaxNum = n; }
	int GetMaxNumPerRow() const { return _nMaxNum; }
	void SetPageShowNum(int n) { _nPageShowNum = n; }
	void SetRowHeight(int v) {
		if (v > 0) {
			_nRowHeight = v;
		}
	}
	int GetRowHeight() const { return _nRowHeight; }
	void SetMargin(int left, int top, int right, int bottom) override;
	void SetIsHaveItem(bool v) { _bIsHaveItem = v; }
	bool GetIsHaveItem() const { return _bIsHaveItem; }
	void SetItemRowNum(int num) { _nItemRowNum = num; }
	void AddItemRowContent(int row, const char* szFunc, const char* szItemEx = "");
	void SetIsHaveMission(bool v) { _bIsHaveMission = v; }
	bool GetIsHaveMission() const { return _bIsHaveMission; }
	void SetMissionRowNum(int num);
	void AddMissionRowContent(int row, const char* szFunc);
	int GetSelectItem() const { return _bIsHaveItem ? _nSelectItem : -1; }
	bool GetSelectItemText(std::string& item, std::string& itemex);
	int GetSelectMission() const { return _bIsHaveMission ? _nSelectMission : -1; }
	void Reset() override;

protected:
	static void _OnScrollChange(CGuiData* pSender) {
		((CMemo*)(pSender->GetParent()))->_OnScrollChange();
	}
	void _OnScrollChange();

protected:
	void _SetScrollRange();
	void _Copy(const CMemo& rhs);
	void _SetSelf();
	void _CheckTextAlign(); // 检查文本排列方式
	void _CheckScroll();	// 检查是否显示滚动轴

	std::string _strCaption = {""};
	std::array<std::string, MEMO_MAX_LINE> _str = {};
	bool _IsTextCenter = {true}; //NOTE: ::Init() sets this to false

	int _nLeftMargin = {0}; // 与周围边框的距离
	int _nTopMargin = {4};
	int _nRightMargin = {4};
	int _nBottomMargin = {4};
	int _nRowHeight = {20};
	int _nMaxNum = {40}; //NOTE: Maximum row length/width in characters
	int _nRowNum = {0};

	bool _bIsHaveItem = {false};	//NOTE: Has talk functions? true
	bool _bIsHaveMission = {false}; //NOTE: Has quests? true

	int _nItemRowNum = {0};	//NOTE: Total number of talk functions ( one per row)
	int _nMissionRowNum = {0}; //NOTE: Total numbers of quests (one per row)
	int _nMemoNum = {0};	   //NOTE: Not in use?
	std::array<std::string, MEMO_MAX_ITEM> _strItem;
	std::array<std::string, MEMO_MAX_ITEM> _strItemEx; // 额外数据,不显示
	std::array<std::string, MEMO_MAX_ITEM> _strMission;
	int _nSelectItem = {-1};
	int _nSelectMission = {-1};

	void RenderRowAsMessage(int row);
	void RenderRowAsQuest(int row);
	void RenderRowAsSelectedQuest(int row);
	void RenderRowAsFunction(int row);

	bool RowIsMessage(int row) const { return (row >= _nRowInfo[0][0] && row <= _nRowInfo[0][1]); }
	bool RowIsQuest(int row) const { return (row >= _nRowInfo[1][0] && row <= _nRowInfo[1][1]); }
	bool RowIsFunction(int row) const { return (row >= _nRowInfo[2][0] && row <= _nRowInfo[2][1]); }
	std::array<std::array<int, 2>, 3> _nRowInfo = {
		//{FIRST_MESSAGE_ROW, LAST_MESSAGE_ROW}
		//{FIRST_MISSION_ROW, LAST_MISSION_ROW}
		//{FIRST_FUNCTION_ROW, LAST_FUNCTION_ROW}
	};

private:
	DWORD _color = {0xff000000};
	std::unique_ptr<CScroll> _pScroll = {nullptr};
	int _firstRow = {0}, _lastRow = {0};
	int _nPageShowNum = {4};

	struct stIconIndex {
		int nIndex;
		CGraph* pGraph;
	};

	using files = std::vector<stIconIndex>;
	files _files;
};

class CGraph;
class CMemoEx : public CCompent {
public:
	CMemoEx(CForm& frmOwn);
	CMemoEx(const CMemoEx& rhs);
	CMemoEx& operator=(const CMemoEx& rhs);
	virtual ~CMemoEx();
	GUI_CLONE(CMemoEx)

	virtual void Render() override;
	virtual void Refresh() override;
	virtual bool MouseRun(int x, int y, MouseClickState key) override;
	virtual bool IsHandleMouse() override { return true; }

	const char* GetCaption() { return _strCaption.c_str(); }
	void SetCaption(const char* str) override { _strCaption = str; }

	virtual void SetAlpha(BYTE alpha) override { _color = (_color & 0x00ffffff) & (alpha << 24); }
	virtual void SetTextColor(DWORD color) override { _color = color; }
	DWORD GetTextColor() { return _color; }

	CScroll* GetScroll() { return _pScroll; }
	void Init() override;

	bool MouseScroll(int nScroll) override;
	GuiEvent evtSelectChange;		// 选择发生了变化
	GuiItemClickEvent evtClickItem; // 点击了莫个字符串
	void SetMaxNumPerRow(int n) { _nMaxNum = n; }
	int GetMaxNumPerRow() { return _nMaxNum; }
	void SetRowNum(int n) { _nRowNum = n; }
	int GetRowNum() const { return _nRowNum; }
	void SetPageShowNum(int n) { _nPageShowNum = n; }
	void SetRowHeight(int v) {
		if (v > 0)
			_nRowHeight = v;
	}
	int GetRowHeight() { return _nRowHeight; }
	void SetMargin(int left, int top, int right, int bottom) override;
	void Clear();
	void SetMisPage(const NET_MISPAGE& page);
	BYTE GetSelPrize() { return m_bySelPrize; }
	void SetIsSelect(BOOL bSel) { m_bIsSelect = bSel; }
	BOOL IsSelPrize();

protected:
	struct MEMO_DESP_SECTION {
		char* pszStart;
		char* pszEnd;
		DWORD dwColor;
		BYTE byType;
		USHORT sData;
	};

	struct MEMO_INFO {
		std::string strDesp;
		DWORD dwColor;
		CGraph* pIcon;
		USHORT sData;
		BYTE byData;
		BYTE byType;		 // 信息类型
		USHORT sxPos, syPos; // 行和列
	};

	// 需要显示的任务数据信息
	USHORT m_sNumInfo;
	MEMO_INFO m_MemoInfo[MEMO_MAX_LINE]; // 任务界面显示解析后数据
	NET_MISPAGE m_PageInfo;				 // 任务数据原始信息
	BOOL m_bUpdate;						 // 任务数据是否已被更新

	// Add by lark.li 20080721 begin
	int m_SelMem;
	// End
	static void _OnScrollChange(CGuiData* pSender) {
		((CMemoEx*)(pSender->GetParent()))->_OnScrollChange();
	}

	void _OnScrollChange();
	void _SetScrollRange();
	void _Copy(const CMemoEx& rhs);
	void _SetSelf();
	void _CheckScroll(); // 检查是否显示滚动轴
	void ParseMisPage();
	BOOL ParseScript(char* pszTemp, USHORT& sNumLine, USHORT& sRow, USHORT& sCom, USHORT sStartCom, USHORT sMaxCom, DWORD dwDefColor, MEMO_INFO* pInfo, USHORT sMaxInfo, BYTE byType);
	BOOL SelPrizeItem(int nxPos, int nyPos, MouseClickState dwKey);

	// Add by lark.li 20080721 begin
	int SelMemInfo(int x, int y, MouseClickState key);
	// End
private:
	std::string _strCaption;

	DWORD _color; // 字体颜色
	CScroll* _pScroll;
	int _nFirst, _nLast; // 显示的第一行和最后一行
	int _nPageShowNum;   // 一页最多可以显示的行数
	int _nLeftMargin;	// 与周围边框的距离
	int _nTopMargin;	 // 与周围边框的距离
	int _nRightMargin;   // 与周围边框的距离
	int _nBottomMargin;  // 与周围边框的距离
	int _nRowHeight;
	int _nTitleHeight;
	int _nIconHeight;
	int _nMaxNum;
	int _nRowNum;
	int _nPhraseNum;

	// 记录奖励物品选择项
	BYTE m_bySelPrize;
	CGuiPic* m_pPrizePic;
	CGuiPic* m_pPrizeSelPic;
	BOOL m_bIsSelect;
};

class CImageList {
public:
	CImageList(){};
	virtual ~CImageList(){};

	virtual CGraph* GetImage(const char* szImage) = 0;
	virtual bool AddImage(const char* szImage, CGraph* pImage) = 0;
};

enum MEMO_COL_TYPE {
	COL_ICON = 0, // 图标
	COL_TEXT = 1, // 文本
};

enum class MEMO_COLOR_TYPE {
	TEXT_COLOR_WIGHT,
	TEXT_COLOR_BLACK,
	TEXT_COLOR_RED,
	TEXT_COLOR_GREEN,
	TEXT_COLOR_BLUE,
	TEXT_COLOR_GRAY,
	TEXT_COLOR_PURPLE,
	TEXT_COLOR_YELLOW,
};

enum MEMO_FONT_TYPE {
	TEXT_FONT_9 = 0,  // 9号字
	TEXT_FONT_10 = 1, // 10号字
};

class CRichMemo : public CCompent {
public:
	CRichMemo(CForm& frmOwn);
	CRichMemo(const CRichMemo& rhs);
	CRichMemo& operator=(const CRichMemo& rhs);
	GUI_CLONE(CRichMemo)
	virtual ~CRichMemo();

	virtual void Init() override;
	virtual void Render() override;
	virtual void Refresh() override;
	virtual bool MouseRun(int x, int y, MouseClickState key) override;
	virtual bool IsHandleMouse() override { return true; }

	// 滚动条处理
	CScroll* GetScroll() { return m_pScroll; }
	bool MouseScroll(int nScroll) override;

	// 滚动条滚动事件处理
	void OnScrollChange();

	// 设置控件显示信息接口
	void Clear();
	void AddText(const char szTitle[], const char szText[], MEMO_COLOR_TYPE byColType, BYTE byFontType);
	void SetClipRect(const RECT& rect) { m_ShowRect = rect; }
	void SetMaxLine(USHORT sNum) { m_sMaxLine = sNum; }
	void SetImageList(CImageList* pList) { m_pImageList = pList; }
	void SetTitleInfo(DWORD dwColor, BYTE byFont) {
		m_dwTitleColor = dwColor;
		m_byTitleFont = byFont;
	}
	void SetIntervalDist(BYTE byDist) { m_byDist = byDist; }
	void SetAutoScroll(BOOL bAuto) { m_bAutoScroll = bAuto; }
	void AutoScroll();

protected:
	// 显示列信息结构
	struct MEMO_COLINFO {
		MEMO_COLINFO(std::string strDesp, BYTE byType, DWORD dwColor, USHORT sxPos, CGraph* pIcon = nullptr)
			: strDesp(strDesp), pIcon(pIcon), byType(byType), dwColor(dwColor), sxPos(sxPos) {}

		std::string strDesp{};
		CGraph* pIcon{nullptr};
		BYTE byType{0};
		DWORD dwColor{0}; // 字体颜色
		USHORT sxPos{0};
	};

	// 每行列信息数组
	using MEMO_COLINFO_ARRAY = std::vector<MEMO_COLINFO>;

	// 行信息显示结构
	struct MEMO_LINE_INFO {
		MEMO_COLINFO_ARRAY ColInfoArray;
		BYTE byFontType;   // 字体类型
		USHORT sFontWidth; // 显示字体宽高
		USHORT sFontHeight;
		USHORT sHeight; // 显示行高度
	};

	// 所有需要显示的行信息列表
	using MEMO_LINEINFO_LIST = std::list<MEMO_LINE_INFO*>;
	MEMO_LINEINFO_LIST m_LineList;

	// 每个标题段信息结构
	struct MEMO_SECTION_INFO {
		BYTE byFontType;   // 字体类型
		USHORT sFontWidth; // 显示字体宽高
		USHORT sFontHeight;
		DWORD dwColor;
		std::string strTitle;
		std::string strDesp;
	};

	static void OnScrollChange(CGuiData* pSender) {
		((CRichMemo*)(pSender->GetParent()))->OnScrollChange();
	}

	void ParseScript(const char szInfo[], DWORD dwDefColor, BYTE byFont, USHORT sStartCom, USHORT sMaxCom);
	DWORD GetColor(MEMO_COLOR_TYPE byType);
	CGraph* GetImage(const char szImage[]);

private:
	CImageList* m_pImageList; // 显示图形列表
	CScroll* m_pScroll;		  // 滚动条控件
	RECT m_ShowRect;		  // 控件有效显示区域裁剪
	USHORT m_sMaxLine;		  // 最大可以显示的段信息数量
	DWORD m_dwTitleColor;	 // 标题字体的颜色
	BYTE m_byTitleFont;		  // 标题字体的大小
	BYTE m_byDist;			  // 设置每行显示间隔距离
	BOOL m_bAutoScroll;		  // 是否自动滚屏
};

class CFaceImage : public CImageList {
	struct sImage {
		std::string name;
		CGraph* pImage;
	};

public:
	CFaceImage();
	virtual ~CFaceImage();

	virtual CGraph* GetImage(const char* szImage) override;
	virtual bool AddImage(const char* szImage, CGraph* pImage) override;

private:
	static std::vector<sImage*> m_pData;
};

} // namespace GUI
