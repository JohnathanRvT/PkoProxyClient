#pragma once
#include "stdafx.h"

#include "GameConfig.h"

// Tables
#include "tabledata.h"
#include "SceneObj.h"
#include "EffectSet.h"
#include "EventSoundSet.h"
#include "MusicSet.h"
#include "CharacterPoseSet.h"
#include "ChaCreateSet.h"
#include "SkillRecord.h"
#include "MapSet.h"
#include "CharacterRecord.h"
#include "ItemRecord.h"
#include "EventRecord.h"
#include "AreaRecord.h"
#include "ServerSet.h"
#include "NotifySet.h"
#include "SkillStateRecord.h"
#include "ChatIconSet.h"
#include "ItemTypeSet.h"
#include "ItemPreSet.h"
#include "ForgeRecord.h"
#include "ShipSet.h"
#include "HairRecord.h"
#include "MPEffectCtrl.h"
#include "ItemRefineSet.h"
#include "ItemRefineEffectSet.h"
#include "StoneSet.h"
#include "helpinfoset.h"
#include "MonsterHelper.h"
#include "NPCHelper.h"
#include "BossHelper.h"
#include "ElfSkillSet.h"
#include "MonsterSet.h"
// ~Tables

#include "RoleCommon.h" // ROLE_MAXIMUM_FORGE

class Tables {
public:
	Tables(const CGameConfig& config, bool binary = true)
		: sceneobjinfo{make_unique_table<CSceneObjSet>(0, g_Config.m_nMaxSceneObjType, "scripts/table/sceneobjinfo", binary)},
		  sceneeffectinfo{make_unique_table<CMagicSet>(0, g_Config.m_nMaxEffectType, "scripts/table/sceneffectinfo", binary)},
		  shadeinfo{make_unique_table<CShadeSet>(0, g_Config.m_nMaxEffectType, "scripts/table/shadeinfo", binary)},
		  eventsound{make_unique_table<CEventSoundSet>(0, 30, "scripts/table/eventsound", binary)},
		  musicinfo{make_unique_table<CMusicSet>(0, 500, "scripts/table/musicinfo", binary)},
		  characterposeinfo{make_unique_table<CPoseSet>(0, 100, "scripts/table/characterposeinfo", binary)},
		  selectcha{make_unique_table<CChaCreateSet>(0, 60, "scripts/table/selectcha", binary)},
		  skillinfo{make_unique_table<CSkillRecordSet>(0, defMAX_SKILL_NO, "scripts/table/skillinfo", binary)},
		  mapinfo{make_unique_table<CMapSet>(0, 100, "scripts/table/mapinfo", binary)},
		  characterinfo{make_unique_table<CChaRecordSet>(0, 2500, "scripts/table/characterinfo", binary)},
		  iteminfo{make_unique_table<CItemRecordSet>(0, g_Config.m_nMaxItemType, "scripts/table/iteminfo", binary)},
		  objevent{make_unique_table<CEventRecordSet>(0, 10, "scripts/table/objevent", binary)},
		  areaset{make_unique_table<CAreaSet>(0, 300, "scripts/table/AreaSet", binary)},
		  serverset{make_unique_table<CServerSet>(0, 100, "scripts/table/serverset", binary)},
		  notifyset{make_unique_table<CNotifySet>(0, 100, "scripts/table/notifyset", binary)},
		  skilleff{make_unique_table<CSkillStateRecordSet>(0, 300, "scripts/table/skilleff", binary)},
		  chaticons{make_unique_table<CChatIconSet>(0, 100, "scripts/table/chaticons", binary)},
		  itemtype{make_unique_table<CItemTypeSet>(0, 100, "scripts/table/itemtype", binary)},
		  itempre{make_unique_table<CItemPreSet>(0, 100, "scripts/table/itempre", binary)},
		  forgeitem{make_unique_table<CForgeRecordSet>(1, ROLE_MAXNUM_FORGE, "scripts/table/forgeitem", binary)},
		  shipinfo{make_unique_table<xShipSet>(0, 120, "scripts/table/shipinfo", binary)},
		  shipiteminfo{make_unique_table<xShipPartSet>(0, 500, "scripts/table/shipiteminfo", binary)},
		  hairs{make_unique_table<CHairRecordSet>(0, 500, "scripts/table/hairs", binary)},
		  terraininfo{make_unique_table<MPTerrainSet>(0, 100, "scripts/table/terraininfo", binary)},
		  magicsingleinfo{make_unique_table<CEff_ParamSet>(0, 100, "scripts/table/magicsingleinfo", binary)},
		  magicgroupinfo{make_unique_table<CGroup_ParamSet>(0, 10, "scripts/table/magicgroupinfo", binary)},
		  itemrefineinfo{make_unique_table<CItemRefineSet>(0, g_Config.m_nMaxItemType, "scripts/table/itemrefineinfo", binary)},
		  itemrefineeffectinfo{make_unique_table<CItemRefineEffectSet>(0, 5000, "scripts/table/itemrefineeffectinfo", binary)},
		  stoneinfo{make_unique_table<CStoneSet>(0, 100, "scripts/table/stoneinfo", binary)},
		  helpinfoset{make_unique_table<CHelpInfoSet>(0, 20, "scripts/table/helpinfoset", binary)},
		  monsterlist{make_unique_table<MonsterHelper>(0, 1000, "scripts/table/monsterlist", binary)},
		  NPCList{make_unique_table<NPCHelper>(0, 1000, "scripts/table/NPCList", binary)},
		  BossList{make_unique_table<BossHelper>(0, 1000, "scripts/table/BossList", binary)},
		  resourceinfo{make_unique_table<MPResourceSet>(0, g_Config.m_nMaxResourceNum, "scripts/table/resourceinfo", binary)},
		  elfskillinfo{make_unique_table<CElfSkillSet>(0, 100, "scripts/table/elfskillinfo", binary)},
		  monsterinfo{make_unique_table<CMonsterSet>(0, 1000, "scripts/table/monsterinfo", binary)} {};

	std::unique_ptr<CSceneObjSet> sceneobjinfo{};
	std::unique_ptr<CMagicSet> sceneeffectinfo{};
	std::unique_ptr<CShadeSet> shadeinfo{};
	std::unique_ptr<CEventSoundSet> eventsound{};
	std::unique_ptr<CMusicSet> musicinfo{};
	std::unique_ptr<CPoseSet> characterposeinfo{};
	std::unique_ptr<CChaCreateSet> selectcha{};
	std::unique_ptr<CSkillRecordSet> skillinfo{};
	std::unique_ptr<CMapSet> mapinfo{};
	std::unique_ptr<CChaRecordSet> characterinfo{};
	std::unique_ptr<CItemRecordSet> iteminfo{};
	std::unique_ptr<CEventRecordSet> objevent{};
	std::unique_ptr<CAreaSet> areaset{};
	std::unique_ptr<CServerSet> serverset{};
	std::unique_ptr<CNotifySet> notifyset{};
	std::unique_ptr<CSkillStateRecordSet> skilleff{};
	std::unique_ptr<CChatIconSet> chaticons{};
	std::unique_ptr<CItemTypeSet> itemtype{};
	std::unique_ptr<CItemPreSet> itempre{};
	std::unique_ptr<CForgeRecordSet> forgeitem{};
	std::unique_ptr<xShipSet> shipinfo{};
	std::unique_ptr<xShipPartSet> shipiteminfo{};
	std::unique_ptr<CHairRecordSet> hairs{};
	std::unique_ptr<MPTerrainSet> terraininfo{};
	std::unique_ptr<CEff_ParamSet> magicsingleinfo{};
	std::unique_ptr<CGroup_ParamSet> magicgroupinfo{};
	std::unique_ptr<CItemRefineSet> itemrefineinfo{};
	std::unique_ptr<CItemRefineEffectSet> itemrefineeffectinfo{};
	std::unique_ptr<CStoneSet> stoneinfo{};
	std::unique_ptr<CHelpInfoSet> helpinfoset{};
	std::unique_ptr<MonsterHelper> monsterlist{};
	std::unique_ptr<NPCHelper> NPCList{};
	std::unique_ptr<BossHelper> BossList{};
	std::unique_ptr<MPResourceSet> resourceinfo{};
	std::unique_ptr<CElfSkillSet> elfskillinfo{};
	std::unique_ptr<CMonsterSet> monsterinfo{};

private:
	template <typename Derived>
	std::unique_ptr<Derived>
	make_unique_table(int nIDStart, int nIDCnt, const std::string& table_filepath, bool binary = false) {
		static_assert(std::is_base_of<CRawDataSet, Derived>(),
					  "Derived is not derived from CRawDataSet");

		auto table = std::make_unique<Derived>(nIDStart, nIDCnt);
		table->LoadRawDataInfo(table_filepath.c_str(), binary);
		return std::move(table);
	}
};
