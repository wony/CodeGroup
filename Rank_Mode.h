#pragma once

#include "Rank_TeamManager.h"
#include "Rank_GroupManager.h"
#include "Rank_TeamUnit.h"
#include <thread>
#include <memory>

#define RANK_MAX_DIFF_ENTER_TEAM_RP 50

enum GROUP_MAKE
{
    GROUP_MAKE_FAIL = 0,
    GROUP_MAKE_WATING,
    GROUP_MAKE_SUCCESS,
};

class CRankMode
{
private:
	int32_t						m_nLastMatchIdx;

    bool                        m_bRun;
    std::thread                 m_Thread;

	CRankTeamManager*			m_pRankTeamManager;
	CRankGroupManager*			m_pRankGroupManager;

private:
    int32_t						_MakeGroup(std::shared_ptr<CRankGroupUnit> pRankGroupInfo);

	void						_Update_MakeMatch();
	void						_Update_RankGroup();
	void						_Update_RankTeam();

	std::shared_ptr<CRankGroupUnit>	_CreateGroup();
    bool                        _DeleteGroup(std::shared_ptr<CRankGroupUnit> pGroupInfo);

public:
	CRankMode(void);
	~CRankMode(void);

	bool						Create(CRankGroupManager* pRankGroupManager, CRankTeamManager* pRankTeamManager);
	void						Update();

    bool						CreateTeam(CUser* pUser);
    bool						EnterTeam(CUser* pUser, uint32_t nTeamId);
    bool						LeaveTeam(uint32_t ui32UserID, uint32_t ui32TeamID);
    bool						StartTeam(uint32_t ui32UserID, uint32_t ui32TeamID, CRankTeamUnit** ppTeamInfo);
};