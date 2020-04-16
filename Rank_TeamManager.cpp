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
	// 매칭중인 팀 타임아웃 확인을 위한 루프
	for (auto it = m_mapTeamList.begin(); it != m_mapTeamList.end(); it++)
	{
		CRankTeamUnit* pTeamInfo = it->second;
		if (NULL == pTeamInfo)
			continue;

		if (false == pTeamInfo->IsState(RANK_TEAM_STATE_MATCHING))
			continue;

		// 타임아웃이 발생한 팀 취소작업.
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

	// 팀을 만드는 유저를 방장으로 만들면서 상태 값 등 데이터 초기화
	if (false == pTeamInfo->TeamCreateInit(pUser))
	{
		delete pTeamInfo;
		return NULL;
	}

	// 팀 관리를 위한 리스트.
	auto InsertResult = m_mapTeamList.insert(std::make_pair(pTeamInfo->GetTeamID(), pTeamInfo));
	if (false == InsertResult.second)
	{
		delete pTeamInfo;
		return NULL;
	}

	// 팀 구성을 위한 리스트
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

	// 매칭 리스트 - multimap 임으로 범위내 검색.
	auto iter = m_multimapRatingTeamList.equal_range(itTeam->second->GetMatchRating());
	for (auto i = iter.first; i != iter.second; ++i)
	{
		if (i->second != itTeam->second)
			continue;

		m_multimapRatingTeamList.erase(i);
		break;
	}

	// 포인터 삭제
	delete itTeam->second;

	// 팀 관리 리스트에서 삭제.
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
		// 팀 상태를 체크.
		if (false == itTeam.second->IsState(RANK_TEAM_STATE_MATCHING))
		{
			continue;
		}

		// 매칭팀 찾을때, 마지막 매칭된 팀 이후의 인덱싱으로 부터 찾음.
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
	// 이미 그룹에 속한 1팀과 레이팅을 비교하여 적절한 팀을 찾아 리턴.

	if (NULL == pTeamInfo)
	{
		return NULL;
	}
		
	int i32TeamRating = (int)pTeamInfo->GetMatchRating();

	// multimap의 바운더리 기능 이용
	// 팀의 현재 레이팅 기준 일정 범위 +- 값으로 찾음
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

		// 레이팅 차이 절대 값이 범위 내에 있는지 확인.
		// 차이가 가장 적은 팀을 찾아낸다.
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