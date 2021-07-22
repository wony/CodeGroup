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
	// ��Ī �� üũ�� ���� �׷� ����
	auto pGroupInfo = _CreateGroup();
	if (!pGroupInfo.get())
	{
		//Write Log
		return;
	}

	// ��Ī�� �Ϸ�� �׷� ���� �ٲ�
	pGroupInfo->SetState(RANK_GROUP_STATE_CREATE_COMPLETE);
	
	// ��Ʋ�� �����ϱ� ���� ���� ���� �� ������ ����.
	// ......
}

int32_t CRankMode::_MakeGroup(std::shared_ptr<CRankGroupUnit> pRankGroupInfo)
{
    int32_t nCount = m_pRankTeamManager->GetMatchingTeamCount();

    int32_t idx = m_nLastMatchIdx;
	if (idx >= (nCount - 1))
		idx = 0;

	// �� ���� �������� ��� ��Ī ������ �ٸ� ���� �˻�
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
	// �׷��� ���¸� üũ �Ͽ�
	// ��Ʋ����, ����, �ı�.
	m_pRankGroupManager->Update();
}

void CRankMode::_Update_RankTeam()
{
	// ���� ���¸� �ľ��Ͽ�
	// ������ �� �ı� �� ���� �� ����.
	m_pRankTeamManager->Update();
}

bool CRankMode::CreateTeam(CUser* pUser)
{
	// NewTeam() - �� ����, �ʱ�ȭ �۾�.
	// ������� �� ����Ʈ�� �߰�.
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
	// ����Ʈ���� �� ã�� ����.
	auto pTeamInfo = m_pRankTeamManager->FindTeam(nTeamId);
    if (!pTeamInfo.get())
    {
        // �� ã�� ���� �α�
        return false;
    }
	
	// ���� ���� ������ �����ΰ�?
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
        // ���� ã�� �� ����
        return false;
    }

	CUser* pUser = pTeamInfo->GetUserInfo_UserIndex(nUserID);
    if (nullptr == pUser)
    {
        // ������ ã�� �� ����
        return false;
    }

	if (false == pTeamInfo->LeaveUser(nUserID))
	{
		// ������ ������ ������ ����
		return false;
	}

	if (0 == pTeamInfo->GetCurUserCount())
	{
        if (false == m_pRankTeamManager->DeleteTeam(nTeamId))
        {
            // �� ���� ����
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

	// ���, �� ���� �ٲ� ���� ��ũ ��⿭�� ����.
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