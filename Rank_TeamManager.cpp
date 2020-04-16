#include "Rank_TeamManager.h"

CRankTeamManager::CRankTeamManager(void)
{
	m_mapTeamList.clear();
    m_multimapRatingTeamList.clear();
}

CRankTeamManager::~CRankTeamManager(void)
{
	Destroy();
}

void CRankTeamManager::Destroy()
{
	m_mapTeamList.clear();
	m_multimapRatingTeamList.clear();
}

void CRankTeamManager::Update()
{
	// ��Ī���� �� Ÿ�Ӿƿ� Ȯ���� ���� ����
	for (auto it = m_mapTeamList.begin(); it != m_mapTeamList.end(); it++)
	{
		CRankTeamUnit* pTeamInfo = it->second;
		if (NULL == pTeamInfo)
			continue;

		if (false == pTeamInfo->IsState(RANK_TEAM_STATE_MATCHING))
			continue;

		// Ÿ�Ӿƿ��� �߻��� �� ����۾�.
		if (pTeamInfo->GetMatchingTimeout())
		{
			pTeamInfo->SetMatchingTimeout();
			//Packet Send
			continue;
		}
	}
}

CRankTeamUnit* CRankTeamManager::NewTeam(CUser* pUser)
{
	if (NULL == pUser)
		return NULL;

	CRankTeamUnit* pTeamInfo = new CRankTeamUnit();
	if (NULL == pTeamInfo)
		return NULL;

	// ���� ����� ������ �������� ����鼭 ���� �� �� ������ �ʱ�ȭ
	if (false == pTeamInfo->TeamCreateInit(pUser))
	{
		delete pTeamInfo;
		return NULL;
	}

	// �� ������ ���� ����Ʈ.
	auto InsertResult = m_mapTeamList.insert(std::make_pair(pTeamInfo->GetTeamID(), pTeamInfo));
	if (false == InsertResult.second)
	{
		delete pTeamInfo;
		return NULL;
	}

	// �� ������ ���� ����Ʈ
	auto mulResult = m_multimapRatingTeamList.insert(std::make_pair(pTeamInfo->GetMatchRating(), pTeamInfo));
	if (false == mulResult->second)
	{
		delete pTeamInfo;
		return NULL;
	}

	return pTeamInfo;
}

bool CRankTeamManager::DeleteTeam(uint32_t ui32TeamID)
{
	auto itTeam = m_mapTeamList.find(ui32TeamID);
	if (itTeam == m_mapTeamList.end())
		return false;

	// ��Ī ����Ʈ - multimap ������ ������ �˻�.
	auto iter = m_multimapRatingTeamList.equal_range(itTeam->second->GetMatchRating());
	for (auto i = iter.first; i != iter.second; ++i)
	{
		if (i->second != itTeam->second)
			continue;

		m_multimapRatingTeamList.erase(i);
		break;
	}

	// ������ ����
	delete itTeam->second;

	// �� ���� ����Ʈ���� ����.
	m_mapTeamList.erase(ui32TeamID);

	return true;
}

CRankTeamUnit* CRankTeamManager::FindTeam(uint32_t ui32TeamID)
{
	auto itTeam = m_mapTeamList.find(ui32TeamID);
	if (itTeam == m_mapTeamList.end())
		return false;

	return (CRankTeamUnit*)itTeam->second;
}

CRankTeamUnit* CRankTeamManager::FindMatchingTeam_Idx(int32_t i32Idx)
{
	if (i32Idx >= m_multimapRatingTeamList.size())
		return NULL;

    int32_t i32Count = 0;
	for (auto itTeam : m_multimapRatingTeamList)
	{
		// �� ���¸� üũ.
		if (false == itTeam.second->IsState(RANK_TEAM_STATE_MATCHING))
		{
			continue;
		}

		// ��Ī�� ã����, ������ ��Ī�� �� ������ �ε������� ���� ã��.
		if (i32Count == i32Idx)
		{
			return itTeam.second;
		}

		i32Count++;
	}

	return NULL;
}

CRankTeamUnit * CRankTeamManager::FindMatchTeam(CRankTeamUnit * pTeamInfo)
{
	// �̹� �׷쿡 ���� 1���� �������� ���Ͽ� ������ ���� ã�� ����.

	if (NULL == pTeamInfo)
	{
		return NULL;
	}
		
	int i32TeamRating = (int)pTeamInfo->GetMatchRating();

	// multimap�� �ٿ���� ��� �̿�
	// ���� ���� ������ ���� ���� ���� +- ������ ã��
	auto iterLower = m_multimapRatingTeamList.lower_bound(i32TeamRating - RANK_MAX_DIFF_MATCHING_POINT);
	auto iterUpper = m_multimapRatingTeamList.upper_bound(i32TeamRating + RANK_MAX_DIFF_MATCHING_POINT);

    uint32_t ui32MinDiffRP = RANK_MAX_DIFF_MATCHING_POINT + 1;
	CRankTeamUnit* pTeamMinDiffRP = NULL;
	for (auto i = iterLower; i != iterUpper; ++i)
	{
		if (false == i->second->IsState(RANK_TEAM_STATE_MATCHING))
			continue;

		if (i->second == pTeamInfo)
			continue;

		// ������ ���� ���� ���� ���� ���� �ִ��� Ȯ��.
		// ���̰� ���� ���� ���� ã�Ƴ���.
		uint32_t ui32DiffRP = (uint32_t)(abs((double)(i->first - i32TeamRating)));
		if (ui32DiffRP < ui32MinDiffRP)
		{
			ui32MinDiffRP = ui32DiffRP;
			pTeamMinDiffRP = i->second;
		}

		if (0 == ui32MinDiffRP)
			break;
	}

	return pTeamMinDiffRP;
}

void CRankTeamManager::AddRatingTeamList(CRankTeamUnit * pTeamInfo)
{
    m_multimapRatingTeamList.insert(std::make_pair(pTeamInfo->GetMatchRating(), pTeamInfo));
}