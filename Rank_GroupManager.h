#pragma once

#include "Rank_GroupUnit.h"
#include <deque>
#include <chrono>
#include <memory>

#define RANK_GROUP_QUEUE_SIZE 100
#define RANK_CHECK_LOOP_COUNT 10
#define RANK_GROUP_COUNTDOWN_TIME 5000
#define RANK_BATTLE_CREATE_TIMEOUT 10000
#define RANK_BATTLE_TIMEOUT 100000

class CRankGroupManager
{
private:
    std::deque<std::shared_ptr<CRankGroupUnit>> m_RankGroupList;
    std::deque<std::shared_ptr<CRankGroupUnit>> m_TempGroupList;

	void                        _SetEmptyGroup(std::shared_ptr<CRankGroupUnit> pGroupInfo);
    bool                        _BattleCreate(std::shared_ptr<CRankGroupUnit> pGroupInfo);

public:
	CRankGroupManager(void);
	~CRankGroupManager(void);
	
	bool						Create();
	void						Update();

	std::shared_ptr<CRankGroupUnit>	GetEmptyGroup();

	bool						AddGroup(std::shared_ptr<CRankGroupUnit> pGroupInfo);
	bool						DeleteGroup(int32_t ui32GroupID);

	std::shared_ptr<CRankGroupUnit>	GetGroupByIdx(int32_t i32idx);
    int32_t						GetGroupCount() { return m_RankGroupList.size(); }

};
