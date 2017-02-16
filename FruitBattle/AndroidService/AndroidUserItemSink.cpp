#include "Stdafx.h"
#include "AndroidUserItemSink.h"
#include "math.h"

//////////////////////////////////////////////////////////////////////////

//ʱ���ʶ
#define IDI_BANK_OPERATE			3									//���ж�ʱ
#define IDI_PLACE_JETTON1			103									//��ע��ʱ
#define IDI_PLACE_JETTON2			104									//��ע��ʱ
#define IDI_PLACE_JETTON3			105									//��ע��ʱ
#define IDI_PLACE_JETTON4			106									//��ע��ʱ
#define IDI_PLACE_JETTON5			107									//��ע��ʱ
#define IDI_CHECK_BANKER			108									//�����ׯ
#define IDI_REQUEST_BANKER			101									//���붨ʱ
#define IDI_GIVEUP_BANKER			102									//��ׯ��ʱ
#define IDI_PLACE_JETTON			110									//��ע���� (Ԥ��110-160)

//////////////////////////////////////////////////////////////////////////

int CAndroidUserItemSink::m_stlApplyBanker = 0L;
LONGLONG CAndroidUserItemSink::m_lRobotJettonLimit[2]={0,0};
int CAndroidUserItemSink::m_nRobotBetTimeLimit[2]={0,0};
bool CAndroidUserItemSink::m_bRobotBanker = false;
int CAndroidUserItemSink::m_nRobotBankerCount=0;
int CAndroidUserItemSink::m_nRobotWaitBanker=0;
int	CAndroidUserItemSink::m_nRobotApplyBanker=0;
bool CAndroidUserItemSink::m_bInitFlag=false;
int CAndroidUserItemSink::m_nMinRobotBankerCount=0;
LONGLONG CAndroidUserItemSink::m_lRobotScoreRange[2]={0,0};
LONGLONG CAndroidUserItemSink::m_lRobotBankGetScore=0;
LONGLONG CAndroidUserItemSink::m_lRobotBankGetScoreBanker=0;
int CAndroidUserItemSink::m_nRobotBankStorageMul=0;
LONGLONG CAndroidUserItemSink::m_lAreaLimitScore = 0;
LONGLONG CAndroidUserItemSink::m_lUserLimitScore = 0;
LONGLONG CAndroidUserItemSink::m_lBankerCondition = 0;
//���캯��
CAndroidUserItemSink::CAndroidUserItemSink()
{
	//��Ϸ����
	m_lMaxChipBanker = 0;
	m_lMaxChipUser = 0;
	m_wCurrentBanker = 0;
	m_nChipTime = 0;
	m_nChipTimeCount = 0;
	m_cbTimeLeave = 0;
	ZeroMemory(m_lAreaChip, sizeof(m_lAreaChip));
	ZeroMemory(m_nChipLimit, sizeof(m_nChipLimit));


	//��ׯ����
	m_bMeApplyBanker=false;
	m_nWaitBanker=0;
	m_nBankerCount=0;
	m_nListUserCount=0;
//	m_lAreaLimitScore=0/*100000000*/;
	return;
}

//��������
CAndroidUserItemSink::~CAndroidUserItemSink()
{
}

//�ӿڲ�ѯ
VOID *  CAndroidUserItemSink::QueryInterface(REFGUID Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(IAndroidUserItemSink,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(IAndroidUserItemSink,Guid,dwQueryVer);
	return NULL;
}

//��ʼ�ӿ�
bool  CAndroidUserItemSink::Initialization(IUnknownEx * pIUnknownEx)
{
	//��ѯ�ӿ�
	m_pIAndroidUserItem=QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,IAndroidUserItem);
	if (m_pIAndroidUserItem==NULL) return false;

	return true;
}

//���ýӿ�
bool  CAndroidUserItemSink::RepositionSink()
{
	//��Ϸ����
	m_lMaxChipBanker = 0;
	m_lMaxChipUser = 0;
	m_wCurrentBanker = 0;
	m_nChipTime = 0;
	m_nChipTimeCount = 0;
	m_cbTimeLeave = 0;
	ZeroMemory(m_lAreaChip, sizeof(m_lAreaChip));
	ZeroMemory(m_nChipLimit, sizeof(m_nChipLimit));

	//��ׯ����
	m_bMeApplyBanker=false;
	m_nWaitBanker=0;
	m_nBankerCount=0;

	return true;
}

//ʱ����Ϣ
bool  CAndroidUserItemSink::OnEventTimer(UINT nTimerID)
{
	switch (nTimerID)
	{
	case IDI_CHECK_BANKER:		//�����ׯ
		{
			m_pIAndroidUserItem->KillGameTimer(nTimerID);
			if (m_wCurrentBanker == m_pIAndroidUserItem->GetChairID())
				return false;

			//��ׯ
			m_nWaitBanker++;

			//��������ׯ
			if (m_bRobotBanker&&
				!m_bMeApplyBanker&&
				m_nWaitBanker>=m_nRobotWaitBanker&&
				m_nListUserCount<m_nMinRobotBankerCount &&
				m_stlApplyBanker < m_nMinRobotBankerCount)
			{
				//�Ϸ��ж�
				IServerUserItem *pIUserItemBanker = m_pIAndroidUserItem->GetMeUserItem();
				if (pIUserItemBanker->GetUserScore() > m_lBankerCondition) 
				{
					//��������ׯ
					m_nBankerCount = 0;
					m_stlApplyBanker++;
					m_pIAndroidUserItem->SetGameTimer(IDI_REQUEST_BANKER, (rand() % m_cbTimeLeave) + 1);
				}
			}

			return false;
		}
	case IDI_REQUEST_BANKER:	//������ׯ
		{
			m_pIAndroidUserItem->KillGameTimer(nTimerID);

			m_pIAndroidUserItem->SendSocketData(SUB_C_APPLY_BANKER);

			return false;
		}
	case IDI_GIVEUP_BANKER:		//������ׯ
		{
			m_pIAndroidUserItem->KillGameTimer(nTimerID);

			m_pIAndroidUserItem->SendSocketData(SUB_C_CANCEL_BANKER);

			return false;
		}
	case IDI_BANK_OPERATE:		//���в���
		{
			m_pIAndroidUserItem->KillGameTimer(nTimerID);

			//��������
			IServerUserItem *pUserItem = m_pIAndroidUserItem->GetMeUserItem();
			LONGLONG lRobotScore = pUserItem->GetUserScore();			
			{

				//�жϴ�ȡ
				if (lRobotScore > m_lRobotScoreRange[1])
				{
					LONGLONG lSaveScore=0L;

					lSaveScore = LONGLONG(lRobotScore*m_nRobotBankStorageMul/100);
					if (lSaveScore > lRobotScore)  lSaveScore = lRobotScore;

					if (lSaveScore > 0 && m_wCurrentBanker != m_pIAndroidUserItem->GetChairID())
						m_pIAndroidUserItem->PerformSaveScore(lSaveScore);

				}
				else if (lRobotScore < m_lRobotScoreRange[0])
				{
					SCORE lScore=rand()%m_lRobotBankGetScoreBanker+m_lRobotBankGetScore;
					if (lScore > 0)
						m_pIAndroidUserItem->PerformTakeScore(lScore);								
				}
			}
			return false;
		}
	default:
		{
			if (nTimerID >= IDI_PLACE_JETTON && nTimerID <= IDI_PLACE_JETTON+MAX_CHIP_TIME)
			{
				srand(GetTickCount());

				//��������
				int nRandNum = 0, nChipArea = 0, nCurChip = 0, nACTotal = 0, nCurJetLmt[2] = {};
				//LONGLONG lMaxChipLmt = __min(m_lMaxChipBanker, m_lMaxChipUser);			//������עֵ
				WORD wMyID = m_pIAndroidUserItem->GetChairID();
				for (int i = 0; i < AREA_COUNT; i++)
					nACTotal += m_RobotInfo.nAreaChance[i];

				//ͳ����ע
				LONGLONG lScore=m_lMaxChipBanker;

				LONGLONG lMaxChipLmt = __min(lScore, m_lMaxChipUser);			//������עֵ

				//ͳ�ƴ���
				m_nChipTimeCount++;

				//����˳�
				if (lMaxChipLmt < m_RobotInfo.nChip[m_nChipLimit[0]])	return false;
				for (int i = 0; i <AREA_COUNT; i++)
				{
					if (m_lAreaChip[i] >= m_RobotInfo.nChip[m_nChipLimit[0]])	break;
					if (i == AREA_COUNT-1)	return false;
				}

				//��ע����
				ASSERT(nACTotal>0);
				static int nStFluc = 1;				//�������
				if (nACTotal <= 0)	return false;
				do {
					nRandNum = (rand()+wMyID+nStFluc*3) % nACTotal;
					for (int i = 0; i < AREA_COUNT; i++)
					{
						nRandNum -= m_RobotInfo.nAreaChance[i];
						if (nRandNum < 0)
						{
							nChipArea = i;
							break;
						}
					}
				}
				while (m_lAreaChip[nChipArea] < m_RobotInfo.nChip[m_nChipLimit[0]]);
				nStFluc = nStFluc%3 + 1;

				//��ע��С
// 				if (m_nChipLimit[0] == m_nChipLimit[1])
// 					nCurChip = m_nChipLimit[0];
// 				else
				{
					//���ñ���
					lMaxChipLmt = __min(lMaxChipLmt, m_lAreaChip[nChipArea]);
					nCurJetLmt[0] = m_nChipLimit[0];
					nCurJetLmt[1] = m_nChipLimit[0];

					//���㵱ǰ������
					for (int i = m_nChipLimit[1]; i > m_nChipLimit[0]; i--)
					{
						if (lMaxChipLmt > m_RobotInfo.nChip[i]) 
						{
							nCurJetLmt[1] = i;
							break;
						}
					}

					//�����ע
					nRandNum = (rand()+wMyID) % (nCurJetLmt[1]-nCurJetLmt[0]+1);
					nCurChip = nCurJetLmt[0] + nRandNum;

					//���¿��� (��ׯ�ҽ�ҽ���ʱ�ᾡ����֤�������)
					if (m_nChipTimeCount < m_nChipTime)
					{
						LONGLONG lLeftJetton = LONGLONG( (lMaxChipLmt-m_RobotInfo.nChip[nCurChip])/(m_nChipTime-m_nChipTimeCount) );

						//�������� (��ȫ����С���Ƴ�����עҲ����)
						if (lLeftJetton < m_RobotInfo.nChip[m_nChipLimit[0]] && nCurChip > m_nChipLimit[0])
							nCurChip--;
					}
				}
				//�������Ʋ�������ע
				if(m_lAreaChip[nCurChip]-m_RobotInfo.nChip[nCurChip]<0) return false;

				//��������
				CMD_C_PlaceJetton PlaceJetton = {};

				//�������
				PlaceJetton.cbJettonArea = nChipArea+1;		//������1��ʼ
				PlaceJetton.lJettonScore = m_RobotInfo.nChip[nCurChip];

				//������Ϣ
				m_pIAndroidUserItem->SendSocketData(SUB_C_PLACE_JETTON, &PlaceJetton, sizeof(PlaceJetton));
			}

			m_pIAndroidUserItem->KillGameTimer(nTimerID);
			return false;
		}
	}
	return false;
}

//��Ϸ��Ϣ
bool  CAndroidUserItemSink::OnEventGameMessage(WORD wSubCmdID, VOID * pBuffer, WORD wDataSize)
{
	switch (wSubCmdID)
	{
	case SUB_S_GAME_FREE:			//��Ϸ���� 
		{
			return OnSubGameFree(pBuffer, wDataSize);
		}
	case SUB_S_GAME_START:			//��Ϸ��ʼ
		{
			return OnSubGameStart(pBuffer, wDataSize);
		}
	case SUB_S_PLACE_JETTON:		//�û���ע
		{
			return OnSubPlaceJetton(pBuffer, wDataSize);
		}
	case SUB_S_APPLY_BANKER:		//������ׯ 
		{
			return OnSubUserApplyBanker(pBuffer,wDataSize);
		}
	case SUB_S_CANCEL_BANKER:		//ȡ����ׯ 
		{
			return OnSubUserCancelBanker(pBuffer,wDataSize);
		}
	case SUB_S_CHANGE_BANKER:		//�л�ׯ�� 
		{
			return OnSubChangeBanker(pBuffer,wDataSize);
		}
	case SUB_S_GAME_END:			//��Ϸ���� 
		{
			return OnSubGameEnd(pBuffer, wDataSize);
		}
	case SUB_S_SEND_RECORD:			//��Ϸ��¼ (����)
		{
			return true;
		}
	case SUB_S_PLACE_JETTON_FAIL:	//��עʧ�� (����)
		{
			return true;
		}
	}

	//�������
//	ASSERT(FALSE);

	return true;
}

//��Ϸ��Ϣ
bool  CAndroidUserItemSink::OnEventFrameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize)
{
	return true;
}

//������Ϣ
bool  CAndroidUserItemSink::OnEventSceneMessage(BYTE cbGameStatus, bool bLookonOther, VOID * pData, WORD wDataSize)
{
	switch (cbGameStatus)
	{
	case GS_GAME_FREE:			//����״̬
		{
			//Ч������
			ASSERT(wDataSize==sizeof(CMD_S_StatusFree));
			if (wDataSize!=sizeof(CMD_S_StatusFree)) return false;

			//��Ϣ����
			CMD_S_StatusFree * pStatusFree=(CMD_S_StatusFree *)pData;
			
			if(m_bInitFlag==false)
			{
				m_lUserLimitScore = pStatusFree->lUserMaxScore;
				m_lAreaLimitScore = pStatusFree->lAreaLimitScore;
				m_lBankerCondition = pStatusFree->lApplyBankerCondition;
			}

			memcpy(m_szRoomName, pStatusFree->szGameRoomName, sizeof(m_szRoomName));

			//ReadConfigInformation(m_RobotInfo.szCfgFileName, m_szRoomName, true);

			//MyDebug(_T("��������ׯ(Free) %d [%d %d] [%d %d]"), m_pIAndroidUserItem->GetChairID(), m_nWaitBanker, 
			//	m_nRobotWaitBanker, m_stlApplyBanker, m_nRobotApplyBanker);

			//��ׯ����
			if (pStatusFree->wBankerUser == INVALID_CHAIR)
			{
				if (m_bRobotBanker && m_nRobotWaitBanker == 0  && m_stlApplyBanker < m_nRobotApplyBanker)
				{
					//�Ϸ��ж�
					IServerUserItem *pIUserItemBanker = m_pIAndroidUserItem->GetMeUserItem();
					if (pIUserItemBanker->GetUserScore() > m_lBankerCondition) 
					{
						//��������ׯ
						m_nBankerCount = 0;
						m_stlApplyBanker++;
						
						BYTE cbTime = (pStatusFree->cbTimeLeave>0?(rand()%pStatusFree->cbTimeLeave+1):2);
						if (cbTime == 0) cbTime = 2;

						m_pIAndroidUserItem->SetGameTimer(IDI_REQUEST_BANKER, cbTime);
					}
				}
			}

			ReadConfigInformation(&pStatusFree->CustomAndroid);

			return true;
		}
	case GS_PLACE_JETTON:		//��Ϸ״̬
	case GS_GAME_END:		//����״̬
		{
			//Ч������
			ASSERT(wDataSize==sizeof(CMD_S_StatusPlay));
			if (wDataSize!=sizeof(CMD_S_StatusPlay)) return false;

			//��Ϣ����
			CMD_S_StatusPlay * pStatusPlay=(CMD_S_StatusPlay *)pData;

			//ׯ����Ϣ
			m_wCurrentBanker = pStatusPlay->wBankerUser;
			
			if(m_bInitFlag==false)
			{
				m_lUserLimitScore = pStatusPlay->lUserMaxScore;
				m_lAreaLimitScore = pStatusPlay->lAreaLimitScore;
				m_lBankerCondition = pStatusPlay->lApplyBankerCondition;
			}

			memcpy(m_szRoomName, pStatusPlay->szGameRoomName, sizeof(m_szRoomName));

			//ReadConfigInformation(m_RobotInfo.szCfgFileName, m_szRoomName, true);
			ReadConfigInformation(&pStatusPlay->CustomAndroid);
			return true;
		}
	}

	return true;
}

//�û�����
VOID  CAndroidUserItemSink::OnEventUserEnter(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//�û��뿪
VOID  CAndroidUserItemSink::OnEventUserLeave(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//�û�����
VOID  CAndroidUserItemSink::OnEventUserScore(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//�û�״̬
VOID  CAndroidUserItemSink::OnEventUserStatus(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//��Ϸ����
bool CAndroidUserItemSink::OnSubGameFree(const VOID * pBuffer, WORD wDataSize)
{
	//��ȡ����
	//if (m_bRefreshCfg)
		//ReadConfigInformation(m_RobotInfo.szCfgFileName, m_szRoomName, false);
	//m_stlApplyBanker=0;

	//��Ϣ����
	CMD_S_GameFree* pGameFree=(CMD_S_GameFree *)pBuffer;

	m_cbTimeLeave = pGameFree->cbTimeLeave;
	m_nListUserCount = pGameFree->nListUserCount;

	//���в�������������ׯ�ҵ�ʱ���ֹ�������в�����
	if (pGameFree->cbTimeLeave > 1 && m_wCurrentBanker!=m_pIAndroidUserItem->GetChairID())
		m_pIAndroidUserItem->SetGameTimer(IDI_BANK_OPERATE, (rand() % (pGameFree->cbTimeLeave-1)) + 1);

	bool bMeGiveUp = false;
	if (m_wCurrentBanker == m_pIAndroidUserItem->GetChairID())
	{
		m_nBankerCount++;
		if ( m_nBankerCount >= m_nRobotBankerCount )
		{
			//��������ׯ
			m_nBankerCount = 0;
			m_pIAndroidUserItem->SetGameTimer(IDI_GIVEUP_BANKER, rand()%2 + 1);
			//��������ׯ��ʱ���ٽ������в���
			m_pIAndroidUserItem->SetGameTimer(IDI_BANK_OPERATE, (rand() % (pGameFree->cbTimeLeave-1)) + 1);

			bMeGiveUp = true;
		}
	}

	//�����ׯ
	if (m_wCurrentBanker != m_pIAndroidUserItem->GetChairID() || bMeGiveUp)
	{
		m_cbTimeLeave = pGameFree->cbTimeLeave - 3;
		m_pIAndroidUserItem->SetGameTimer(IDI_CHECK_BANKER, 3);
	}
	//�һ�����
	CMD_C_ChangeGold ChangeGold;
	ZeroMemory(&ChangeGold,sizeof(ChangeGold));
	ChangeGold.lChangeGold = 1;
	m_pIAndroidUserItem->SendSocketData(SUB_C_CHANGE_GOLD,&ChangeGold,sizeof(ChangeGold));
	return true;
}

//��Ϸ��ʼ
bool CAndroidUserItemSink::OnSubGameStart(const VOID * pBuffer, WORD wDataSize)
{	
	//Ч������
	ASSERT(wDataSize==sizeof(CMD_S_GameStart));
	if (wDataSize!=sizeof(CMD_S_GameStart)) return false;

	//��Ϣ����
	CMD_S_GameStart * pGameStart=(CMD_S_GameStart *)pBuffer;

	srand(GetTickCount());

	//�Լ���ׯ������ע������
	if (pGameStart->wBankerUser == m_pIAndroidUserItem->GetChairID() || pGameStart->nChipRobotCount <= 0)
		return true;	

	//���ñ���
	m_lMaxChipBanker = pGameStart->lBankerScore;///m_RobotInfo.nMaxTime;
	m_lMaxChipUser = pGameStart->lUserMaxScore;///m_RobotInfo.nMaxTime;

	m_wCurrentBanker = pGameStart->wBankerUser;
	m_nChipTimeCount = 0;
	ZeroMemory(m_nChipLimit, sizeof(m_nChipLimit));
	//������������ע���ƣ�ȡ������Сֵ(ϵͳ�涨��ͳһ���ƺ�ׯ�ҳ�����������������)
	for (int i = 0; i < AREA_COUNT; i++)
	{
		if(m_wCurrentBanker != INVALID_CHAIR)
			m_lAreaChip[i] = min(m_lAreaLimitScore,m_lMaxChipBanker/m_RobotInfo.nAreaOdds[i]);
		else
			m_lAreaChip[i] = m_lAreaLimitScore;
	}
		

	//ϵͳ��ׯ
	if (pGameStart->wBankerUser == INVALID_CHAIR)
	{
		m_lMaxChipBanker = 2147483647/m_RobotInfo.nMaxTime;
	}
	else
		m_lMaxChipUser = __min( m_lMaxChipUser, m_lMaxChipBanker );

	m_stlApplyBanker=pGameStart->nAndroidCount;

	//������ע����
	int nElapse = 0;												
	WORD wMyID = m_pIAndroidUserItem->GetChairID();

	if (m_nRobotBetTimeLimit[0] == m_nRobotBetTimeLimit[1])
		m_nChipTime = m_nRobotBetTimeLimit[0];
	else
		m_nChipTime = (rand()+wMyID)%(m_nRobotBetTimeLimit[1]-m_nRobotBetTimeLimit[0]+1) + m_nRobotBetTimeLimit[0];
	ASSERT(m_nChipTime>=0);		
	if (m_nChipTime <= 0)	
		m_nChipTime = rand()%6+3;
	//��ȷ,2����������
	if (m_nChipTime > MAX_CHIP_TIME)	m_nChipTime = MAX_CHIP_TIME;	//�޶�MAX_CHIP����ע

	//���㷶Χ
	if (!CalcJettonRange(__min(m_lMaxChipBanker, m_lMaxChipUser), m_lRobotJettonLimit, m_nChipTime, m_nChipLimit))
		return true;

	//����ʱ��
	int nTimeGrid = int(pGameStart->cbTimeLeave-2)*800/m_nChipTime;		//ʱ���,ǰ2�벻��ע,����-2,800��ʾ��������עʱ�䷶Χǧ�ֱ�
	for (int i = 0; i < m_nChipTime; i++)
	{
		int nRandRage = int( nTimeGrid * i / (1300*sqrt((double)m_nChipTime)) ) + 1;		//������Χ
		nElapse = 2 + (nTimeGrid*i)/1000 + ( (rand()+wMyID)%(nRandRage*2) - (nRandRage-1) );
		ASSERT(nElapse>=2&&nElapse<=pGameStart->cbTimeLeave);
		if (nElapse < 2 || nElapse > pGameStart->cbTimeLeave)	continue;
		
		m_pIAndroidUserItem->SetGameTimer(IDI_PLACE_JETTON+i+1, nElapse);		
	}

	//ASSERT( MyDebug(_T("������ %d ��ע���� %d ��Χ [%d %d] ������ %d ���� [%I64d %I64d] ��ׯ [%d %d]"), wMyID, m_nChipTime, m_nChipLimit[0], m_nChipLimit[1], 
	//	pGameStart->nChipRobotCount, m_lMaxChipBanker, m_lMaxChipUser, m_stlApplyBanker, m_nRobotApplyBanker) );

	return true;
}

//�û���ע
bool CAndroidUserItemSink::OnSubPlaceJetton(const VOID * pBuffer, WORD wDataSize)
{
	//Ч������
	ASSERT(wDataSize==sizeof(CMD_S_PlaceJetton));
	if (wDataSize!=sizeof(CMD_S_PlaceJetton)) return false;

	//��Ϣ����
	CMD_S_PlaceJetton * pPlaceJetton=(CMD_S_PlaceJetton *)pBuffer;

	//���ñ���
	m_lMaxChipBanker -= pPlaceJetton->lJettonScore;
	m_lAreaChip[pPlaceJetton->cbJettonArea-1] -= pPlaceJetton->lJettonScore;
	if (pPlaceJetton->wChairID == m_pIAndroidUserItem->GetChairID())
		m_lMaxChipUser -= pPlaceJetton->lJettonScore;

	return true;
}

//��עʧ��
bool CAndroidUserItemSink::OnSubPlaceJettonFail(const VOID * pBuffer, WORD wDataSize)
{
	return true;
}

//��Ϸ����
bool CAndroidUserItemSink::OnSubGameEnd(const VOID * pBuffer, WORD wDataSize)
{
	//Ч������
	ASSERT(wDataSize==sizeof(CMD_S_GameEnd));
	if (wDataSize!=sizeof(CMD_S_GameEnd)) return false;

	//��Ϣ����
	CMD_S_GameEnd * pGameEnd=(CMD_S_GameEnd *)pBuffer;
	
	return true;
}

//������ׯ
bool CAndroidUserItemSink::OnSubUserApplyBanker(const VOID * pBuffer, WORD wDataSize)
{
	//Ч������
	ASSERT(wDataSize==sizeof(CMD_S_ApplyBanker));
	if (wDataSize!=sizeof(CMD_S_ApplyBanker)) return false;

	//��Ϣ����
	CMD_S_ApplyBanker * pApplyBanker=(CMD_S_ApplyBanker *)pBuffer;

	//�Լ��ж�
	if (m_pIAndroidUserItem->GetChairID()==pApplyBanker->wApplyUser) 
		m_bMeApplyBanker=true;

	return true;
}

//ȡ����ׯ
bool CAndroidUserItemSink::OnSubUserCancelBanker(const VOID * pBuffer, WORD wDataSize)
{
	//Ч������
	ASSERT(wDataSize==sizeof(CMD_S_CancelBanker));
	if (wDataSize!=sizeof(CMD_S_CancelBanker)) return false;

	//��Ϣ����
	CMD_S_CancelBanker * pCancelBanker=(CMD_S_CancelBanker *)pBuffer;

	//�Լ��ж�
	if (m_pIAndroidUserItem->GetChairID()==pCancelBanker->wCancelUser) 
		m_bMeApplyBanker=false;

	return true;
}

//�л�ׯ��
bool CAndroidUserItemSink::OnSubChangeBanker(const VOID * pBuffer, WORD wDataSize)
{
	//Ч������
	ASSERT(wDataSize==sizeof(CMD_S_ChangeBanker));
	if (wDataSize!=sizeof(CMD_S_ChangeBanker)) return false;

	//��Ϣ����
	CMD_S_ChangeBanker * pChangeBanker = (CMD_S_ChangeBanker *)pBuffer;
	if(m_wCurrentBanker != pChangeBanker->wBankerUser)
	{
		m_bMeApplyBanker=false;

		if ( m_wCurrentBanker == m_pIAndroidUserItem->GetChairID())
		{
			m_nWaitBanker = 0;
		}
	}


	m_wCurrentBanker = pChangeBanker->wBankerUser;

	return true;
}

//��ȡ����
VOID CAndroidUserItemSink::ReadConfigInformation(tagCustomAndroid *pCustomAndroid)
{
	if(m_bInitFlag==true) return;

	m_bInitFlag=true;
	//��������
	m_lRobotJettonLimit[0]=pCustomAndroid->lRobotMinJetton;
	m_lRobotJettonLimit[1]=pCustomAndroid->lRobotMaxJetton;
	if (m_lRobotJettonLimit[1] > 5000000)					m_lRobotJettonLimit[1] = 5000000;
	if (m_lRobotJettonLimit[0] < 1000)						m_lRobotJettonLimit[0] = 100;
	if (m_lRobotJettonLimit[1] < m_lRobotJettonLimit[0])	m_lRobotJettonLimit[1] = m_lRobotJettonLimit[0];
	
	//��������
	m_nRobotBetTimeLimit[0]=pCustomAndroid->lRobotMinBetTime;
	m_nRobotBetTimeLimit[1]=pCustomAndroid->lRobotMaxBetTime;
	if (m_nRobotBetTimeLimit[0] < 0)							m_nRobotBetTimeLimit[0] = 3;
	if (m_nRobotBetTimeLimit[1] < m_nRobotBetTimeLimit[0])		m_nRobotBetTimeLimit[1] = m_nRobotBetTimeLimit[0];

	//�Ƿ���ׯ
	m_bRobotBanker=pCustomAndroid->nEnableRobotBanker;
	//��ׯ����
	m_nRobotBankerCount=pCustomAndroid->lRobotBankerCountMin+rand()%(pCustomAndroid->lRobotBankerCountMax-pCustomAndroid->lRobotBankerCountMin+1);
	//��������
	m_nRobotWaitBanker=pCustomAndroid->lRobotWaitBanker;
	//����������
	m_nRobotApplyBanker=pCustomAndroid->lRobotApplyBanker;
	//�б�����
	m_nMinRobotBankerCount=pCustomAndroid->lRobotListMinCount+rand()%(pCustomAndroid->lRobotListMaxCount-pCustomAndroid->lRobotListMinCount+1);
	//��������
	m_lRobotScoreRange[0]=pCustomAndroid->lRobotScoreMin;
	m_lRobotScoreRange[1]=pCustomAndroid->lRobotScoreMax;
	if (m_lRobotScoreRange[1] < m_lRobotScoreRange[0])	m_lRobotScoreRange[1] = m_lRobotScoreRange[0];
	//�������(�������Сֵ֮�����)
	m_lRobotBankGetScore=pCustomAndroid->lRobotBankGetMin+rand()%(pCustomAndroid->lRobotBankGetMax-pCustomAndroid->lRobotBankGetMin+1);
	//������� (ׯ��)
	m_lRobotBankGetScoreBanker=pCustomAndroid->lRobotBankGetBankerMin+rand()%(pCustomAndroid->lRobotBankGetBankerMax-pCustomAndroid->lRobotBankGetBankerMin+1);
	//����
	m_nRobotBankStorageMul=pCustomAndroid->lRobotBankStoMul;
	if(m_nRobotBankStorageMul<0 || m_nRobotBankStorageMul>100) m_nRobotBankStorageMul=50;

}

//���㷶Χ	(����ֵ��ʾ�Ƿ����ͨ���½����޴ﵽ��ע)
bool CAndroidUserItemSink::CalcJettonRange(LONGLONG lMaxScore, LONGLONG lChipLmt[], int & nChipTime, int lJetLmt[])
{
	//�������
	bool bHaveSetMinChip = false;

	//����һע
	if (lMaxScore < m_RobotInfo.nChip[0])	return false;

	//���÷�Χ
	for (int i = 0; i < CountArray(m_RobotInfo.nChip); i++)
	{
		if (!bHaveSetMinChip && m_RobotInfo.nChip[i] >= lChipLmt[0])
		{ 
			lJetLmt[0] = i;
			bHaveSetMinChip = true;
		}
		if (m_RobotInfo.nChip[i] <= lChipLmt[1])
			lJetLmt[1] = i;
	}
	if (lJetLmt[0] > lJetLmt[1])	lJetLmt[0] = lJetLmt[1];

	
// 	if (m_bReduceJettonLimit)
// 	{
	if (nChipTime * m_RobotInfo.nChip[lJetLmt[0]] > lMaxScore)
	{
		//�Ƿ񽵵���ע����
		if (nChipTime * m_RobotInfo.nChip[0] > lMaxScore)
		{
			nChipTime = int(lMaxScore/m_RobotInfo.nChip[0]);
			lJetLmt[0] = 0;
			lJetLmt[1] = 0;
		}
		//�Ƿ񽵵�����
		else if(m_bReduceJettonLimit)
		{
			//���͵���������
			while (nChipTime * m_RobotInfo.nChip[lJetLmt[0]] > lMaxScore)
			{
				lJetLmt[0]--;
				ASSERT(lJetLmt[0]>=0);
			}
		}
	}
	//}

	return true;
}

//�����������
DECLARE_CREATE_MODULE(AndroidUserItemSink);

//////////////////////////////////////////////////////////////////////////