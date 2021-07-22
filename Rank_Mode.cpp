#include "Rank_Mode.h"
#include <windows.h>

CRankMode::CRankMode() : m_bRun(false)
{

}

CRankMode::~CRankMode(void)
{
    m_bRun = false;

    if (m_Thread.joinable())
        m_Thread.join();
}

bool CRankMode::Create(CRankGroupManager* pRankGroupManager, CRankTeamManager* pRankTeamManager)
{
	m_pRankTeamManager = pRankTeamManager;

	if (nullptr == m_pRankTeamManager)
		return false;
    
	m_pRankGroupManager = pRankGroupManager;

	if (nullptr == m_pRankGroupManager)
		return false;

	if (false == m_pRankGroupManager->Create())
	{
		return false;
	}

	m_Thread = std::thread(&CRankMode::Update, this);
    m_bRun = true;

	return true;
}

void CRankMode::Update()
{
    while (m_bRun)
	{
		_Update_MakeMatch();
		_Update_RankGroup();
		_Update_RankTeam();
		
        Sleep(1);
	}
}

void CRankMode::_Update_MakeMatch()
{
	// 매칭 팀 체크를 위한 그룹 생성
	auto pGroupInfo = _CreateGroup();
	if (!pGroupInfo.get())
	{
		//Write Log
		return;
	}

	// 매칭이 완료된 그룹 상태 바꿈
	pGroupInfo->SetState(RANK_GROUP_STATE_CREATE_COMPLETE);
	
	// 배틀을 시작하기 위한 유저 세팅 및 아이템 세팅.
	// ......
}

int32_t CRankMode::_MakeGroup(std::shared_ptr<CRankGroupUnit> pRankGroupInfo)
{
    int32_t nCount = m_pRankTeamManager->GetMatchingTeamCount();

    int32_t idx = m_nLastMatchIdx;
	if (idx >= (nCount - 1))
		idx = 0;

	// 한 팀을 기준으로 잡고 매칭 가능한 다른 팀을 검색
	for (int32_t idx = 0; idx < nCount; idx++)
	{
		auto pTeamInfo = m_pRankTeamManager->FindMatchingTeam_Idx(idx);
		if (!pTeamInfo.get())
			continue;

		if (false == pRankGroupInfo->InsertTeam(pTeamInfo))
		{
			pRankGroupInfo->ResetTeamInfo();
			return GROUP_MAKE_FAIL;
		}

		auto pOtherTeamInfo = m_pRankTeamManager->FindMatchTeam(pTeamInfo);
		if (!pOtherTeamInfo.get())
			continue;

		if (false == pRankGroupInfo->InsertTeam(pTeamInfo))
		{
			pRankGroupInfo->ResetTeamInfo();
			return GROUP_MAKE_FAIL;
		}
		
		m_nLastMatchIdx = idx;
		return GROUP_MAKE_SUCCESS;
	}

	pRankGroupInfo->ResetTeamInfo();

	return GROUP_MAKE_FAIL;
}


void CRankMode::_Update_RankGroup()
{
	// 그룹의 상태를 체크 하여
	// 배틀시작, 유지, 파괴.
	m_pRankGroupManager->Update();
}

void CRankMode::_Update_RankTeam()
{
	// 팀의 상태를 파악하여
	// 해제된 팀 파괴 및 정상 팀 유지.
	m_pRankTeamManager->Update();
}

bool CRankMode::CreateTeam(CUser* pUser)
{
	// NewTeam() - 팀 생성, 초기화 작업.
	// 대기중인 팀 리스트에 추가.
	auto pTeamInfo = m_pRankTeamManager->NewTeam(pUser);
    if (!pTeamInfo.get())
    {
        // Log
        return false;
    }

	return true;
}

bool CRankMode::EnterTeam(CUser* pUser, uint32_t nTeamId)
{
	// 리스트에서 팀 찾아 리턴.
	auto pTeamInfo = m_pRankTeamManager->FindTeam(nTeamId);
    if (!pTeamInfo.get())
    {
        // 팀 찾기 에러 로그
        return false;
    }
	
	// 팀이 입장 가능한 상태인가?
    if (false == pTeamInfo->IsState(RANK_TEAM_STATE_MAKE_UP))
        return false;

	int i32RatingDiff = (int)abs((double)(pTeamInfo->GetMatchRating() - pUser->GetRankRating()));
    if (RANK_MAX_DIFF_ENTER_TEAM_RP < i32RatingDiff)
    {
        return false;
    }

	if (false == pTeamInfo->EnterUser(pUser))
	{
		// Write Log
		return false;
	}

	return true;
}

bool CRankMode::LeaveTeam(uint32_t nUserID, uint32_t nTeamId)
{
	auto pTeamInfo = m_pRankTeamManager->FindTeam(nTeamId);
    if (!pTeamInfo.get())
    {
        // 팀을 찾을 수 없음
        return false;
    }

	CUser* pUser = pTeamInfo->GetUserInfo_UserIndex(nUserID);
    if (nullptr == pUser)
    {
        // 유저를 찾을 수 없음
        return false;
    }

	if (false == pTeamInfo->LeaveUser(nUserID))
	{
		// 모종의 이유로 나가기 실패
		return false;
	}

	if (0 == pTeamInfo->GetCurUserCount())
	{
        if (false == m_pRankTeamManager->DeleteTeam(nTeamId))
        {
            // 팀 삭제 실패
            return false;
        }

		return true;
	}

	return true;
}

bool CRankMode::StartTeam(uint32_t nUserID, uint32_t nTeamID, CRankTeamUnit** ppTeamInfo)
{
	auto pTeamInfo = m_pRankTeamManager->FindTeam(nTeamID);
	if (!pTeamInfo.get())
		return false;

	CUser* pUser = pTeamInfo->GetUserInfo_UserIndex(nUserID);
	if (nullptr == pUser)
		return false;
	if (false == pTeamInfo->IsState(RANK_TEAM_STATE_MAKE_UP))
		return false;

	for (char i = 0; i < RANK_TEAM_USER_COUNT; i++)
	{
        CUser* pMember = pTeamInfo->GetUser_Sequence(i);
		if (nullptr == pMember)
			continue;

		pMember->SetRankState(RANK_USER_STATE_MATCHING_WAIT);
	}

	pTeamInfo->SetState(RANK_TEAM_STATE_MATCHING);

	// 멤버, 팀 상태 바꾼 이후 랭크 대기열에 넣음.
	m_pRankTeamManager->AddRatingTeamList(pTeamInfo);

	return true;
}

std::shared_ptr<CRankGroupUnit> CRankMode::_CreateGroup()
{
	auto pEmptyGroupInfo = m_pRankGroupManager->GetEmptyGroup();
	if (!pEmptyGroupInfo.get())
		return nullptr;

	int32_t nResult = _MakeGroup(pEmptyGroupInfo);
	if (GROUP_MAKE_SUCCESS != nResult)
	{
		_DeleteGroup(pEmptyGroupInfo);
		return nullptr;
	}

	if (false == m_pRankGroupManager->AddGroup(pEmptyGroupInfo))
		return nullptr;

	return pEmptyGroupInfo;
}

bool CRankMode::_DeleteGroup(std::shared_ptr<CRankGroupUnit> pGroupInfo)
{
	if (false == m_pRankGroupManager->DeleteGroup(pGroupInfo->GetGroupID()))
	{
		return false;
	}

	return true;
}