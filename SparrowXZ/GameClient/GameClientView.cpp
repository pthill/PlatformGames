#include "StdAfx.h"
#include "Resource.h"
#include "GameClientView.h"
#include "GameClientEngine.h"

//////////////////////////////////////////////////////////////////////////
//按钮标识 

#define IDC_START						100								//开始按钮
#define IDC_TRUSTEE_CONTROL				101								//托管控制
#define IDC_CALL_CARD_1					102								//缺门按钮
#define IDC_CALL_CARD_2					103								//缺门按钮
#define IDC_CALL_CARD_3					104								//缺门按钮
#define IDC_OPEN_HISTORY				105								//积分按钮
#define IDC_CLOSE_HISTROY				106								//积分按钮

//动作标识
#define IDI_BOMB_EFFECT					201								//动作标识
#define IDI_GANG_SOCRE					202								//动作标识

//动作数目
#define BOMB_EFFECT_COUNT				12								//动作数目
#define DISC_EFFECT_COUNT				8								//丢弃效果

#define TIME_GANG_SCORE					30								//动作间隔
#define GANG_MOVE_COUNT					12								//动作数目

#define TIME_CX							65 
#define TIME_CY							69 

#define BORDER_CX						0 
#define BORDER_CY						0 

//成绩窗口
#define IDC_GAME_SCORE					600								//成绩窗口

//按钮标识
#define IDC_MIN							226								//设置按钮标识
#define IDC_CLOSE						227								//设置按钮标识
#define IDC_BANK						214								//银行按钮

//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGameClientView, CGameFrameViewD3D)
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_CALL_CARD_1, OnCallCard1)
	ON_BN_CLICKED(IDC_CALL_CARD_2, OnCallCard2)
	ON_BN_CLICKED(IDC_CALL_CARD_3, OnCallCard3)
	ON_BN_CLICKED(IDC_TRUSTEE_CONTROL,OnStusteeControl)
	ON_BN_CLICKED(IDC_OPEN_HISTORY, OnBnClickedOpenHistory)
	ON_BN_CLICKED(IDC_CLOSE_HISTROY, OnBnClickedCloseHistory)
	ON_WM_TIMER()

	ON_MESSAGE(WM_SET_CAPTION, OnSetCaption)

	//关闭按钮
	ON_BN_CLICKED(IDC_MIN, OnButtonMin)
	ON_BN_CLICKED(IDC_CLOSE, OnButtonClose)

END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////

//构造函数
CGameClientView::CGameClientView()
{
	//标志变量
	m_bOutCard=false;
	m_bWaitOther=false;
	m_bShowScore = false;
	m_bShowSelectTip=false;
	m_bShowWaitChooseTip=false;
	ZeroMemory(m_cbVoidShowing, sizeof(m_cbVoidShowing));
	ZeroMemory( m_lGangScore,sizeof(m_lGangScore) );
	m_wGangChairId = INVALID_CHAIR;

	//游戏属性
	m_lCellScore=0L;
	m_wBankerUser=INVALID_CHAIR;
	m_wCurrentUser=INVALID_CHAIR;
	ZeroMemory( m_cbWinOrder,sizeof(m_cbWinOrder) );

	//动作动画
	m_bBombEffect=false;
	m_cbBombFrameIndex=0;

	//丢弃效果
	m_wDiscUser=INVALID_CHAIR;
	m_cbDiscFrameIndex=0;

	//用户状态
	m_cbCardData=0;
	m_wOutCardUser=INVALID_CHAIR;
	ZeroMemory(m_cbUserAction,sizeof(m_cbUserAction));
	ZeroMemory(m_bTrustee,sizeof(m_bTrustee));

	//成绩信息
	ZeroMemory(&m_pHistoryScore,sizeof(m_pHistoryScore));

	return;
}

//析构函数
CGameClientView::~CGameClientView(void)
{
}

//建立消息
int CGameClientView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct)==-1) return -1;
	
	//标题字体
	m_FontCaption.CreateFont( this, TEXT("宋体"), 12, 400 );
	
	//框架按钮
	CRect rcCreate(0,0,0,0);
	m_btMin.Create(NULL,WS_CHILD|WS_VISIBLE,rcCreate,this,IDC_MIN);
	m_btClose.Create(NULL,WS_CHILD|WS_VISIBLE,rcCreate,this,IDC_CLOSE);
	
	HINSTANCE hResInstance = AfxGetInstanceHandle();
	m_btMin.SetButtonImage( IDB_BT_MIN,hResInstance,false, false );
	m_btClose.SetButtonImage( IDB_BT_CLOSE, hResInstance, false, false);

	//变量定义
	enDirection Direction[]={Direction_North,Direction_East,Direction_South,Direction_West};

	//设置控件
	for (WORD i=0;i<4;i++)
	{
		//用户扑克
		m_HeapCard[i].SetDirection(Direction[i]);
		m_TableCard[i].SetDirection(Direction[i]);
		m_DiscardCard[i].SetDirection(Direction[i]);

		//组合扑克
		m_WeaveCard[i][0].SetDisplayItem(true);
		m_WeaveCard[i][1].SetDisplayItem(true);
		m_WeaveCard[i][2].SetDisplayItem(true);
		m_WeaveCard[i][3].SetDisplayItem(true);
		m_WeaveCard[i][0].SetDirection(Direction[i]);
		m_WeaveCard[i][1].SetDirection(Direction[i]);
		m_WeaveCard[i][2].SetDirection(Direction[i]);
		m_WeaveCard[i][3].SetDirection(Direction[i]);
	}

	//设置控件
	m_UserCard[0].SetDirection(Direction_North);
	m_UserCard[1].SetDirection(Direction_East);
	m_UserCard[2].SetDirection(Direction_West);

	SetTimer(IDI_DISC_EFFECT, 180, NULL);

	return 0;
}

//重置界面
void CGameClientView::ResetGameView()
{
	//标志变量
	m_bOutCard=false;
	m_bWaitOther=false;
	m_bShowScore = false;
	m_bShowSelectTip=false;
	m_bShowWaitChooseTip=false;
	ZeroMemory(m_cbVoidShowing, sizeof(m_cbVoidShowing));
	ZeroMemory(m_lGangScore,sizeof(m_lGangScore));
	m_wGangChairId = INVALID_CHAIR;

	//游戏属性
	m_lCellScore=0L;
	m_wBankerUser=INVALID_CHAIR;
	m_wCurrentUser=INVALID_CHAIR;
	ZeroMemory( m_cbWinOrder,sizeof(m_cbWinOrder) );

	//动作动画
	m_bBombEffect=false;
	m_cbBombFrameIndex=0;

	//丢弃效果
	m_wDiscUser=INVALID_CHAIR;
	m_cbDiscFrameIndex=0;

	//用户状态
	m_cbCardData=0;
	m_wOutCardUser=INVALID_CHAIR;
	ZeroMemory(m_cbUserAction,sizeof(m_cbUserAction));

	//界面设置
	m_btStart.ShowWindow(SW_HIDE);
	m_ControlWnd.ShowWindow(SW_HIDE);
	
	//禁用控件
	m_btStustee.ShowWindow(true);
	m_btStustee.EnableWindow(false);
	m_btCancelStustee.ShowWindow(false);
	m_btOpenHistoryScore.ShowWindow(false);
	m_btCloseHistoryScore.ShowWindow(false);

	//扑克设置
	m_UserCard[0].SetCardData(0,false);
	m_UserCard[1].SetCardData(0,false);
	m_UserCard[2].SetCardData(0,false);
	m_HandCardControl.SetPositively(false);
	m_HandCardControl.SetDisplayItem(false);
	m_HandCardControl.SetCardData(NULL,0,0);

	//扑克设置
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		m_HeapCard[i].SetCardData(0,0,0);
		m_TableCard[i].SetCardData(NULL,0);
		m_DiscardCard[i].SetCardData(NULL,0);
		m_WeaveCard[i][0].SetCardData(NULL,0);
		m_WeaveCard[i][1].SetCardData(NULL,0);
		m_WeaveCard[i][2].SetCardData(NULL,0);
		m_WeaveCard[i][3].SetCardData(NULL,0);
	}

	//成绩信息
	ZeroMemory(&m_pHistoryScore,sizeof(m_pHistoryScore));

	//销毁定时器
	KillTimer(IDI_DISC_EFFECT);
	KillTimer(IDI_BOMB_EFFECT);

	return;
}

//调整控件
void CGameClientView::RectifyControl(int nWidth, int nHeight)
{
	//框架位置
	CSize UserFrameH;
	CSize UserFrameV;
	UserFrameH.SetSize(m_TextureUserFrameH.GetWidth(), m_TextureUserFrameH.GetHeight());
	UserFrameV.SetSize(m_TextureUserFrameV.GetWidth(), m_TextureUserFrameV.GetHeight());

	//用户框架
	m_ptUserFrame[0].SetPoint(nWidth/2-UserFrameH.cx/2, FRAME_HEIGHT);
	m_ptUserFrame[1].SetPoint(5, (nHeight-UserFrameV.cy)/2);
	m_ptUserFrame[2].SetPoint((nWidth-UserFrameH.cx)/2, nHeight-UserFrameH.cy);
	m_ptUserFrame[3].SetPoint(nWidth-UserFrameV.cx-5, (nHeight-UserFrameV.cy)/2);

	//设置坐标
	m_ptAvatar[0].x=nWidth/2-FACE_CX-29; 
	m_ptAvatar[0].y=8;
	m_ptNickName[0].x=m_ptAvatar[0].x+60;
	m_ptNickName[0].y=m_ptAvatar[0].y;
	m_ptClock[0].x=nWidth/2-FACE_CX-TIME_CX-2;
	m_ptClock[0].y=17+BORDER_CY;
	m_ptReady[0].x=nWidth/2;
	m_ptReady[0].y=nHeight/2-130;
	m_UserFlagPos[0].x=nWidth/2-125;
	m_UserFlagPos[0].y=20;
	m_PointTrustee[0].x=nWidth/2-FACE_CX-29;
	m_PointTrustee[0].y=8;
	m_ptWinOrder[0].x=nWidth/2+110;
	m_ptWinOrder[0].y=32;
	m_ptGangScore[0].x = m_ptAvatar[0].x+16;
	m_ptGangScore[0].y = m_ptAvatar[0].y+100;
	m_ptVoidShowPos[0].x = nWidth/2-160;
	m_ptVoidShowPos[0].y = 20;

	m_ptAvatar[1].x=nWidth-FACE_CX-24;
	m_ptAvatar[1].y=nHeight/2-FACE_CY-17;
	m_ptNickName[1].x=m_ptAvatar[1].x;
	m_ptNickName[1].y=m_ptAvatar[1].y+FACE_CY+10;
	m_ptClock[1].x=nWidth-BORDER_CX-FACE_CX+5;
	m_ptClock[1].y=nHeight/2-FACE_CY-27-TIME_CY;
	m_ptReady[1].x=nWidth/2+150;
	m_ptReady[1].y=nHeight/2-30;
	m_UserFlagPos[1].x=nWidth-65;
	m_UserFlagPos[1].y=nHeight/2-110;
	m_PointTrustee[1].x=nWidth-FACE_CX-24;
	m_PointTrustee[1].y=nHeight/2-FACE_CY-17;
	m_ptWinOrder[1].x=nWidth-46;
	m_ptWinOrder[1].y=nHeight/2+95;
	m_ptGangScore[1].x = m_ptAvatar[1].x - 100;
	m_ptGangScore[1].y = m_ptAvatar[1].y + 16;
	m_ptVoidShowPos[1].x = nWidth-65;
	m_ptVoidShowPos[1].y = nHeight/2-150;

	m_ptAvatar[2].x=nWidth/2-FACE_CX-29;
	m_ptAvatar[2].y=nHeight-FACE_CY-8;
	m_ptNickName[2].x=m_ptAvatar[2].x+60;
	m_ptNickName[2].y=m_ptAvatar[2].y;
	m_ptClock[2].x=nWidth/2-FACE_CX/2-TIME_CX-2;
	m_ptClock[2].y=nHeight/2+350-TIME_CY-8+40;
	m_ptReady[2].x=nWidth/2;
	m_ptReady[2].y = nHeight/2+130;
	m_UserFlagPos[2].x=nWidth/2-125;
	m_UserFlagPos[2].y=nHeight-FACE_CY;
	m_PointTrustee[2].x=nWidth/2-FACE_CX-29;
	m_PointTrustee[2].y=nHeight-FACE_CY-8;
	m_ptWinOrder[2].x=nWidth/2+110;
	m_ptWinOrder[2].y=nHeight-33;
	m_ptGangScore[2].x = m_ptAvatar[2].x + 16;
	m_ptGangScore[2].y = m_ptAvatar[2].y - 250;
	m_ptVoidShowPos[2].x = nWidth/2-160;
	m_ptVoidShowPos[2].y = nHeight-FACE_CY;

	m_ptAvatar[3].x=BORDER_CX+24;
	m_ptAvatar[3].y=nHeight/2-FACE_CY-17;
	m_ptNickName[3].x=m_ptAvatar[3].x;
	m_ptNickName[3].y=m_ptAvatar[3].y+FACE_CY+10;
	m_ptClock[3].x=BORDER_CX+22;
	m_ptClock[3].y=nHeight/2-FACE_CY-27-TIME_CY;
	m_ptReady[3].x=nWidth/2-150;
	m_ptReady[3].y=nHeight/2-30;
	m_UserFlagPos[3].x=28;
	m_UserFlagPos[3].y=nHeight/2-110;
	m_PointTrustee[3].x=BORDER_CX+24;
	m_PointTrustee[3].y=nHeight/2-FACE_CY-17;
	m_ptWinOrder[3].x=48;
	m_ptWinOrder[3].y=nHeight/2+95;
	m_ptGangScore[3].x = m_ptAvatar[3].x + 100;
	m_ptGangScore[3].y = m_ptAvatar[3].y + 16;
	m_ptVoidShowPos[3].x=28;
	m_ptVoidShowPos[3].y=nHeight/2-150;

	int nXCenter=nWidth/2;
	int nYCenter=nHeight/2-40;

	//用户扑克
	m_UserCard[0].SetControlPoint(nXCenter-223, 90+FRAME_HEIGHT);					//上
	m_UserCard[1].SetControlPoint(nWidth-145 ,nHeight/2-250);						//右
	m_UserCard[2].SetControlPoint(127, nHeight/2+184);								//左
	m_HandCardControl.SetBenchmarkPos(nWidth/2 - 5, nHeight-80, enXCenter, enYBottom);

	//桌面扑克
	m_TableCard[0].SetControlPoint(nWidth/2-170, 95+FRAME_HEIGHT);					//上
	m_TableCard[1].SetControlPoint(nWidth-115, nHeight/2-135);						//右
	m_TableCard[2].SetControlPoint(nWidth/2+310, nHeight-140);						//下
	m_TableCard[3].SetControlPoint(112, nHeight/2+65);								//左

	//堆积扑克
	m_HeapCard[0].SetControlPoint(nXCenter-143, nYCenter-180+FRAME_HEIGHT);			//上
	m_HeapCard[1].SetControlPoint(nXCenter+195, nYCenter-90);						//右
	m_HeapCard[2].SetControlPoint(nXCenter-156, nYCenter+190);						//下
	m_HeapCard[3].SetControlPoint(nXCenter-227, nYCenter-90);						//左

	//丢弃扑克
	m_DiscardCard[0].SetControlPoint(nXCenter-106, nYCenter-88+FRAME_HEIGHT);		//上
	m_DiscardCard[1].SetControlPoint(nXCenter+115, nYCenter-43);					//右
	m_DiscardCard[2].SetControlPoint(nXCenter+85, nYCenter+112);					//下
	m_DiscardCard[3].SetControlPoint(nXCenter-150, nYCenter+80);					//左

	//组合扑克			上
	m_WeaveCard[0][0].SetControlPoint(nWidth/2 + 145 - 72*0, 160+FRAME_HEIGHT);
	m_WeaveCard[0][1].SetControlPoint(nWidth/2 + 135 - 72*1, 160+FRAME_HEIGHT);
	m_WeaveCard[0][2].SetControlPoint(nWidth/2 + 125 - 72*2, 160+FRAME_HEIGHT);
	m_WeaveCard[0][3].SetControlPoint(nWidth/2 + 115 - 72*3, 160+FRAME_HEIGHT);

	//组合扑克			右
	m_WeaveCard[1][0].SetControlPoint(nWidth-163, nHeight/2 + 110 - 70*0);
	m_WeaveCard[1][1].SetControlPoint(nWidth-163, nHeight/2 + 110 - 70*1);
	m_WeaveCard[1][2].SetControlPoint(nWidth-163, nHeight/2 + 110 - 70*2);
	m_WeaveCard[1][3].SetControlPoint(nWidth-163, nHeight/2 + 110 - 70*3);

	//组合扑克			下
	m_WeaveCard[2][0].SetControlPoint(nWidth/2 - 190 + 135*0, nHeight-135);
	m_WeaveCard[2][1].SetControlPoint(nWidth/2 - 180 + 135*1, nHeight-135);
	m_WeaveCard[2][2].SetControlPoint(nWidth/2 - 170 + 135*2, nHeight-135);
	m_WeaveCard[2][3].SetControlPoint(nWidth/2 - 160 + 135*3, nHeight-135);

	//组合扑克			左
	m_WeaveCard[3][0].SetControlPoint(180, nHeight/2 - 135 + 70*0);
	m_WeaveCard[3][1].SetControlPoint(180, nHeight/2 - 135 + 70*1);
	m_WeaveCard[3][2].SetControlPoint(180, nHeight/2 - 135 + 70*2);
	m_WeaveCard[3][3].SetControlPoint(180, nHeight/2 - 135 + 70*3);

	if (m_bInitD3D==true)
	{
		//色子窗口
		CRect rcSice;
		m_DrawSiceWnd.GetWindowRect(&rcSice);
		m_DrawSiceWnd.SetBenchmarkPos((nWidth-rcSice.Width())/2,(nHeight-rcSice.Height())/2);
		m_DrawSiceWnd.MoveWindow((nWidth-rcSice.Width())/2,(nHeight-rcSice.Height())/2,rcSice.Width(),rcSice.Height());

		//开始按钮
		CRect rcStart;
		m_btStart.GetClientRect(rcStart);
		m_btStart.SetWindowPos((nWidth-rcStart.Width())/2, nHeight/2 + 50,0,0,SWP_NOSIZE);

		//叫分按钮
		CRect rcCallButton;
		m_btCallCard1.GetClientRect(rcCallButton);
		m_btCallCard1.SetWindowPos(nWidth/2-rcCallButton.Width()*2+20, nHeight/2+45, 0, 0, SWP_NOSIZE);
		m_btCallCard2.SetWindowPos(nWidth/2-rcCallButton.Width()+40, nHeight/2+45, 0, 0, SWP_NOSIZE);
		m_btCallCard3.SetWindowPos(nWidth/2+60, nHeight/2+45, 0, 0, SWP_NOSIZE);

		//功能按钮
		CRect rcTrustee;
		m_btStustee.GetClientRect(rcTrustee);
		m_btStustee.SetWindowPos(nWidth-(rcTrustee.Width() + 5),nHeight-rcTrustee.Height()-4,0,0,SWP_NOSIZE);
		m_btCancelStustee.SetWindowPos(nWidth-(rcTrustee.Width() + 5),nHeight-rcTrustee.Height()-4,0,0,SWP_NOSIZE);

		//积分按钮
		CRect rcHistory;
		m_btOpenHistoryScore.GetClientRect(rcHistory);
		m_btOpenHistoryScore.SetWindowPos(nWidth-rcHistory.Width()-5,FRAME_HEIGHT+5,0,0,SWP_NOSIZE);
		m_btCloseHistoryScore.SetWindowPos(nWidth-rcHistory.Width()-5,FRAME_HEIGHT+5,0,0,SWP_NOSIZE);

		//控制窗口
		m_ControlWnd.SetBenchmarkPos(nWidth/2+298,nHeight-150);

		if (m_ScoreControl[0].IsWindowActive()==true)
		{
			//获取位置
			CRect rcClient;
			GetClientRect(&rcClient);
			CRect rcScoreControl;
			m_ScoreControl[0].GetClientRect(rcScoreControl);

			//移动位置
			m_ScoreControl[0].SetWindowPos(nWidth/2-130, nHeight/2-300, 218, 177, SWP_NOSIZE);
			m_ScoreControl[1].SetWindowPos(nWidth/2+80, nHeight/2-125 , 218, 177, SWP_NOSIZE);
			m_ScoreControl[2].SetWindowPos(nWidth/2-130, nHeight/2+75 , 218, 177, SWP_NOSIZE);
			m_ScoreControl[3].SetWindowPos(nWidth/2-310, nHeight/2-125 , 218, 177, SWP_NOSIZE);
		}
	}

	//按钮控件
	HDWP hDwp=BeginDeferWindowPos(32);
	const UINT uFlags=SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOCOPYBITS|SWP_NOSIZE;

	//主框架
	DeferWindowPos(hDwp, m_btMin, NULL, nWidth-10-(25*2)-5, 2, 0, 0, uFlags|SWP_NOSIZE);
	DeferWindowPos(hDwp, m_btClose, NULL, nWidth-10-25, 2, 0, 0, uFlags|SWP_NOSIZE);
	// DeferWindowPos(hDwp, m_btBank, NULL, 17, nHeight-247, 0, 0, uFlags|SWP_NOSIZE);

	EndDeferWindowPos(hDwp);

	return;
}

//动画驱动
VOID CGameClientView::CartoonMovie()
{
	if ((m_wGangChairId!=INVALID_CHAIR) && (m_CartoonGangLapseCount.GetLapseCount(130)>0L))
	{
		//设置变量
		m_cbGangIndex = (m_cbGangIndex+1)%4;

		//设置位置
		if( ++m_nYGangStep >= GANG_MOVE_COUNT )
		{
			m_wGangChairId = INVALID_CHAIR;
			ZeroMemory(m_lGangScore,sizeof(m_lGangScore));
		}
	}

	return ;
}

//配置设备
VOID CGameClientView::InitGameView(CD3DDevice * pD3DDevice, INT nWidth, INT nHeight)
{
	//变量定义
	CRect rcCreate(0,0,0,0);
	HINSTANCE hResInstance=AfxGetInstanceHandle();

	//开始按钮
	m_btStart.ActiveWindow(rcCreate, 0, IDC_START, &m_VirtualEngine, NULL);
	m_btStart.SetButtonImage(pD3DDevice, TEXT("BT_START"), TEXT("PNG"), hResInstance);

	//缺门按钮
	m_btCallCard1.ActiveWindow(rcCreate, 0, IDC_CALL_CARD_1, &m_VirtualEngine, NULL);
	m_btCallCard2.ActiveWindow(rcCreate, 0, IDC_CALL_CARD_2, &m_VirtualEngine, NULL);
	m_btCallCard3.ActiveWindow(rcCreate, 0, IDC_CALL_CARD_3, &m_VirtualEngine, NULL);
	m_btCallCard1.SetButtonImage(pD3DDevice, TEXT("BT_CALL_CARD_1"), TEXT("PNG"), hResInstance);
	m_btCallCard2.SetButtonImage(pD3DDevice, TEXT("BT_CALL_CARD_2"), TEXT("PNG"), hResInstance);
	m_btCallCard3.SetButtonImage(pD3DDevice, TEXT("BT_CALL_CARD_3"), TEXT("PNG"), hResInstance);

	//托管按钮
	m_btStustee.ActiveWindow(rcCreate, WS_VISIBLE|WS_DISABLED, IDC_TRUSTEE_CONTROL, &m_VirtualEngine, NULL);
	m_btCancelStustee.ActiveWindow(rcCreate, WS_VISIBLE|WS_DISABLED, IDC_TRUSTEE_CONTROL, &m_VirtualEngine, NULL);
	m_btOpenHistoryScore.ActiveWindow(rcCreate,WS_VISIBLE|WS_DISABLED,IDC_OPEN_HISTORY,&m_VirtualEngine,NULL);
	m_btCloseHistoryScore.ActiveWindow(rcCreate,WS_VISIBLE|WS_DISABLED,IDC_CLOSE_HISTROY,&m_VirtualEngine,NULL);
	m_btStustee.SetButtonImage(pD3DDevice, TEXT("BT_START_TRUSTEE"), TEXT("PNG"), hResInstance);
	m_btCancelStustee.SetButtonImage(pD3DDevice, TEXT("BT_STOP_TRUSTEE"), TEXT("PNG"), hResInstance);
	m_btOpenHistoryScore.SetButtonImage(pD3DDevice,TEXT("BT_SHOW_SCORE"),TEXT("PNG"),hResInstance);
	m_btCloseHistoryScore.SetButtonImage(pD3DDevice,TEXT("BT_CLOSE_SCORE"),TEXT("PNG"),hResInstance);
	m_btStustee.EnableWindow(false);
	m_btCancelStustee.EnableWindow(false);
	m_btCancelStustee.ShowWindow(false);
	m_btOpenHistoryScore.ShowWindow(true);
	m_btOpenHistoryScore.EnableWindow(true);
	m_btCloseHistoryScore.ShowWindow(false);

	//创建控件
	m_ControlWnd.ActiveWindow(rcCreate, WS_VISIBLE|WS_DISABLED, 10, &m_VirtualEngine,NULL);
	m_DrawSiceWnd.Create(NULL,NULL,WS_CHILD|WS_CLIPCHILDREN, rcCreate,this,11,NULL);
	m_ControlWnd.ShowWindow(false);

	//加载资源
	g_CardResource.LoadResource(pD3DDevice);

	//背景资源
	m_ImageDisc.LoadImage(pD3DDevice, hResInstance,TEXT("DISC"), TEXT("PNG"));
	m_ImageArrow.LoadImage(pD3DDevice, hResInstance,TEXT("ARROW"), TEXT("PNG"));
	m_PngXiaYu.LoadImage(pD3DDevice, hResInstance, TEXT("XIAO_YU"), TEXT("PNG"));
	m_PngNumWin.LoadImage(pD3DDevice, hResInstance, TEXT("NUM_WIN"), TEXT("PNG"));
	m_ImageWait.LoadImage(pD3DDevice, hResInstance, TEXT("WAIT_TIP"), TEXT("PNG"));
	m_ImageBack.LoadImage(pD3DDevice, hResInstance, TEXT("VIEW_BACK"), TEXT("PNG"));
	m_PngGuaFeng.LoadImage(pD3DDevice, hResInstance, TEXT("GUA_FENG"), TEXT("PNG"));
	m_PngNumLost.LoadImage(pD3DDevice, hResInstance, TEXT("NUM_LOST"), TEXT("PNG"));
	m_ImageTrustee.LoadImage(pD3DDevice, hResInstance,TEXT("TRUSTEE"), TEXT("PNG"));
	m_ImageUserFlag.LoadImage(pD3DDevice, hResInstance, TEXT("USER_FLAG"), TEXT("PNG"));
	m_ImageCenter.LoadImage(pD3DDevice, hResInstance, TEXT("VIEW_CENTER"), TEXT("PNG"));
	m_ImageWinOrder.LoadImage(pD3DDevice, hResInstance, TEXT("WIN_ORDER"), TEXT("PNG"));
	m_ImageActionAni.LoadImage(pD3DDevice, hResInstance,TEXT("ACTIONANI"), TEXT("PNG"));
	m_ImageCellScore.LoadImage(pD3DDevice, hResInstance, TEXT("CELL_SCORE"), TEXT("PNG"));
	m_ImageOutCard.LoadImage(pD3DDevice, hResInstance, TEXT("OUT_CARD_TIP"), TEXT("PNG"));
	m_ImageActionBack.LoadImage(pD3DDevice, hResInstance, TEXT("ACTION_BACK"), TEXT("PNG"));
	m_TextureUserFrameH.LoadImage(pD3DDevice, hResInstance, TEXT("USER_FRAME_H"), TEXT("PNG"));
	m_TextureUserFrameV.LoadImage(pD3DDevice, hResInstance, TEXT("USER_FRAME_V"), TEXT("PNG"));
	m_TextureVoidShowing.LoadImage(pD3DDevice, hResInstance, TEXT("VOID_SHOWING"), TEXT("PNG"));
	m_TextureVoidShowingTip.LoadImage(pD3DDevice, hResInstance, TEXT("VOID_SHOWING_TIP"), TEXT("PNG"));
	m_TextureWaitChooseTip.LoadImage(pD3DDevice, hResInstance, TEXT("WAIT_CHOOSE_TIP"), TEXT("PNG"));
	m_TextureHistoryScore.LoadImage(pD3DDevice,hResInstance,TEXT("HISTORY_SCORE"),TEXT("PNG"));

	//游戏框架
	m_ImageGameFrame[0].LoadImage(pD3DDevice, hResInstance, TEXT("FRAME_TL"), TEXT("PNG"));
	m_ImageGameFrame[1].LoadImage(pD3DDevice, hResInstance, TEXT("FRAME_TM"), TEXT("PNG"));
	m_ImageGameFrame[2].LoadImage(pD3DDevice, hResInstance, TEXT("FRAME_TR"), TEXT("PNG"));
	m_ImageGameFrame[3].LoadImage(pD3DDevice, hResInstance, TEXT("FRAME_LM"), TEXT("PNG"));
	m_ImageGameFrame[4].LoadImage(pD3DDevice, hResInstance, TEXT("FRAME_RM"), TEXT("PNG"));
	m_ImageGameFrame[5].LoadImage(pD3DDevice, hResInstance, TEXT("FRAME_BL"), TEXT("PNG"));
	m_ImageGameFrame[6].LoadImage(pD3DDevice, hResInstance, TEXT("FRAME_BM"), TEXT("PNG"));
	m_ImageGameFrame[7].LoadImage(pD3DDevice, hResInstance, TEXT("FRAME_BR"), TEXT("PNG"));

	//启动渲染
	StartRenderThread();

	return ;
}

//绘画界面
void CGameClientView::DrawGameView(CD3DDevice * pD3DDevice, int nWidth, int nHeight)
{
	//获取大小
	CSize SizeCenter, SizeScoreInfo;
	SizeCenter.SetSize(m_ImageCenter.GetWidth(), m_ImageCenter.GetHeight());
	SizeScoreInfo.SetSize(m_ImageCellScore.GetWidth(), m_ImageCellScore.GetHeight());

	//绘画背景
	DrawViewImage(pD3DDevice,m_ImageBack,DRAW_MODE_SPREAD);
	m_ImageCenter.DrawImage(pD3DDevice, (nWidth-SizeCenter.cx)/2, (nHeight-SizeCenter.cy)*2/5, SizeCenter.cx, SizeCenter.cy, 0, 0);

	//单元背景
	m_ImageCellScore.DrawImage(pD3DDevice, 5, FRAME_HEIGHT+5, SizeScoreInfo.cx, SizeScoreInfo.cy, 0, 0);

	//单元积分
	TCHAR szBuffer[32] = TEXT("");
	CRect rcCellScore(65, FRAME_HEIGHT+12, 128, FRAME_HEIGHT+26);
	_sntprintf( szBuffer, CountArray(szBuffer), TEXT("%s"), AddDecimal(m_lCellScore) );
	DrawTextString(pD3DDevice,szBuffer,rcCellScore,DT_LEFT|DT_END_ELLIPSIS|DT_WORD_ELLIPSIS,D3DCOLOR_XRGB(255,255,0),D3DCOLOR_XRGB(0,50,0));

	//用户框架
	m_TextureUserFrameH.DrawImage(pD3DDevice, m_ptUserFrame[0].x, m_ptUserFrame[0].y);
	m_TextureUserFrameV.DrawImage(pD3DDevice, m_ptUserFrame[1].x, m_ptUserFrame[1].y);
	m_TextureUserFrameH.DrawImage(pD3DDevice, m_ptUserFrame[2].x, m_ptUserFrame[2].y);
	m_TextureUserFrameV.DrawImage(pD3DDevice, m_ptUserFrame[3].x, m_ptUserFrame[3].y);

	//绘制框架	
	DrawGameFrame(pD3DDevice, nWidth, nHeight);

	//绘画用户
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		//变量定义
		IClientUserItem * pUserData=GetClientUserItem(i);

		if (pUserData!=NULL)
		{
			//用户名字
			CRect rcNickName(m_ptNickName[i].x, m_ptNickName[i].y, m_ptNickName[i].x+68, m_ptNickName[i].y+12);
			DrawTextString(pD3DDevice, pUserData->GetNickName(), rcNickName, DT_LEFT|DT_END_ELLIPSIS|DT_WORD_ELLIPSIS, D3DCOLOR_XRGB(153,255,0), D3DCOLOR_XRGB(0,50,0));

			//用户积分
			TCHAR szUserScore[64] = TEXT("");
			CRect rcUserScore(m_ptNickName[i].x, m_ptNickName[i].y+17, m_ptNickName[i].x+68, m_ptNickName[i].y+29);
			_sntprintf(szUserScore,CountArray(szUserScore), TEXT("%s"), AddDecimal(pUserData->GetUserScore()) );
			DrawTextString(pD3DDevice, szUserScore, rcUserScore, DT_LEFT|DT_END_ELLIPSIS|DT_WORD_ELLIPSIS, D3DCOLOR_XRGB(255,255,0), D3DCOLOR_XRGB(0,50,0));

			//用户位置
			TCHAR szAddress[64]=TEXT("");
			tagUserInfo * pUserInfo = pUserData->GetUserInfo();
			_sntprintf( szAddress, CountArray(szAddress), TEXT("%s"), pUserInfo->szUserAddress );
			CRect rcUserLevel(m_ptNickName[i].x, m_ptNickName[i].y+34, m_ptNickName[i].x+68, m_ptNickName[i].y+46);
			DrawTextString(pD3DDevice, szAddress, rcUserLevel,DT_LEFT|DT_END_ELLIPSIS, D3DCOLOR_XRGB(255,255,255), D3DCOLOR_XRGB(0,50,0));

			//游戏等级
			/* LPCTSTR pszUserLevel = GetLevelDescribe(pUserData);
			CRect rcUserLevel(m_ptNickName[i].x, m_ptNickName[i].y+34, m_ptNickName[i].x+68, m_ptNickName[i].y+46);
			DrawTextString(pD3DDevice, pszUserLevel, rcUserLevel,DT_LEFT|DT_END_ELLIPSIS, D3DCOLOR_XRGB(255,255,255), D3DCOLOR_XRGB(0,50,0)); */

			//胡牌先后
			if(m_cbWinOrder[i] != 0)
			{
				INT nWinOrderWidth = m_ImageWinOrder.GetWidth()/4;
				INT nWinOrderHeight = m_ImageWinOrder.GetHeight();
				m_ImageWinOrder.DrawImage(pD3DDevice, m_ptWinOrder[i].x-nWinOrderWidth/2, m_ptWinOrder[i].y-nWinOrderHeight/2, 
					nWinOrderWidth, nWinOrderHeight, (m_cbWinOrder[i]-1)*nWinOrderWidth, 0);
			}

			//其他信息
			WORD wUserTimer=GetUserClock(i);
			if ((wUserTimer!=0)&&(m_wCurrentUser!=INVALID_CHAIR))
			{
				DrawUserClock(pD3DDevice,nWidth/2,nHeight/2 - 20,wUserTimer);
				if(m_wCurrentUser==0)
					m_ImageArrow.DrawImage(
										pD3DDevice,
										nWidth/2-15,								//X坐标
										nHeight/2-m_ImageArrow.GetHeight()*2 - 20,	//Y坐标
										m_ImageArrow.GetWidth()/4,					//宽
										m_ImageArrow.GetHeight(),					//高
										m_ImageArrow.GetWidth()/4*m_wCurrentUser,	//起始位置X
										0);
				if(m_wCurrentUser==1)
					m_ImageArrow.DrawImage(pD3DDevice,nWidth/2+m_ImageArrow.GetWidth()/4,nHeight/2-29,m_ImageArrow.GetWidth()/4,m_ImageArrow.GetHeight(),m_ImageArrow.GetWidth()/4*m_wCurrentUser,0);
				if(m_wCurrentUser==2)
					m_ImageArrow.DrawImage(pD3DDevice,nWidth/2-15,nHeight/2+m_ImageArrow.GetHeight()-20,m_ImageArrow.GetWidth()/4,m_ImageArrow.GetHeight(),m_ImageArrow.GetWidth()/4*m_wCurrentUser,0);
				if(m_wCurrentUser==3)
					m_ImageArrow.DrawImage(pD3DDevice,nWidth/2-m_ImageArrow.GetWidth()/4*2,nHeight/2-29,m_ImageArrow.GetWidth()/4,m_ImageArrow.GetHeight(),m_ImageArrow.GetWidth()/4*m_wCurrentUser,0);
			}
			if((wUserTimer!=0)&&(m_wCurrentUser==INVALID_CHAIR))
			{
				DrawUserClock(pD3DDevice,nWidth/2,nHeight/2-20,wUserTimer);
				if(i==0)
					m_ImageArrow.DrawImage(pD3DDevice,nWidth/2-15,nHeight/2-m_ImageArrow.GetHeight()*2+20,m_ImageArrow.GetWidth()/4,m_ImageArrow.GetHeight(),m_ImageArrow.GetWidth()/4*i,0);
				if(i==1)
					m_ImageArrow.DrawImage(pD3DDevice,nWidth/2+m_ImageArrow.GetWidth()/4,nHeight/2-15,m_ImageArrow.GetWidth()/4,m_ImageArrow.GetHeight(),m_ImageArrow.GetWidth()/4*i,0);
				if(i==2)
					m_ImageArrow.DrawImage(pD3DDevice,nWidth/2-15,nHeight/2+m_ImageArrow.GetHeight()-20,m_ImageArrow.GetWidth()/4,m_ImageArrow.GetHeight(),m_ImageArrow.GetWidth()/4*i,0);
				if(i==3)
					m_ImageArrow.DrawImage(pD3DDevice,nWidth/2-m_ImageArrow.GetWidth()/4*2,nHeight/2-15,m_ImageArrow.GetWidth()/4,m_ImageArrow.GetHeight(),m_ImageArrow.GetWidth()/4*i,0);

			}

			if (pUserData->GetUserStatus() == US_READY) 
			{
				DrawUserReady(pD3DDevice,m_ptReady[i].x,m_ptReady[i].y);
			}

			DrawUserAvatar(pD3DDevice,m_ptAvatar[i].x,m_ptAvatar[i].y,pUserData);

			//缺门标志
			if (m_cbVoidShowing[i]!=0)
			{
				m_TextureVoidShowing.DrawImage(pD3DDevice, 
												m_ptVoidShowPos[i].x, m_ptVoidShowPos[i].y, 
												m_TextureVoidShowing.GetWidth()/3, 
												m_TextureVoidShowing.GetHeight(), 
												m_TextureVoidShowing.GetWidth()/3*(m_cbVoidShowing[i]-1), 
												0);
			}

			//托管
			if(m_bTrustee[i]) {
				m_ImageTrustee.DrawImage(pD3DDevice,m_PointTrustee[i].x,m_PointTrustee[i].y);
			}
		}
	}

	////TEST+
	//m_wBankerUser = 3;
	//BYTE cbShow[4] = {1, 2, 3, 1};
	//CopyMemory(m_cbWinOrder, cbShow, sizeof(BYTE)*4);
	//CopyMemory(m_cbVoidShowing, cbShow, sizeof(BYTE)*4);
	////TEST+

	//用户标志
	if (m_wBankerUser!=INVALID_CHAIR)
	{
		//加载位图
		int nImageWidth=m_ImageUserFlag.GetWidth()/4;
		int nImageHeight=m_ImageUserFlag.GetHeight();

		//绘画标志
		for (WORD i=0;i<GAME_PLAYER;i++)
		{
			WORD wIndex=((i+GAME_PLAYER)-m_wBankerUser)%GAME_PLAYER;
			m_ImageUserFlag.DrawImage(pD3DDevice, m_UserFlagPos[i].x, m_UserFlagPos[i].y, nImageWidth, nImageHeight, nImageWidth*wIndex, 0);

			////TEST+
			//m_TextureVoidShowing.DrawImage(pD3DDevice, 
			//	m_ptVoidShowPos[i].x, m_ptVoidShowPos[i].y, 
			//	m_TextureVoidShowing.GetWidth()/3, 
			//	m_TextureVoidShowing.GetHeight(), 
			//	m_TextureVoidShowing.GetWidth()/3*(m_cbVoidShowing[i]-1), 
			//	0);
			//m_ImageTrustee.DrawImage(pD3DDevice,m_PointTrustee[i].x,m_PointTrustee[i].y);
			//DrawUserReady(pD3DDevice,m_ptReady[i].x,m_ptReady[i].y);

			////胡牌先后
			//if(m_cbWinOrder[i] != 0)
			//{
			//	INT nWinOrderWidth = m_ImageWinOrder.GetWidth()/4;
			//	INT nWinOrderHeight = m_ImageWinOrder.GetHeight();
			//	m_ImageWinOrder.DrawImage(pD3DDevice, m_ptWinOrder[i].x-nWinOrderWidth/2, m_ptWinOrder[i].y-nWinOrderHeight/2, 
			//		nWinOrderWidth, nWinOrderHeight, (m_cbWinOrder[i]-1)*nWinOrderWidth, 0);
			//}
			////TEST+
		}
	}
	
	//用户扑克
	m_UserCard[0].DrawCardControl(pD3DDevice);
	m_TableCard[0].DrawCardControl(pD3DDevice);
	
	//组合
	m_WeaveCard[0][0].DrawCardControl(pD3DDevice);	
	m_WeaveCard[0][1].DrawCardControl(pD3DDevice);
	m_WeaveCard[0][2].DrawCardControl(pD3DDevice);
	m_WeaveCard[0][3].DrawCardControl(pD3DDevice);

	//堆积扑克
	m_HeapCard[0].DrawCardControl(pD3DDevice);	//上

	//桌面扑克
	m_TableCard[1].DrawCardControl(pD3DDevice);
	m_TableCard[2].DrawCardControl(pD3DDevice);
	m_TableCard[3].DrawCardControl(pD3DDevice);
	m_UserCard[1].DrawCardControl(pD3DDevice);

	//组合
	m_WeaveCard[1][3].DrawCardControl(pD3DDevice);	
	m_WeaveCard[1][2].DrawCardControl(pD3DDevice);
	m_WeaveCard[1][1].DrawCardControl(pD3DDevice);
	m_WeaveCard[1][0].DrawCardControl(pD3DDevice);

	//堆积扑克
	m_HeapCard[2].DrawCardControl(pD3DDevice);	//下
	m_HeapCard[3].DrawCardControl(pD3DDevice);	//左
	m_HeapCard[1].DrawCardControl(pD3DDevice);	//右

	//组合
	m_WeaveCard[2][0].DrawCardControl(pD3DDevice);	
	m_WeaveCard[2][1].DrawCardControl(pD3DDevice);
	m_WeaveCard[2][2].DrawCardControl(pD3DDevice);
	m_WeaveCard[2][3].DrawCardControl(pD3DDevice);

	//组合
	m_WeaveCard[3][0].DrawCardControl(pD3DDevice);	
	m_WeaveCard[3][1].DrawCardControl(pD3DDevice);
	m_WeaveCard[3][2].DrawCardControl(pD3DDevice);
	m_WeaveCard[3][3].DrawCardControl(pD3DDevice);

	//丢弃牌
	m_DiscardCard[0].DrawCardControl(pD3DDevice);	//丢的
	m_DiscardCard[1].DrawCardControl(pD3DDevice);	//丢的
	m_DiscardCard[3].DrawCardControl(pD3DDevice);	//丢的
	m_DiscardCard[2].DrawCardControl(pD3DDevice);	//丢的

	//用户扑克
	m_UserCard[2].DrawCardControl(pD3DDevice);
	m_HandCardControl.DrawCardControl(pD3DDevice);

	//出牌提示
	if (m_bOutCard == true)
	{
		m_ImageOutCard.DrawImage(pD3DDevice,(nWidth-m_ImageOutCard.GetWidth())/2, nHeight - 180);
	}

	//等待提示
	if (m_bWaitOther == true)
	{
		m_ImageWait.DrawImage(pD3DDevice,(nWidth-m_ImageWait.GetWidth())/2, nHeight - 180);
	}

	//显示定缺
	if (m_bShowSelectTip == true)
	{
		m_TextureVoidShowingTip.DrawImage(pD3DDevice, (nWidth-m_TextureVoidShowingTip.GetWidth())/2, nHeight/2-120);
	}

	//显示定缺
	if (m_bShowWaitChooseTip == true)
	{
		m_TextureWaitChooseTip.DrawImage(pD3DDevice, (nWidth-m_TextureWaitChooseTip.GetWidth())/2, nHeight/2-120);
	}

	//历史积分
	if(m_bShowScore)
	{
		//获取大小
		CSize SizeHistoryScore;
		SizeHistoryScore.SetSize(m_TextureHistoryScore.GetWidth(),m_TextureHistoryScore.GetHeight());

		//积分背景
		m_TextureHistoryScore.DrawImage(pD3DDevice,nWidth-SizeHistoryScore.cx-40,5+FRAME_HEIGHT);

		//计算位置
		INT nYBenchmark=5;
		INT nXBenchmark=nWidth-SizeHistoryScore.cx-40;

		//绘画信息
		for (WORD i=0;i<GAME_PLAYER;i++)
		{			
			//获取用户
			IClientUserItem * pIClientUserItem=GetClientUserItem(i);
			if ((pIClientUserItem==NULL)||(m_pHistoryScore[i]==NULL)) continue;

			//位置计算
			CRect rcAccounts(nXBenchmark+13,nYBenchmark+31+i*23+FRAME_HEIGHT,nXBenchmark+83,nYBenchmark+43+i*23+FRAME_HEIGHT);
			CRect rcTurnScore(nXBenchmark+85,nYBenchmark+31+i*23+FRAME_HEIGHT,nXBenchmark+135,nYBenchmark+43+i*23+FRAME_HEIGHT);
			CRect rcCollectScore(nXBenchmark+139,nYBenchmark+31+i*23+FRAME_HEIGHT,nXBenchmark+185,nYBenchmark+43+i*23+FRAME_HEIGHT);

			//构造信息
			TCHAR szTurnScore[16]=TEXT("");
			TCHAR szCollectScore[16]=TEXT("");
			_sntprintf(szTurnScore,CountArray(szTurnScore), TEXT("%s"), AddDecimal(m_pHistoryScore[i]->lTurnScore) );
			_sntprintf(szCollectScore,CountArray(szCollectScore), TEXT("%s"), AddDecimal(m_pHistoryScore[i]->lCollectScore) );

			//绘画信息
			D3DCOLOR crColor1=D3DCOLOR_XRGB(255,153,0);
			D3DCOLOR crColor2=D3DCOLOR_XRGB(0,255,153);
			LPCTSTR pszNickName=pIClientUserItem->GetNickName();
			DrawTextString(pD3DDevice,pszNickName,&rcAccounts,DT_END_ELLIPSIS|DT_LEFT,D3DCOLOR_XRGB(255,255,255));
			DrawTextString(pD3DDevice,szTurnScore,&rcTurnScore,DT_END_ELLIPSIS|DT_CENTER,(m_pHistoryScore[i]->lTurnScore>=0)?crColor1:crColor2);
			DrawTextString(pD3DDevice,szCollectScore,&rcCollectScore,DT_END_ELLIPSIS|DT_CENTER,(m_pHistoryScore[i]->lCollectScore>=0)?crColor1:crColor2);
		}
	}

	//用户状态
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		if ((m_wOutCardUser==i)||(m_cbUserAction[i]!=0))
		{
			//计算位置
			int nXPos=0,nYPos=0;
			switch (i)
			{
			case 0:	//北向	上
				{
					nXPos=nWidth/2 - 35;
					nYPos=150;
					break;
				}
			case 1:	//东向   右
				{
					nXPos=nWidth - 270;
					nYPos=nHeight/2 - 45;
					break;
				}
			case 2:	//南向	下
				{
					nXPos=nWidth/2 - 35;
					nYPos=nHeight - 260;
					break;
				}
			case 3:	//西向	左
				{
					nXPos=195;
					nYPos=nHeight/2 - 45;
					break;
				}
			}

			//绘画动作
			if (m_cbUserAction[i]!=WIK_NULL)
			{
				//绘画动作
				if (m_bBombEffect==true)
				{
					//绘画效果
					INT nItemHeight=m_ImageActionAni.GetHeight()/6;
					INT nItemWidth=m_ImageActionAni.GetWidth()/BOMB_EFFECT_COUNT;

					//绘画动作
					int nYImagePos=0;
					if (m_cbUserAction[i]&WIK_PENG) nYImagePos=nItemHeight;
					else if (m_cbUserAction[i]&WIK_GANG) nYImagePos=nItemHeight*2;
					else if (m_cbUserAction[i]==WIK_CHI_HU) nYImagePos=nItemHeight*4;
					else if (m_cbUserAction[i]&WIK_ZI_MO) nYImagePos=nItemHeight*5;
					else nYImagePos=0;
					m_ImageActionAni.DrawImage(pD3DDevice, nXPos-nItemWidth/2+54, nYPos+42-nItemHeight/2, nItemWidth, nItemHeight,
						nItemWidth*(m_cbBombFrameIndex%BOMB_EFFECT_COUNT), nYImagePos, nItemWidth, nItemHeight);
				}
			}
			else
			{
				//动作背景
				m_ImageActionBack.DrawImage(pD3DDevice,nXPos,nYPos);

				//绘画扑克
				g_CardResource.m_ImageUserBottom.DrawCardItem(pD3DDevice,m_cbCardData,nXPos+15,nYPos+13);
			}
		}
	}

	//丢弃效果
	if(m_wDiscUser!=INVALID_CHAIR)
	{
		CSize SizeDisc(m_ImageDisc.GetWidth()/DISC_EFFECT_COUNT,m_ImageDisc.GetHeight());
		CPoint pt=m_DiscardCard[m_wDiscUser].GetLastCardPosition();
		pt.Offset(-SizeDisc.cx/2,-SizeDisc.cy);
		//绘画信息
		m_ImageDisc.DrawImage(pD3DDevice, pt.x, pt.y, SizeDisc.cx, SizeDisc.cy,
			m_cbDiscFrameIndex*SizeDisc.cx, 0, SizeDisc.cx, SizeDisc.cy);
	}

	//杠牌得分动画
	if( m_wGangChairId != INVALID_CHAIR )
	{
		//刮风下雨动画
		if( m_cbXiaYu )
		{
			int nImageWidth = m_PngXiaYu.GetWidth()/4;
			int nImageHeight = m_PngXiaYu.GetHeight();
			m_PngXiaYu.DrawImage(pD3DDevice, m_ptGangScore[m_wGangChairId].x, m_ptGangScore[m_wGangChairId].y,
				nImageWidth, nImageHeight, nImageWidth*m_cbGangIndex, 0, nImageWidth, nImageHeight);
		}
		else
		{
			int nImageWidth = m_PngGuaFeng.GetWidth()/4;
			int nImageHeight = m_PngGuaFeng.GetHeight();
			m_PngGuaFeng.DrawImage(pD3DDevice, m_ptGangScore[m_wGangChairId].x, m_ptGangScore[m_wGangChairId].y,
				nImageWidth, nImageHeight, nImageWidth*m_cbGangIndex, 0, nImageWidth, nImageHeight);
		}
		for( WORD i = 0; i < GAME_PLAYER; i++ )
		{
			if( m_lGangScore[i] > 0 )
				DrawDecimalString(pD3DDevice, &m_PngNumWin, TEXT("+-,.0123456789"), m_lGangScore[i], m_ptGangScore[i].x, m_ptGangScore[i].y-m_nYGangStep*3, DT_LEFT);
			else 
				DrawDecimalString(pD3DDevice, &m_PngNumLost, TEXT("+-,.0123456789"), m_lGangScore[i], m_ptGangScore[i].x, m_ptGangScore[i].y-m_nYGangStep*3, DT_LEFT);
		}
	}

	return;
}

//绘画框架
void CGameClientView::DrawGameFrame(CD3DDevice *pD3DDevice, int nWidth, int nHeight)
{
	//上部分
	m_ImageGameFrame[0].DrawImage(pD3DDevice,0,0);
	for (int i=m_ImageGameFrame[0].GetWidth();i<nWidth-m_ImageGameFrame[2].GetWidth();i+=m_ImageGameFrame[1].GetWidth())
	{
		m_ImageGameFrame[1].DrawImage(pD3DDevice,i,0);
	}
	m_ImageGameFrame[2].DrawImage(pD3DDevice,nWidth-m_ImageGameFrame[2].GetWidth(),0);

	//中间两边部分
	for (int iH=m_ImageGameFrame[0].GetHeight();iH<nHeight-m_ImageGameFrame[3].GetHeight();iH+=m_ImageGameFrame[3].GetHeight())
	{
		m_ImageGameFrame[3].DrawImage(pD3DDevice,0,iH);
		m_ImageGameFrame[4].DrawImage(pD3DDevice,nWidth-m_ImageGameFrame[4].GetWidth(),iH);
	}
	
	//下部分
	m_ImageGameFrame[5].DrawImage(pD3DDevice,0,nHeight-m_ImageGameFrame[5].GetHeight());
	for (int i=m_ImageGameFrame[5].GetWidth();i<nWidth-m_ImageGameFrame[6].GetWidth();i+=m_ImageGameFrame[6].GetWidth())
	{
		m_ImageGameFrame[6].DrawImage(pD3DDevice,i,nHeight-m_ImageGameFrame[6].GetHeight());
	}	
	m_ImageGameFrame[7].DrawImage(pD3DDevice,nWidth-m_ImageGameFrame[7].GetWidth(),nHeight-m_ImageGameFrame[7].GetHeight());

	//标题
	CRect rcCaption(5,7,1024,30);
	DrawTextString(pD3DDevice,m_StringCaption,rcCaption,DT_END_ELLIPSIS|DT_LEFT,D3DCOLOR_XRGB(255,255,0),D3DCOLOR_XRGB(0,50,0));
}

//基础积分
void CGameClientView::SetCellScore(LONGLONG lCellScore)
{
	//设置扑克
	if (lCellScore!=m_lCellScore)
	{
		//设置变量
		m_lCellScore=lCellScore;

		//更新界面
		InvalidGameView(0,0,0,0);
	}

	return;
}

//庄家用户
void CGameClientView::SetBankerUser(WORD wBankerUser)
{
	//设置用户
	if (wBankerUser!=m_wBankerUser)
	{
		//设置变量
		m_wBankerUser=wBankerUser;

		//更新界面
		InvalidGameView(0,0,0,0);
	}

	return;
}

//状态标志
void CGameClientView::SetStatusFlag(bool bOutCard, bool bWaitOther)
{
	//设置变量
	m_bOutCard=bOutCard;
	m_bWaitOther=bWaitOther;

	//更新界面
	InvalidGameView(0,0,0,0);

	return;
}

//出牌信息
void CGameClientView::SetOutCardInfo(WORD wViewChairID, BYTE cbCardData)
{
	//设置变量
	m_cbCardData=cbCardData;
	m_wOutCardUser=wViewChairID;

	//更新界面
	InvalidGameView(0,0,0,0);

	return;
}

//动作信息
void CGameClientView::SetUserAction(WORD wViewChairID, BYTE bUserAction)
{
	//设置变量
	if (wViewChairID<GAME_PLAYER)
	{
		m_cbUserAction[wViewChairID]=bUserAction;
		SetBombEffect(true);
	}
	else 
	{
		ZeroMemory(m_cbUserAction,sizeof(m_cbUserAction));
		if(m_bBombEffect)
			SetBombEffect(false);
	}

	//更新界面
	InvalidGameView(0,0,0,0);

	return;
}

//设置动作
bool CGameClientView::SetBombEffect(bool bBombEffect)
{
	if (bBombEffect==true)
	{
		//设置变量
		m_bBombEffect=true;
		m_cbBombFrameIndex=0;

		//启动时间
		SetTimer(IDI_BOMB_EFFECT, 180, NULL);
	}
	else
	{
		//停止动画
		if (m_bBombEffect==true)
		{
			//删除时间
			KillTimer(IDI_BOMB_EFFECT);

			//设置变量
			m_bBombEffect=false;
			m_cbBombFrameIndex=0;

			//更新界面
			InvalidGameView(0,0,0,0);
		}
	}

	return true;
}

//丢弃用户
void CGameClientView::SetDiscUser(WORD wDiscUser)
{
	if(m_wDiscUser != wDiscUser)
	{
		//更新变量
		m_wDiscUser=wDiscUser;

		//更新界面
		InvalidGameView(0,0,0,0);
	}
	return;
}

//定时玩家
void CGameClientView::SetCurrentUser(WORD wCurrentUser)
{
	if (m_wCurrentUser != wCurrentUser)
	{
		//更新变量 
		m_wCurrentUser=wCurrentUser;
		
		//更新界面
		InvalidGameView(0,0,0,0);
	}
	return;
}

//设置托管
void CGameClientView::SetTrustee(WORD wTrusteeUser,bool bTrustee)
{
	//校验数据 
	ASSERT(wTrusteeUser>=0&&wTrusteeUser<GAME_PLAYER);

	if(m_bTrustee[wTrusteeUser] !=bTrustee)	
	{
		//设置数据
		m_bTrustee[wTrusteeUser]=bTrustee;

		//更新界面
		InvalidGameView(0,0,0,0);
	}
	return;
}

//光标消息
BOOL CGameClientView::OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT uMessage)
{
	//获取光标
	CPoint MousePoint;
	GetCursorPos(&MousePoint);
	ScreenToClient(&MousePoint);

	//点击测试
	bool bRePaint=false;
	bool bHandle=m_HandCardControl.OnEventSetCursor(MousePoint,bRePaint);

	//重画控制
	if (bRePaint==true)
		InvalidGameView(0,0,0,0);

	//光标控制
	if (bHandle==false)
		__super::OnSetCursor(pWnd,nHitTest,uMessage);

	return TRUE;
}


//开始按钮
void CGameClientView::OnStart()
{
	//发送消息
	SendEngineMessage(IDM_START,0,0);

	return;
}

//定缺按钮
void CGameClientView::OnCallCard1()
{
	//发送消息
	SendEngineMessage(IDM_CALL_CARD, 1, 1);

	return;
}

//定缺按钮
void CGameClientView::OnCallCard2()
{
	//发送消息
	SendEngineMessage(IDM_CALL_CARD, 2, 2);

	return;
}

//定缺按钮
void CGameClientView::OnCallCard3()
{
	//发送消息
	SendEngineMessage(IDM_CALL_CARD, 3, 3);

	return;
}

//拖管控制
void CGameClientView::OnStusteeControl()
{
	SendEngineMessage(IDM_TRUSTEE_CONTROL,0,0);
	return;
}

//历史积分
void CGameClientView::OnBnClickedOpenHistory()
{
	m_bShowScore=!m_bShowScore;
	m_btOpenHistoryScore.ShowWindow(false);
	m_btCloseHistoryScore.ShowWindow(true);
	m_btCloseHistoryScore.EnableWindow(true);
}

//历史积分
void CGameClientView::OnBnClickedCloseHistory()
{
	m_bShowScore=!m_bShowScore;
	m_btOpenHistoryScore.ShowWindow(true);
	m_btCloseHistoryScore.ShowWindow(false);
	m_btOpenHistoryScore.EnableWindow(true);
}

//定时器
void CGameClientView::OnTimer(UINT nIDEvent)
{
	//动作动画
	if (nIDEvent==IDI_BOMB_EFFECT)
	{
		//停止判断
		if (m_bBombEffect==false)
		{
			KillTimer(IDI_BOMB_EFFECT);
			return;
		}

		//设置变量
		if ((m_cbBombFrameIndex+1)>=BOMB_EFFECT_COUNT)
		{
			//删除时间
			KillTimer(IDI_BOMB_EFFECT);

			//设置变量
			m_bBombEffect=false;
			m_cbBombFrameIndex=0;
		}
		else m_cbBombFrameIndex++;

		//更新界面
		InvalidGameView(0,0,0,0);

		return;
	}
	if (nIDEvent==IDI_DISC_EFFECT)
	{
		//设置变量
		if ((m_cbDiscFrameIndex+1)>=DISC_EFFECT_COUNT)
		{
			m_cbDiscFrameIndex=0;
		}
		else m_cbDiscFrameIndex++;

		//更新界面
		InvalidGameView(0,0,0,0);

		return;
	}

	__super::OnTimer(nIDEvent);
}

//设置顺序
void CGameClientView::SetWinOrder( WORD wChairId, BYTE cbWinOrder )
{
	if( wChairId == INVALID_CHAIR )
		ZeroMemory( m_cbWinOrder,sizeof(m_cbWinOrder) );
	else m_cbWinOrder[wChairId] = cbWinOrder;
	InvalidGameView(0,0,0,0);
}

//设置杠分
void CGameClientView::SetGangScore( WORD wChairId, BYTE cbXiaYu, LONGLONG lGangScore[GAME_PLAYER] )
{
	//设置变量
	m_cbXiaYu = cbXiaYu;
	m_wGangChairId = wChairId;
	CopyMemory( m_lGangScore,lGangScore,sizeof(m_lGangScore) );

	//启动动画
	for( WORD i = 0; i < GAME_PLAYER; i++ )
	{
		if( m_lGangScore[i] != 0 )
		{
			m_nYGangStep = 0;
			m_cbGangIndex = 0;

			//启动时间
			m_CartoonGangLapseCount.Initialization();
			break;
		}
	}

	//更新界面
	CRect rcClient;
	GetClientRect(&rcClient);			
	InvalidGameView(0,0,rcClient.Width(),rcClient.Height());

	return ;
}

//设置缺门
void CGameClientView::SetVoidShowing(WORD wChairID, BYTE cbShowIndex)
{
	if (wChairID != INVALID_CHAIR)
	{
		if (cbShowIndex == 0x01)
		{
			m_cbVoidShowing[wChairID] = 1;
		}
		else if (cbShowIndex == 0x11)
		{
			m_cbVoidShowing[wChairID] = 2;
		}
		else if (cbShowIndex == 0x21)
		{
			m_cbVoidShowing[wChairID] = 3;
		}
		else ASSERT(FALSE);
	}
	else
	{
		ZeroMemory(m_cbVoidShowing, sizeof(m_cbVoidShowing));
	}

	return;
}

//设置积分
VOID CGameClientView::SetHistoryScore(WORD wChairID, tagHistoryScore * pHistoryScore)
{
	//效验参数
	ASSERT(wChairID<GAME_PLAYER);
	if (wChairID>=GAME_PLAYER) return;

	//设置变量
	m_pHistoryScore[wChairID]=pHistoryScore;

	//更新界面
	CRect rcClient;
	GetClientRect(&rcClient);
	InvalidGameView(0,0,rcClient.Width(),rcClient.Height());

	return;
}

//显示定缺
void CGameClientView::ShowSelectTip(bool bShow)
{
	if (bShow != m_bShowSelectTip)
	{
		m_bShowSelectTip = bShow;
	}

	return;
}

//显示定缺
void CGameClientView::ShowWaitChooseTip(bool bShow)
{
	if (bShow != m_bShowWaitChooseTip)
	{
		m_bShowWaitChooseTip = bShow;
	}

	return;
}

//设置结束
VOID CGameClientView::ShowGameScoreInfo()
{
	//创建控件
	for (WORD i=0; i<GAME_PLAYER; i++)
	{
		if (m_ScoreControl[i].IsWindowActive()==false)
		{
			//创建窗口
			CRect rcCreate(0,0,0,0);
			m_ScoreControl[i].ActiveWindow(rcCreate, WS_VISIBLE, IDC_GAME_SCORE+i, &m_VirtualEngine, NULL);	
		}
	}

	//获取位置
	CRect rcClient;
	GetClientRect(&rcClient);
	CRect rcScoreControl;
	m_ScoreControl[0].GetClientRect(rcScoreControl);

	//移动位置
	m_ScoreControl[0].SetWindowPos(rcClient.Width()/2-130, rcClient.Height()/2-300, 190, 170, SWP_NOSIZE);
	m_ScoreControl[1].SetWindowPos(rcClient.Width()/2+80, rcClient.Height()/2-125 , 190, 170, SWP_NOSIZE);
	m_ScoreControl[2].SetWindowPos(rcClient.Width()/2-130, rcClient.Height()/2+75 , 190, 170, SWP_NOSIZE);
	m_ScoreControl[3].SetWindowPos(rcClient.Width()/2-310, rcClient.Height()/2-125 , 190, 170, SWP_NOSIZE);

	return;
}

//绘画小数点
void CGameClientView::DrawDecimalString(CD3DDevice * pD3DDevice, CD3DTexture* ImageNumber, TCHAR * szImageNum, SCORE lOutNum, INT nXPos, INT nYPos, UINT uFormat /*= DT_LEFT*/)
{
	TCHAR szOutNum[128] = {0};
	_sntprintf( szOutNum, CountArray(szOutNum), TEXT("%s"), AddDecimal(lOutNum) );

	// 加载资源
	INT nNumberHeight=ImageNumber->GetHeight();
	INT nNumberWidth=ImageNumber->GetWidth()/lstrlen(szImageNum);

	if ( uFormat == DT_CENTER )
	{
		nXPos -= (INT)(((DOUBLE)(lstrlen(szOutNum)) / 2.0) * nNumberWidth);
	}
	else if ( uFormat == DT_RIGHT )
	{
		nXPos -= lstrlen(szOutNum) * nNumberWidth;
	}

	for ( INT i = 0; i < lstrlen(szOutNum); ++i )
	{
		for ( INT j = 0; j < lstrlen(szImageNum); ++j )
		{
			if ( szOutNum[i] == szImageNum[j] && szOutNum[i] != '\0' )
			{
				ImageNumber->DrawImage(pD3DDevice, nXPos, nYPos, nNumberWidth, nNumberHeight, j * nNumberWidth, 0, nNumberWidth, nNumberHeight);
				nXPos += nNumberWidth;
				break;
			}
		}
	}
}

//绘画数字
void CGameClientView::DrawNumberString(CD3DDevice * pD3DDevice, CD3DTexture* ImageNumber, TCHAR * szImageNum, SCORE lOutNum, INT nXPos, INT nYPos, UINT uFormat /*= DT_LEFT*/)
{
	TCHAR szOutNum[128] = {0};
	_sntprintf(szOutNum,CountArray(szOutNum),SCORE_STRING,lOutNum);

	// 加载资源
	INT nNumberHeight=ImageNumber->GetHeight();
	INT nNumberWidth=ImageNumber->GetWidth()/lstrlen(szImageNum);

	if ( uFormat == DT_CENTER )
	{
		nXPos -= (INT)(((DOUBLE)(lstrlen(szOutNum)) / 2.0) * nNumberWidth);
	}
	else if ( uFormat == DT_RIGHT )
	{
		nXPos -= lstrlen(szOutNum) * nNumberWidth;
	}

	for ( INT i = 0; i < lstrlen(szOutNum); ++i )
	{
		for ( INT j = 0; j < lstrlen(szImageNum); ++j )
		{
			if ( szOutNum[i] == szImageNum[j] && szOutNum[i] != '\0' )
			{
				ImageNumber->DrawImage(pD3DDevice, nXPos, nYPos, nNumberWidth, nNumberHeight, j * nNumberWidth, 0, nNumberWidth, nNumberHeight);
				nXPos += nNumberWidth;
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

//设置标题
LRESULT CGameClientView::OnSetCaption(WPARAM wParam, LPARAM lParam)
{
	m_StringCaption = (LPCTSTR)lParam;
	CRect rcCaption(5, 0, 1024, FRAME_CAPTION);
	CDC* pDC = GetDC();
	
	SetWindowText(TEXT(""));
	return 0;
}

//鼠标消息
void CGameClientView::OnLButtonDown(UINT nFlags, CPoint Point)
{
	__super::OnLButtonDown(nFlags, Point);

	//发送引擎消息
	SendEngineMessage(IDM_LBUTTONDOWN_VIEW, 0, 0);

	//更新窗口
	LPARAM lParam = MAKEWPARAM(Point.x,Point.y);
	AfxGetMainWnd()->PostMessage(WM_LBUTTONDOWN,0,lParam);

	//获取扑克
	BYTE cbHoverCard = m_HandCardControl.GetHoverCard();
	if (cbHoverCard != 0) SendEngineMessage(IDM_OUT_CARD, cbHoverCard, cbHoverCard);

	return;
}

//游戏最小化
void CGameClientView::OnButtonMin()
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND,IDC_MIN,0);
}

//关闭游戏
void CGameClientView::OnButtonClose()
{
	CGameClientEngine *pGameClientEngine=CONTAINING_RECORD( this, CGameClientEngine, m_GameClientView);
	if ( pGameClientEngine->GetGameStatus()!=GAME_STATUS_FREE )
	{
		//提示消息
		CInformation Information(this);
		INT nRes=0;
		nRes = Information.ShowMessageBox( TEXT("您正在游戏中，确定要强退吗？"), MB_ICONQUESTION|MB_YESNO, 0);
		if (nRes==IDYES)
		{
			AfxGetMainWnd()->PostMessage(WM_COMMAND,IDC_CLOSE);
		}

		return;
	}
	else
		AfxGetMainWnd()->PostMessage(WM_COMMAND,IDC_CLOSE);
}

//////////////////////////////////////////////////////////////////////////

// 添加小数点
CString CGameClientView::AddDecimal( LONGLONG lScore ,  bool bComma /*= true*/, bool bPlus /*= false*/)
{
	CString strScore;
	CString strReturn;
	LONGLONG lNumber = lScore;
	if ( lScore < 0 )
		lNumber = -lNumber;

	strScore.Format(TEXT("%I64d"), lNumber);

	//长度
	int nLength = strScore.GetLength();
	if (nLength==1) {
		strReturn.Insert(0, strScore.GetAt(nLength-1) );
		strReturn.Insert(0, TEXT("0.0") );
	}
	if (nLength==2) 
	{
		strReturn.Insert(0, strScore.GetAt(nLength-1) );
		strReturn.Insert(0, strScore.GetAt(nLength-2) );
		strReturn.Insert(0, TEXT("0.") );
	}
	else if(nLength>2)
	{
		strReturn.Insert(0, strScore.GetAt(nLength-1) );
		strReturn.Insert(0, strScore.GetAt(nLength-2) );
		strReturn.Insert(0, TEXT(".") );

		int nStrCount = 0;
		for( int i = strScore.GetLength() - 3; i >= 0; )
		{
			if( (nStrCount%3)==0 && nStrCount!=0 && bComma)
			{
				strReturn.Insert(0, ',');
				nStrCount = 0;
			}
			else
			{
				strReturn.Insert(0, strScore.GetAt(i));
				nStrCount++;
				i--;
			}
		}
	}

	if ( lScore < 0 )
		strReturn.Insert(0, '-');

	if ( bPlus && lScore > 0 )
		strReturn.Insert(0, '+');

	return strReturn;
}

//////////////////////////////////////////////////////////////////////////
