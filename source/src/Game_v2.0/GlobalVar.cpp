#include "Stdafx.h"

#include "GameApp.h"

#include "MPEditor.h"
#include "FindPath.h"

#include "InputBox.h"
#include "GameWG.h"
#include "GameMovie.h"

#ifndef USE_DSOUND
#include "AudioThread.h"
CAudioThread g_AudioThread;
DWORD g_dwCurMusicID = 0;
#endif

// 多语言库须先初始化
//CLanguageRecord g_oLangRec("./scripts/table/StringSet.bin", "./scripts/table/StringSet.txt");
//pi_LeakReporter pi_leakReporter("gamememleak.log");
//CResourceBundleManage g_ResourceBundleManage("Game.loc"); //Add by lark.li 20080130

bool volatile g_bLoadRes = false;
std::unique_ptr<CGameApp> g_pGameApp{nullptr};

CEffectSet* CEffectSet::_Instance = nullptr;
CShadeSet* CShadeSet::_Instance = nullptr;
CMusicSet* CMusicSet::_Instance = nullptr;
CPoseSet* CPoseSet::_Instance = nullptr;
CMapSet* CMapSet::_Instance = nullptr;
CChaCreateSet* CChaCreateSet::_Instance = nullptr;
CEventSoundSet* CEventSoundSet::_Instance = nullptr;
CAreaSet* CAreaSet::_Instance = nullptr;
CServerSet* CServerSet::_Instance = nullptr;
CNotifySet* CNotifySet::_Instance = nullptr;
CChatIconSet* CChatIconSet::_Instance = nullptr;
CItemTypeSet* CItemTypeSet::_Instance = nullptr;
CItemPreSet* CItemPreSet::_Instance = nullptr;
CItemRefineSet* CItemRefineSet::_Instance = nullptr;
CItemRefineEffectSet* CItemRefineEffectSet::_Instance = nullptr;
CStoneSet* CStoneSet::_Instance = nullptr;
CElfSkillSet* CElfSkillSet::_Instance = nullptr;
CMonsterSet* CMonsterSet::_Instance = nullptr; //Add by sunny.sun 20080903
CHelpInfoSet* CHelpInfoSet::_Instance = nullptr;

MPEditor g_Editor;
CFindPath g_cFindPath(128, 38);
CFindPathEx g_cFindPathEx;
CInputBox g_InputBox;

CGameWG g_oGameWG;
CGameMovie g_GameMovie;

// 客户端版本号, 在GateServer有验证
short g_sClientVer = 110; //CHANGED: v1 was 10100
//short g_sClientVer = 138;
short g_sKeyData = short(g_sClientVer * g_sClientVer * 0x1232222);
char g_szSendKey[4];
char g_szRecvKey[4];

DWORD g_dwCurMusicID = 0;

// CLightEff plight;
