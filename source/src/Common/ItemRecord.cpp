#include "ItemRecord.h"
CItemRecord::CItemRecord() {
}

const char* CItemRecord::GetIconFile() const {
	static char buf[32] = {0};
	_snprintf_s(buf, sizeof(buf), _TRUNCATE, "texture/icon/%s.tga", szICON);
	return buf;
}

const char* CItemRecord::GetIconFileInStore() const {
	static char buf[32] = {0};
	_snprintf_s(buf, sizeof(buf), _TRUNCATE, "texture/Icon2/%s.tga", szICON);
	return buf;
}

bool CItemRecord::IsVaildFusionID(CItemRecord* pItem) {
	if (pItem->sEndure[1] == enumItemFusionEndure) { // 改融合道具的条件
		return true;
	}
	return false;
}

void CItemRecord::RefreshData() {
	_IsBody = {};
	if (IsAllEquip()) {
		for (int i = 1; i < _IsBody.size(); i++) {
			_IsBody[i] = true;
		}
	} else {
		for (int i = 0; i < defITEM_BODY; i++)
			if (chBody[i] > 0 && chBody[i] < 5) {
				_IsBody[chBody[i]] = true;
			}
	}

	bool IsTmpBody[5] = {};
	for (int i = 1; i < 5; i++) {
		IsTmpBody[i] = chModule[i][1] == '\0' ? false : true;
	}

	if (sType < 31) {
		switch (szAbleLink[0]) {
		case enumEQUIP_HEAD:
		case enumEQUIP_FACE:
		case enumEQUIP_BODY:
		case enumEQUIP_GLOVE:
		case enumEQUIP_SHOES:
		case enumEQUIP_LHAND:
		case enumEQUIP_RHAND: {
			for (int i = 1; i < _IsBody.size(); i++) {
				if (_IsBody[i] && !IsTmpBody[i]) {
					LG("iteminfoerror", "%d,%s,cha ID:%d\n", lID, szName, i);
					return;
				}
			}
		}
		};
	}
}

int CItemRecord::GetTypeValue(int nType) const {
	switch (nType) {
	case ITEMATTR_COE_STR:
		return sStrCoef;
	case ITEMATTR_COE_AGI:
		return sAgiCoef;
	case ITEMATTR_COE_DEX:
		return sDexCoef;
	case ITEMATTR_COE_CON:
		return sConCoef;
	case ITEMATTR_COE_STA:
		return sStaCoef;
	case ITEMATTR_COE_LUK:
		return sLukCoef;
	case ITEMATTR_COE_ASPD:
		return sASpdCoef;
	case ITEMATTR_COE_ADIS:
		return sADisCoef;
	case ITEMATTR_COE_MNATK:
		return sMnAtkCoef;
	case ITEMATTR_COE_MXATK:
		return sMxAtkCoef;
	case ITEMATTR_COE_DEF:
		return sDefCoef;
	case ITEMATTR_COE_MXHP:
		return sMxHpCoef;
	case ITEMATTR_COE_MXSP:
		return sMxSpCoef;
	case ITEMATTR_COE_FLEE:
		return sFleeCoef;
	case ITEMATTR_COE_HIT:
		return sHitCoef;
	case ITEMATTR_COE_CRT:
		return sCrtCoef;
	case ITEMATTR_COE_MF:
		return sMfCoef;
	case ITEMATTR_COE_HREC:
		return sHRecCoef;
	case ITEMATTR_COE_SREC:
		return sSRecCoef;
	case ITEMATTR_COE_MSPD:
		return sMSpdCoef;
	case ITEMATTR_COE_COL:
		return sColCoef;

	case ITEMATTR_COE_PDEF:
		return 0;

	case ITEMATTR_VAL_STR:
		return sStrValu[0];
	case ITEMATTR_VAL_AGI:
		return sAgiValu[0];
	case ITEMATTR_VAL_DEX:
		return sDexValu[0];
	case ITEMATTR_VAL_CON:
		return sConValu[0];
	case ITEMATTR_VAL_STA:
		return sStaValu[0];
	case ITEMATTR_VAL_LUK:
		return sLukValu[0];
	case ITEMATTR_VAL_ASPD:
		return sASpdValu[0];
	case ITEMATTR_VAL_ADIS:
		return sADisValu[0];
	case ITEMATTR_VAL_MNATK:
		return sMnAtkValu[0];
	case ITEMATTR_VAL_MXATK:
		return sMxAtkValu[0];
	case ITEMATTR_VAL_DEF:
		return sDefValu[0];
	case ITEMATTR_VAL_MXHP:
		return sMxHpValu[0];
	case ITEMATTR_VAL_MXSP:
		return sMxSpValu[0];
	case ITEMATTR_VAL_FLEE:
		return sFleeValu[0];
	case ITEMATTR_VAL_HIT:
		return sHitValu[0];
	case ITEMATTR_VAL_CRT:
		return sCrtValu[0];
	case ITEMATTR_VAL_HREC:
		return sHRecValu[0];
	case ITEMATTR_VAL_SREC:
		return sSRecValu[0];
	case ITEMATTR_VAL_MSPD:
		return sMSpdValu[0];
	case ITEMATTR_VAL_COL:
		return sColValu[0];
	case ITEMATTR_VAL_PDEF:
		return sPDef[0];
	}
	return 0;
}

bool CItemRecord::isWeapon() const {
	using E = EItemType;
	return sType == E::Sword ||
		   sType == E::Glave ||
		   sType == E::Bow ||
		   sType == E::Harquebus ||
		   sType == E::Falchion ||
		   sType == E::Mitten ||
		   sType == E::Stylet ||
		   sType == E::Moneybag ||
		   sType == E::Cosh ||
		   sType == E::Sinker;
}

bool CItemRecord::isArmor() const {
	using E = EItemType;
	return sType == E::Clothing ||
		   sType == E::Shield ||
		   sType == E::Tattoo;
}

bool CItemRecord::IsNecklace() const {
	using E = EItemType;
	return sType == EItemType::Necklace;
}

bool CItemRecord::IsRing() const {
	using E = EItemType;
	return sType == E::Ring;
}

bool CItemRecord::IsGlove() const {
	using E = EItemType;
	return sType == E::Glove;
}

bool CItemRecord::IsBoot() const {
	using E = EItemType;
	return sType == E::Boot;
}

bool CItemRecord::IsHair() const {
	using E = EItemType;
	return sType == E::Hair;
}

bool CItemRecord::IsConsumable() const {
	using E = EItemType;
	return sType == E::Medicine ||
		   sType == E::Ovum ||
		   sType == E::UnkownConsumable;
}

bool CItemRecord::IsMission() const {
	using E = EItemType;
	return sType == E::Mission;
}

//---------------------------------------------------------------------------
// class CItemRecordSet
//---------------------------------------------------------------------------
CItemRecordSet* CItemRecordSet::_Instance = nullptr;

BOOL CItemRecordSet::_ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) {
	if (ParamList.empty()) {
		return false;
	}

	auto pInfo = static_cast<CItemRecord*>(pRawDataInfo);

	int m = 0, n = 0;
	std::string strList[80];
	std::string strLine;

	// 编号
	pInfo->lID = pInfo->nID;
	// 名称
	//_tcsncpy(pInfo->szName, pInfo->szDataName, defITEM_NAME_LEN);
	_tcsncpy_s(pInfo->szName, sizeof(pInfo->szName), pInfo->szDataName, _TRUNCATE);
	//pInfo->szName[defITEM_NAME_LEN - 1] = '\0';
	// ICON
	//_tcsncpy(pInfo->szICON, ParamList[m++].c_str(), defITEM_ICON_NAME_LEN);
	_tcsncpy_s(pInfo->szICON, sizeof(pInfo->szICON), ParamList[m++].c_str(), _TRUNCATE);
	//pInfo->szICON[defITEM_ICON_NAME_LEN - 1] = '\0';
	// 模型
	for (int i = 0; i < defITEM_MODULE_NUM; i++) {
		//_tcsncpy(pInfo->chModule[i], ParamList[m++].c_str(), defITEM_MODULE_LEN);
		_tcsncpy_s(pInfo->chModule[i], sizeof(pInfo->chModule[i]), ParamList[m++].c_str(), _TRUNCATE);
		//pInfo->chModule[i][defITEM_MODULE_LEN - 1] = 0;
	}
	// 船标志
	pInfo->sShipFlag = Str2Int(ParamList[m++]);
	// 船体型号
	pInfo->sShipType = Str2Int(ParamList[m++]);
	// 类型
	pInfo->sType = Str2Int(ParamList[m++]);

	// 获得前缀几率
	strLine = Str2Int(ParamList[m++]);
	// 套装编号
	strLine = Str2Int(ParamList[m++]);
	// 精炼等级
	pInfo->chForgeLv = Str2Int(ParamList[m++]);
	// 安定值
	pInfo->chForgeSteady = Str2Int(ParamList[m++]);
	// 唯一ID
	pInfo->chExclusiveID = Str2Int(ParamList[m++]);
	// 交易
	pInfo->chIsTrade = Str2Int(ParamList[m++]);
	// 拾取
	pInfo->chIsPick = Str2Int(ParamList[m++]);
	// 丢弃
	pInfo->chIsThrow = Str2Int(ParamList[m++]);
	// 销毁
	pInfo->chIsDel = Str2Int(ParamList[m++]);
	// 在道具栏一格中可以放置的最多个数
	pInfo->nPileMax = Str2Int(ParamList[m++]);
	// 是否实例化
	pInfo->chInstance = Str2Int(ParamList[m++]);
	// 价格
	pInfo->lPrice = Str2Int(ParamList[m++]);
	// 体形
	pInfo->chBody.fill(cchItemRecordKeyValue);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > defITEM_BODY ? defITEM_BODY : n;
	for (int i = 0; i < n; i++) {
		pInfo->chBody[i] = Str2Int(strList[i]);
	}
	// 角色等级
	pInfo->sNeedLv = Str2Int(ParamList[m++]);
	// 职业
	pInfo->szWork.fill(cchItemRecordKeyValue);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > MAX_JOB_TYPE ? MAX_JOB_TYPE : n;
	for (int i = 0; i < n; i++) {
		pInfo->szWork[i] = Str2Int(strList[i]);
	}
	// 角色称号
	strLine = Str2Int(ParamList[m++]);
	// 角色名声
	strLine = Str2Int(ParamList[m++]);
	// 道具可装备位置
	pInfo->szAbleLink.fill(cchItemRecordKeyValue);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > enumEQUIP_NUM ? enumEQUIP_NUM : n;
	for (int i = 0; i < n; i++) {
		pInfo->szAbleLink[i] = Str2Int(strList[i]);
	}
	// 道具装备所必需的所有位置
	pInfo->szNeedLink.fill(cchItemRecordKeyValue);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > enumEQUIP_NUM ? enumEQUIP_NUM : n;
	for (int i = 0; i < n; i++) {
		pInfo->szNeedLink[i] = Str2Int(strList[i]);
	}

	// 拾取进入位置
	pInfo->chPickTo = Str2Int(ParamList[m++]);

	// 力量系数加成
	pInfo->sStrCoef = Str2Int(ParamList[m++]);
	// 敏捷系数加成
	pInfo->sAgiCoef = Str2Int(ParamList[m++]);
	// 专注系数加成
	pInfo->sDexCoef = Str2Int(ParamList[m++]);
	// 体质系数加成
	pInfo->sConCoef = Str2Int(ParamList[m++]);
	// 精力系数加成
	pInfo->sStaCoef = Str2Int(ParamList[m++]);
	// 幸运系数加成
	pInfo->sLukCoef = Str2Int(ParamList[m++]);
	// 攻击频率系数加成
	pInfo->sASpdCoef = Str2Int(ParamList[m++]);
	// 攻击距离系数加成
	pInfo->sADisCoef = Str2Int(ParamList[m++]);
	// 最小攻击力系数加成
	pInfo->sMnAtkCoef = Str2Int(ParamList[m++]);
	// 最大攻击力系数加成
	pInfo->sMxAtkCoef = Str2Int(ParamList[m++]);
	// 防御系数加成
	pInfo->sDefCoef = Str2Int(ParamList[m++]);
	// 最大Hp系数加成
	pInfo->sMxHpCoef = Str2Int(ParamList[m++]);
	// 最大Sp系数加成
	pInfo->sMxSpCoef = Str2Int(ParamList[m++]);
	// 闪避率系数加成
	pInfo->sFleeCoef = Str2Int(ParamList[m++]);
	// 命中率系数加成
	pInfo->sHitCoef = Str2Int(ParamList[m++]);
	// 爆击率系数加成
	pInfo->sCrtCoef = Str2Int(ParamList[m++]);
	// 寻宝率系数加成
	pInfo->sMfCoef = Str2Int(ParamList[m++]);
	// hp恢复速度系数加成
	pInfo->sHRecCoef = Str2Int(ParamList[m++]);
	// sp恢复速度系数加成
	pInfo->sSRecCoef = Str2Int(ParamList[m++]);
	// 移动速度系数加成
	pInfo->sMSpdCoef = Str2Int(ParamList[m++]);
	// 资源采集速度系数加成
	pInfo->sColCoef = Str2Int(ParamList[m++]);

	// 力量常数加成
	pInfo->sStrValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sStrValu[i] = Str2Int(strList[i]);
	}
	// 敏捷常数加成
	pInfo->sAgiValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sAgiValu[i] = Str2Int(strList[i]);
	}
	// 专注常数加成
	pInfo->sDexValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sDexValu[i] = Str2Int(strList[i]);
	}
	// 体质常数加成
	pInfo->sConValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sConValu[i] = Str2Int(strList[i]);
	}
	// 精力常数加成
	pInfo->sStaValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sStaValu[i] = Str2Int(strList[i]);
	}
	// 幸运常数加成
	pInfo->sLukValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sLukValu[i] = Str2Int(strList[i]);
	}
	// 攻击频率常数加成
	pInfo->sASpdValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sASpdValu[i] = Str2Int(strList[i]);
	}
	// 攻击距离常数加成
	pInfo->sADisValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sADisValu[i] = Str2Int(strList[i]);
	}
	// 最小攻击力常数加成
	pInfo->sMnAtkValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sMnAtkValu[i] = Str2Int(strList[i]);
	}
	// 最大攻击力常数加成
	pInfo->sMxAtkValu.fill(0);
	std::fill(pInfo->sMxAtkValu.begin(), pInfo->sMxAtkValu.end(), 0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sMxAtkValu[i] = Str2Int(strList[i]);
	}
	// 防御常数加成
	pInfo->sDefValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sDefValu[i] = Str2Int(strList[i]);
	}
	// 最大Hp常数加成
	pInfo->sMxHpValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sMxHpValu[i] = Str2Int(strList[i]);
	}
	// 最大Sp常数加成
	pInfo->sMxSpValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sMxSpValu[i] = Str2Int(strList[i]);
	}
	// 闪避率常数加成
	pInfo->sFleeValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sFleeValu[i] = Str2Int(strList[i]);
	}
	// 命中率常数加成
	pInfo->sHitValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sHitValu[i] = Str2Int(strList[i]);
	}
	// 爆击率常数加成
	pInfo->sCrtValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sCrtValu[i] = Str2Int(strList[i]);
	}
	// 寻宝率常数加成
	pInfo->sMfValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sMfValu[i] = Str2Int(strList[i]);
	}
	// hp恢复速度常数加成
	pInfo->sHRecValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sHRecValu[i] = Str2Int(strList[i]);
	}
	// sp恢复速度常数加成
	pInfo->sStrValu.fill(0);
	std::fill(pInfo->sSRecValu.begin(), pInfo->sSRecValu.end(), 0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sSRecValu[i] = Str2Int(strList[i]);
	}
	// 移动速度常数加成
	pInfo->sMSpdValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sMSpdValu[i] = Str2Int(strList[i]);
	}
	// 资源采集速度常数加成
	pInfo->sColValu.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sColValu[i] = Str2Int(strList[i]);
	}

	// 物理抵抗
	pInfo->sPDef.fill(0);
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	n = n > 2 ? 2 : n;
	for (int i = 0; i < n; i++) {
		pInfo->sPDef[i] = Str2Int(strList[i]);
	}
	// 道具左手发挥
	pInfo->sLHandValu = Str2Int(ParamList[m++]);
	// 能量
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	for (int i = 0; i < 2; i++) {
		if (i < n)
			pInfo->sEnergy[i] = Str2Int(strList[i]);
		else
			pInfo->sEnergy[i] = 0;
	}
	// 耐久度
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	for (int i = 0; i < 2; i++) {
		if (i < n)
			pInfo->sEndure[i] = Str2Int(strList[i]);
		else
			pInfo->sEndure[i] = 0;
	}
	// 洞数
	pInfo->sHole = Str2Int(ParamList[m++]);
	// 船耐久恢复
	strLine = Str2Int(ParamList[m++]);
	// 可装炮数量
	strLine = Str2Int(ParamList[m++]);
	// 船员人数
	strLine = Str2Int(ParamList[m++]);
	// 人员标配
	strLine = Str2Int(ParamList[m++]);
	// 货舱容量
	strLine = Str2Int(ParamList[m++]);
	// 补给消耗
	strLine = Str2Int(ParamList[m++]);
	// 炮弹飞行速度
	strLine = Str2Int(ParamList[m++]);
	// 船移动速度
	strLine = Str2Int(ParamList[m++]);
	// 使用效果
	//strncpy(pInfo->szAttrEffect, ParamList[m++].c_str(), defITEM_ATTREFFECT_NAME_LEN);
	strncpy_s(pInfo->szAttrEffect, sizeof(pInfo->szAttrEffect), ParamList[m++].c_str(), _TRUNCATE);
	//pInfo->szAttrEffect[defITEM_ATTREFFECT_NAME_LEN - 1] = '\0';

	//drap
	pInfo->sDrap = Str2Int(ParamList[m++]);
	// 武器捆绑特效
	pInfo->sEffect = {};
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	if (n > defITEM_BIND_EFFECT_NUM) {
		n = defITEM_BIND_EFFECT_NUM;
	}
	for (int e = 0; e < n; e++) {
		pInfo->sEffect[e][0] = Str2Int(strList[e]);
	}
	if (n == 1 && pInfo->sEffect[0][0] == 0) {
		pInfo->sEffNum = 0;
	} else {
		pInfo->sEffNum = n;
	}

	// 捆绑定特效的dummy
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	if (n > defITEM_BIND_EFFECT_NUM)
		n = defITEM_BIND_EFFECT_NUM;
	for (int e = 0; e < n; e++) {
		pInfo->sEffect[e][1] = Str2Int(strList[e]);
	}

	// 道具外观特效
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	int ItemNum = sizeof(pInfo->sItemEffect) / sizeof(pInfo->sItemEffect[0]);
	if (n > ItemNum)
		n = ItemNum;
	std::fill(pInfo->sItemEffect.begin(), pInfo->sItemEffect.end(), 0);
	for (int e = 0; e < n; e++) {
		pInfo->sItemEffect[e] = Str2Int(strList[e]);
	}

	// 放在地上时的特效
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	int AreaNum = sizeof(pInfo->sAreaEffect) / sizeof(pInfo->sAreaEffect[0]);
	if (n > AreaNum)
		n = AreaNum;
	std::fill(pInfo->sAreaEffect.begin(), pInfo->sAreaEffect.end(), 0);
	for (int e = 0; e < n; e++) {
		pInfo->sAreaEffect[e] = Str2Int(strList[e]);
	}

	// 使用时的特效
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	int nNum = sizeof(pInfo->sUseItemEffect) / sizeof(pInfo->sUseItemEffect[0]);
	if (n > nNum)
		n = nNum;
	std::fill(pInfo->sUseItemEffect.begin(), pInfo->sUseItemEffect.end(), 0);
	for (int e = 0; e < n; e++) {
		pInfo->sUseItemEffect[e] = Str2Int(strList[e]);
	}

	//描述
	strLine = ParamList[m++];

	if (strLine == "0") {
		pInfo->szDescriptor[0] = '\0';
	} else {
		// modify by lark.li 20080424 begin
		//strncpy( pInfo->szDescriptor, strLine.c_str(), defITEM_DESCRIPTOR_NAME_LEN );
		//strncpy( pInfo->szDescriptor, ConvertResString(strLine.c_str()), defITEM_DESCRIPTOR_NAME_LEN );
		strncpy_s(pInfo->szDescriptor, sizeof(pInfo->szDescriptor), ConvertResString(strLine.c_str()), _TRUNCATE);
		// End
		//pInfo->szDescriptor[defITEM_DESCRIPTOR_NAME_LEN - 1] = '\0';
	}
	return TRUE;
}

void CItemRecordSet::_ProcessRawDataInfo(CRawDataInfo* pInfo) {
	CItemRecord* pItem = (CItemRecord*)pInfo;

	pItem->RefreshData();
}
