#pragma once

#define RANK_MAX_DIFF_MATCHING_POINT 50

#include "Rank_TeamUnit.h"
#include <map>
#include <queue>
#include <atomic>
#include <memory>

class CRankTeamManager
{
private:
	std::map<uint32_t, std::shared_ptr<CRankTeamUnit>>			m_mapTeamList;				// TeamID
	std::multimap<uint32_t, std::shared_ptr<CRankTeamUnit>>		m_multimapRatingTeamList;	// Rating
public:
	//static std::shared_ptr<CRankTeamUnit> Alloc();

	CRankTeamManager(void);
	~CRankTeamManager(void);

	void						Destroy();
	void						Update();

	std::shared_ptr<CRankTeamUnit>	NewTeam(CUser* pUser);
	bool							DeleteTeam(uint32_t ui32TeamID);
	std::shared_ptr<CRankTeamUnit>	FindTeam(uint32_t ui32TeamID);

    uint32_t						GetMatchingTeamCount() { return (uint32_t)m_multimapRatingTeamList.size(); }
    uint32_t						GetTeamCount() { return (uint32_t)m_mapTeamList.size(); }

    std::shared_ptr<CRankTeamUnit>	FindMatchingTeam_Idx(int32_t i32Idx);
    std::shared_ptr<CRankTeamUnit>	FindMatchTeam(std::shared_ptr<CRankTeamUnit> pTeamInfo);

    void							AddRatingTeamList(std::shared_ptr<CRankTeamUnit> pTeamInfo);
};