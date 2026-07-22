#pragma once

#include <string>

struct SPlaceCha {
	int nTypeID;
	int nX;
	int nY;
};

class CGameConfig {
public:
	CGameConfig();

	void Load(const char* pszFileName); // 读配置文件
	void SetMoveClient(bool v);   //是否与客户端同步

public:
	// 请注意此处不要放置任何非固定内存尺寸的变量, 比如模板, string之类的,
	// 此类将被直接写盘和读盘
	BOOL m_bAutoLogin{false};
	BOOL m_bFullScreen{false};
	std::array<SPlaceCha, 20> m_ChaList{};
	int m_nChaCnt{0};
	float m_LightDir[3]{1.0f, 1.0f, -1.0f};
	float m_LightColor[3]{1.0f, 1.0f, 1.0f};
	BOOL m_bNoObj{false};
	BOOL m_bEditor{false}; //  游戏编辑器
	BYTE m_btScreenMode{0};

#ifdef TESTDEMO
	int m_iFogR;
	int m_iFogG;
	int m_iFogB;
	float m_fExp2;
#endif

	int m_nMaxChaType{350};
	int m_nMaxSceneObjType{800};
	int m_nMaxEffectType{14'000}; // 用于增加精炼道具的特效数	modify by Michael 2005-11-9
	int m_nMaxResourceNum{3'000};
	int m_nMaxItemType{20'000}; //CHANGED: From 10000 (and 7000 from v1)
	BOOL m_bEnableMusic{true};
	BOOL m_bCheckOvermax{true};

	// 网络
	//NOTE: m_SendHeartbeat type does not match default value!
	BOOL m_nSendHeartbeat{30};  // 多少时间发送一次心跳,单位:s,最小值10s,关闭0
	DWORD m_nConnectTimeOut{0}; // 网络连接超时

	BOOL m_bEnableLG{true};	// 是否允许输出LG信息
	BOOL m_bEnableLGMsg{true}; // 是否允许弹出LG-Box
	BOOL m_bMThreadRes{true};  // 多线程资源载入

	int m_nCreateScene{1}; // 初始场景

	//{lemon add@2004.9.16
	float m_fCameraVel{0.0f};
	float m_fCameraAccl{0.0f};

	float fnear{20.0f};
	float ffar{2'000.0f};

	float m_fLgtFactor{0.0f};
	DWORD m_dwLgtBkColor{0xffc0c0c0};

	// lh add@2004.12.8用于单机可以拿武器
	int nLeftHand{0}; // 单机左右手武器设置
	int nRightHand{0};

	BOOL m_bRenderSceneObj{true};
	BOOL m_bRenderCha{true};
	BOOL m_bRenderEffect{true};
	BOOL m_bRenderTerrain{true};
	BOOL m_bRenderUI{true};
	BOOL m_bRenderMinimap{true};
	BOOL m_bRender{true};

	DWORD m_dwFullScreenAntialias{0};

	// 用于网络切换地图时的参数
	DWORD m_dwMaxCha{300};
	DWORD m_dwMaxEff{500};
	DWORD m_dwMaxItem{400};
	DWORD m_dwMaxObj{600};

	char m_szMD5Pass[48]{}; // 填写的MD5密码

	bool m_IsShowConsole{false}; // 是否可以操作控件台
	bool m_IsTomServer;			 // 登陆Tom服务器

	bool m_IsMoveClient{true}; // 是否客户端非同步走路

	char m_szVerErrorHTTP[256]{}; // 版本不匹配时，调用的网页

	bool m_IsBill{false};

	bool IsPower() { return m_IsShowConsole; } // 如果是编辑器模式,自动可以操作控件台

	bool m_IsDoublePwd{false}; // 是否有二次密码

	// Add by lark.li 20080429 for res
	char m_szLocale[256];
	char m_szResPath[256];

	char m_szFontName1[50]{};
	char m_szFontName2[50]{};
	// End

	// Add by lark.li 20080825 begin
	int m_nMovieW{-1};
	int m_nMovieH{-1};
	// End
};

extern CGameConfig g_Config;
