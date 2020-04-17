#include "Rank_GroupManager.h"

CRankGroupManager::CRankGroupManager()
{
}

CRankGroupManager::~CRankGroupManager()
{
    for (int32_t i = 0; i < m_TempGroupList.size(); i++)
    {
        CRankGroupUnit* Group = m_TempGroupList.front();
        delete(Group);

        m_TempGroupList.pop_front();
    }
}

bool CRankGroupManager::Create()
{
	for (int32_t i = 0; i < RANK_GROUP_QUEUE_SIZE; i++)
	{
        CRankGroupUnit* pRankGroupInfo = new CRankGroupUnit;

		// 빈 그룹 세팅
        m_TempGroupList.push_back(pRankGroupInfo);
	}

	return true;
}

void CRankGroupManager::Update()
{
	size_t nCount = m_RankGroupList.size();
	if (0 == nCount)
		return;
    
	// 한번에 체크 하는 카운트 양의 제한.
	int32_t i32LoopCount = RANK_CHECK_LOOP_COUNT;
	if (i32LoopCount >= nCount)
		i32LoopCount = nCount;

	// 비정상 그룹으로 의심되면 삭제.
	std::deque<uint32_t> DeleteGroupIdList;
	for (int32_t i = 0; i < i32LoopCount; i++)
	{
		CRankGroupUnit* pGroupInfo = (CRankGroupUnit*)m_RankGroupList.front();
		if (nullptr == pGroupInfo)
			continue;

        m_RankGroupList.pop_front();

        // 비정상 상태 삭제
		if (true == pGroupInfo->CheckState(RANK_GROUP_STATE_NONE))
		{
            DeleteGroup(pGroupInfo->GetGroupID());
			continue;
		}

		switch (pGroupInfo->GetState())
		{
		case RANK_GROUP_STATE_CREATE_COMPLETE:
		{
            // 그룹 생성 완료
			// 카운트다운 상태로 전환
			pGroupInfo->SetCountdown();
		}
		break;
		case RANK_GROUP_STATE_COUNTDOWN:
		{
            __int64 nElapseSecond = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - pGroupInfo->GetCreateTime()).count();
			if (nElapseSecond > RANK_GROUP_COUNTDOWN_TIME)
			{
				// 카운트 다운이 종료 되면 배틀 생성 프로세스.
				if (false == _BattleCreate(pGroupInfo))
				{
					pGroupInfo->SetState(RANK_GROUP_STATE_BATTLE_FAIL);
					// Write Log
					break;
				}

				pGroupInfo->SetBattleRoomCreate();
			}
		}
		break;
		case RANK_GROUP_STATE_BATTLE_ROOM_CREATE:
		{
            __int64 nElapseSecond = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - pGroupInfo->GetCreateTime()).count();
			if (nElapseSecond > RANK_BATTLE_CREATE_TIMEOUT)
			{
				// 일정 시간동안 배틀룸이 생성되지 않으면 에러를 처리합니다.
				pGroupInfo->SetState(RANK_GROUP_STATE_BATTLE_FAIL);
				// Write Log
			}

			pGroupInfo->SetCreateTime();
		}
		break;
		case RANK_GROUP_STATE_BATTLE_FAIL:
		{
			// 배틀 생성이 실패 할 경우
			// 팀 상태를 매칭 이전 시점으로 돌린 후 그룹 해체
			for (char idx = 0; idx != 2; idx++)
			{
				CRankTeamUnit* pTeamInfo = pGroupInfo->GetTeamInfo(idx);
				if (nullptr == pTeamInfo)
					continue;

				// 각 팀의 유저들에게 실패 패킷 전송.
                // 팀 상태 초기화
			}

			if (false == DeleteGroup(pGroupInfo->GetGroupID()))
			{
                // 실패에 따른 처리.
                // 로그
			}
		}
		break;
		case RANK_GROUP_STATE_BATTLE:
		case RANK_GROUP_STATE_BATTLE_END:
		{
			// 배틀 시작 이후 배틀 진행 최대 시간이 지날 동안 그룹이 살아있으면 강제 삭제
			// 배틀이 종료 되었음에도 그룹이 존재 한다면 에러 - 삭제처리
            __int64 nElapseSecond = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - pGroupInfo->GetCreateTime()).count();
			if (nElapseSecond > RANK_BATTLE_TIMEOUT)
			{
				// Write Log
                DeleteGroupIdList.push_back(pGroupInfo->GetGroupID());
			}
		}
		break;
        default:
        {
            // 다음번에 다시 체크 하도록
            m_RankGroupList.push_back(pGroupInfo);
        }
        break;
		}
	}

	// 그룹 삭제
	for (auto& rVal : DeleteGroupIdList)
	{
		DeleteGroup(rVal);
	}
}

CRankGroupUnit* CRankGroupManager::GetEmptyGroup()
{
	if (0 == m_TempGroupList.size())
		return nullptr;

	// 매칭 그룹 세팅을 위해 빈 그룹 리스트에서 얻어 옴
	CRankGroupUnit* pGroupInfo = static_cast<CRankGroupUnit*>(m_TempGroupList.front());
	if (nullptr == pGroupInfo)
		return nullptr;

    m_TempGroupList.pop_front();

	return pGroupInfo;
}

void CRankGroupManager::_SetEmptyGroup(CRankGroupUnit* pGroupInfo)
{
	// 사용이 끝난 그룹 재 삽입
	return m_TempGroupList.push_back(pGroupInfo);
}

bool CRankGroupManager::_BattleCreate(CRankGroupUnit* pGroupInfo)
{
    // 배틀관련 정보를 세팅
    pGroupInfo;
    return true;
}

bool CRankGroupManager::AddGroup(CRankGroupUnit* pGroupInfo)
{
	if (nullptr == pGroupInfo)
		return false;

	// 매칭 그룹 초기화, 매칭 작업 시작.
    m_RankGroupList.push_back(pGroupInfo);
	return true;
}

bool CRankGroupManager::DeleteGroup(int32_t ui32GroupID)
{
    // 삭제된 그룹 초기화 이후 풀에 다시 넣음

    for (auto& rVal : m_RankGroupList)
    {
        if (rVal->GetGroupID() == ui32GroupID)
        {
            _SetEmptyGroup(rVal);
            return true;
        }
    }

    return false;
}

CRankGroupUnit* CRankGroupManager::GetGroupByIdx(int32_t i32idx)
{
    for (auto& rVal : m_RankGroupList)
    {
        if (rVal->GetGroupID() == i32idx)
        {
            return rVal;
        }
    }

    return nullptr;
}
