//=============================================================================
// FileName: ItemRecord.h
// Creater: ZhangXuedong
// Date: 2004.09.01
// Comment: CItemRecordSet class
//=============================================================================

#ifndef ITEMRECORD_H
#define ITEMRECORD_H

#include <tchar.h>
#include "util.h"
#include "TableData.h"
#include "CompCommand.h"
#include "JobType.h"

const char cchItemRecordKeyValue = (char)0xfe; // 道具对职业的需求中占用了-1

#define defITEM_NAME_LEN 80
#define defITEM_MODULE_NUM 5
#define defITEM_MODULE_LEN 19
#define defITEM_ICON_NAME_LEN 17
#define defITEM_ATTREFFECT_NAME_LEN 33

// Modify by lark.li 20080318 begin
#define defITEM_DESCRIPTOR_NAME_LEN 257
//#pragma message("the version is long struct, are you clear? lark.li 20080411")
// End

#define defITEM_BIND_EFFECT_NUM 8

#define defITEM_BODY 4

enum EItemType {
	Sword = 1,	 // 剑
	Glave = 2,	 // 巨剑
	Bow = 3,	   // 弓
	Harquebus = 4, // 火绳枪
	Falchion = 5,  // 刀
	Mitten = 6,	// 拳套
	Stylet = 7,	// 匕首
	Moneybag = 8,  // 钱袋
	Cosh = 9,	  // 短棒
	Sinker = 10,   // 锤子
	Shield = 11,   // 盾
	Arrow = 12,	// 箭
	Ammo = 13,	 // 弹药
	//14 unused
	//15 unused
	//16 unused
	LifeSkillRepairHammer = 17,
	LifeSkillAxe = 18,
	LifeSkillPickAxe = 19,
	Hair = 20,	 // 头发
	Face = 21,	 // 脸型
	Clothing = 22, // 衣服
	Glove = 23,	// 手套
	Boot = 24,	 // 靴子
	Necklace = 25,
	Ring = 26,
	Tattoo = 27,
	Conch = 29,	// 贝壳
	Medicine = 31, // 药品
	Ovum = 32,	 // 海卵
	UnkownConsumable = 33,  //TODO: Proper naming 
	SkillBook = 34,
	Scroll = 36,  // 回城卷轴
	General = 41, // 一般物品
	Mission = 42, // 任务道具
	Boat = 43,	// 船长证明
	Wing = 44,	// 翅膀，客户端用于显示特效用
	Trade = 45,   // 贸易证明
	Bravery = 46, // 勇者之证
	GemCombineScroll = 47,
	GemStone = 49,
	RefiningStone = 50,
	Hull = 51,		// 船身
	Embolon = 52,   // 撞角
	Engine = 53,	// 发动机
	Artillery = 54, // 火炮
	Airscrew = 55,  // 螺旋浆
	BoatSign = 56,  // 船标志
	PetFodder = 57, // 宠物饲料
	PetSock = 58,   // 能量零食
	Pet = 59,		// 守护精灵
	FusionScroll = 60,
	FusionCatalyst = 61,
	StrengthenScroll = 62,
	PetEgg = 67,
	Blueprint = 69,
	LifeSkillTool = 70,
	ResetEquipStone = 80,

	// Add by lark.li 20080514 begin
	ItemTypeNo = 99, // 彩票小球
					 // End
};

enum EItemPickTo {
	enumITEM_PICKTO_KITBAG,
	enumITEM_PICKTO_CABIN,
};

class CItemRecord : public CRawDataInfo {
public:
	enum EItemExtendInfo {
		enumItemNormalStart = 0, // 普通道具
		enumItemNormalEnd = 4999,
		enumItemFusionEndure = 25000, // 融合道具所要求的最大耐久度//Modify by ning.yan 20080821 （大澎要求融合道具的条件由道具起始、结束栏改为最大耐久度）
		//enumItemFusionStart		= 5000,		// 熔合道具起始栏位
		//enumItemFusionEnd		= 6000,		// 熔合道具结束栏位
		// Modify by lark.li 20080703
		enumItemMax = 10000, // 道具表最大记录数量
							 // End

	};

	CItemRecord();

	long lID;												 // 编号
	_TCHAR szName[defITEM_NAME_LEN];						 // 名称
	char szICON[defITEM_ICON_NAME_LEN];						 // ICON
	_TCHAR chModule[defITEM_MODULE_NUM][defITEM_MODULE_LEN]; // 模型
	short sShipFlag;										 // 船标志
	short sShipType;										 // 船体型号
	short sType;											 // 类型

	char chForgeLv;						   // 精炼等级
	char chForgeSteady;					   // 安定值（精炼物品的稳定值，超过这个值，每精炼一次就递减一定成功率）
	char chExclusiveID;					   // 是否拥有唯一ID
	char chIsTrade;						   // 可交易
	char chIsPick;						   // 可拾取
	char chIsThrow;						   // 可丢弃
	char chIsDel;						   // 可销毁
	long lPrice;						   // 交易价格
	std::array<char, defITEM_BODY> chBody; // 体形限制
	short sNeedLv;						   // 要求的等级
	std::array<char, MAX_JOB_TYPE> szWork; // 职业

	int nPileMax;								// 在道具栏一格中可以放置的最多个数
	char chInstance;							// 是否实例化
	std::array<char, enumEQUIP_NUM> szAbleLink; // 可装备的位置
	std::array<char, enumEQUIP_NUM> szNeedLink; // 需要的位置
	char chPickTo;								// 0 拾取时进入背包，1 进入船舱

	short sStrCoef;   // 力量系数加成
	short sAgiCoef;   // 敏捷系数加成
	short sDexCoef;   // 专注系数加成
	short sConCoef;   // 体质系数加成
	short sStaCoef;   // 精力系数加成
	short sLukCoef;   // 幸运系数加成
	short sASpdCoef;  // 攻击频率系数加成
	short sADisCoef;  // 攻击距离系数加成
	short sMnAtkCoef; // 最小攻击力系数加成
	short sMxAtkCoef; // 最大攻击力系数加成
	short sDefCoef;   // 防御系数加成
	short sMxHpCoef;  // 最大Hp系数加成
	short sMxSpCoef;  // 最大Sp系数加成
	short sFleeCoef;  // 闪避率系数加成
	short sHitCoef;   // 命中率系数加成
	short sCrtCoef;   // 爆击率系数加成
	short sMfCoef;	// 寻宝率系数加成
	short sHRecCoef;  // hp恢复速度系数加成
	short sSRecCoef;  // sp恢复速度系数加成
	short sMSpdCoef;  // 移动速度系数加成
	short sColCoef;   // 资源采集速度系数加成

	std::array<short, 2> sStrValu;   // 力量常数加成（最小值，最大值）
	std::array<short, 2> sAgiValu;   // 敏捷常数加成
	std::array<short, 2> sDexValu;   // 专注常数加成
	std::array<short, 2> sConValu;   // 体质常数加成
	std::array<short, 2> sStaValu;   // 精力常数加成
	std::array<short, 2> sLukValu;   // 幸运常数加成
	std::array<short, 2> sASpdValu;  // 攻击频率常数加成
	std::array<short, 2> sADisValu;  // 攻击距离常数加成
	std::array<short, 2> sMnAtkValu; // 最小攻击力常数加成
	std::array<short, 2> sMxAtkValu; // 最大攻击力常数加成
	std::array<short, 2> sDefValu;   // 防御常数加成
	std::array<short, 2> sMxHpValu;  // 最大Hp常数加成
	std::array<short, 2> sMxSpValu;  // 最大Sp常数加成
	std::array<short, 2> sFleeValu;  // 闪避率常数加成
	std::array<short, 2> sHitValu;   // 命中率常数加成
	std::array<short, 2> sCrtValu;   // 爆击率常数加成
	std::array<short, 2> sMfValu;	// 寻宝率常数加成
	std::array<short, 2> sHRecValu;  // hp恢复速度常数加成
	std::array<short, 2> sSRecValu;  // sp恢复速度常数加成
	std::array<short, 2> sMSpdValu;  // 移动速度常数加成
	std::array<short, 2> sColValu;   // 资源采集速度常数加成

	std::array<short, 2> sPDef;										   // 物理抵抗
	short sLHandValu;												   // 道具左手发挥
	std::array<short, 2> sEndure;									   // 耐久度
	std::array<short, 2> sEnergy;									   // 能量
	short sHole;													   // 洞数
	char szAttrEffect[defITEM_ATTREFFECT_NAME_LEN];					   // 使用效果
	short sDrap;													   // drap
	std::array<std::array<short, 2>, defITEM_BIND_EFFECT_NUM> sEffect; // 武器捆绑特效以及对应dummy
	std::array<short, 2> sItemEffect;								   // 道具外观特效
	std::array<short, 2> sAreaEffect;								   // 放在地上时的特效
	std::array<short, 2> sUseItemEffect;							   // 使用时的特效
	_TCHAR szDescriptor[defITEM_DESCRIPTOR_NAME_LEN];				   // 描述

public:
	const char* GetIconFile() const; // 得到完整的icon文件名称
	const char* GetIconFileInStore() const;

	static bool IsVaildFusionID(CItemRecord* pItem);

	bool GetIsPile() const { return nPileMax > 1; }

	// 检查角色体型是否可以装备
	bool IsAllowEquip(unsigned int nChaID) const;

	// 检查是否没有体型限制
	bool IsAllEquip() const { return chBody[0] == -1; }

	void RefreshData();

	int GetTypeValue(int nType) const;

	bool IsFaceItem() const { return sItemEffect[0] != 0; }

	bool IsSendUseItem() const { return sUseItemEffect[0] != 0; }

	bool isWeapon() const;
	bool isArmor() const;
	bool IsNecklace() const;
	bool IsRing() const;
	bool IsGlove() const;
	bool IsBoot() const;
	bool IsHair() const;
	bool IsConsumable() const;
	bool IsMission() const;

public:
	short sEffNum; // 武器捆绑特效数量

private:
	std::array<bool, 5> _IsBody;
	//bool _IsBody[5];
};

class CItemRecordSet : public CRawDataSet {
public:
	static CItemRecordSet* I() { return _Instance; }

	CItemRecordSet(int nIDStart, int nIDCnt, int nCol = 128)
		: CRawDataSet(nIDStart, nIDCnt, nCol) {
		_Instance = this;
		_Init();
	}

protected:
	static CItemRecordSet* _Instance; // 相当于单键, 把自己记住

	virtual CRawDataInfo* _CreateRawDataArray(int nCnt) override {
		return new CItemRecord[nCnt];
	}

	virtual void _DeleteRawDataArray() override {
		delete[](CItemRecord*) _RawDataArray;
	}

	virtual int _GetRawDataInfoSize() const override {
		return sizeof(CItemRecord);
	}

	virtual void* _CreateNewRawData(CRawDataInfo* pInfo) override {
		return nullptr;
	}

	virtual void _DeleteRawData(CRawDataInfo* pInfo) override {
		SAFE_DELETE(pInfo->pData);
	}

	virtual BOOL _ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) override;
	virtual void _ProcessRawDataInfo(CRawDataInfo* pInfo) override;
};

inline CItemRecord* GetItemRecordInfo(int nTypeID) {
	return (CItemRecord*)CItemRecordSet::I()->GetRawDataInfo(nTypeID);
}

inline CItemRecord* GetItemRecordInfo(const char* pszItemName) {
	return (CItemRecord*)CItemRecordSet::I()->GetRawDataInfo(pszItemName);
}

inline bool CItemRecord::IsAllowEquip(unsigned int nChaID) const {
	if (nChaID < 5) {
		return _IsBody[nChaID];
	}

	return false;
}

#endif //ITEMRECORD_H
