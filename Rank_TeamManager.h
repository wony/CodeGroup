#pragma once

#define RANK_MAX_DIFF_MATCHING_POINT 50

#include "Rank_TeamUnit.h"
#include <map>

class CRankTeamManager
{
private:
	std::map<uint32_t, CRankTeamUnit*>			m_mapTeamList;				// TeamID
	std::multimap<uint32_t, CRankTeamUnit*>		m_multimapRatingTeamList;	// Rating
public:
	CRankTeamManager(void);
	~CRankTeamManager(void);

	void						Destroy();
	void						Update();

    CRankTeamUnit*				NewTeam(CUser* pUser);
	bool						DeleteTeam(uint32_t ui32TeamID);
    CRankTeamUnit*				FindTeam(uint32_t ui32TeamID);

    uint32_t                    GetMatchingTeamCount() { return (uint32_t)m_multimapRatingTeamList.size(); }
    uint32_t                    GetTeamCount() { return (uint32_t)m_mapTeamList.size(); }

    CRankTeamUnit*				FindMatchingTeam_Idx(int32_t i32Idx);
    CRankTeamUnit*				FindMatchTeam(CRankTeamUnit* pTeamInfo);

    void                        AddRatingTeamList(CRankTeamUnit * pTeamInfo);
};