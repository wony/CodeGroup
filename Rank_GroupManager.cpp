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

		// �� �׷� ����
        m_TempGroupList.push_back(pRankGroupInfo);
	}

	return true;
}

void CRankGroupManager::Update()
{
	size_t nCount = m_RankGroupList.size();
	if (0 == nCount)
		return;
    
	// �ѹ��� üũ �ϴ� ī��Ʈ ���� ����.
	int32_t i32LoopCount = RANK_CHECK_LOOP_COUNT;
	if (i32LoopCount >= nCount)
		i32LoopCount = nCount;

	// ������ �׷����� �ǽɵǸ� ����.
	std::deque<uint32_t> DeleteGroupIdList;
	for (int32_t i = 0; i < i32LoopCount; i++)
	{
		CRankGroupUnit* pGroupInfo = (CRankGroupUnit*)m_RankGroupList.front();
		if (nullptr == pGroupInfo)
			continue;

        m_RankGroupList.pop_front();

        // ������ ���� ����
		if (true == pGroupInfo->CheckState(RANK_GROUP_STATE_NONE))
		{
            DeleteGroup(pGroupInfo->GetGroupID());
			continue;
		}

		switch (pGroupInfo->GetState())
		{
		case RANK_GROUP_STATE_CREATE_COMPLETE:
		{
            // �׷� ���� �Ϸ�
			// ī��Ʈ�ٿ� ���·� ��ȯ
			pGroupInfo->SetCountdown();
		}
		break;
		case RANK_GROUP_STATE_COUNTDOWN:
		{
            __int64 nElapseSecond = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - pGroupInfo->GetCreateTime()).count();
			if (nElapseSecond > RANK_GROUP_COUNTDOWN_TIME)
			{
				// ī��Ʈ �ٿ��� ���� �Ǹ� ��Ʋ ���� ���μ���.
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
				// ���� �ð����� ��Ʋ���� �������� ������ ������ ó���մϴ�.
				pGroupInfo->SetState(RANK_GROUP_STATE_BATTLE_FAIL);
				// Write Log
			}

			pGroupInfo->SetCreateTime();
		}
		break;
		case RANK_GROUP_STATE_BATTLE_FAIL:
		{
			// ��Ʋ ������ ���� �� ���
			// �� ���¸� ��Ī ���� �������� ���� �� �׷� ��ü
			for (char idx = 0; idx != 2; idx++)
			{
				CRankTeamUnit* pTeamInfo = pGroupInfo->GetTeamInfo(idx);
				if (nullptr == pTeamInfo)
					continue;

				// �� ���� �����鿡�� ���� ��Ŷ ����.
                // �� ���� �ʱ�ȭ
			}

			if (false == DeleteGroup(pGroupInfo->GetGroupID()))
			{
                // ���п� ���� ó��.
                // �α�
			}
		}
		break;
		case RANK_GROUP_STATE_BATTLE:
		case RANK_GROUP_STATE_BATTLE_END:
		{
			// ��Ʋ ���� ���� ��Ʋ ���� �ִ� �ð��� ���� ���� �׷��� ��������� ���� ����
			// ��Ʋ�� ���� �Ǿ������� �׷��� ���� �Ѵٸ� ���� - ����ó��
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
            // �������� �ٽ� üũ �ϵ���
            m_RankGroupList.push_back(pGroupInfo);
        }
        break;
		}
	}

	// �׷� ����
	for (auto& rVal : DeleteGroupIdList)
	{
		DeleteGroup(rVal);
	}
}

CRankGroupUnit* CRankGroupManager::GetEmptyGroup()
{
	if (0 == m_TempGroupList.size())
		return nullptr;

	// ��Ī �׷� ������ ���� �� �׷� ����Ʈ���� ��� ��
	CRankGroupUnit* pGroupInfo = static_cast<CRankGroupUnit*>(m_TempGroupList.front());
	if (nullptr == pGroupInfo)
		return nullptr;

    m_TempGroupList.pop_front();

	return pGroupInfo;
}

void CRankGroupManager::_SetEmptyGroup(CRankGroupUnit* pGroupInfo)
{
	// ����� ���� �׷� �� ����
	return m_TempGroupList.push_back(pGroupInfo);
}

bool CRankGroupManager::_BattleCreate(CRankGroupUnit* pGroupInfo)
{
    // ��Ʋ���� ������ ����
    pGroupInfo;
    return true;
}

bool CRankGroupManager::AddGroup(CRankGroupUnit* pGroupInfo)
{
	if (nullptr == pGroupInfo)
		return false;

	// ��Ī �׷� �ʱ�ȭ, ��Ī �۾� ����.
    m_RankGroupList.push_back(pGroupInfo);
	return true;
}

bool CRankGroupManager::DeleteGroup(int32_t ui32GroupID)
{
    // ������ �׷� �ʱ�ȭ ���� Ǯ�� �ٽ� ����

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
