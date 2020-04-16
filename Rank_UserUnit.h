#pragma once

enum RANK_USER_STATE : char
{
    RANK_USER_STATE_NONE = 0,
    RANK_USER_STATE_TEAM,
    RANK_USER_STATE_MATCHING_WAIT,
    RANK_USER_STATE_MATCHING_COMPLETE,
    RANK_USER_STATE_BATTLE,
};

class CUser
{
private:
    RANK_USER_STATE m_eState;
    uint32_t    m_nRating;
    uint32_t    m_nUserIndex;
public:
    CUser() : m_eState(RANK_USER_STATE_NONE), m_nRating(0), m_nUserIndex(0) {};
    ~CUser() {};

    void        SetRankState(RANK_USER_STATE eState) { m_nRating = eState; }

    uint32_t    GetRankRating() { return m_nRating; }
    uint32_t    GetUserIndex() { return m_nUserIndex; }
};