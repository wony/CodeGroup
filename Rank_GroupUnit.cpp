#include "Rank_GroupUnit.h"

CRankGroupUnit::CRankGroupUnit() : m_nRating(0), m_nTeamCount(0), m_ui32GroupID(0), m_eState(RANK_GROUP_STATE_NONE)
{
    m_pTeamInfo.reserve(RANK_TEAM_SIZE);
}

CRankGroupUnit::~CRankGroupUnit()
{
    std::vector<std::shared_ptr<CRankTeamUnit>>().swap(m_pTeamInfo);
}

bool CRankGroupUnit::InsertTeam(std::shared_ptr<CRankTeamUnit> pTeamInfo)
{
    if (RANK_TEAM_SIZE <= m_pTeamInfo.size())
        return false;

    m_pTeamInfo.push_back(pTeamInfo);
}

std::shared_ptr<CRankTeamUnit> CRankGroupUnit::GetTeamInfo(uint32_t nTeamIndex)
{
    if (nTeamIndex >= m_pTeamInfo.size())
        return nullptr;

    return m_pTeamInfo[nTeamIndex];
}

void CRankGroupUnit::ResetTeamInfo()
{
    m_nRating = 0;
    m_nTeamCount = 0;
    m_ui32GroupID = 0;
    m_eState = RANK_GROUP_STATE_NONE;

    std::vector<std::shared_ptr<CRankTeamUnit>>().swap(m_pTeamInfo);
}
