#include "Rank_TeamUnit.h"


CRankTeamUnit::CRankTeamUnit() : m_nRating(0), m_nUserCount(0), m_nTeamID(0), m_eState(RANK_TEAM_STATE_NONE)
{
}

CRankTeamUnit::~CRankTeamUnit()
{
    if (0 != m_UserList.size())
    {
        std::vector<CUser*>().swap(m_UserList);
    }
}

bool CRankTeamUnit::TeamCreateInit(CUser* pUser)
{
    if (nullptr == pUser)
        return false;
    if (0 != m_UserList.size())
        return false;

    m_eState = RANK_TEAM_STATE_CREATE;
    m_nRating = pUser->GetRankRating();

    EnterUser(pUser);

    return true;
}

bool CRankTeamUnit::GetMatchingTimeout()
{
    __int64 nElapseMillisecond = std::chrono::duration_cast<std::chrono::milliseconds>(m_tStartTime - std::chrono::system_clock::now()).count();
    if (nElapseMillisecond > RANK_TEAM_MATCHING_TIMEOUT)
        return true;

    return false;
}

void CRankTeamUnit::SetMatchingTimeout()
{
    m_eState = RANK_TEAM_STATE_MATCHING_TIMEOUT;
    m_tStartTime = std::chrono::system_clock::now();
}

bool CRankTeamUnit::EnterUser(CUser* pUser)
{
    if (nullptr == pUser)
        return false;
    if (m_UserList.size() >= RANK_TEAM_USER_COUNT)
        return false;

    m_UserList.push_back(pUser);

    uint32_t nAvrRating = 0;
    for (auto& rVal : m_UserList)
    {
        nAvrRating += rVal->GetRankRating();
    }

    m_nRating = (nAvrRating / m_UserList.size());

    return true;
}

bool CRankTeamUnit::LeaveUser(uint32_t nUserIndex)
{
    for (auto iter = m_UserList.begin(); iter != m_UserList.end(); iter++)
    {
        CUser *pUser = (*iter);
        if (pUser->GetUserIndex() == nUserIndex)
        {
            m_UserList.erase(iter);
            return true;
        }
    }

    return false;
}

CUser* CRankTeamUnit::GetUser_Sequence(char index)
{
    if (index > (char)m_UserList.size())
        return nullptr;

    return m_UserList[index];
}

CUser* CRankTeamUnit::GetUserInfo_UserIndex(uint32_t nUserIndex)
{
    for (auto iter = m_UserList.begin(); iter != m_UserList.end(); iter++)
    {
        CUser *pUser = (*iter);
        if (pUser->GetUserIndex() == nUserIndex)
        {
            return pUser;
        }
    }

    return nullptr;
}
