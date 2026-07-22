//=============================================================================
// FileName: SkillStateType.h
// Creater: ZhangXuedong
// Date: 2005.02.04
// Comment: Skill State Type
//=============================================================================

#ifndef SKILLSTATETYPE_H
#define SKILLSTATETYPE_H

constexpr unsigned char SSTATE_NONE = 0; // 空状态

// 前八种也用于地面的状态
constexpr unsigned char SSTATE_CUT = 1;			 // 烈焰斩
constexpr unsigned char SSTATE_FERVOR_ARROR = 2; // 炽热之箭
constexpr unsigned char SSTATE_FROST_ARROR = 3;  // 霜冻之箭
constexpr unsigned char SSTATE_SKYROCKET = 4;	// 焰火表演
constexpr unsigned char SSTATE_MURRAIN = 5;		 // 瘟神
constexpr unsigned char SSTATE_GIDDY = 6;		 // 眩晕
constexpr unsigned char SSTATE_FREEZE = 7;		 // 僵硬
constexpr unsigned char SSTATE_SLEEP = 8;		 // 昏睡
constexpr unsigned char SSTATE_BIND = 9;		 // 束缚
constexpr unsigned char SSTATE_FROST = 10;		 // 冰冻
constexpr unsigned char SSTATE_BEAT_BACK = 11;   // 击退
constexpr unsigned char SSTATE_UNBEATABLE = 12;  // 无敌
constexpr unsigned char SSTATE_TOXIN = 13;		 // 巫毒
constexpr unsigned char SSTATE_REBOUND = 14;	 // 反弹
constexpr unsigned char SSTATE_AVATAR = 15;		 // 变身
constexpr unsigned char SSTATE_TITAN = 16;		 // 巨人
constexpr unsigned char SSTATE_BLINDNESS = 17;   // 失明
constexpr unsigned char SSTATE_HAIR = 18;		 // 理发
constexpr unsigned char SSTATE_FLOAT = 19;		 // 浮空
constexpr unsigned char SSTATE_CALL = 20;		 // 召唤
constexpr unsigned char SSTATE_SHIELD = 21;		 // 盾牌熟练
constexpr unsigned char SSTATE_TIGER = 22;		 // 虎啸
constexpr unsigned char SSTATE_HIDE = 43;		 // 隐身
constexpr unsigned char SSTATE_TRADE = 85;		 // 交易
constexpr unsigned char SSTATE_STALL = 99;		 // 摆摊
constexpr unsigned char SSTATE_REPAIR = 100;	 // 修理
constexpr unsigned char SSTATE_FORGE = 101;		 // 精炼

constexpr unsigned char AREA_STATE_MAXID = 255;

// Modify by lark.li 20080828 begin
//const unsigned char	SKILL_STATE_MAXID	= 240;
constexpr unsigned char SKILL_STATE_MAXID = 254;
// End

constexpr unsigned char SKILL_STATE_LEVEL = 20;

constexpr long SSTATE_SIGN_BYTE_NUM = (SKILL_STATE_MAXID + sizeof(char) * 8 - 1) / (sizeof(char) * 8);

constexpr unsigned char AREA_STATE_NUM = AREA_STATE_MAXID;
constexpr unsigned char CHA_STATE_NUM = 16;

#endif // SKILLSTATETYPE_H
