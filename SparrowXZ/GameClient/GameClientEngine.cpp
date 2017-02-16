#include "Stdafx.h"
#include "GameClient.h"
#include "GameOption.h"
#include "GameClientEngine.h"

//////////////////////////////////////////////////////////////////////////

//游戏定时器
#define IDI_START_GAME				200									//开始定时器
#define IDI_CALL_CARD				201									//定缺定时器
#define IDI_OPERATE_CARD			202									//操作定时器

//游戏定时器
#define TIME_START_GAME				30									//开始定时器
#define TIME_CALL_CARD				30									//定缺定时器
#define TIME_OPERATE_CARD			30									//操作定时器

#define MAX_TIME_OUT				3									//最大超时次数

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

static CString GetFileDialogPath()
{
	CString strFileDlgPath;
	TCHAR szModuleDirectory[MAX_PATH];	//模块目录
	GetModuleFileName(AfxGetInstanceHandle(),szModuleDirectory,sizeof(szModuleDirectory));
	int nModuleLen=lstrlen(szModuleDirectory);
	int nProcessLen=lstrlen(AfxGetApp()->m_pszExeName)+lstrlen(TEXT(".EXE")) + 1;
	if (nModuleLen<=nProcessLen) 
		return TEXT("");
	szModuleDirectory[nModuleLen-nProcessLen]=0;
	strFileDlgPath = szModuleDirectory;
	return strFileDlgPath;
}

static void NcaTextOut(CString strInfo)
{
	CTime time = CTime::GetCurrentTime() ;

	CString strName;
	strName.Format(TEXT("%s\\SparrowXZ%04d%02d%02d.log"), 
		GetFileDialogPath(),
		time.GetYear(),
		time.GetMonth(),
		time.GetDay()
		);

	CString strTime ;
	strTime.Format( TEXT( "%04d-%02d-%02d %02d:%02d:%02d" ) ,
		time.GetYear() ,
		time.GetMonth() ,
		time.GetDay() ,
		time.GetHour() ,
		time.GetMinute() ,
		time.GetSecond()
		);
	CString strMsg;
	strMsg.Format(TEXT("%s,%s\r\n"),strTime,strInfo);

	CFile fLog;
	if(fLog.Open( strName, CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite ))
	{

		fLog.SeekToEnd(); 	
		int strLength=strMsg.GetLength();
#ifdef _UNICODE
		BYTE bom[2] = {0xff, 0xfe};
		fLog.Write(bom,sizeof(BYTE)*2);
		strLength*=2;
#endif
		fLog.Write((LPCTSTR)strMsg,strLength);
		fLog.Close();
	}
}

//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGameClientEngine, CGameFrameEngine)
	ON_MESSAGE(IDM_START,OnStart)
	ON_MESSAGE(IDM_CALL_CARD,OnCallCard)
	ON_MESSAGE(IDM_OUT_CARD,OnOutCard)
	ON_MESSAGE(IDM_CARD_OPERATE,OnCardOperate)
	ON_MESSAGE(IDM_TRUSTEE_CONTROL,OnStusteeControl)
	ON_MESSAGE(IDM_SICE_TWO,OnSiceTwo)
	ON_MESSAGE(IDM_SICE_FINISH,OnSiceFinish)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////

//构造函数
CGameClientEngine::CGameClientEngine()
{
	//游戏变量
	m_bCheatMode=false;
	m_wBankerUser=INVALID_CHAIR;
	m_wCurrentUser=INVALID_CHAIR;
	m_cbUserAction = 0;
	m_lSiceCount = 0;

	//缺门变量
	ZeroMemory(m_bCallCard, sizeof(m_bCallCard));
	ZeroMemory(m_cbCallCard, sizeof(m_cbCallCard));

	//堆立变量
	m_wHeapHand=0;
	m_wHeapTail=0;
	ZeroMemory(m_cbHeapCardInfo,sizeof(m_cbHeapCardInfo));

	//托管变量
	m_bStustee=false;
	m_wTimeOutCount =0;

	//出牌信息
	m_cbOutCardData=0;
	m_wOutCardUser=INVALID_CHAIR;
	ZeroMemory(m_cbDiscardCard,sizeof(m_cbDiscardCard));
	ZeroMemory(m_cbDiscardCount,sizeof(m_cbDiscardCount));

	//组合扑克
	ZeroMemory(m_cbWeaveCount,sizeof(m_cbWeaveCount));
	ZeroMemory(m_WeaveItemArray,sizeof(m_WeaveItemArray));

	//扑克变量
	m_cbLeftCardCount=0;
	ZeroMemory(m_cbCardIndex,sizeof(m_cbCardIndex));

	//随机种子
	srand((unsigned int)time(NULL));

	return;
}

//析构函数
CGameClientEngine::~CGameClientEngine()
{
}

//初始函数
bool CGameClientEngine::OnInitGameEngine()
{
	//设置图标
	HICON hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	m_pIClientKernel->SetGameAttribute(KIND_ID,GAME_PLAYER,VERSION_CLIENT,hIcon,GAME_NAME);
	SetIcon(hIcon,TRUE);
	SetIcon(hIcon,FALSE);

	//加载声音
	m_DirectSound.Create( TEXT("OUT_CARD") );

	////TEST+
	//m_GameClientView.m_btStart.ShowWindow(true);
	////TEST+

	return true;
}

//重置框架
bool CGameClientEngine::OnResetGameEngine()
{
	//游戏变量
	m_wBankerUser=INVALID_CHAIR;
	m_wCurrentUser=INVALID_CHAIR;
	m_cbUserAction = 0;
	m_lSiceCount = 0;

	//缺门变量
	ZeroMemory(m_bCallCard, sizeof(m_bCallCard));
	ZeroMemory(m_cbCallCard, sizeof(m_cbCallCard));
	
	//托管变量
	m_bStustee=false;
	m_wTimeOutCount =0;

	//堆立变量
	m_wHeapHand=0;
	m_wHeapTail=0;
	ZeroMemory(m_cbHeapCardInfo,sizeof(m_cbHeapCardInfo));

	//出牌信息
	m_cbOutCardData=0;
	m_wOutCardUser=INVALID_CHAIR;
	ZeroMemory(m_cbDiscardCard,sizeof(m_cbDiscardCard));
	ZeroMemory(m_cbDiscardCount,sizeof(m_cbDiscardCount));

	//组合扑克
	ZeroMemory(m_cbWeaveCount,sizeof(m_cbWeaveCount));
	ZeroMemory(m_WeaveItemArray,sizeof(m_WeaveItemArray));

	//扑克变量
	m_cbLeftCardCount=0;
	ZeroMemory(m_cbCardIndex,sizeof(m_cbCardIndex));

	//叫分按钮
	m_GameClientView.m_btCallCard1.ShowWindow(false);
	m_GameClientView.m_btCallCard2.ShowWindow(false);
	m_GameClientView.m_btCallCard3.ShowWindow(false);

	return true;
}

//时钟删除
bool CGameClientEngine::OnEventGameClockKill(WORD wChairID)
{
	return true;
}

//时间消息
bool CGameClientEngine::OnEventGameClockInfo(WORD wChairID, UINT nElapse, WORD wClockID)
{
	switch (wClockID)
	{
	case IDI_START_GAME:		//开始游戏
		{
			if( m_bStustee && nElapse < TIME_START_GAME )
			{
				if (IsLookonMode() == false) 
				{
					OnStart(0,0);
				}
				return true;
			}
			if (nElapse==0)
			{
				AfxGetMainWnd()->PostMessage(WM_CLOSE);
				return true;
			}
			if (nElapse <= 5 && IsLookonMode() == false)
			{
				PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_WARN"));
			}

			return true;
		}
	case IDI_OPERATE_CARD:		//操作定时器
		{
			//自动出牌
			bool bAutoOutCard=false;
			if ((bAutoOutCard==true)&&(m_GameClientView.m_ControlWnd.IsWindowVisible()))
				bAutoOutCard=false;
			if((bAutoOutCard==false)&&(m_bStustee==true))
			{
				bAutoOutCard=true;
			}

			//超时判断
			if ((IsLookonMode()==false)&&
				( nElapse==0 || bAutoOutCard || 
				nElapse < TIME_OPERATE_CARD-1&&(m_cbUserAction&WIK_CHI_HU)&&m_cbLeftCardCount<4 )
				)
			{
				//获取位置
				WORD wMeChairID=GetMeChairID();

				//动作处理
				if (wChairID==wMeChairID)
				{
					if(m_bStustee==false && ++m_wTimeOutCount>=MAX_TIME_OUT )
					{
						m_wTimeOutCount = 0;
						OnStusteeControl(TRUE,0);
						if ( m_pIStringMessage )
							m_pIStringMessage->InsertSystemString(TEXT("由于您多次超时，切换为“系统托管”模式."));
					}

					if (m_wCurrentUser==wMeChairID)
					{
						if( m_cbLeftCardCount < 4 && (m_cbUserAction&WIK_CHI_HU) )
						{
							OnCardOperate( WIK_CHI_HU,0 );
							return true;
						}

						//获取扑克
						BYTE cbCardData=m_GameClientView.m_HandCardControl.GetCurrentCard();

						//出牌效验
						if (VerdictOutCard(cbCardData)==false)
						{
							for (BYTE i=0;i<MAX_INDEX;i++)
							{
								//出牌效验
								if (m_cbCardIndex[i]==0) continue;
								if (VerdictOutCard(m_GameLogic.SwitchToCardData(i))==false) 
									continue;

								//设置变量
								cbCardData=m_GameLogic.SwitchToCardData(i);
							}
						}

						//出牌动作
						OnOutCard(cbCardData,cbCardData);
					}
					else if( m_cbLeftCardCount < 4 && (m_cbUserAction&WIK_CHI_HU) )
						OnCardOperate( WIK_CHI_HU,0 );
					else
						OnCardOperate(WIK_NULL,0);
				}

				return true;
			}

			//播放声音
			if ((nElapse<=3)&&(wChairID==GetMeChairID())&&(IsLookonMode()==false)) 
			{
				PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_WARN"));
			}

			return true;
		}
	case IDI_CALL_CARD:			//定缺定时器
		{
			//托管操作
			if (m_bStustee && nElapse < TIME_CALL_CARD)
			{
				if (IsLookonMode() == false)
				{
					if (m_bCallCard[GetMeChairID()] == false)
					{
						BYTE cbCallCard = rand() % 3 + 1;
						OnCallCard(cbCallCard, cbCallCard);
						return true;
					}
				}
			}

			//定时操作
			if (nElapse == 0)
			{
				//删除时间
				KillGameClock(IDI_CALL_CARD);

				if (IsLookonMode() == false)
				{
					if (m_bCallCard[GetMeChairID()] == false)
					{
						BYTE cbCallCard = rand() % 3 + 1;
						OnCallCard(cbCallCard, cbCallCard);
						return true;
					}
				}
			}

			//警告操作
			if (nElapse <= 5 && IsLookonMode()==false)
			{
				PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_WARN"));
			}
		}
	}

	return true;
}

//旁观状态
bool CGameClientEngine::OnEventLookonMode(VOID * pData, WORD wDataSize)
{
	//扑克控制
	m_GameClientView.m_HandCardControl.SetDisplayItem(IsAllowLookon());
	m_GameClientView.InvalidGameView(0,0,0,0);
	
	return true;
}

//网络消息
bool CGameClientEngine::OnEventGameMessage(WORD wSubCmdID, VOID * pBuffer, WORD wDataSize)
{
	switch (wSubCmdID)
	{
	case SUB_S_GAME_START:		//游戏开始
		{
			return OnSubGameStart(pBuffer,wDataSize);
		}
	case SUB_S_CALL_CARD:		//用户选缺
		{
			return OnSubCallCard(pBuffer, wDataSize);
		}
	case SUB_S_BANKER_INFO:		//庄家信息
		{
			return OnSubBankerInfo(pBuffer,wDataSize);
		}
	case SUB_S_OUT_CARD:		//用户出牌
		{
			return OnSubOutCard(pBuffer,wDataSize);
		}
	case SUB_S_SEND_CARD:		//发牌消息
		{
			return OnSubSendCard(pBuffer,wDataSize);
		}
	case SUB_S_OPERATE_NOTIFY:	//操作提示
		{
			return OnSubOperateNotify(pBuffer,wDataSize);
		}
	case SUB_S_OPERATE_RESULT:	//操作结果
		{
			return OnSubOperateResult(pBuffer,wDataSize);
		}
	case SUB_S_GAME_END:		//游戏结束
		{
			return OnSubGameEnd(pBuffer,wDataSize);
		}
	case SUB_S_TRUSTEE:			//用户托管
		{
			return OnSubTrustee(pBuffer,wDataSize);
		}
	case SUB_S_CHI_HU:			//用户吃胡
		{
			return OnSubUserChiHu( pBuffer,wDataSize );
		}
	case SUB_S_GANG_SCORE:		//杠牌分数
		{
			return OnSubGangScore(pBuffer,wDataSize);
		}
	case SUB_S_WAIT_OPERATE:	//等待操作
		{
			m_GameClientView.SetStatusFlag(false, true);
			return true;
		}
	}

	return true;
}

//游戏场景
bool CGameClientEngine::OnEventSceneMessage(BYTE cbGameStation, bool bLookonOther, VOID * pBuffer, WORD wDataSize)
{
	switch (cbGameStation)
	{
	case GS_MJ_FREE:	//空闲状态
		{
			//效验数据
			if (wDataSize!=sizeof(CMD_S_StatusFree)) return false;
			CMD_S_StatusFree * pStatusFree=(CMD_S_StatusFree *)pBuffer;

			//是否启用声音
			if( !CGlobalUnits::GetInstance()->m_bAllowSound )
			{
				m_DirectSound.EnableSound(FALSE);
			}

			//设置数据
			m_bCheatMode=pStatusFree->bCheatMode;
			m_wBankerUser=pStatusFree->wBankerUser;
			m_GameClientView.SetCellScore(pStatusFree->lCellScore);
			m_GameClientView.m_HandCardControl.SetDisplayItem(true);

			//托管设置
			for (WORD i=0;i<GAME_PLAYER;i++)
			{
				m_GameClientView.SetTrustee(SwitchViewChairID(i),pStatusFree->bTrustee[i]);
			}

			//设置界面
			for (WORD i=0;i<GAME_PLAYER;i++)
			{
				m_cbHeapCardInfo[i][0]=0;
				m_cbHeapCardInfo[i][1]=0;
				m_GameClientView.m_HeapCard[SwitchViewChairID(i)].SetCardData(m_cbHeapCardInfo[i][0],m_cbHeapCardInfo[i][1],HEAP_FULL_COUNT);
			}

			//设置控件
			if (IsLookonMode()==false)
			{
				if (!m_bCheatMode)
				{
					m_GameClientView.m_btStart.ShowWindow(TRUE);
					SetGameClock(GetMeChairID(),IDI_START_GAME,TIME_START_GAME);
				}
				else
				{
					m_GameClientView.m_btStart.ShowWindow(FALSE);
				}
				m_GameClientView.m_btStustee.ShowWindow(true);
				m_GameClientView.m_btCancelStustee.ShowWindow(false);
				m_GameClientView.m_btStustee.EnableWindow(TRUE);
			}

			//历史成绩
			for (BYTE i=0;i<GAME_PLAYER;i++)
			{
				//获取变量
				tagHistoryScore * pHistoryScore=m_HistoryScore.GetHistoryScore(i);

				//设置积分
				pHistoryScore->lTurnScore=pStatusFree->lTurnScore[i];
				pHistoryScore->lCollectScore=pStatusFree->lCollectScore[i];

				//绑定设置
				WORD wViewChairID=SwitchViewChairID(i);
				m_GameClientView.SetHistoryScore(wViewChairID,pHistoryScore);
			}
			m_GameClientView.m_btOpenHistoryScore.ShowWindow(true);

			//旁观界面
			m_GameClientView.m_btCancelStustee.EnableWindow(!IsLookonMode());
			m_GameClientView.m_btStustee.EnableWindow(!IsLookonMode());

			//丢弃效果
			m_GameClientView.SetDiscUser(INVALID_CHAIR);
			m_GameClientView.SetTimer(IDI_DISC_EFFECT, 180, NULL);

			//更新界面
			m_GameClientView.InvalidGameView(0,0,0,0);

			return true;
		}
	case GS_MJ_CALL:	//游戏状态
		{
			//效验数据
			if (wDataSize != sizeof(CMD_S_StatusCall)) return false;
			CMD_S_StatusCall * pStatusCall = (CMD_S_StatusCall *)pBuffer;

			//是否启用声音
			if( !CGlobalUnits::GetInstance()->m_bAllowSound )
			{
				m_DirectSound.EnableSound(FALSE);
			}

			//辅助变量
			WORD wViewChairID[GAME_PLAYER]={0,0,0,0};
			for (WORD i=0;i<GAME_PLAYER;i++) wViewChairID[i]=SwitchViewChairID(i);

			//设置数据
			m_wBankerUser=pStatusCall->wBankerUser;
			m_GameClientView.SetCellScore(pStatusCall->lCellScore);

			//旁观界面
			if (IsLookonMode()==true)
			{
				bool bAllowLookon = IsAllowLookon();
				m_GameClientView.m_HandCardControl.SetDisplayItem(bAllowLookon);
			}

			//托管设置
			for (WORD i=0;i<GAME_PLAYER;i++)
			{
				m_GameClientView.SetTrustee(SwitchViewChairID(i),pStatusCall->bTrustee[i]);
			}
			if( pStatusCall->bTrustee[GetMeChairID()] )
			{
				m_bStustee = true;
				OnStusteeControl(0,0);
			}

			//界面设置
			m_GameClientView.SetBankerUser(wViewChairID[m_wBankerUser]);

			//缺门信息
			CopyMemory(m_bCallCard, pStatusCall->bCallCard, sizeof(m_bCallCard));
			CopyMemory(m_cbCallCard, pStatusCall->cbCallCard, sizeof(m_cbCallCard));

			//堆立信息
			m_wHeapHand = pStatusCall->wHeapHand;
			m_wHeapTail = pStatusCall->wHeapTail;
			CopyMemory(m_cbHeapCardInfo, pStatusCall->cbHeapCardInfo, sizeof(m_cbHeapCardInfo));

			//堆立界面
			for (WORD i=0; i<GAME_PLAYER; i++)
			{
				m_GameClientView.m_HeapCard[wViewChairID[i]].SetCardData(m_cbHeapCardInfo[i][0], m_cbHeapCardInfo[i][1], HEAP_FULL_COUNT);
			}

			//扑克变量
			m_GameLogic.SwitchToCardIndex(pStatusCall->cbCardData,pStatusCall->cbCardCount,m_cbCardIndex);

			//扑克设置
			for (WORD i=0; i<GAME_PLAYER; i++)
			{
				//用户扑克
				if (i != GetMeChairID())
				{
					BYTE cbCardCount = MAX_COUNT-1;
					WORD wUserCardIndex = (wViewChairID[i]<2) ? wViewChairID[i] : 2;
					m_GameClientView.m_UserCard[wUserCardIndex].SetCardData(cbCardCount, false);
				}
			}

			//扑克设置
			if (m_wBankerUser == GetMeChairID())
			{
				//调整扑克
				if (pStatusCall->cbSendCardData != 0x00)
				{
					//变量定义
					BYTE cbCardCount = pStatusCall->cbCardCount;
					BYTE cbRemoveCard[] = {pStatusCall->cbSendCardData};

					//调整扑克
					m_GameLogic.RemoveCard(pStatusCall->cbCardData, cbCardCount, cbRemoveCard, 1);
					pStatusCall->cbCardData[pStatusCall->cbCardCount-1] = pStatusCall->cbSendCardData;
				}
				//设置扑克
				BYTE cbCardCount=pStatusCall->cbCardCount;
				m_GameClientView.m_HandCardControl.SetCardData(pStatusCall->cbCardData,cbCardCount-1,pStatusCall->cbCardData[cbCardCount-1]);
			}
			else m_GameClientView.m_HandCardControl.SetCardData(pStatusCall->cbCardData, pStatusCall->cbCardCount, 0);

			//历史成绩
			for (BYTE i=0;i<GAME_PLAYER;i++)
			{
				//获取变量
				tagHistoryScore * pHistoryScore=m_HistoryScore.GetHistoryScore(i);

				//设置积分
				pHistoryScore->lTurnScore=pStatusCall->lTurnScore[i];
				pHistoryScore->lCollectScore=pStatusCall->lCollectScore[i];

				//绑定设置
				WORD wViewChairID=SwitchViewChairID(i);
				m_GameClientView.SetHistoryScore(wViewChairID,pHistoryScore);
			}
			m_GameClientView.m_btOpenHistoryScore.ShowWindow(true);

			//缺门设置
			if ((IsLookonMode()==false)&&(!m_bCallCard[GetMeChairID()]))
			{
				//界面显示
				m_GameClientView.ShowSelectTip(true);
				m_GameClientView.ShowWaitChooseTip(false);

				//显示按钮
				m_GameClientView.m_btCallCard1.ShowWindow(true);
				m_GameClientView.m_btCallCard2.ShowWindow(true);
				m_GameClientView.m_btCallCard3.ShowWindow(true);

				//禁用按钮
				m_GameClientView.m_btCallCard1.EnableWindow(true);
				m_GameClientView.m_btCallCard2.EnableWindow(true);
				m_GameClientView.m_btCallCard3.EnableWindow(true);
			}

			//控制设置
			if (IsLookonMode()==false) 
			{
				m_GameClientView.m_HandCardControl.SetPositively(true);
				m_GameClientView.m_HandCardControl.SetDisplayItem(true);
				m_GameClientView.m_btStustee.EnableWindow(TRUE);
			}

			//设置时间
			if (m_wCurrentUser == INVALID_CHAIR)
			{
				//设置时间
				SetGameClock(GetMeChairID(), IDI_CALL_CARD, TIME_CALL_CARD);
			}

			//更新界面
			m_GameClientView.InvalidGameView(0,0,0,0);

			return true;
		}
	case GS_MJ_PLAY:	//游戏状态
		{
			//效验数据
			if (wDataSize!=sizeof(CMD_S_StatusPlay)) return false;
			CMD_S_StatusPlay * pStatusPlay=(CMD_S_StatusPlay *)pBuffer;

			//是否启用声音
			if( !CGlobalUnits::GetInstance()->m_bAllowSound )
			{
				m_DirectSound.EnableSound(FALSE);
			}

			//用户名称
			for( WORD i = 0; i < GAME_PLAYER; i++ )
			{
				IClientUserItem * pUserData = GetTableUserItem(i);
				if( pUserData == NULL ) 
				{
					m_szAccounts[i][0] = 0;
					continue;
				}
				lstrcpy(m_szAccounts[i], pUserData->GetNickName());
			}

			WORD wMeChairID=GetMeChairID();

			//设置变量
			m_wBankerUser=pStatusPlay->wBankerUser;
			m_wCurrentUser=pStatusPlay->wCurrentUser;
			m_cbLeftCardCount=pStatusPlay->cbLeftCardCount;

			//旁观界面
			if (IsLookonMode()==true)
			{
				bool bAllowLookon = IsAllowLookon();
				m_GameClientView.m_HandCardControl.SetDisplayItem(bAllowLookon);
			}

			//托管设置
			for (WORD i=0;i<GAME_PLAYER;i++)
			{
				m_GameClientView.SetTrustee(SwitchViewChairID(i),pStatusPlay->bTrustee[i]);
			}
			if( pStatusPlay->bTrustee[wMeChairID] )
			{
				m_bStustee = true;
				OnStusteeControl(0,0);
			}

			//历史变量
			m_wOutCardUser=pStatusPlay->wOutCardUser;
			m_cbOutCardData=pStatusPlay->cbOutCardData;
			CopyMemory(m_cbDiscardCard,pStatusPlay->cbDiscardCard,sizeof(m_cbDiscardCard));
			CopyMemory(m_cbDiscardCount,pStatusPlay->cbDiscardCount,sizeof(m_cbDiscardCount));
			if( pStatusPlay->wOutCardUser != INVALID_CHAIR )
				m_cbDiscardCard[pStatusPlay->wOutCardUser][m_cbDiscardCount[pStatusPlay->wOutCardUser]++] = pStatusPlay->cbOutCardData;

			//设置界面
			CopyMemory(m_cbCallCard, pStatusPlay->cbCallCard, sizeof(pStatusPlay->cbCallCard));
			for (WORD i=0; i<GAME_PLAYER; i++)
				m_GameClientView.SetVoidShowing(SwitchViewChairID(i), pStatusPlay->cbCallCard[i]);

			//丢弃效果
			if(m_wOutCardUser != INVALID_CHAIR)
				m_GameClientView.SetDiscUser(SwitchViewChairID(m_wOutCardUser));
			m_GameClientView.SetTimer(IDI_DISC_EFFECT, 180, NULL);

			//扑克变量
			CopyMemory(m_cbWeaveCount,pStatusPlay->cbWeaveCount,sizeof(m_cbWeaveCount));
			CopyMemory(m_WeaveItemArray,pStatusPlay->WeaveItemArray,sizeof(m_WeaveItemArray));
			m_GameLogic.SwitchToCardIndex(pStatusPlay->cbCardData,pStatusPlay->cbCardCount,m_cbCardIndex);

			//辅助变量
			WORD wViewChairID[GAME_PLAYER]={0,0,0,0};
			for (WORD i=0;i<GAME_PLAYER;i++) wViewChairID[i]=SwitchViewChairID(i);

			//界面设置
			m_GameClientView.SetCellScore(pStatusPlay->lCellScore);
			m_GameClientView.SetBankerUser(wViewChairID[m_wBankerUser]);

			//组合扑克
			BYTE cbWeaveCard[4]={0,0,0,0};
			for (WORD i=0;i<GAME_PLAYER;i++)
			{
				WORD wOperateViewID = SwitchViewChairID(i);
				for (BYTE j=0;j<m_cbWeaveCount[i];j++)
				{
					BYTE cbWeaveKind=m_WeaveItemArray[i][j].cbWeaveKind;
					BYTE cbCenterCard=m_WeaveItemArray[i][j].cbCenterCard;
					BYTE cbWeaveCardCount=m_GameLogic.GetWeaveCard(cbWeaveKind,cbCenterCard,cbWeaveCard);
					m_GameClientView.m_WeaveCard[wViewChairID[i]][j].SetCardData(cbWeaveCard,cbWeaveCardCount);
					if ((cbWeaveKind&WIK_GANG)&&(m_WeaveItemArray[i][j].wProvideUser==i)) 
						m_GameClientView.m_WeaveCard[wViewChairID[i]][j].SetDisplayItem(false);
					WORD wProviderViewID = SwitchViewChairID(m_WeaveItemArray[i][j].wProvideUser);
					m_GameClientView.m_WeaveCard[wOperateViewID][j].SetDirectionCardPos(3-(wOperateViewID-wProviderViewID+4)%4);
				}
			}

			//用户扑克
			if (m_wCurrentUser==GetMeChairID())
			{
				//调整扑克
				if (pStatusPlay->cbSendCardData!=0x00)
				{
					//变量定义
					BYTE cbCardCount=pStatusPlay->cbCardCount;
					BYTE cbRemoveCard[]={pStatusPlay->cbSendCardData};

					//调整扑克
					m_GameLogic.RemoveCard(pStatusPlay->cbCardData,cbCardCount,cbRemoveCard,1);
					pStatusPlay->cbCardData[pStatusPlay->cbCardCount-1]=pStatusPlay->cbSendCardData;
				}

				//设置扑克
				BYTE cbCardCount=pStatusPlay->cbCardCount;
				m_GameClientView.m_HandCardControl.SetCardData(pStatusPlay->cbCardData,cbCardCount-1,pStatusPlay->cbCardData[cbCardCount-1]);
			}
			else 
			{
				m_GameClientView.m_HandCardControl.SetCardData(pStatusPlay->cbCardData,pStatusPlay->cbCardCount,0); 
			}

			//扑克设置
			for (WORD i=0;i<GAME_PLAYER;i++)
			{
				//用户扑克
				if (i!=GetMeChairID())
				{
					BYTE cbCardCount=13-m_cbWeaveCount[i]*3;
					WORD wUserCardIndex=(wViewChairID[i]<2)?wViewChairID[i]:2;
					m_GameClientView.m_UserCard[wUserCardIndex].SetCardData(cbCardCount,(m_wCurrentUser==i));
				}

				//丢弃扑克
				WORD wViewChairID=SwitchViewChairID(i);
				m_GameClientView.m_DiscardCard[wViewChairID].SetCardData(m_cbDiscardCard[i],m_cbDiscardCount[i]);
			}

			//控制设置
			if (IsLookonMode()==false) 
			{
				m_GameClientView.m_HandCardControl.SetPositively(true);
				m_GameClientView.m_HandCardControl.SetDisplayItem(true);
				m_GameClientView.m_btStustee.EnableWindow(TRUE);
			}

			//旁观界面
			m_GameClientView.m_btCancelStustee.EnableWindow(!IsLookonMode());
			m_GameClientView.m_btStustee.EnableWindow(!IsLookonMode());

			//堆立信息
			m_wHeapHand = pStatusPlay->wHeapHand;
			m_wHeapTail = pStatusPlay->wHeapTail;
			CopyMemory(m_cbHeapCardInfo,pStatusPlay->cbHeapCardInfo,sizeof(m_cbHeapCardInfo));

			//堆立界面
			for (WORD i=0;i<GAME_PLAYER;i++)
			{
				m_GameClientView.m_HeapCard[wViewChairID[i]].SetCardData(m_cbHeapCardInfo[i][0],m_cbHeapCardInfo[i][1],HEAP_FULL_COUNT);
			}

			//历史扑克
			if (m_wOutCardUser!=INVALID_CHAIR)
			{
				WORD wOutChairID=SwitchViewChairID(m_wOutCardUser);
				m_GameClientView.SetOutCardInfo(wOutChairID,m_cbOutCardData);
			}

			//名次状态
			for( WORD i = 0; i < CountArray(pStatusPlay->wWinOrder); i++ )
			{
				if( pStatusPlay->wWinOrder[i] != INVALID_CHAIR )
				{
					m_GameClientView.SetWinOrder( SwitchViewChairID(pStatusPlay->wWinOrder[i]),i+1 );
				}
			}

			//操作界面
			if ((IsLookonMode()==false)&&(pStatusPlay->cbActionMask!=WIK_NULL))
			{
				//获取变量
				BYTE cbActionMask=pStatusPlay->cbActionMask;
				BYTE cbActionCard=pStatusPlay->cbActionCard;

				m_cbUserAction = pStatusPlay->cbActionMask;

				//变量定义
				tagGangCardResult GangCardResult;
				ZeroMemory(&GangCardResult,sizeof(GangCardResult));

				//杠牌判断
				if ((cbActionMask&WIK_GANG)!=0)
				{
					//桌面杆牌
					if ((m_wCurrentUser==INVALID_CHAIR)&&(cbActionCard!=0))
					{
						GangCardResult.cbCardCount=1;
						GangCardResult.cbCardData[0]=cbActionCard;
					}

					//自己杆牌
					if ((m_wCurrentUser==GetMeChairID())||(cbActionCard==0))
					{
						WORD wMeChairID=GetMeChairID();
						m_GameLogic.AnalyseGangCard(m_cbCardIndex,m_WeaveItemArray[wMeChairID],m_cbWeaveCount[wMeChairID],GangCardResult);
					}
				}

				//设置界面
				if (m_wCurrentUser==INVALID_CHAIR)
					SetGameClock(GetMeChairID(),IDI_OPERATE_CARD,TIME_OPERATE_CARD);
				if (IsLookonMode()==false) 
					m_GameClientView.m_ControlWnd.SetControlInfo(cbActionCard,cbActionMask,GangCardResult);
			}

			//历史成绩
			for (BYTE i=0;i<GAME_PLAYER;i++)
			{
				//获取变量
				tagHistoryScore * pHistoryScore=m_HistoryScore.GetHistoryScore(i);

				//设置积分
				pHistoryScore->lTurnScore=pStatusPlay->lTurnScore[i];
				pHistoryScore->lCollectScore=pStatusPlay->lCollectScore[i];

				//绑定设置
				WORD wViewChairID=SwitchViewChairID(i);
				m_GameClientView.SetHistoryScore(wViewChairID,pHistoryScore);
			}
			m_GameClientView.m_btOpenHistoryScore.ShowWindow(true);

			//设置时间
			if (m_wCurrentUser!=INVALID_CHAIR)
			{
				//设置时间
				SetGameClock(m_wCurrentUser, IDI_OPERATE_CARD, TIME_OPERATE_CARD);
			}

			//丢弃效果
			m_GameClientView.SetDiscUser(SwitchViewChairID(m_wOutCardUser));
			m_GameClientView.SetTimer(IDI_DISC_EFFECT, 180, NULL);

			//更新界面
			m_GameClientView.InvalidGameView(0,0,0,0);

			return true;
		}
	}

	return false;
}

//游戏开始
bool CGameClientEngine::OnSubGameStart(const void * pBuffer, WORD wDataSize)
{
	//起始时间
	DWORD dwStartTime = GetTickCount();

	//效验数据
	ASSERT(wDataSize==sizeof(CMD_S_GameStart));
	if (wDataSize!=sizeof(CMD_S_GameStart)) return false;

	//变量定义
	CMD_S_GameStart * pGameStart=(CMD_S_GameStart *)pBuffer;

	//用户名称
	for( WORD i = 0; i < GAME_PLAYER; i++ )
	{
		IClientUserItem * pUserData = GetTableUserItem( i );
		if( pUserData == NULL ) 
		{
			m_szAccounts[i][0] = 0;
			continue;
		}
		lstrcpy( m_szAccounts[i],pUserData->GetNickName() );
	}

	//设置状态
	SetGameStatus(GS_MJ_CALL);
	m_GameClientView.SetCurrentUser(INVALID_CHAIR);

	//设置变量
	m_wBankerUser=pGameStart->wBankerUser;
	m_wCurrentUser=pGameStart->wCurrentUser;
	m_cbLeftCardCount=pGameStart->cbLeftCardCount;
	m_cbUserAction = pGameStart->cbUserAction;
	m_lSiceCount = pGameStart->lSiceCount;

	//出牌信息
	m_cbOutCardData=0;
	m_wOutCardUser=INVALID_CHAIR;
	ZeroMemory(m_cbDiscardCard,sizeof(m_cbDiscardCard));
	ZeroMemory(m_cbDiscardCount,sizeof(m_cbDiscardCount));

	//组合扑克
	ZeroMemory(m_cbWeaveCount,sizeof(m_cbWeaveCount));
	ZeroMemory(m_WeaveItemArray,sizeof(m_WeaveItemArray));

	//设置扑克
	BYTE cbCardCount=(GetMeChairID()==m_wBankerUser)?MAX_COUNT:(MAX_COUNT-1);
	m_GameLogic.SwitchToCardIndex(pGameStart->cbCardData,cbCardCount,m_cbCardIndex);

	//旁观界面
	if (IsLookonMode()==true)
	{
		//设置界面
		m_GameClientView.SetDiscUser(INVALID_CHAIR);
		m_GameClientView.SetStatusFlag(false,false);
		m_GameClientView.SetBankerUser(INVALID_CHAIR);
		m_GameClientView.SetUserAction(INVALID_CHAIR,0);
		m_GameClientView.SetOutCardInfo(INVALID_CHAIR,0);
		m_GameClientView.SetWinOrder(INVALID_CHAIR,0);

		//扑克设置
		m_GameClientView.m_UserCard[0].SetCardData(0,false);
		m_GameClientView.m_UserCard[1].SetCardData(0,false);
		m_GameClientView.m_UserCard[2].SetCardData(0,false);
		m_GameClientView.m_HandCardControl.SetCardData(NULL,0,0);

		//扑克设置
		for (WORD i=0;i<GAME_PLAYER;i++)
		{
			m_GameClientView.m_ScoreControl[i].CloseControl();
			m_GameClientView.m_TableCard[i].SetCardData(NULL,0);
			m_GameClientView.m_DiscardCard[i].SetCardData(NULL,0);
			m_GameClientView.m_WeaveCard[i][0].SetCardData(NULL,0);
			m_GameClientView.m_WeaveCard[i][1].SetCardData(NULL,0);
			m_GameClientView.m_WeaveCard[i][2].SetCardData(NULL,0);
			m_GameClientView.m_WeaveCard[i][3].SetCardData(NULL,0);
			m_GameClientView.m_WeaveCard[i][0].SetGameEnd(false);
			m_GameClientView.m_WeaveCard[i][1].SetGameEnd(false);
			m_GameClientView.m_WeaveCard[i][2].SetGameEnd(false);
			m_GameClientView.m_WeaveCard[i][3].SetGameEnd(false);
			m_GameClientView.m_WeaveCard[i][0].SetDisplayItem(true);
			m_GameClientView.m_WeaveCard[i][1].SetDisplayItem(true);
			m_GameClientView.m_WeaveCard[i][2].SetDisplayItem(true);
			m_GameClientView.m_WeaveCard[i][3].SetDisplayItem(true);
		}

		//堆立扑克
		for (WORD i=0;i<GAME_PLAYER;i++)
		{
			m_cbHeapCardInfo[i][0]=0;
			m_cbHeapCardInfo[i][1]=0;
			m_GameClientView.m_HeapCard[SwitchViewChairID(i)].SetCardData(m_cbHeapCardInfo[i][0],m_cbHeapCardInfo[i][1],HEAP_FULL_COUNT);
		}
	}

	//堆立信息
	m_wHeapHand = pGameStart->wHeapHand;
	m_wHeapTail = pGameStart->wHeapTail;
	CopyMemory(m_cbHeapCardInfo,pGameStart->cbHeapCardInfo,sizeof(m_cbHeapCardInfo));

	//设置界面
	m_GameClientView.m_btStart.ShowWindow(SW_HIDE);
	m_GameClientView.m_HandCardControl.SetPositively(false);
	if ((m_lSiceCount&0xFFFF0000)==0)m_GameClientView.SetBankerUser(SwitchViewChairID(m_wBankerUser));

	//环境处理
	PlayGameSound(AfxGetInstanceHandle(),TEXT("DRAW_SICE"));

	//摇色子动画
	m_GameClientView.m_DrawSiceWnd.SetSiceInfo(m_GameClientView.GetDC(),200,(WORD)(pGameStart->lSiceCount>>16),(WORD)pGameStart->lSiceCount);
	m_GameClientView.m_DrawSiceWnd.ShowWindow(SW_SHOW);

	//时间间隔
	DWORD dwEndTime = GetTickCount();
	DWORD dwDiffenTime = dwEndTime - dwStartTime;

	//写入日记
	CString strValue;
	strValue.Format(TEXT("游戏开始：起始时间=%d；结束时间=%d；时间间隔=%d；"), dwStartTime, dwEndTime, dwDiffenTime);
	NcaTextOut(strValue);

	return true;
}

//用户选缺
bool CGameClientEngine::OnSubCallCard(const void * pBuffer, WORD wDataSize)
{
	//效验消息
	ASSERT(wDataSize == sizeof(CMD_S_CallCard));
	if (wDataSize != sizeof(CMD_S_CallCard)) return false;

	//消息处理
	CMD_S_CallCard * pCallCard = (CMD_S_CallCard *)pBuffer;

	//设置变量
	m_bCallCard[pCallCard->wCallUser] = true;

	return true;
}

//庄家信息
bool CGameClientEngine::OnSubBankerInfo(const void * pBuffer, WORD wDataSize)
{
	//效验参数
	ASSERT(wDataSize==sizeof(CMD_S_BankerInfo));
	if (wDataSize!=sizeof(CMD_S_BankerInfo)) return false;

	//变量定义
	CMD_S_BankerInfo * pBankerInfo=(CMD_S_BankerInfo *)pBuffer;

	//设置状态
	SetGameStatus(GS_MJ_PLAY);

	//激活框架
	if (m_bStustee==false) ActiveGameFrame();

	//设置变量
	m_wBankerUser = pBankerInfo->wBankerUser;
	m_wCurrentUser = pBankerInfo->wCurrentUser;

	//设置界面
	for (WORD i=0; i<GAME_PLAYER; i++)
	{
		m_GameClientView.SetVoidShowing(SwitchViewChairID(i), pBankerInfo->cbCallCard[i]);
	}
	m_GameClientView.ShowSelectTip(false);
	m_GameClientView.ShowWaitChooseTip(false);

	//出牌提示
	if ((IsLookonMode()==false)&&(m_wCurrentUser!=INVALID_CHAIR))
	{
		WORD wMeChairID=GetMeChairID();
		if (m_wCurrentUser==wMeChairID) m_GameClientView.SetStatusFlag(true, false);
	}

	//动作处理
	if ((IsLookonMode()==false)&&(m_cbUserAction!=WIK_NULL))
	{
		ShowOperateControl(m_cbUserAction, 0);
	}

	//设置时钟
	m_GameClientView.SetCurrentUser(SwitchViewChairID(m_wCurrentUser));
	SetGameClock(m_wCurrentUser, IDI_OPERATE_CARD, TIME_OPERATE_CARD);
	m_GameClientView.m_HandCardControl.SetPositively(IsLookonMode()==false);

	//环境处理
	PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_START"));

	return true;
}

//用户出牌
bool CGameClientEngine::OnSubOutCard(const void * pBuffer, WORD wDataSize)
{
	//效验消息
	ASSERT(wDataSize==sizeof(CMD_S_OutCard));
	if (wDataSize!=sizeof(CMD_S_OutCard)) return false;

	//消息处理
	CMD_S_OutCard * pOutCard=(CMD_S_OutCard *)pBuffer;

	//变量定义
	WORD wMeChairID=GetMeChairID();
	WORD wOutViewChairID=SwitchViewChairID(pOutCard->wOutCardUser);

	//设置变量
	m_wCurrentUser=INVALID_CHAIR;
	m_wOutCardUser=pOutCard->wOutCardUser;
	m_cbOutCardData=pOutCard->cbOutCardData;

	//其他用户
	if ((IsLookonMode()==true)||(pOutCard->wOutCardUser!=wMeChairID))
	{
		//环境设置
		KillGameClock(IDI_OPERATE_CARD); 

		//播放声音
		PlayCardSound(pOutCard->wOutCardUser,pOutCard->cbOutCardData);

		//出牌界面
		m_GameClientView.SetUserAction(INVALID_CHAIR,0);
		m_GameClientView.SetOutCardInfo(wOutViewChairID,pOutCard->cbOutCardData);

		//设置扑克
		if (wOutViewChairID==2)
		{
			//删除扑克
			BYTE cbCardData[MAX_COUNT];
			m_GameLogic.RemoveCard(m_cbCardIndex,pOutCard->cbOutCardData);

			//设置扑克
			BYTE cbCardCount=m_GameLogic.SwitchToCardData(m_cbCardIndex,cbCardData);
			m_GameClientView.m_HandCardControl.SetCardData(cbCardData,cbCardCount,0);
		}
		else
		{
			WORD wUserIndex=(wOutViewChairID>2)?2:wOutViewChairID;
			m_GameClientView.m_UserCard[wUserIndex].SetCurrentCard(false);
		}
	}

	return true;
}

//发牌消息
bool CGameClientEngine::OnSubSendCard(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	ASSERT(wDataSize==sizeof(CMD_S_SendCard));
	if (wDataSize!=sizeof(CMD_S_SendCard)) return false;

	//变量定义
	CMD_S_SendCard * pSendCard=(CMD_S_SendCard *)pBuffer;

	//设置变量
	WORD wMeChairID=GetMeChairID();
	m_wCurrentUser=pSendCard->wCurrentUser;

	//隐藏控件
	m_GameClientView.m_ControlWnd.ShowWindow( SW_HIDE );
	//删除定时器
	KillGameClock( IDI_OPERATE_CARD );

	//丢弃扑克
	if ((m_wOutCardUser!=INVALID_CHAIR)&&(m_cbOutCardData!=0))
	{
		//丢弃扑克
		WORD wOutViewChairID=SwitchViewChairID(m_wOutCardUser);
		m_GameClientView.m_DiscardCard[wOutViewChairID].AddCardItem(m_cbOutCardData);
		m_GameClientView.SetDiscUser(wOutViewChairID);

		//设置变量
		m_cbOutCardData=0;
		m_wOutCardUser=INVALID_CHAIR;
	}

	//发牌处理
	if (pSendCard->cbCardData!=0)
	{
		//取牌界面
		WORD wViewChairID=SwitchViewChairID(m_wCurrentUser);
		if (wViewChairID!=2)
		{
			WORD wUserIndex=(wViewChairID>2)?2:wViewChairID;
			m_GameClientView.m_UserCard[wUserIndex].SetCurrentCard(true);
		}
		else
		{
			m_cbCardIndex[m_GameLogic.SwitchToCardIndex(pSendCard->cbCardData)]++;
			m_GameClientView.m_HandCardControl.SetCurrentCard(pSendCard->cbCardData);
		}

		//扣除扑克
		DeductionTableCard(pSendCard->bTail==false);
	}

	//当前用户
	if ((IsLookonMode()==false)&&(m_wCurrentUser==wMeChairID))
	{
		//激活框架
		ActiveGameFrame();

		//动作处理
		if (pSendCard->cbActionMask!=WIK_NULL)
		{
			//获取变量
			BYTE cbActionCard=pSendCard->cbCardData;
			BYTE cbActionMask=pSendCard->cbActionMask;

			m_cbUserAction = pSendCard->cbActionMask;

			//变量定义
			tagGangCardResult GangCardResult;
			ZeroMemory(&GangCardResult,sizeof(GangCardResult));

			//杠牌判断
			if ((cbActionMask&WIK_GANG)!=0)
			{
				WORD wMeChairID=GetMeChairID();
				m_GameLogic.AnalyseGangCard(m_cbCardIndex,m_WeaveItemArray[wMeChairID],m_cbWeaveCount[wMeChairID],GangCardResult);
			}

			//设置界面
			m_GameClientView.m_ControlWnd.SetControlInfo(cbActionCard,cbActionMask,GangCardResult);
		}
	}

	//出牌提示
	m_GameClientView.SetStatusFlag((IsLookonMode()==false)&&(m_wCurrentUser==wMeChairID),false);

	//更新界面
	m_GameClientView.InvalidGameView(0,0,0,0);

	//计算时间
	WORD wTimeCount=TIME_OPERATE_CARD;

	//设置时间
	m_GameClientView.SetCurrentUser(SwitchViewChairID(m_wCurrentUser));
	SetGameClock(m_wCurrentUser,IDI_OPERATE_CARD,wTimeCount);

	return true;
}

//操作提示
bool CGameClientEngine::OnSubOperateNotify(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	ASSERT(wDataSize==sizeof(CMD_S_OperateNotify));
	if (wDataSize!=sizeof(CMD_S_OperateNotify)) 
		return false;

	//变量定义
	CMD_S_OperateNotify * pOperateNotify=(CMD_S_OperateNotify *)pBuffer;

	//用户界面
	if ((IsLookonMode()==false)&&(pOperateNotify->cbActionMask!=WIK_NULL))
	{
		//获取变量
		WORD wMeChairID=GetMeChairID();
		BYTE cbActionMask=pOperateNotify->cbActionMask;
		BYTE cbActionCard=pOperateNotify->cbActionCard;

		m_cbUserAction = pOperateNotify->cbActionMask;

		//变量定义
		tagGangCardResult GangCardResult;
		ZeroMemory(&GangCardResult,sizeof(GangCardResult));

		//杠牌判断
		if ((cbActionMask&WIK_GANG)!=0)
		{
			//桌面杆牌
			if ((m_wCurrentUser==INVALID_CHAIR)&&(cbActionCard!=0))
			{
				GangCardResult.cbCardCount=1;
				GangCardResult.cbCardData[0]=cbActionCard;
			}

			//自己杆牌
			if ((m_wCurrentUser==wMeChairID)||(cbActionCard==0))
			{
				WORD wMeChairID=GetMeChairID();
				m_GameLogic.AnalyseGangCard(m_cbCardIndex,m_WeaveItemArray[wMeChairID],m_cbWeaveCount[wMeChairID],GangCardResult);
			}
		}

		//设置界面
		ActiveGameFrame();
		m_GameClientView.m_ControlWnd.SetControlInfo(cbActionCard,cbActionMask,GangCardResult);

		//设置时间
		m_GameClientView.SetCurrentUser(INVALID_CHAIR);
		SetGameClock(GetMeChairID(),IDI_OPERATE_CARD,TIME_OPERATE_CARD);
	}

	return true;
}

//操作结果
bool CGameClientEngine::OnSubOperateResult(const void * pBuffer, WORD wDataSize)
{
	//效验消息
	ASSERT(wDataSize==sizeof(CMD_S_OperateResult));
	if (wDataSize!=sizeof(CMD_S_OperateResult)) 
		return false;

	//消息处理
	CMD_S_OperateResult * pOperateResult=(CMD_S_OperateResult *)pBuffer;

	//变量定义
	BYTE cbPublicCard=TRUE;
	WORD wOperateUser=pOperateResult->wOperateUser;
	BYTE cbOperateCard=pOperateResult->cbOperateCard;
	WORD wOperateViewID=SwitchViewChairID(wOperateUser);
	WORD wProviderViewID = SwitchViewChairID(pOperateResult->wProvideUser);

	//出牌变量
	if (pOperateResult->cbOperateCode!=WIK_NULL)
	{
		m_cbOutCardData=0;
		m_wOutCardUser=INVALID_CHAIR;
	}

	//设置变量
	m_wCurrentUser=pOperateResult->wOperateUser;

	//设置组合
	if ((pOperateResult->cbOperateCode&WIK_GANG)!=0)
	{
		m_wCurrentUser = INVALID_CHAIR;

		//暗杠判断
		cbPublicCard=(pOperateResult->wProvideUser==wOperateUser)?FALSE:TRUE;

		//组合扑克
		BYTE cbWeaveIndex=0xFF;
		for (BYTE i=0;i<m_cbWeaveCount[wOperateUser];i++)
		{
			BYTE cbWeaveKind=m_WeaveItemArray[wOperateUser][i].cbWeaveKind;
			BYTE cbCenterCard=m_WeaveItemArray[wOperateUser][i].cbCenterCard;
			if ((cbCenterCard==cbOperateCard)&&(cbWeaveKind==WIK_PENG))
			{
				cbWeaveIndex=i;
				m_WeaveItemArray[wOperateUser][cbWeaveIndex].cbPublicCard=TRUE;
				m_WeaveItemArray[wOperateUser][cbWeaveIndex].cbWeaveKind=pOperateResult->cbOperateCode;
				m_WeaveItemArray[wOperateUser][cbWeaveIndex].wProvideUser=pOperateResult->wProvideUser;
				break;
			}
		}

		//组合扑克
		if (cbWeaveIndex==0xFF)
		{
			//设置扑克
			cbWeaveIndex=m_cbWeaveCount[wOperateUser]++;
			m_WeaveItemArray[wOperateUser][cbWeaveIndex].cbPublicCard=cbPublicCard;
			m_WeaveItemArray[wOperateUser][cbWeaveIndex].cbCenterCard=cbOperateCard;
			m_WeaveItemArray[wOperateUser][cbWeaveIndex].cbWeaveKind=pOperateResult->cbOperateCode;
			m_WeaveItemArray[wOperateUser][cbWeaveIndex].wProvideUser=pOperateResult->wProvideUser;
		}

		//组合界面
		BYTE cbWeaveCard[4]={0,0,0,0},cbWeaveKind=pOperateResult->cbOperateCode;
		BYTE cbWeaveCardCount=m_GameLogic.GetWeaveCard(cbWeaveKind,cbOperateCard,cbWeaveCard);
		m_GameClientView.m_WeaveCard[wOperateViewID][cbWeaveIndex].SetCardData(cbWeaveCard,cbWeaveCardCount);
		m_GameClientView.m_WeaveCard[wOperateViewID][cbWeaveIndex].SetDisplayItem((m_WeaveItemArray[wOperateUser][cbWeaveIndex].cbPublicCard==TRUE)?true:false);
		if (m_WeaveItemArray[wOperateUser][cbWeaveIndex].cbPublicCard==FALSE) m_GameClientView.m_WeaveCard[2][cbWeaveIndex].SetGameEnd(true);

		//扑克设置
		if (GetMeChairID()==wOperateUser)
		{
			m_cbCardIndex[m_GameLogic.SwitchToCardIndex(pOperateResult->cbOperateCard)]=0;
		}

		//设置扑克
		if (GetMeChairID()==wOperateUser)
		{
			BYTE cbCardData[MAX_COUNT];
			BYTE cbCardCount=m_GameLogic.SwitchToCardData(m_cbCardIndex,cbCardData);
			m_GameClientView.m_HandCardControl.SetCardData(cbCardData,cbCardCount,0);
		}
		else
		{
			WORD wUserIndex=(wOperateViewID>=3)?2:wOperateViewID;
			BYTE cbCardCount=MAX_COUNT-m_cbWeaveCount[wOperateUser]*3;
			m_GameClientView.m_UserCard[wUserIndex].SetCardData(cbCardCount-1,false);
		}
	}
	else if (pOperateResult->cbOperateCode!=WIK_NULL)
	{
		//设置组合
		BYTE cbWeaveIndex=m_cbWeaveCount[wOperateUser]++;
		m_WeaveItemArray[wOperateUser][cbWeaveIndex].cbPublicCard=TRUE;
		m_WeaveItemArray[wOperateUser][cbWeaveIndex].cbCenterCard=cbOperateCard;
		m_WeaveItemArray[wOperateUser][cbWeaveIndex].cbWeaveKind=pOperateResult->cbOperateCode;
		m_WeaveItemArray[wOperateUser][cbWeaveIndex].wProvideUser=pOperateResult->wProvideUser;

		//组合界面
		BYTE cbWeaveCard[4]={0,0,0,0},cbWeaveKind=pOperateResult->cbOperateCode;
		BYTE cbWeaveCardCount=m_GameLogic.GetWeaveCard(cbWeaveKind,cbOperateCard,cbWeaveCard);
		m_GameClientView.m_WeaveCard[wOperateViewID][cbWeaveIndex].SetCardData(cbWeaveCard,cbWeaveCardCount);
		m_GameClientView.m_WeaveCard[wOperateViewID][cbWeaveIndex].SetDirectionCardPos(3-(wOperateViewID-wProviderViewID+4)%4);

		//删除扑克
		if (GetMeChairID()==wOperateUser)
		{
			VERIFY( m_GameLogic.RemoveCard(cbWeaveCard,cbWeaveCardCount,&cbOperateCard,1) );
			VERIFY( m_GameLogic.RemoveCard(m_cbCardIndex,cbWeaveCard,cbWeaveCardCount-1) );
		}

		//设置扑克
		if (GetMeChairID()==wOperateUser)
		{
			BYTE cbCardData[MAX_COUNT];
			BYTE cbCardCount=m_GameLogic.SwitchToCardData(m_cbCardIndex,cbCardData);
			m_GameClientView.m_HandCardControl.SetCardData(cbCardData,cbCardCount-1,cbCardData[cbCardCount-1]);
		}
		else
		{
			WORD wUserIndex=(wOperateViewID>=3)?2:wOperateViewID;
			BYTE cbCardCount=MAX_COUNT-m_cbWeaveCount[wOperateUser]*3;
			m_GameClientView.m_UserCard[wUserIndex].SetCardData(cbCardCount-1,true);
		}
	}

	//设置界面
	m_GameClientView.SetOutCardInfo(INVALID_CHAIR,0);
	m_GameClientView.m_ControlWnd.ShowWindow(SW_HIDE);
	m_GameClientView.SetUserAction(wOperateViewID,pOperateResult->cbOperateCode);
	m_GameClientView.SetStatusFlag((IsLookonMode()==false)&&(m_wCurrentUser==GetMeChairID()),false);

	//更新界面
	m_GameClientView.InvalidGameView(0,0,0,0);

	//环境设置
	PlayActionSound(wOperateUser,pOperateResult->cbOperateCode);

	//设置时间
	if (m_wCurrentUser!=INVALID_CHAIR)
	{
		//计算时间
		WORD wTimeCount=TIME_OPERATE_CARD;

		//设置时间
		m_GameClientView.SetCurrentUser(SwitchViewChairID(m_wCurrentUser));
		SetGameClock(m_wCurrentUser,IDI_OPERATE_CARD,wTimeCount);
	}

	return true;
}

//游戏结束
bool CGameClientEngine::OnSubGameEnd(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	ASSERT(wDataSize==sizeof(CMD_S_GameEnd));
	if (wDataSize!=sizeof(CMD_S_GameEnd)) return false;

	//消息处理
	CMD_S_GameEnd * pGameEnd=(CMD_S_GameEnd *)pBuffer;

	//设置状态
	SetGameStatus(GS_MJ_FREE);
	m_GameClientView.SetStatusFlag(false,false);

	//删除定时器
	KillGameClock(IDI_OPERATE_CARD);

	//设置控件
	m_GameClientView.ShowSelectTip(false);
	m_GameClientView.ShowWaitChooseTip(false);
	m_GameClientView.SetStatusFlag(false,false);
	m_GameClientView.m_ControlWnd.ShowWindow(SW_HIDE);
	m_GameClientView.m_HandCardControl.SetPositively(false);

	//设置扑克
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		m_GameClientView.m_WeaveCard[i][0].SetGameEnd(true);
		m_GameClientView.m_WeaveCard[i][1].SetGameEnd(true);
		m_GameClientView.m_WeaveCard[i][2].SetGameEnd(true);
		m_GameClientView.m_WeaveCard[i][3].SetGameEnd(true);
	}
	
	//变量定义
	tagScoreInfo ScoreInfo;
	ZeroMemory(&ScoreInfo,sizeof(ScoreInfo));

	//设置积分
	CString strTemp ,strEnd;
	if( pGameEnd->wLeftUser != INVALID_CHAIR )
	{
		strEnd.AppendFormat( TEXT("玩家[]逃跑，将收回刮风下雨所得，并垫付其它玩家的刮风下雨，及未胡牌者花猪。"),
			m_szAccounts[pGameEnd->wLeftUser] );
		if ( m_pIStringMessage )
			m_pIStringMessage->InsertSystemString( strEnd );
	}
	strEnd = TEXT("本局结束,成绩统计：\r\n");
	for( WORD i = 0; i < GAME_PLAYER; i++ )
	{
		if( m_szAccounts[i][0] == 0 ) continue;

		lstrcpy( ScoreInfo.szUserName[SwitchViewChairID(i)],m_szAccounts[i] );
		strTemp.Format(TEXT("%s： %I64d\n"),m_szAccounts[i],pGameEnd->lGameScore[i]);
		strEnd += strTemp;
	}
	//消息积分
	if ( m_pIStringMessage )
		m_pIStringMessage->InsertSystemString((LPCTSTR)strEnd);

	//成绩变量
	for( WORD i = 0; i < GAME_PLAYER; i++ )
	{
		WORD wViewId = SwitchViewChairID(i);
		ScoreInfo.wMeChairId = wViewId;

		WORD j = 0;
		for( j = 0; j < CountArray(pGameEnd->wWinOrder); j++ )
		{
			if(pGameEnd->wWinOrder[j] == i)
			{
				ScoreInfo.cbWinOrder = (BYTE)j+1;
				break;
			}
		}
		if (j == CountArray(pGameEnd->wWinOrder)) ScoreInfo.cbWinOrder = 0;
		
		ScoreInfo.lGameTax = pGameEnd->lGameTax[i];
		ScoreInfo.cbGenCount = pGameEnd->cbGenCount[i];
		ScoreInfo.lGameScore = pGameEnd->lGameScore[i];
		ScoreInfo.lGangScore = pGameEnd->lGangScore[i];
		ScoreInfo.wProvideUser = SwitchViewChairID(pGameEnd->wProvideUser[i]);
		CopyMemory(ScoreInfo.dwChiHuRight, &pGameEnd->dwChiHuRight[i], sizeof(DWORD)*MAX_RIGHT_COUNT);
		ZeroMemory(ScoreInfo.wLostFanShu, sizeof(ScoreInfo.wLostFanShu));
		for (WORD j = 0; j < GAME_PLAYER; j++)
		{
			if (j == i) continue;
			ScoreInfo.wLostFanShu[SwitchViewChairID(j)] = pGameEnd->wLostFanShu[i][j];
			ScoreInfo.lHuaZhuScore[SwitchViewChairID(j)] = pGameEnd->lHuaZhuScore[i][j];
			ScoreInfo.lChaJiaoScore[SwitchViewChairID(j)] = pGameEnd->lChaJiaoScore[i][j];
			ScoreInfo.lLostHuaZhuScore[SwitchViewChairID(j)] = pGameEnd->lLostHuaZhuScore[i][j];
			ScoreInfo.lLostChaJiaoScore[SwitchViewChairID(j)] = pGameEnd->lLostChaJiaoScore[i][j];
		}

		m_GameClientView.m_ScoreControl[wViewId].SetScoreInfo(ScoreInfo);
	}

	//显示成绩
	m_GameClientView.ShowGameScoreInfo();

	//用户扑克
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		WORD wViewChairID=SwitchViewChairID(i);
		m_GameClientView.m_TableCard[wViewChairID].SetCardData(pGameEnd->cbCardData[i],pGameEnd->cbCardCount[i]);
	}

	//历史成绩
	for (WORD i=0; i<GAME_PLAYER; i++)
	{
		tagHistoryScore * pHistoryScore=m_HistoryScore.GetHistoryScore(i);
		m_HistoryScore.OnEventUserScore(i,pGameEnd->lGameScore[i]);
	}

	//设置扑克
	m_GameClientView.m_UserCard[0].SetCardData(0,false);
	m_GameClientView.m_UserCard[1].SetCardData(0,false);
	m_GameClientView.m_UserCard[2].SetCardData(0,false);
	m_GameClientView.m_HandCardControl.SetCardData(NULL,0,0);

	//播放声音
	LONGLONG lScore=pGameEnd->lGameScore[GetMeChairID()];
	if (lScore>0L)
	{
		PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_WIN"));
	}
	else if (lScore<0L)
	{
		PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_LOST"));
	}
	else 
	{
		PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_END"));
	}

	//设置界面
	if (IsLookonMode()==false)
	{
		m_GameClientView.m_btStart.ShowWindow(TRUE);
		m_GameClientView.SetCurrentUser(INVALID_CHAIR);
		SetGameClock(GetMeChairID(),IDI_START_GAME,TIME_START_GAME);
	}

	//隐藏按钮
	m_GameClientView.m_btCallCard1.ShowWindow(false);
	m_GameClientView.m_btCallCard2.ShowWindow(false);
	m_GameClientView.m_btCallCard3.ShowWindow(false);

	//取消托管
	if(m_bStustee)
		OnStusteeControl(0,0);

	//更新界面
	m_GameClientView.InvalidGameView(0,0,0,0);

	return true;
}

//用户托管
bool CGameClientEngine::OnSubTrustee(const void * pBuffer,WORD wDataSize)
{
	//效验数据
	ASSERT(wDataSize==sizeof(CMD_S_Trustee));
	if (wDataSize!=sizeof(CMD_S_Trustee)) return false;

	//消息处理
	CMD_S_Trustee * pTrustee=(CMD_S_Trustee *)pBuffer;
	m_GameClientView.SetTrustee(SwitchViewChairID(pTrustee->wChairID),pTrustee->bTrustee);
	if ((IsLookonMode()==true)||(pTrustee->wChairID!=GetMeChairID()))
	{
		IClientUserItem * pUserData = GetTableUserItem(pTrustee->wChairID);
		TCHAR szBuffer[256];
		if(pTrustee->bTrustee==true)
			_sntprintf(szBuffer,sizeof(szBuffer),TEXT("玩家[%s]选择了托管功能。"),pUserData->GetNickName());
		else
			_sntprintf(szBuffer,sizeof(szBuffer),TEXT("玩家[%s]取消了托管功能。"),pUserData->GetNickName());
		if ( m_pIStringMessage )
			m_pIStringMessage->InsertSystemString(szBuffer);
	}

	return true;
}

//胡牌消息
bool CGameClientEngine::OnSubUserChiHu( const void *pBuffer,WORD wDataSize )
{
	//效验消息
	ASSERT(wDataSize==sizeof(CMD_S_ChiHu));
	if (wDataSize!=sizeof(CMD_S_ChiHu)) return false;

	//消息处理
	CMD_S_ChiHu * pChiHu=(CMD_S_ChiHu *)pBuffer;

	WORD wViewId = SwitchViewChairID( pChiHu->wChiHuUser );

	//隐藏控件
	m_GameClientView.m_ControlWnd.ShowWindow( SW_HIDE );
	//删除定时器
	KillGameClock( IDI_OPERATE_CARD );

	BYTE cbCardData[MAX_COUNT];
	ZeroMemory( cbCardData,sizeof(cbCardData) );
	cbCardData[pChiHu->cbCardCount-1] = pChiHu->cbChiHuCard;

	//设置扑克,名次
	if( wViewId == 2 )
	{
		m_GameClientView.m_HandCardControl.SetPositively( false );
		m_GameClientView.m_HandCardControl.SetCardData( NULL,0,0 );
		m_GameClientView.m_TableCard[2].SetCardData( cbCardData,pChiHu->cbCardCount );
	}
	else
	{
		WORD wUserIndex=(wViewId>2)?2:wViewId;
		m_GameClientView.m_UserCard[wUserIndex].SetCardData( 0,false );
		
		m_GameClientView.m_TableCard[wViewId].SetCardData( cbCardData,pChiHu->cbCardCount );
	}
	m_GameClientView.SetWinOrder(wViewId, pChiHu->cbWinOrder);

	////提示消息
	//TCHAR szBuffer[128] = TEXT("");
	//_sntprintf( szBuffer,CountArray(szBuffer),TEXT("\n玩家[%s]胡牌,得分:%+I64d\n"),m_szAccounts[pChiHu->wChiHuUser],pChiHu->lGameScore );
	//InsertSystemString( szBuffer );

	//设置界面
	m_GameClientView.SetOutCardInfo(INVALID_CHAIR,0);
	BYTE cbAction = WIK_CHI_HU;
	if (pChiHu->wChiHuUser == pChiHu->wProviderUser) cbAction |= WIK_ZI_MO;
	m_GameClientView.SetUserAction(wViewId,cbAction);
	m_GameClientView.SetStatusFlag(false,false);

	//更新界面
	m_GameClientView.InvalidGameView(0,0,0,0);

	//环境设置
	PlayActionSound(pChiHu->wChiHuUser,WIK_CHI_HU);

	return true;
}

//播放出牌声音
void CGameClientEngine::PlayCardSound(WORD wChairID, BYTE cbCardData)
{
	if(m_GameLogic.IsValidCard(cbCardData) == false)
	{
		return;
	}
	if(wChairID < 0 || wChairID > 3)
	{
		return;
	}

	//判断性别
	IClientUserItem * pUserData = GetTableUserItem(wChairID);
	if(pUserData == 0)
	{
		assert(0 && "得不到玩家信息");
		return;
	}
	bool bBoy = (pUserData->GetGender() == GENDER_FEMALE ? false : true);
	BYTE cbType= (cbCardData & MASK_COLOR);
	BYTE cbValue= (cbCardData & MASK_VALUE);
	CString strSoundName;
	switch(cbType)
	{
	case 0X30:	//风
		{
			switch(cbValue) 
			{
			case 1:
				{
					strSoundName = _T("F_1");
					break;
				}
			case 2:
				{
					strSoundName = _T("F_2");
					break;
				}
			case 3:
				{
					strSoundName = _T("F_3");
					break;
				}
			case 4:
				{
					strSoundName = _T("F_4");
					break;
				}
			case 5:
				{
					strSoundName = _T("F_5");
					break;
				}
			case 6:
				{
					strSoundName = _T("F_6");
					break;
				}
			case 7:
				{
					strSoundName = _T("F_7");
					break;
				}
			default:
				{
					strSoundName=_T("BU_HUA");
				}

			}
			break;
		}		
	case 0X20:	//筒
		{
			strSoundName.Format(_T("T_%d"), cbValue);
			break;
		}

	case 0X10:	//索
		{
			strSoundName.Format(_T("S_%d"), cbValue);
			break;
		}
	case 0X00:	//万
		{
			strSoundName.Format(_T("W_%d"), cbValue);
			break;
		}
	}	

	if(bBoy)
	{
		strSoundName = _T("BOY_") +strSoundName;
	}
	else
	{
		strSoundName = _T("GIRL_") + strSoundName;
	}
	m_DirectSound.Play();
	PlayGameSound(AfxGetInstanceHandle(), strSoundName);
}

//播放声音
void CGameClientEngine::PlayActionSound(WORD wChairID,BYTE cbAction)
{
	//判断性别
	IClientUserItem * pUserData = GetTableUserItem(wChairID);
	if(pUserData == 0)
	{
		assert(0 && "得不到玩家信息");
		return;
	}
	if(wChairID < 0 || wChairID > 3)
	{
		return;
	}
	bool bBoy = (pUserData->GetGender() == GENDER_FEMALE ? false : true);

	switch (cbAction)
	{
	case WIK_NULL:		//取消
		{
			if(bBoy)
				PlayGameSound(AfxGetInstanceHandle(),TEXT("BOY_OUT_CARD"));
			else
				PlayGameSound(AfxGetInstanceHandle(),TEXT("GIRL_OUT_CARD"));
			break;
		}
	case WIK_LEFT:
	case WIK_CENTER:
	case WIK_RIGHT:		//上牌
		{
			if(bBoy)
				PlayGameSound(AfxGetInstanceHandle(),TEXT("BOY_CHI"));
			else
				PlayGameSound(AfxGetInstanceHandle(),TEXT("GIRL_CHI"));		
			break;
		}
	case WIK_PENG:		//碰牌
		{
			if(bBoy)
				PlayGameSound(AfxGetInstanceHandle(),TEXT("BOY_PENG"));
			else
				PlayGameSound(AfxGetInstanceHandle(),TEXT("GIRL_PENG"));	
			break;
		}
	case WIK_GANG:		//杠牌
		{
			if(bBoy)
				PlayGameSound(AfxGetInstanceHandle(),TEXT("BOY_GANG"));
			else
				PlayGameSound(AfxGetInstanceHandle(),TEXT("GIRL_GANG"));		
			break;
		}
	case WIK_CHI_HU:	//吃胡
		{

			if(bBoy)
				PlayGameSound(AfxGetInstanceHandle(),TEXT("BOY_CHI_HU"));
			else
				PlayGameSound(AfxGetInstanceHandle(),TEXT("GIRL_CHI_HU"));
			break;
		}
	}

	return;
}
//出牌判断
bool CGameClientEngine::VerdictOutCard(BYTE cbCardData)
{
	return true;
}

//扣除扑克
void CGameClientEngine::DeductionTableCard(bool bHeadCard)
{
	if (bHeadCard==true)
	{
		//切换索引
		BYTE cbHeapCount=m_cbHeapCardInfo[m_wHeapHand][0]+m_cbHeapCardInfo[m_wHeapHand][1];
		if (cbHeapCount==HEAP_FULL_COUNT)
			m_wHeapHand=(m_wHeapHand+1)%CountArray(m_cbHeapCardInfo);

		//减少扑克
		m_cbLeftCardCount--;
		m_cbHeapCardInfo[m_wHeapHand][0]++;

		//堆立扑克
		WORD wHeapViewID=SwitchViewChairID(m_wHeapHand);
		WORD wMinusHeadCount=m_cbHeapCardInfo[m_wHeapHand][0];
		WORD wMinusLastCount=m_cbHeapCardInfo[m_wHeapHand][1];
		m_GameClientView.m_HeapCard[wHeapViewID].SetCardData(wMinusHeadCount,wMinusLastCount,HEAP_FULL_COUNT);
	}
	else
	{
		//切换索引
		BYTE cbHeapCount=m_cbHeapCardInfo[m_wHeapTail][0]+m_cbHeapCardInfo[m_wHeapTail][1];
		if (cbHeapCount==HEAP_FULL_COUNT)
			m_wHeapTail=(m_wHeapTail+3)%CountArray(m_cbHeapCardInfo);

		//减少扑克
		m_cbLeftCardCount--;
		m_cbHeapCardInfo[m_wHeapTail][1]++;

		//堆立扑克
		WORD wHeapViewID=SwitchViewChairID(m_wHeapTail);
		WORD wMinusHeadCount=m_cbHeapCardInfo[m_wHeapTail][0];
		WORD wMinusLastCount=m_cbHeapCardInfo[m_wHeapTail][1];
		m_GameClientView.m_HeapCard[wHeapViewID].SetCardData(wMinusHeadCount,wMinusLastCount,HEAP_FULL_COUNT);
	}

	return;
}

//显示控制
bool CGameClientEngine::ShowOperateControl(BYTE cbUserAction, BYTE cbActionCard)
{
	//变量定义
	tagGangCardResult GangCardResult;
	ZeroMemory(&GangCardResult,sizeof(GangCardResult));

	//杠牌判断
	if ((cbUserAction&WIK_GANG)!=0)
	{
		//桌面杆牌
		if (cbActionCard!=0)
		{
			GangCardResult.cbCardCount=1;
			GangCardResult.cbCardData[0]=cbActionCard;
		}

		//自己杆牌
		if (cbActionCard==0)
		{
			WORD wMeChairID=GetMeChairID();
			m_GameLogic.AnalyseGangCard(m_cbCardIndex,m_WeaveItemArray[wMeChairID],m_cbWeaveCount[wMeChairID],GangCardResult);
		}
	}

	//显示界面
	if (IsLookonMode()==false)
		m_GameClientView.m_ControlWnd.SetControlInfo(cbActionCard,cbUserAction,GangCardResult);

	return true;
}

//开始按钮
LRESULT CGameClientEngine::OnStart(WPARAM wParam, LPARAM lParam)
{
	////TEST+
	//BYTE cbTempDebug[56] = 
	//{
	//	0x04,0x04,0x04,0x05,0x06,0x07,0x09,0x01,0x02,0x03,0x01,0x02,0x03,
	//	0x04,0x04,0x04,0x05,0x06,0x07,0x09,0x01,0x02,0x03,0x01,0x02,0x03,
	//	0x04,0x04,0x04,0x05,0x06,0x07,0x09,0x01,0x02,0x03,0x01,0x02,0x03,
	//	0x04,0x04,0x04,0x05,0x06,0x07,0x09,0x01,0x02,0x03,0x01,0x02,0x03,
	//};

	//m_GameClientView.m_UserCard[0].SetCardData(10,true);
	//m_GameClientView.m_UserCard[1].SetCardData(10,true);
	//m_GameClientView.m_UserCard[2].SetCardData(10,true);
	//m_GameClientView.m_HandCardControl.SetDisplayItem(true);
	//m_GameClientView.m_HandCardControl.SetPositively(true);
	//m_GameClientView.m_HandCardControl.SetCardData(cbTempDebug, 10, 0x01);

	//m_GameClientView.m_btCallCard1.ShowWindow(true);
	//m_GameClientView.m_btCallCard2.ShowWindow(true);
	//m_GameClientView.m_btCallCard3.ShowWindow(true);

	//WORD wMeChairID = 2;
	//BYTE cbCardWeave[4] = {0x01,0x01,0x01,0x01};

	////扑克设置
	//for (WORD i=0;i<GAME_PLAYER;i++)
	//{
	//	//m_GameClientView.m_TableCard[i].SetCardData(cbTempDebug,10);
	//	//m_GameClientView.m_DiscardCard[i].SetCardData(cbTempDebug,20);
	//	m_GameClientView.m_WeaveCard[i][0].SetCardData(cbCardWeave,4);
	//	m_GameClientView.m_WeaveCard[i][1].SetCardData(NULL,0);
	//	m_GameClientView.m_WeaveCard[i][2].SetCardData(NULL,0);
	//	m_GameClientView.m_WeaveCard[i][3].SetCardData(NULL,0);
	//	m_GameClientView.m_WeaveCard[i][0].SetGameEnd(i==wMeChairID);
	//	m_GameClientView.m_WeaveCard[i][1].SetGameEnd(i==wMeChairID);
	//	m_GameClientView.m_WeaveCard[i][2].SetGameEnd(i==wMeChairID);
	//	m_GameClientView.m_WeaveCard[i][3].SetGameEnd(i==wMeChairID);
	//	m_GameClientView.m_WeaveCard[i][0].SetDisplayItem(false);
	//	m_GameClientView.m_WeaveCard[i][1].SetDisplayItem(true);
	//	m_GameClientView.m_WeaveCard[i][2].SetDisplayItem(true);
	//	m_GameClientView.m_WeaveCard[i][3].SetDisplayItem(true);
	//}

	////堆立扑克
	//for (WORD i=0;i<GAME_PLAYER;i++)
	//{
	//	m_cbHeapCardInfo[i][0]=0;
	//	m_cbHeapCardInfo[i][1]=0;
	//	m_GameClientView.m_HeapCard[i].SetCardData(m_cbHeapCardInfo[i][0],m_cbHeapCardInfo[i][1],HEAP_FULL_COUNT);
	//}

	//////变量定义
	////BYTE cbActionCard = 0x01;
	////BYTE cbActionMask = WIK_PENG;
	////tagGangCardResult GangCardResult;
	////ZeroMemory(&GangCardResult,sizeof(GangCardResult));
	////GangCardResult.cbCardCount=1;
	////GangCardResult.cbCardData[0]=cbActionCard;
	////m_GameClientView.m_ControlWnd.SetControlInfo(cbActionCard,cbActionMask,GangCardResult);
	////m_GameClientView.ShowGameScoreInfo();
	//return 0L;
	////TEST+

	//环境设置
	KillGameClock(IDI_START_GAME);
	m_GameClientView.m_btStart.ShowWindow(SW_HIDE);
	m_GameClientView.m_ControlWnd.ShowWindow(SW_HIDE);

	//设置界面
	m_GameClientView.SetDiscUser(INVALID_CHAIR);
	m_GameClientView.SetStatusFlag(false,false);
	m_GameClientView.SetBankerUser(INVALID_CHAIR);
	m_GameClientView.SetUserAction(INVALID_CHAIR,0);
	m_GameClientView.SetOutCardInfo(INVALID_CHAIR,0);
	m_GameClientView.SetWinOrder(INVALID_CHAIR,0);
	m_GameClientView.SetVoidShowing(INVALID_CHAIR,0);

	//扑克设置
	m_GameClientView.m_UserCard[0].SetCardData(0,false);
	m_GameClientView.m_UserCard[1].SetCardData(0,false);
	m_GameClientView.m_UserCard[2].SetCardData(0,false);
	m_GameClientView.m_HandCardControl.SetCardData(NULL,0,0);

	//成绩窗口
	for (WORD i=0; i<GAME_PLAYER; i++)
	{
		m_GameClientView.m_ScoreControl[i].CloseControl();
	}

	//扑克设置
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		m_GameClientView.m_TableCard[i].SetCardData(NULL,0);
		m_GameClientView.m_DiscardCard[i].SetCardData(NULL,0);
		m_GameClientView.m_WeaveCard[i][0].SetCardData(NULL,0);
		m_GameClientView.m_WeaveCard[i][1].SetCardData(NULL,0);
		m_GameClientView.m_WeaveCard[i][2].SetCardData(NULL,0);
		m_GameClientView.m_WeaveCard[i][3].SetCardData(NULL,0);
		m_GameClientView.m_WeaveCard[i][0].SetGameEnd(false);
		m_GameClientView.m_WeaveCard[i][1].SetGameEnd(false);
		m_GameClientView.m_WeaveCard[i][2].SetGameEnd(false);
		m_GameClientView.m_WeaveCard[i][3].SetGameEnd(false);
		m_GameClientView.m_WeaveCard[i][0].SetDisplayItem(true);
		m_GameClientView.m_WeaveCard[i][1].SetDisplayItem(true);
		m_GameClientView.m_WeaveCard[i][2].SetDisplayItem(true);
		m_GameClientView.m_WeaveCard[i][3].SetDisplayItem(true);
	}

	//堆立扑克
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		m_cbHeapCardInfo[i][0]=0;
		m_cbHeapCardInfo[i][1]=0;
		m_GameClientView.m_HeapCard[SwitchViewChairID(i)].SetCardData(m_cbHeapCardInfo[i][0],m_cbHeapCardInfo[i][1],HEAP_FULL_COUNT);
	}

	//游戏变量
	m_wCurrentUser=INVALID_CHAIR;

	//出牌信息
	m_cbOutCardData=0;
	m_wOutCardUser=INVALID_CHAIR;
	ZeroMemory(m_cbDiscardCard,sizeof(m_cbDiscardCard));
	ZeroMemory(m_cbDiscardCount,sizeof(m_cbDiscardCount));

	//组合扑克
	ZeroMemory(m_cbWeaveCount,sizeof(m_cbWeaveCount));
	ZeroMemory(m_WeaveItemArray,sizeof(m_WeaveItemArray));

	//堆立扑克
	m_wHeapHand=0;
	m_wHeapTail=0;
	ZeroMemory(m_cbHeapCardInfo,sizeof(m_cbHeapCardInfo));

	//扑克变量
	m_cbLeftCardCount=0;
	ZeroMemory(m_cbCardIndex,sizeof(m_cbCardIndex));

	//发送消息
	SendUserReady(NULL,0);

	return 0;
}

//定缺消息
LRESULT CGameClientEngine::OnCallCard(WPARAM wParam, LPARAM lParam)
{
	////删除时间
	//KillGameClock(IDI_CALL_CARD);

	//设置界面
	m_GameClientView.ShowSelectTip(false);
	m_GameClientView.ShowWaitChooseTip(true);

	//设置界面
	m_GameClientView.m_btCallCard1.ShowWindow(false);
	m_GameClientView.m_btCallCard2.ShowWindow(false);
	m_GameClientView.m_btCallCard3.ShowWindow(false);

	//发送数据
	CMD_C_CallCard CallCard;
	CallCard.cbCallCard = (BYTE)wParam;
	SendSocketData(SUB_C_CALL_CARD, &CallCard, sizeof(CallCard));

	return 0;
}

//出牌操作
LRESULT CGameClientEngine::OnOutCard(WPARAM wParam, LPARAM lParam)
{
	//出牌判断
	if ((IsLookonMode()==true)||(m_wCurrentUser!=GetMeChairID()))
		return 0;

	//听牌判断
	if (VerdictOutCard((BYTE)wParam)==false)
	{
		if ( m_pIStringMessage )
			m_pIStringMessage->InsertSystemString(TEXT("出此牌不符合游戏规则!"));
		return 0;
	}

	//设置变量
	m_cbUserAction = 0;
	m_wCurrentUser=INVALID_CHAIR;
	BYTE cbOutCardData=(BYTE)wParam;
	m_GameLogic.RemoveCard(m_cbCardIndex,cbOutCardData);

	//设置扑克
	BYTE cbCardData[MAX_COUNT];
	BYTE cbCardCount=m_GameLogic.SwitchToCardData(m_cbCardIndex,cbCardData);
	m_GameClientView.m_HandCardControl.SetCardData(cbCardData,cbCardCount,0);

	//设置界面
	KillGameClock(IDI_OPERATE_CARD);
	m_GameClientView.InvalidGameView(0,0,0,0);
	m_GameClientView.SetStatusFlag(false,false);
	m_GameClientView.SetUserAction(INVALID_CHAIR,0);
	m_GameClientView.SetOutCardInfo(2,cbOutCardData);
	m_GameClientView.m_ControlWnd.ShowWindow(SW_HIDE);

	//发送数据
	CMD_C_OutCard OutCard;
	OutCard.cbCardData=cbOutCardData;
	SendSocketData(SUB_C_OUT_CARD,&OutCard,sizeof(OutCard));

	//播放声音
	PlayCardSound(GetMeChairID(),cbOutCardData);

	return 0;
}

//扑克操作
LRESULT CGameClientEngine::OnCardOperate(WPARAM wParam, LPARAM lParam)
{
	//变量定义
	BYTE cbOperateCode=(BYTE)(wParam);
	BYTE cbOperateCard=(BYTE)(LOWORD(lParam));

	//状态判断
	if ((m_wCurrentUser==GetMeChairID())&&(cbOperateCode==WIK_NULL))
	{
		m_GameClientView.m_ControlWnd.ShowWindow(SW_HIDE);
		return 0;
	}

	//删除时间
	KillGameClock(IDI_OPERATE_CARD);

	//环境设置
	m_cbUserAction = 0;
	m_GameClientView.SetStatusFlag(false,false);
	m_GameClientView.m_ControlWnd.ShowWindow(SW_HIDE);

	//发送命令
	CMD_C_OperateCard OperateCard;
	OperateCard.cbOperateCode=cbOperateCode;
	OperateCard.cbOperateCard=cbOperateCard;
	SendSocketData(SUB_C_OPERATE_CARD,&OperateCard,sizeof(OperateCard));

	return 0;
}
//拖管控制
LRESULT CGameClientEngine::OnStusteeControl(WPARAM wParam, LPARAM lParam)
{
	//设置变量
	m_wTimeOutCount=0;

	//设置状态
	if (m_bStustee==true)
	{
		m_bStustee=false;
		m_GameClientView.m_btStustee.ShowWindow(true);
		m_GameClientView.m_btStustee.EnableWindow(true);
		m_GameClientView.m_btCancelStustee.ShowWindow(false);

		if ( m_pIStringMessage )m_pIStringMessage->InsertSystemString(_T("您取消了托管功能."));

		CMD_C_Trustee Trustee;
		Trustee.bTrustee = false;
		SendSocketData(SUB_C_TRUSTEE,&Trustee,sizeof(Trustee));
	}
	else
	{
		m_bStustee=true;
		m_GameClientView.m_btStustee.ShowWindow(false);
		m_GameClientView.m_btCancelStustee.ShowWindow(true);
		m_GameClientView.m_btCancelStustee.EnableWindow(true);

		if ( m_pIStringMessage ) m_pIStringMessage->InsertSystemString(_T("您选择了托管功能."));

		CMD_C_Trustee Trustee;
		Trustee.bTrustee = true;
		SendSocketData(SUB_C_TRUSTEE,&Trustee,sizeof(Trustee));
	}

	return 0;
}

//第二次摇色子消息
LRESULT CGameClientEngine::OnSiceTwo(WPARAM wParam, LPARAM lParam)
{
	//环境处理
	PlayGameSound(AfxGetInstanceHandle(),TEXT("DRAW_SICE"));

	//设置界面
	m_GameClientView.SetBankerUser(SwitchViewChairID(m_wBankerUser));

	return 0;
}

//摇色子结束消息
LRESULT CGameClientEngine::OnSiceFinish(WPARAM wParam, LPARAM lParam)
{
	//起始时间
	DWORD dwStartTime = GetTickCount();

	//设置界面
	m_GameClientView.ShowSelectTip(true);
	m_GameClientView.ShowWaitChooseTip(false);
	m_GameClientView.m_DrawSiceWnd.ShowWindow(SW_HIDE);
	bool bPlayerMode = (IsLookonMode()==false);

	//扑克设置
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		//变量定义
		WORD wViewChairID=SwitchViewChairID(i);

		//组合界面
		m_GameClientView.m_WeaveCard[i][0].SetDisplayItem(true);
		m_GameClientView.m_WeaveCard[i][1].SetDisplayItem(true);
		m_GameClientView.m_WeaveCard[i][2].SetDisplayItem(true);
		m_GameClientView.m_WeaveCard[i][3].SetDisplayItem(true);
		m_GameClientView.m_HeapCard[wViewChairID].SetCardData(m_cbHeapCardInfo[i][0],m_cbHeapCardInfo[i][1],HEAP_FULL_COUNT);

		//旁观界面
		if (bPlayerMode==false)
		{
			m_GameClientView.m_TableCard[wViewChairID].SetCardData(NULL,0);
			m_GameClientView.m_DiscardCard[wViewChairID].SetCardData(NULL,0);
			m_GameClientView.m_WeaveCard[wViewChairID][0].SetCardData(NULL,0);
			m_GameClientView.m_WeaveCard[wViewChairID][1].SetCardData(NULL,0);
			m_GameClientView.m_WeaveCard[wViewChairID][2].SetCardData(NULL,0);
			m_GameClientView.m_WeaveCard[wViewChairID][3].SetCardData(NULL,0);
			m_GameClientView.m_HandCardControl.SetDisplayItem(IsAllowLookon());
		}
		
		if( GetTableUserItem(i) == NULL ) continue;

		//用户扑克
		if (wViewChairID != 2)
		{
			WORD wIndex = (wViewChairID>=3) ? 2 : wViewChairID;
			m_GameClientView.m_UserCard[wIndex].SetCardData(MAX_COUNT-1, (i==m_wBankerUser));
		}
		else
		{
			BYTE cbCardData[MAX_COUNT];
			m_GameLogic.SwitchToCardData(m_cbCardIndex,cbCardData);
			BYTE cbBankerCard = (i == m_wBankerUser ? cbCardData[MAX_COUNT-1] : 0);
			m_GameClientView.m_HandCardControl.SetCardData(cbCardData, MAX_COUNT-1, cbBankerCard);
		}
	}

	//显示按钮
	if ((IsLookonMode()==false))
	{
		//控制按钮
		m_GameClientView.m_btCallCard1.EnableWindow(true);
		m_GameClientView.m_btCallCard2.EnableWindow(true);
		m_GameClientView.m_btCallCard3.EnableWindow(true);

		//显示按钮
		m_GameClientView.m_btCallCard1.ShowWindow(true);
		m_GameClientView.m_btCallCard2.ShowWindow(true);
		m_GameClientView.m_btCallCard3.ShowWindow(true);
	}

	//更新界面
	m_GameClientView.InvalidGameView(0,0,0,0);

	//设置时间
	SetGameClock(GetMeChairID(), IDI_CALL_CARD, TIME_CALL_CARD);

	//时间间隔
	DWORD dwEndTime = GetTickCount();
	DWORD dwDiffenTime = dwEndTime - dwStartTime;

	//写入日记
	CString strValue;
	strValue.Format(TEXT("掷骰结束：起始时间=%d；结束时间=%d；时间间隔=%d；"), dwStartTime, dwEndTime, dwDiffenTime);
	NcaTextOut(strValue);

	return 0;
}

//刮风下雨
bool CGameClientEngine::OnSubGangScore( const void *pBuffer, WORD wDataSize )
{
	//效验消息
	ASSERT(wDataSize==sizeof(CMD_S_GangScore));
	if (wDataSize!=sizeof(CMD_S_GangScore)) return false;

	//消息处理
	CMD_S_GangScore * pGangScore=(CMD_S_GangScore *)pBuffer;

	//设置消息
	CString strTemp ,strEnd;
	strEnd.Format( TEXT("\n----------%s----------\n"),pGangScore->cbXiaYu==TRUE?TEXT("下雨"):TEXT("刮风") );
	for( WORD i = 0; i < GAME_PLAYER; i++ )
	{
		strTemp.Format(TEXT("%s: %I64d\n"),m_szAccounts[i],pGangScore->lGangScore[i]);
		strEnd += strTemp;
	}
	strTemp = TEXT("------------------------\n");
	strEnd += strTemp;

	//消息积分
	if ( m_pIStringMessage )
		m_pIStringMessage->InsertSystemString(strEnd);

	//设置界面
	LONGLONG lGangScore[GAME_PLAYER];
	for( WORD i = 0; i < GAME_PLAYER; i++ )
		lGangScore[SwitchViewChairID(i)] = pGangScore->lGangScore[i];
	m_GameClientView.SetGangScore( SwitchViewChairID(pGangScore->wChairId),pGangScore->cbXiaYu,lGangScore );

	return true;
}

//////////////////////////////////////////////////////////////////////////
