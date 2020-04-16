#pragma once

#include "Rank_UserUnit.h"
#include <vector>
#include <chrono>

enum RANK_TEAM_STATE : char
{
    RANK_TEAM_STATE_NONE = 0,
    RANK_TEAM_STATE_CREATE,
    RANK_TEAM_STATE_MAKE_UP,
    RANK_TEAM_STATE_MATCHING,
    RANK_TEAM_STATE_MATCHING_TIMEOUT,
    RANK_TEAM_STATE_MATCHING_PROCESS,
    RANK_TEAM_STATE_COMPLETE,
    RANK_TEAM_STATE_BATTLE,
};

#define RANK_TEAM_USER_COUNT 5
#define RANK_TEAM_MATCHING_TIMEOUT 120000

class CRankTeamUnit
{
private:
    uint32_t    m_nRating;
    char        m_nUserCount;
    uint32_t    m_nTeamID;
    RANK_TEAM_STATE m_eState;

    std::chrono::system_clock::time_point m_tStartTime;

    std::vector<CUser*> m_UserList;

public:
    CRankTeamUnit();
    ~CRankTeamUnit();

    uint32_t    GetMatchRating() { return m_nRating; }
    uint32_t    GetTeamID() { return m_nTeamID; }


    bool    TeamCreateInit(CUser* pUser);

    void    SetState(RANK_TEAM_STATE eState) { m_eState = eState; }
    bool    IsState(RANK_TEAM_STATE eState) { (m_eState == eState) ? true : false; }
    
    bool    GetMatchingTimeout();
    void    SetMatchingTimeout();

    size_t  GetCurUserCount() { m_UserList.size(); }

    bool    EnterUser(CUser* pUser);
    bool    LeaveUser(uint32_t nUserIndex);

    CUser*  GetUser_Sequence(char index);
    CUser*  GetUserInfo_UserIndex(uint32_t nUserIndex);
};