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

	if (NULL == m_pRankTeamManager)
		return false;
    
	m_pRankGroupManager = pRankGroupManager;

	if (NULL == m_pRankGroupManager)
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
	CRankGroupUnit	stGroupInfo;
    stGroupInfo.ResetTeamInfo();

	int32_t nResult = _MakeGroup(&stGroupInfo);
	if (GROUP_MAKE_SUCCESS != nResult)
		return;

	CRankGroupUnit* pGroupInfo = _CreateGroup(&stGroupInfo);
	if (NULL == pGroupInfo)
	{
		//Write Log
		return;
	}

	// ��Ī�� �Ϸ�� �׷� ���� �ٲ�
	pGroupInfo->SetState(RANK_GROUP_STATE_CREATE_COMPLETE);
	
	// ��Ʋ�� �����ϱ� ���� ���� ���� �� ������ ����.
	// ......
}

int32_t CRankMode::_MakeGroup(CRankGroupUnit* pRankGroupInfo)
{
    int32_t nCount = m_pRankTeamManager->GetMatchingTeamCount();

    int32_t idx = m_nLastMatchIdx;
	if (idx >= (nCount - 1))
		idx = 0;

	// �� ���� �������� ��� ��Ī ������ �ٸ� ���� �˻�
	for (int32_t idx = 0; idx < nCount; idx++)
	{
		CRankTeamUnit* pTeamInfo = m_pRankTeamManager->FindMatchingTeam_Idx(idx);
		if (NULL == pTeamInfo)
			continue;

		if (false == pRankGroupInfo->InsertTeam(pTeamInfo))
		{
			pRankGroupInfo->ResetTeamInfo();
			return GROUP_MAKE_FAIL;
		}

		CRankTeamUnit* pOtherTeamInfo = m_pRankTeamManager->FindMatchTeam(pTeamInfo);
		if (NULL == pOtherTeamInfo)
		{
			continue;
		}

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
	CRankTeamUnit* pTeamInfo = m_pRankTeamManager->NewTeam(pUser);
    if (NULL == pTeamInfo)
    {
        // Log
        return false;
    }

	return true;
}

bool CRankMode::EnterTeam(CUser* pUser, uint32_t nTeamId)
{
	// ����Ʈ���� �� ã�� ����.
	CRankTeamUnit* pTeamInfo = m_pRankTeamManager->FindTeam(nTeamId);
    if (NULL == pTeamInfo)
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
	CRankTeamUnit* pTeamInfo = m_pRankTeamManager->FindTeam(nTeamId);
    if (NULL == pTeamInfo)
    {
        // ���� ã�� �� ����
        return false;
    }

	CUser* pUser = pTeamInfo->GetUserInfo_UserIndex(nUserID);
    if (NULL == pUser)
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
	CRankTeamUnit* pTeamInfo = m_pRankTeamManager->FindTeam(nTeamID);
	if (NULL == pTeamInfo)
		return false;

	CUser* pUser = pTeamInfo->GetUserInfo_UserIndex(nUserID);
	if (NULL == pUser)
		return false;
	if (false == pTeamInfo->IsState(RANK_TEAM_STATE_MAKE_UP))
		return false;

	for (char i = 0; i < RANK_TEAM_USER_COUNT; i++)
	{
        CUser* pMember = pTeamInfo->GetUser_Sequence(i);
		if (NULL == pMember)
			continue;

		pMember->SetRankState(RANK_USER_STATE_MATCHING_WAIT);
	}

	pTeamInfo->SetState(RANK_TEAM_STATE_MATCHING);

	// ���, �� ���� �ٲ� ���� ��ũ ��⿭�� ����.
	m_pRankTeamManager->AddRatingTeamList(pTeamInfo);

	return true;
}

CRankGroupUnit* CRankMode::_CreateGroup(CRankGroupUnit* pGroupInfo)
{
	CRankGroupUnit* pEmptyGroupInfo = m_pRankGroupManager->GetEmptyGroup();
	if (NULL == pEmptyGroupInfo)
		return NULL;

	if (false == m_pRankGroupManager->AddGroup(pEmptyGroupInfo))
		return NULL;

	return pEmptyGroupInfo;
}

bool CRankMode::_DeleteGroup(CRankGroupUnit* pGroupInfo)
{
	if (false == m_pRankGroupManager->DeleteGroup(pGroupInfo->GetGroupID()))
	{
		return false;
	}

	return true;
}