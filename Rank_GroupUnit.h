#pragma once

#include "Rank_TeamUnit.h"
#include <chrono>

enum RANK_GROUP_STATE
{
    RANK_GROUP_STATE_NONE,
    RANK_GROUP_STATE_CREATE,
    RANK_GROUP_STATE_CREATE_COMPLETE,
    RANK_GROUP_STATE_COUNTDOWN,
    RANK_GROUP_STATE_BATTLE_ROOM_CREATE,
    RANK_GROUP_STATE_BATTLE,
    RANK_GROUP_STATE_BATTLE_END,
    RANK_GROUP_STATE_BATTLE_FAIL,

    RANK_GROUP_STATE_MAX,
};

#define RANK_TEAM_SIZE 2

class CRankGroupUnit
{
private:
    uint32_t    m_nRating;
    char        m_nTeamCount;
    uint32_t    m_ui32GroupID;
    RANK_GROUP_STATE m_eState;

    std::chrono::system_clock::time_point m_tCreateTime;

    std::vector<CRankTeamUnit*> m_pTeamInfo;

public:
    CRankGroupUnit();
    ~CRankGroupUnit();

    bool            InsertTeam(CRankTeamUnit* pTeamInfo);
    CRankTeamUnit*  GetTeamInfo(uint32_t nTeamIndex);
    void            ResetTeamInfo();

    uint32_t    GetMatchRating() { return m_nRating; }
    uint32_t    GetGroupID() { return m_ui32GroupID; }

    void        SetState(RANK_GROUP_STATE eState) { m_eState = eState; }
    RANK_GROUP_STATE    GetState() { return m_eState; }
    bool        CheckState(RANK_GROUP_STATE eState) { (m_eState == eState) ? true : false; }

    std::chrono::system_clock::time_point   GetCreateTime() { return m_tCreateTime; }
    

    void        SetCountdown()          { m_eState = RANK_GROUP_STATE_COUNTDOWN; }
    void        SetBattleRoomCreate()   { m_eState = RANK_GROUP_STATE_BATTLE_ROOM_CREATE; }
    void        SetCreateTime()         { m_tCreateTime = std::chrono::system_clock::now(); }
};
