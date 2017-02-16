#include "StdAfx.h"
#include "GameClient.h"
#include "GameLogic.h"
#include "CardControl.h"
#include "ControlWnd.h"

//////////////////////////////////////////////////////////////////////////

//按钮标识
#define IDC_CHIHU					100									//吃胡按钮
#define IDC_LISTEN					101									//听牌按钮
#define IDC_GIVEUP					102									//放弃按钮

//位置标识
#define ITEM_WIDTH					90									//子项宽度
#define ITEM_HEIGHT					55									//子项高度
#define CONTROL_WIDTH				173									//控制宽度
#define CONTROL_HEIGHT				41									//控制高度

//////////////////////////////////////////////////////////////////////////


//构造函数
CControlWnd::CControlWnd()
{
	//设置变量
	m_cbAlphaIndex=0;

	//配置变量
	m_cbActionMask=0;
	m_cbCenterCard=0;
	m_PointBenchmark.SetPoint(0,0);
	ZeroMemory(m_cbGangCard,sizeof(m_cbGangCard));

	//状态变量
	m_cbItemCount=0;
	m_cbCurrentItem=0xFF;

	return;
}

//析构函数
CControlWnd::~CControlWnd()
{
}

//动画消息
VOID CControlWnd::OnWindowMovie()
{
	//灰度动画
	if ((IsWindowVisible()==true)&&((m_cbAlphaIndex<110L)&&(m_AlphaLapseCount.GetLapseCount(20)>0L)))
	{
		m_cbAlphaIndex+=5L;
	}

	return;
}

//基准位置
void CControlWnd::SetBenchmarkPos(int nXPos, int nYPos)
{
	//位置变量
	m_PointBenchmark.SetPoint(nXPos,nYPos);

	//调整控件
	RectifyControl();

	return;
}

//设置扑克
void CControlWnd::SetControlInfo(BYTE cbCenterCard, BYTE cbActionMask, tagGangCardResult & GangCardResult)
{
	//设置变量
	m_cbItemCount=0;
	m_cbCurrentItem=0xFF;
	m_cbActionMask=cbActionMask;
	m_cbCenterCard=cbCenterCard;

	//杠牌信息
	ZeroMemory(m_cbGangCard,sizeof(m_cbGangCard));
	for (BYTE i=0;i<GangCardResult.cbCardCount;i++) 
	{
		m_cbItemCount++;
		m_cbGangCard[i]=GangCardResult.cbCardData[i];
	}

	//计算数目
	BYTE cbItemKind[4]={WIK_LEFT,WIK_CENTER,WIK_RIGHT,WIK_PENG};
	for (BYTE i=0;i<CountArray(cbItemKind);i++) 
		if ((m_cbActionMask&cbItemKind[i])!=0) 
			m_cbItemCount++;

	//按钮控制
	m_btChiHu.EnableWindow((cbActionMask&WIK_CHI_HU) != 0);
	//m_btListen.EnableWindow(((cbActionMask&WIK_LISTEN)) != 0);

	//调整控件
	RectifyControl();

	//显示窗口
	ShowWindow(true);

	return;
}

//调整控件
void CControlWnd::RectifyControl()
{
	//设置位置
	CRect rcRect;
	rcRect.right=m_PointBenchmark.x;
	rcRect.bottom=m_PointBenchmark.y;
	rcRect.left=m_PointBenchmark.x-CONTROL_WIDTH;
	rcRect.top=m_PointBenchmark.y-ITEM_HEIGHT*m_cbItemCount-CONTROL_HEIGHT;
	if (m_cbItemCount>0)
		rcRect.top-=5;

	m_ControlSize.cx = rcRect.Width();
	m_ControlSize.cy = rcRect.Height();
	
	//移动窗口
	SetWindowPos(rcRect.left,rcRect.top,m_ControlSize.cx,m_ControlSize.cy,0);

	//按钮位置
	CRect rcButton;
	m_btChiHu.GetWindowRect(rcButton);

	int nYPos=rcRect.Height()-rcButton.Height()-9;
	m_btChiHu.SetWindowPos(rcRect.Width()-rcButton.Width()*3-12,nYPos, 0, 0, SWP_NOSIZE);
	//m_btListen.SetWindowPos(rcRect.Width()-rcButton.Width()*2-10,nYPos, 0, 0, SWP_NOSIZE);
	m_btGiveUp.SetWindowPos(rcRect.Width()-rcButton.Width()-6,nYPos, 0, 0, SWP_NOSIZE);

	return;
}


//创建消息
VOID CControlWnd::OnWindowCreate(CD3DDevice * pD3DDevice)
{
	//创建按钮
	CRect rcCreate(0,0,0,0);	

	//m_btListen.ActiveWindow(rcCreate,WS_VISIBLE,IDC_LISTEN,GetVirtualEngine(),this);
	//m_btListen.SetButtonImage(pD3DDevice,TEXT("BT_LISTEN"),TEXT("PNG"),AfxGetInstanceHandle());

	m_btGiveUp.ActiveWindow(rcCreate,WS_VISIBLE,IDC_GIVEUP,GetVirtualEngine(),this);
	m_btGiveUp.SetButtonImage(pD3DDevice,TEXT("BT_GIVEUP"),TEXT("PNG"),AfxGetInstanceHandle());

	m_btChiHu.ActiveWindow(rcCreate,WS_VISIBLE,IDC_CHIHU,GetVirtualEngine(),this);
	m_btChiHu.SetButtonImage(pD3DDevice,TEXT("BT_CHIHU"),TEXT("PNG"),AfxGetInstanceHandle());

	//加载资源
	HINSTANCE hInstance=AfxGetInstanceHandle();
	m_TextureControlTop.LoadImage(pD3DDevice,AfxGetInstanceHandle(),TEXT("CONTROL_TOP"),TEXT("PNG"));
	m_TextureControlMid.LoadImage(pD3DDevice,AfxGetInstanceHandle(),TEXT("CONTROL_MID"),TEXT("PNG"));
	m_TextureControlButtom.LoadImage(pD3DDevice,AfxGetInstanceHandle(),TEXT("CONTROL_BOTTOM"),TEXT("PNG"));
	m_TextureActionExplain.LoadImage(pD3DDevice,AfxGetInstanceHandle(),TEXT("ACTION_EXPLAIN"),TEXT("PNG"));
	m_TextureItemFrame.LoadImage(pD3DDevice,AfxGetInstanceHandle(),TEXT("ITEM_FRAME"),TEXT("PNG"));

	return;
}

//销毁消息
VOID CControlWnd::OnWindowDestory(CD3DDevice * pD3DDevice)
{
	//销户资源
	if (m_TextureControlTop.IsNull()==false) m_TextureControlTop.Destory();
	if (m_TextureControlMid.IsNull()==false) m_TextureControlMid.Destory();
	if (m_TextureControlButtom.IsNull()==false) m_TextureControlButtom.Destory();
	if (m_TextureActionExplain.IsNull()==false) m_TextureActionExplain.Destory();

	return;
}

//鼠标事件
VOID CControlWnd::OnEventMouse(UINT uMessage, UINT nFlags, INT nXMousePos, INT nYMousePos)
{
	if(512 == uMessage)								//鼠标移动
	{
		if (m_cbItemCount!=0)
		{
			//获取位置
			CPoint MousePoint(nXMousePos, nYMousePos);

			//计算索引
			BYTE bCurrentItem=0xFF;
			CRect rcItem(5,5,m_ControlSize.cx-5,ITEM_HEIGHT*m_cbItemCount+5);

			if (rcItem.PtInRect(MousePoint))
				bCurrentItem=(BYTE)((MousePoint.y-7)/ITEM_HEIGHT);

			//设置索引
			if (m_cbCurrentItem!=bCurrentItem)
			{
				m_cbCurrentItem=bCurrentItem;
			}
		}
	}
	else if(514 == uMessage)			//鼠标左键按下
	{
		//索引判断
		if (m_cbCurrentItem!=0xFF)
		{
			//变量定义
			BYTE cbIndex=0;
			BYTE cbItemKind[4]={WIK_LEFT,WIK_CENTER,WIK_RIGHT,WIK_PENG};

			CGameFrameEngine * pGameFrameEngine=CGameFrameEngine::GetInstance();

			//类型子项
			for (BYTE i=0;i<CountArray(cbItemKind);i++)
			{
				if (((m_cbActionMask&cbItemKind[i])!=0)&&(m_cbCurrentItem==cbIndex++))
				{
					pGameFrameEngine->PostMessage(IDM_CARD_OPERATE,cbItemKind[i],m_cbCenterCard);
					return;
				}
			}

			//杠牌子项
			for (BYTE i=0;i<CountArray(m_cbGangCard);i++)
			{
				if ((m_cbGangCard[i]!=0)&&(m_cbCurrentItem==cbIndex++))
				{
					pGameFrameEngine->PostMessage(IDM_CARD_OPERATE,WIK_GANG,m_cbGangCard[i]);
					return;
				}
			}

			//错误断言
			ASSERT(FALSE);
		}
	}
	return;
}

//按钮事件
VOID CControlWnd::OnEventButton(UINT uButtonID, UINT uMessage, INT nXMousePos, INT nYMousePos)
{
	//发送消息
	CGameFrameEngine * pGameFrameEngine=CGameFrameEngine::GetInstance();
	if( pGameFrameEngine )
	{
		if (uButtonID==IDC_LISTEN)
		{
		 	pGameFrameEngine->PostMessage(IDM_LISTEN_CARD,0,0);
		}
		else if (uButtonID==IDC_CHIHU)
		{
			pGameFrameEngine->PostMessage(IDM_CARD_OPERATE,WIK_CHI_HU,0);
		}
		else if (uButtonID==IDC_GIVEUP)
		{
			pGameFrameEngine->PostMessage(IDM_CARD_OPERATE,WIK_NULL,0);
		}
	}

	return;
}

//绘画窗口
VOID CControlWnd::OnEventDrawWindow(CD3DDevice * pD3DDevice, INT nXOriginPos, INT nYOriginPos)
{
	//绘画背景
	int nTopHeight = m_TextureControlTop.GetHeight();
	int nMidHeight = m_TextureControlMid.GetHeight();
	int nMidWidth = m_TextureControlMid.GetWidth();
	int nButtomHeight = m_TextureControlButtom.GetHeight();

	m_TextureControlTop.DrawImage(pD3DDevice, nXOriginPos, nYOriginPos);
	m_TextureControlMid.DrawImage(pD3DDevice, nXOriginPos, nYOriginPos + nTopHeight, m_ControlSize.cx, m_ControlSize.cy - nButtomHeight - nTopHeight, 0, 0, nMidWidth, nMidHeight);
 	m_TextureControlButtom.DrawImage(pD3DDevice, nXOriginPos, nYOriginPos + (m_ControlSize.cy - nButtomHeight));
 
 	//变量定义
 	int nYPos=5;
 	BYTE cbCurrentItem=0;
 	BYTE cbExcursion[3]={0,1,2};
 	BYTE cbItemKind[4]={WIK_LEFT,WIK_CENTER,WIK_RIGHT,WIK_PENG};
 
 	//绘画扑克
 	for (BYTE i=0;i<CountArray(cbItemKind);i++)
 	{
 		if ((m_cbActionMask&cbItemKind[i])!=0)
 		{
			//绘画边框
			if (cbCurrentItem==m_cbCurrentItem)
			{
				m_TextureItemFrame.DrawImage(pD3DDevice,nXOriginPos + 9,nYOriginPos + nYPos);
			}

 			//绘画扑克
 			for (BYTE j=0;j<3;j++)
 			{
 				BYTE cbCardData=m_cbCenterCard;
 				if (i<CountArray(cbExcursion))
 					cbCardData=cbCardData+j-cbExcursion[i];
 				g_CardResource.m_ImageTableBottom.DrawCardItem(pD3DDevice,cbCardData,nXOriginPos + j*24+12,nYOriginPos + nYPos+5);
 			}
 
 			//计算位置
 			int nXImagePos=0;
 			int nItemWidth=m_TextureActionExplain.GetWidth()/7;
 			if ((m_cbActionMask&cbItemKind[i])&WIK_PENG)
 				nXImagePos=nItemWidth;
 
 			//绘画标志
 			int nItemHeight=m_TextureActionExplain.GetHeight();
 			m_TextureActionExplain.DrawImage(pD3DDevice,nXOriginPos + 126,nYOriginPos + nYPos+5,nItemWidth,nItemHeight,nXImagePos,0);
 
 			//设置变量
 			++cbCurrentItem;
 			nYPos+=ITEM_HEIGHT;
 		}
 	}
 
 	//杠牌扑克
 	for (BYTE i=0;i<CountArray(m_cbGangCard);i++)
 	{
 		if (m_cbGangCard[i]!=0)
 		{
 			//绘画边框
 			if (cbCurrentItem==m_cbCurrentItem)
 			{
 				m_TextureItemFrame.DrawImage(pD3DDevice,nXOriginPos + 9,nYOriginPos + nYPos);
 			}
			
			//绘画扑克
 			for (BYTE j=0;j<4;j++)
 			{
 				g_CardResource.m_ImageTableBottom.DrawCardItem(pD3DDevice,m_cbGangCard[i],nXOriginPos + j*24+12,nYOriginPos + nYPos+5);
 			} 			
 
 			//绘画标志
 			int nItemWidth=m_TextureActionExplain.GetWidth()/7;
 			int nItemHeight=m_TextureActionExplain.GetHeight();
 			m_TextureActionExplain.DrawImage(pD3DDevice,nXOriginPos + 126,nYOriginPos + nYPos+5,nItemWidth,nItemHeight,nItemWidth*3,0);
 
 			//设置变量
 			cbCurrentItem++;
 			nYPos+=ITEM_HEIGHT;
 		}
 		else break;
 	}

	return;
}

//隐藏窗口
VOID CControlWnd::CloseControl()
{
	//关闭窗口
	if (IsWindowActive()==true)
	{
		//关闭窗口
		DeleteWindow();
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////////
//变量声明
extern CCardResource						g_CardResource;						//扑克资源
//////////////////////////////////////////////////////////////////////////////////
