#include "StdAfx.h"
#include "Resource.h"
#include "CardControl.h"

//////////////////////////////////////////////////////////////////////////
//宏定义

//公共定义
#define POS_SHOOT					8									//弹起象素
#define POS_SPACE					8									//分隔间隔
#define ITEM_COUNT					28									//子项数目
#define INVALID_ITEM				0xFFFF								//无效索引

//扑克大小
#define CARD_WIDTH					50									//扑克宽度
#define CARD_HEIGHT					72									//扑克高度

//////////////////////////////////////////////////////////////////////////

//构造函数
CCardListImage::CCardListImage()
{
	//位置变量
	m_nItemWidth=0;
	m_nItemHeight=0;
	m_nViewWidth=0;
	m_nViewHeight=0;

	return;
}

//析构函数
CCardListImage::~CCardListImage()
{
}

//加载资源
bool CCardListImage::LoadResource(CD3DDevice * pD3DDevice, LPCTSTR uResourceID, int nViewWidth, int nViewHeight)
{
	//加载资源
	m_CardListImage.LoadImage(pD3DDevice, AfxGetInstanceHandle(),uResourceID, TEXT("PNG"));

	//设置变量
	m_nViewWidth=nViewWidth;
	m_nViewHeight=nViewHeight;
	m_nItemHeight=m_CardListImage.GetHeight();
	m_nItemWidth=m_CardListImage.GetWidth()/ITEM_COUNT;

	return true;
}

//释放资源
bool CCardListImage::DestroyResource()
{
	//设置变量
	m_nItemWidth=0;
	m_nItemHeight=0;

	//释放资源
	//m_CardListImage.Destroy();

	return true;
}

//获取位置
int CCardListImage::GetImageIndex(BYTE cbCardData)
{
	//背景判断
	if (cbCardData==0) 
		return 0;

	//计算位置
	BYTE cbValue=cbCardData&MASK_VALUE;
	BYTE cbColor=(cbCardData&MASK_COLOR)>>4;
	return (cbColor>=0x03)?(cbValue+27):(cbColor*9+cbValue);
}

//绘画扑克
bool CCardListImage::DrawCardItem(CD3DDevice * pD3DDevice, BYTE cbCardData, int xDest, int yDest)
{
	//效验状态
	ASSERT(m_CardListImage.IsNull()==false);
	ASSERT((m_nItemWidth!=0)&&(m_nItemHeight!=0));

	//绘画子项
	if(cbCardData<=0x37)
	{
		int nImageXPos=GetImageIndex(cbCardData)*m_nItemWidth;
		m_CardListImage.DrawImage(pD3DDevice,
									xDest,	//X坐标
									yDest,	//Y坐标
									m_CardListImage.GetWidth()/28,				//宽
									m_CardListImage.GetHeight(),				//高
									nImageXPos,//起始位置X
									0);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CCardResource::CCardResource()
{
}

//析构函数
CCardResource::~CCardResource()
{
}

//加载资源
bool CCardResource::LoadResource(CD3DDevice * pD3DDevice)
{
	//变量定义
	HINSTANCE hInstance=AfxGetInstanceHandle();

	//用户扑克
	m_ImageUserTop.LoadImage(pD3DDevice, hInstance, TEXT("CARD_USER_TOP"), TEXT("PNG"));
	m_ImageUserBottom.LoadResource(pD3DDevice, TEXT("CARD_USER_BOTTOM"),CARD_WIDTH,CARD_HEIGHT);

	m_ImageUserLeft.LoadImage(pD3DDevice, AfxGetInstanceHandle(),TEXT("CARD_USER_LEFT"), TEXT("PNG"));
	m_ImageUserRight.LoadImage(pD3DDevice, AfxGetInstanceHandle(),TEXT("CARD_USER_RIGHT"), TEXT("PNG"));

	//桌子扑克
	m_ImageTableTop.LoadResource(pD3DDevice, TEXT("CARD_TABLE_TOP"), 34, 64);
	m_ImageTableLeft.LoadResource(pD3DDevice, TEXT("CARD_TABLE_LEFT"), 53, 35);
	m_ImageTableRight.LoadResource(pD3DDevice, TEXT("CARD_TABLE_RIGHT"), 53, 35);
	m_ImageTableBottom.LoadResource(pD3DDevice, TEXT("CARD_TABLE_BOTTOM"), 34, 70);
	m_TextureUserTableBottom.LoadResource(pD3DDevice, TEXT("CARD_USER_TABLE_BOTTOM"), 45, 77);

	//左边
	m_ImageHeapDoubleLV.LoadImage(pD3DDevice, AfxGetInstanceHandle(), TEXT("CARD_HEAP_DOUBLE_LV"), TEXT("PNG"));
	m_ImageHeapSingleLV.LoadImage(pD3DDevice, AfxGetInstanceHandle(), TEXT("CARD_HEAP_SINGLE_LV"), TEXT("PNG"));

	//右边
	m_ImageHeapDoubleRV.LoadImage(pD3DDevice, AfxGetInstanceHandle(), TEXT("CARD_HEAP_DOUBLE_RV"), TEXT("PNG"));
	m_ImageHeapSingleRV.LoadImage(pD3DDevice, AfxGetInstanceHandle(), TEXT("CARD_HEAP_SINGLE_RV"), TEXT("PNG"));
	
	//上面的
	m_ImageHeapDoubleTV.LoadImage(pD3DDevice, AfxGetInstanceHandle(), TEXT("CARD_HEAP_DOUBLE_TV"), TEXT("PNG"));
	m_ImageHeapSingleTV.LoadImage(pD3DDevice, AfxGetInstanceHandle(), TEXT("CARD_HEAP_SINGLE_TV"), TEXT("PNG"));

	//下面的
	m_ImageHeapDoubleBV.LoadImage(pD3DDevice, AfxGetInstanceHandle(), TEXT("CARD_HEAP_DOUBLE_BV"), TEXT("PNG"));
	m_ImageHeapSingleBV.LoadImage(pD3DDevice, AfxGetInstanceHandle(), TEXT("CARD_HEAP_SINGLE_BV"), TEXT("PNG"));

	return true;
}

//消耗资源
bool CCardResource::DestroyResource()
{
	//用户扑克
	m_ImageUserBottom.DestroyResource();

	//桌子扑克
	m_ImageTableTop.DestroyResource();
	m_ImageTableLeft.DestroyResource();
	m_ImageTableRight.DestroyResource();
	m_ImageTableBottom.DestroyResource();
	m_TextureUserTableBottom.DestroyResource();

	return true;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CHeapCard::CHeapCard()
{
	//控制变量
	m_ControlPoint.SetPoint(0,0);
	m_CardDirection=Direction_East;

	//扑克变量
	m_wFullCount=0;
	m_wMinusHeadCount=0;
	m_wMinusLastCount=0;

	return;
}

//析构函数
CHeapCard::~CHeapCard()
{
}

//绘画扑克
void CHeapCard::DrawCardControl(CD3DDevice * pDC)
{
	switch (m_CardDirection)
	{
	case Direction_East:	//东向   右		右
		{
			//绘画扑克
			if ((m_wFullCount-m_wMinusHeadCount-m_wMinusLastCount)>0)
			{
				//变量定义
				int nXPos=0,nYPos=0;
				WORD wHeapIndex=m_wMinusHeadCount/2;
				WORD wDoubleHeap=(m_wMinusHeadCount+1)/2;
				WORD wDoubleLast=(m_wFullCount-m_wMinusLastCount)/2;
				WORD wFinallyIndex=(m_wFullCount-m_wMinusLastCount)/2;

				//头部扑克
				if (m_wMinusHeadCount%2!=0)
				{
					nXPos=m_ControlPoint.x;
					nYPos=m_ControlPoint.y+wHeapIndex*16+13;

					g_CardResource.m_ImageHeapSingleRV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapSingleRV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapSingleRV.GetHeight(),				//高
									0,//起始位置X
									0);
				}

				//中间扑克
				for (WORD i=wDoubleHeap;i<wFinallyIndex;i++)
				{
					nXPos=m_ControlPoint.x;
					nYPos=m_ControlPoint.y+i*16;

					g_CardResource.m_ImageHeapDoubleRV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapDoubleRV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapDoubleRV.GetHeight(),				//高
									0,//起始位置X
									0);
				}

				//尾部扑克
				if (m_wMinusLastCount%2!=0)
				{
					nXPos=m_ControlPoint.x;
					nYPos=m_ControlPoint.y+wFinallyIndex*16+13;

					g_CardResource.m_ImageHeapSingleRV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapSingleRV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapSingleRV.GetHeight(),				//高
									0,//起始位置X
									0);
				}
			}
			
			break;
		}
	case Direction_South:	//南向	下
		{
			//绘画扑克
			if ((m_wFullCount-m_wMinusHeadCount-m_wMinusLastCount)>0)
			{
				//变量定义
				int nXPos=0,nYPos=0;
				WORD wHeapIndex=m_wMinusLastCount/2;
				WORD wDoubleHeap=(m_wMinusLastCount+1)/2;
				WORD wDoubleLast=(m_wFullCount-m_wMinusHeadCount)/2;
				WORD wFinallyIndex=(m_wFullCount-m_wMinusHeadCount)/2;

				//尾部扑克
				if (m_wMinusLastCount%2!=0)
				{
					nYPos=m_ControlPoint.y+16;
					nXPos=m_ControlPoint.x+wHeapIndex*24; //42
					g_CardResource.m_ImageHeapSingleBV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapSingleBV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapSingleBV.GetHeight(),				//高
									0,//起始位置X
									0);
				}

				//中间扑克
				for (WORD i=wDoubleHeap;i<wFinallyIndex;i++)
				{
					nYPos=m_ControlPoint.y;
					nXPos=m_ControlPoint.x+i*24;
					
					g_CardResource.m_ImageHeapDoubleBV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapDoubleBV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapDoubleBV.GetHeight(),				//高
									0,//起始位置X
									0);
				}

				//头部扑克
				if (m_wMinusHeadCount%2!=0)
				{
					nYPos=m_ControlPoint.y+16;
					nXPos=m_ControlPoint.x+wFinallyIndex*24;

					g_CardResource.m_ImageHeapSingleBV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapSingleBV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapSingleBV.GetHeight(),				//高
									0,//起始位置X
									0);
				}
			}
			
			break;
		}
	case Direction_West:	//西向		左
		{
			//绘画扑克
			if ((m_wFullCount-m_wMinusHeadCount-m_wMinusLastCount)>0)
			{
				//变量定义
				int nXPos=0,nYPos=0;
				WORD wHeapIndex=m_wMinusLastCount/2;
				WORD wDoubleHeap=(m_wMinusLastCount+1)/2;
				WORD wDoubleLast=(m_wFullCount-m_wMinusHeadCount)/2;
				WORD wFinallyIndex=(m_wFullCount-m_wMinusHeadCount)/2;

				//尾部扑克
				if (m_wMinusLastCount%2!=0)
				{
					nXPos=m_ControlPoint.x;
					nYPos=m_ControlPoint.y+wHeapIndex*16+13;

					g_CardResource.m_ImageHeapSingleLV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapSingleLV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapSingleLV.GetHeight(),				//高
									0,//起始位置X
									0);
				}

				//中间扑克
				for (WORD i=wDoubleHeap;i<wFinallyIndex;i++)
				{
					nXPos=m_ControlPoint.x;
					nYPos=m_ControlPoint.y+i*16;
				
					g_CardResource.m_ImageHeapDoubleLV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapDoubleLV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapDoubleLV.GetHeight(),				//高
									0,//起始位置X
									0);
				}

				//头部扑克
				if (m_wMinusHeadCount%2!=0)
				{
					nXPos=m_ControlPoint.x;
					nYPos=m_ControlPoint.y+wFinallyIndex*16+13;
				
					g_CardResource.m_ImageHeapSingleLV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapSingleLV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapSingleLV.GetHeight(),				//高
									0,//起始位置X
									0);
				}
			}
			
			break;
		}
	case Direction_North:	//北向		上
		{
			//绘画扑克
			if ((m_wFullCount-m_wMinusHeadCount-m_wMinusLastCount)>0)
			{
				//变量定义
				int nXPos=0,nYPos=0;
				WORD wHeapIndex=m_wMinusHeadCount/2;
				WORD wDoubleHeap=(m_wMinusHeadCount+1)/2;
				WORD wDoubleLast=(m_wFullCount-m_wMinusLastCount)/2;
				WORD wFinallyIndex=(m_wFullCount-m_wMinusLastCount)/2;

				//头部扑克
				if (m_wMinusHeadCount%2!=0)
				{
					nYPos=m_ControlPoint.y+15;
					nXPos=m_ControlPoint.x+wHeapIndex*22;

					g_CardResource.m_ImageHeapSingleTV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapSingleTV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapSingleTV.GetHeight(),				//高
									0,//起始位置X
									0);
				}

				//中间扑克
				for (WORD i=wDoubleHeap;i<wFinallyIndex;i++)
				{
					nYPos=m_ControlPoint.y;
					nXPos=m_ControlPoint.x+i*22;

					g_CardResource.m_ImageHeapDoubleTV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapDoubleTV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapDoubleTV.GetHeight(),				//高
									0,//起始位置X
									0);
				}

				//尾部扑克
				if (m_wMinusLastCount%2!=0)
				{
					nYPos=m_ControlPoint.y+15;
					nXPos=m_ControlPoint.x+wFinallyIndex*22;

					g_CardResource.m_ImageHeapSingleTV.DrawImage(pDC,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageHeapSingleTV.GetWidth(),				//宽
									g_CardResource.m_ImageHeapSingleTV.GetHeight(),				//高
									0,//起始位置X
									0);
				}
			}
			
			break;
		}
	}

	return;
}

//设置扑克
bool CHeapCard::SetCardData(WORD wMinusHeadCount, WORD wMinusLastCount, WORD wFullCount)
{
	//设置变量
	m_wFullCount=wFullCount;
	m_wMinusHeadCount=wMinusHeadCount;
	m_wMinusLastCount=wMinusLastCount;

	return true;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CWeaveCard::CWeaveCard()
{
	//状态变量
	m_bDisplayItem=false;
	m_ControlPoint.SetPoint(0,0);
	m_CardDirection=Direction_South;
	m_cbDirectionCardPos = 1;
	m_bGameEnd = false;

	//扑克数据
	m_wCardCount=0;
	ZeroMemory(&m_cbCardData,sizeof(m_cbCardData));

	return;
}

//析构函数
CWeaveCard::~CWeaveCard()
{
}

//绘画扑克
void CWeaveCard::DrawCardControl(CD3DDevice * pD3DDevice)
{
	//显示判断
	if (m_wCardCount==0) 
		return;

	//变量定义
	int nXScreenPos=0,nYScreenPos=0;
	int nItemWidth=0,nItemHeight=0,nItemWidthEx=0,nItemHeightEx=0;

	//绘画扑克
	switch (m_CardDirection)
	{
	case Direction_East:	//东向   右
		{
			//变量定义
			nItemWidth=g_CardResource.m_ImageTableRight.GetViewWidth();
			nItemHeight=g_CardResource.m_ImageTableRight.GetViewHeight()-17;
			nItemWidthEx=g_CardResource.m_ImageTableTop.GetViewWidth();
			nItemHeightEx=g_CardResource.m_ImageTableTop.GetViewHeight()-10;

			//第一扑克
			nYScreenPos = m_ControlPoint.y;
			nXScreenPos=m_ControlPoint.x+nItemWidthEx*2-nItemWidth;
			g_CardResource.m_ImageTableRight.DrawCardItem(pD3DDevice,GetCardData(2),nXScreenPos,nYScreenPos);
			nYScreenPos += nItemHeight;

			//第二扑克
			nXScreenPos=m_ControlPoint.x+nItemWidthEx*2-nItemWidth;
			g_CardResource.m_ImageTableRight.DrawCardItem(pD3DDevice,GetCardData(1),nXScreenPos,nYScreenPos);

			//第四扑克
			if (m_wCardCount==4)
			{
				g_CardResource.m_ImageTableRight.DrawCardItem(pD3DDevice,GetCardData(3),nXScreenPos,nYScreenPos-12);
			}

			//第三扑克
			nYScreenPos += nItemHeight;
			nXScreenPos=m_ControlPoint.x+nItemWidthEx*2-nItemWidth;
			g_CardResource.m_ImageTableRight.DrawCardItem(pD3DDevice,GetCardData(0),nXScreenPos,nYScreenPos);

			break;
		}
	case Direction_South:	//南向	下
		{
			//变量定义
			nItemWidth=g_CardResource.m_TextureUserTableBottom.GetViewWidth();
			nItemHeight=g_CardResource.m_TextureUserTableBottom.GetViewHeight();
			nItemWidthEx=g_CardResource.m_ImageTableRight.GetViewWidth();
			nItemHeightEx=g_CardResource.m_ImageTableRight.GetViewHeight();

			//第三扑克
			nXScreenPos = m_ControlPoint.x-50;
			nXScreenPos -= nItemWidth;
			nYScreenPos=m_ControlPoint.y+nItemHeightEx*2-nItemHeight;
			g_CardResource.m_TextureUserTableBottom.DrawCardItem(pD3DDevice,GetCardData(2),nXScreenPos,nYScreenPos);		

			//第二扑克
			nXScreenPos -= nItemWidth;
			nYScreenPos=m_ControlPoint.y+nItemHeightEx*2-nItemHeight;
			g_CardResource.m_TextureUserTableBottom.DrawCardItem(pD3DDevice,GetCardData(1),nXScreenPos,nYScreenPos);

			//第四扑克
			if (m_wCardCount==4)
			{
				g_CardResource.m_TextureUserTableBottom.DrawCardItem(pD3DDevice,GetCardData(3),nXScreenPos,nYScreenPos-21);
			}

			//第一扑克
			nXScreenPos -= nItemWidth;
			nYScreenPos=m_ControlPoint.y+nItemHeightEx*2-nItemHeight;
			g_CardResource.m_TextureUserTableBottom.DrawCardItem(pD3DDevice,GetCardData(0),nXScreenPos,nYScreenPos);

			break;
		}
	case Direction_West:	//西向	左
		{
			//变量定义
			nItemWidth=g_CardResource.m_ImageTableLeft.GetViewWidth();
			nItemHeight=g_CardResource.m_ImageTableLeft.GetViewHeight()-17;
			nItemWidthEx=g_CardResource.m_ImageTableBottom.GetViewWidth();
			nItemHeightEx=g_CardResource.m_ImageTableBottom.GetViewHeight()-10;

			//if(m_cbDirectionCardPos != 1)
			{
				//第一扑克
				nYScreenPos=m_ControlPoint.y-nItemHeightEx-nItemHeight*2;
				nXScreenPos=m_ControlPoint.x-nItemWidthEx*2;
				g_CardResource.m_ImageTableLeft.DrawCardItem(pD3DDevice,GetCardData(0),nXScreenPos,nYScreenPos);
				nYScreenPos += nItemHeight;

				//第二扑克
				nXScreenPos=m_ControlPoint.x-nItemWidthEx*2;
				g_CardResource.m_ImageTableLeft.DrawCardItem(pD3DDevice,GetCardData(1),nXScreenPos,nYScreenPos);

				//第四扑克
				if (m_wCardCount==4)
				{
					g_CardResource.m_ImageTableLeft.DrawCardItem(pD3DDevice,GetCardData(3),nXScreenPos,nYScreenPos-12);
				}

				//第三扑克
				nYScreenPos += nItemHeight;
				nXScreenPos=m_ControlPoint.x-nItemWidthEx*2;
				g_CardResource.m_ImageTableLeft.DrawCardItem(pD3DDevice,GetCardData(2),nXScreenPos,nYScreenPos);
			}

			break;
		}
	case Direction_North:	//北向	上。
		{
			//变量定义
			nItemWidth=g_CardResource.m_ImageTableTop.GetViewWidth()-10;
			nItemHeight=g_CardResource.m_ImageTableTop.GetViewHeight();
			nItemWidthEx=g_CardResource.m_ImageTableLeft.GetViewWidth();
			nItemHeightEx=g_CardResource.m_ImageTableLeft.GetViewHeight();

			//第三扑克
			nXScreenPos = m_ControlPoint.x;
			nYScreenPos = m_ControlPoint.y-nItemHeight;
			g_CardResource.m_ImageTableTop.DrawCardItem(pD3DDevice,GetCardData(2),nXScreenPos,nYScreenPos);
			nXScreenPos += nItemWidth;

			//第二扑克
			nYScreenPos = m_ControlPoint.y-nItemHeight;
			g_CardResource.m_ImageTableTop.DrawCardItem(pD3DDevice,GetCardData(1),nXScreenPos,nYScreenPos);	

			//第四扑克
			if (m_wCardCount==4)
			{
				g_CardResource.m_ImageTableTop.DrawCardItem(pD3DDevice,GetCardData(3),nXScreenPos,nYScreenPos-11);
			}

			//第一扑克
			nXScreenPos += nItemWidth;
			nYScreenPos=m_ControlPoint.y-nItemHeight;		
			g_CardResource.m_ImageTableTop.DrawCardItem(pD3DDevice,GetCardData(0),nXScreenPos,nYScreenPos);

			break;
		}
	}

	return;
}

//设置扑克
bool CWeaveCard::SetCardData(const BYTE cbCardData[], WORD wCardCount)
{
	//效验大小
	ASSERT(wCardCount<=CountArray(m_cbCardData));
	if (wCardCount>CountArray(m_cbCardData)) return false;

	//设置扑克
	m_wCardCount=wCardCount;
	CopyMemory(m_cbCardData,cbCardData,sizeof(BYTE)*wCardCount);

	return true;
}

//获取扑克
BYTE CWeaveCard::GetCardData(WORD wIndex)
{
	ASSERT(wIndex<CountArray(m_cbCardData));
	return ((m_bDisplayItem==true)||(wIndex==3&&m_bGameEnd))?m_cbCardData[wIndex]:0;
}

//绘画扑克
void CWeaveCard::DrawCardControl(CD3DDevice * pD3DDevice, int nXPos, int nYPos)
{
	//设置位置
	SetControlPoint(nXPos,nYPos);

	//显示判断
	if (m_wCardCount==0) 
		return;

	//变量定义
	int nXScreenPos=0,nYScreenPos=0;
	int nItemWidth=0,nItemHeight=0,nItemWidthEx=0,nItemHeightEx=0;

	//绘画扑克
	switch (m_CardDirection)
	{
	case Direction_East:	//东向   右
		{

			//绘画扑克
			for (WORD i=0;i<3;i++)
			{
				int nXScreenPos=m_ControlPoint.x-g_CardResource.m_ImageTableRight.GetViewWidth();
				int nYScreenPos=m_ControlPoint.y+i*g_CardResource.m_ImageTableRight.GetViewHeight();
				g_CardResource.m_ImageTableRight.DrawCardItem(pD3DDevice,GetCardData(2-i),nXScreenPos,nYScreenPos);
			}

			//第四扑克
			if (m_wCardCount==4)
			{
				int nXScreenPos=m_ControlPoint.x-g_CardResource.m_ImageTableRight.GetViewWidth();
				int nYScreenPos=m_ControlPoint.y-5+g_CardResource.m_ImageTableRight.GetViewHeight();
				g_CardResource.m_ImageTableRight.DrawCardItem(pD3DDevice,GetCardData(3),nXScreenPos,nYScreenPos);
			}

			break;
	}
	case Direction_South:	//南向
		{
			//绘画扑克
			for (WORD i=0;i<3;i++)
			{
				int nXScreenPos=m_ControlPoint.x+i*g_CardResource.m_TextureUserTableBottom.GetViewWidth();
				int nYScreenPos=m_ControlPoint.y-g_CardResource.m_TextureUserTableBottom.GetViewHeight()-5;
				g_CardResource.m_TextureUserTableBottom.DrawCardItem(pD3DDevice,GetCardData(i),nXScreenPos,nYScreenPos);
			}

			//第四扑克
			if (m_wCardCount==4)
			{
				int nXScreenPos=m_ControlPoint.x+g_CardResource.m_TextureUserTableBottom.GetViewWidth();
				int nYScreenPos=m_ControlPoint.y-g_CardResource.m_TextureUserTableBottom.GetViewHeight()-5*2;
				g_CardResource.m_TextureUserTableBottom.DrawCardItem(pD3DDevice,GetCardData(3),nXScreenPos,nYScreenPos);
			}
			break;

		}
	case Direction_West:	//西向
		{
			//绘画扑克
			for (WORD i=0;i<3;i++)
			{
				int nXScreenPos=m_ControlPoint.x;
				int nYScreenPos=m_ControlPoint.y+i*g_CardResource.m_ImageTableLeft.GetViewHeight();
				g_CardResource.m_ImageTableLeft.DrawCardItem(pD3DDevice,GetCardData(i),nXScreenPos,nYScreenPos);
			}

			//第四扑克
			if (m_wCardCount==4)
			{
				int nXScreenPos=m_ControlPoint.x;
				int nYScreenPos=m_ControlPoint.y+g_CardResource.m_ImageTableLeft.GetViewHeight()-5;
				g_CardResource.m_ImageTableLeft.DrawCardItem(pD3DDevice,GetCardData(3),nXScreenPos,nYScreenPos);
			}

			break;

		}
	case Direction_North:	//北向
		{
			//绘画扑克
			for (WORD i=0;i<3;i++)
			{
				int nYScreenPos=m_ControlPoint.y;
				int nXScreenPos=m_ControlPoint.x-(i+1)*g_CardResource.m_ImageTableTop.GetViewWidth();
				g_CardResource.m_ImageTableTop.DrawCardItem(pD3DDevice,GetCardData(2-i),nXScreenPos,nYScreenPos);
			}

			//第四扑克
			if (m_wCardCount==4)
			{
				int nYScreenPos=m_ControlPoint.y-5;
				int nXScreenPos=m_ControlPoint.x-2*g_CardResource.m_ImageTableTop.GetViewWidth();
				g_CardResource.m_ImageTableTop.DrawCardItem(pD3DDevice,GetCardData(3),nXScreenPos,nYScreenPos);
			}

			break;

		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CUserCard::CUserCard()
{
	//扑克数据
	m_wCardCount=0;
	m_bCurrentCard=false;

	//控制变量
	m_ControlPoint.SetPoint(0,0);
	m_CardDirection=Direction_East;

	return;
}

//析构函数
CUserCard::~CUserCard()
{
}

//绘画扑克
void CUserCard::DrawCardControl(CD3DDevice * pD3DDevice)
{
	switch (m_CardDirection)
	{
	case Direction_East:	//东向   右	右边
		{
			//当前扑克
			if (m_bCurrentCard==true)
			{
				int nXPos=m_ControlPoint.x;
				int nYPos=m_ControlPoint.y;
				g_CardResource.m_ImageUserRight.DrawImage(pD3DDevice,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageUserRight.GetWidth(),				//宽
									g_CardResource.m_ImageUserRight.GetHeight(),				//高
									0,//起始位置X
									0);
			}

			//正常扑克
			if (m_wCardCount>0)
			{
				int nXPos=0,nYPos=0;
				for (WORD i=0;i<m_wCardCount;i++)
				{
					nXPos=m_ControlPoint.x;
					nYPos=m_ControlPoint.y+i*27+40;
					g_CardResource.m_ImageUserRight.DrawImage(pD3DDevice,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageUserRight.GetWidth(),				//宽
									g_CardResource.m_ImageUserRight.GetHeight(),				//高
									0,//起始位置X
									0);
				}
			}

			break;
		}
	case Direction_West:	//西向	左
		{
			//正常扑克
			if (m_wCardCount>0)
			{
				int nXPos=0,nYPos=0;
				for (WORD i=0;i<m_wCardCount;i++)
				{
					nXPos=m_ControlPoint.x;
					nYPos=m_ControlPoint.y-(m_wCardCount-i-1)*27-92;
					g_CardResource.m_ImageUserLeft.DrawImage(pD3DDevice,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageUserLeft.GetWidth(),				//宽
									g_CardResource.m_ImageUserLeft.GetHeight(),				//高
									0,//起始位置X
									0);
				}
			}

			//当前扑克
			if (m_bCurrentCard==true)
			{
				int nXPos=m_ControlPoint.x;
				int nYPos=m_ControlPoint.y-50;
				g_CardResource.m_ImageUserLeft.DrawImage(pD3DDevice,
									nXPos,	//X坐标
									nYPos,	//Y坐标
									g_CardResource.m_ImageUserLeft.GetWidth(),				//宽
									g_CardResource.m_ImageUserLeft.GetHeight(),				//高
									0,//起始位置X
									0);
			}

			break;
		}
	case Direction_North:	//北向
		{
			//当前扑克
			if (m_bCurrentCard==true)
			{
				int nXPos=m_ControlPoint.x;
				int nYPos=m_ControlPoint.y;
				g_CardResource.m_ImageUserTop.DrawImage(pD3DDevice,nXPos,nYPos);
			}

			//正常扑克
			if (m_wCardCount>0)
			{
				int nXPos=0,nYPos=0;
				for (WORD i=0;i<m_wCardCount;i++)
				{
					nYPos=m_ControlPoint.y;
					nXPos=m_ControlPoint.x+i*31+40;
					g_CardResource.m_ImageUserTop.DrawImage(pD3DDevice,nXPos,nYPos);
				}
			}

			break;
		}
	}

	return;
}

//设置扑克
bool CUserCard::SetCurrentCard(bool bCurrentCard)
{
	//设置变量
	m_bCurrentCard=bCurrentCard;

	return true;
}

//设置扑克
bool CUserCard::SetCardData(WORD wCardCount, bool bCurrentCard)
{
	//设置变量
	m_wCardCount=wCardCount;
	m_bCurrentCard=bCurrentCard;

	return true;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CDiscardCard::CDiscardCard()
{
	//扑克数据
	m_wCardCount=0;
	ZeroMemory(m_cbCardData,sizeof(m_cbCardData));

	//控制变量
	m_ControlPoint.SetPoint(0,0);
	m_CardDirection=Direction_East;

	return;
}

//析构函数
CDiscardCard::~CDiscardCard()
{
}

//绘画扑克
void CDiscardCard::DrawCardControl(CD3DDevice * pD3DDevice)
{
	//绘画控制
	switch (m_CardDirection)
	{
	case Direction_East:	//东向   右
		{
			//绘画扑克
			
			for (WORD i=0;i<m_wCardCount;i++)
			{
				if (i>=12) continue;
				int nXPos=m_ControlPoint.x+(i/12)*37;
				int nYPos=m_ControlPoint.y+(i%12)*18;
				g_CardResource.m_ImageTableRight.DrawCardItem(pD3DDevice,m_cbCardData[i],nXPos,nYPos);
			}

			for (WORD i=0;i<m_wCardCount;i++)
			{
				if (i<12) continue; 
				int nXPos=m_ControlPoint.x+(i/12)*37;
				int nYPos=m_ControlPoint.y+(i%12)*18;
				g_CardResource.m_ImageTableRight.DrawCardItem(pD3DDevice,m_cbCardData[i],nXPos,nYPos);
			}


			break;
		}
	case Direction_South:	//南向		下
		{
			//绘画扑克
			for (WORD i=0;i<m_wCardCount;i++)
			{
				int nXPos=m_ControlPoint.x-(i%12)*24;
				int nYPos=m_ControlPoint.y+(i/12)*32;
				g_CardResource.m_ImageTableBottom.DrawCardItem(pD3DDevice,m_cbCardData[i],nXPos,nYPos);
			}

			break;
		}
	case Direction_West:	//西向	左
		{
			//绘画扑克

			for (WORD i=0;i<m_wCardCount;i++)
			{
				if ((m_wCardCount-i-1)>=12) continue;
				int nXPos=m_ControlPoint.x-((m_wCardCount-1-i)/12)*37;
				int nYPos=m_ControlPoint.y-((m_wCardCount-1-i)%12)*18;
				g_CardResource.m_ImageTableLeft.DrawCardItem(pD3DDevice,m_cbCardData[m_wCardCount-i-1],nXPos,nYPos);
			}

			for (WORD i=0;i<m_wCardCount;i++)
			{
				if ((m_wCardCount-i-1)<12) continue; 
				int nXPos=m_ControlPoint.x-((m_wCardCount-1-i)/12)*37;
				int nYPos=m_ControlPoint.y-((m_wCardCount-1-i)%12)*18;
				g_CardResource.m_ImageTableLeft.DrawCardItem(pD3DDevice,m_cbCardData[m_wCardCount-i-1],nXPos,nYPos);
			}

			break;
		}
	case Direction_North:	//北向		上
		{
			//绘画扑克
			for (WORD i=0;i<m_wCardCount;i++)
			{
				int nXPos=m_ControlPoint.x+((m_wCardCount-1-i)%12)*24;
				int nYPos=m_ControlPoint.y-((m_wCardCount-1-i)/12)*32;
				g_CardResource.m_ImageTableTop.DrawCardItem(pD3DDevice,m_cbCardData[m_wCardCount-i-1],nXPos,nYPos);
			}

			break;
		}
	}

	return;
}

//增加扑克
bool CDiscardCard::AddCardItem(BYTE cbCardData)
{
	//清理扑克
	if (m_wCardCount>=CountArray(m_cbCardData))
	{
		m_wCardCount--;
		MoveMemory(m_cbCardData,m_cbCardData+1,CountArray(m_cbCardData)-1);
	}

	//设置扑克
	m_cbCardData[m_wCardCount++]=cbCardData;

	return true;
}

//设置扑克
bool CDiscardCard::SetCardData(const BYTE cbCardData[], WORD wCardCount)
{
	//效验大小
	ASSERT(wCardCount<=CountArray(m_cbCardData));
	if (wCardCount>CountArray(m_cbCardData))
		return false;

	//设置扑克
	m_wCardCount=wCardCount;
	CopyMemory(m_cbCardData,cbCardData,sizeof(m_cbCardData[0])*wCardCount);

	return true;
}
//获取位置
CPoint CDiscardCard::GetLastCardPosition()
{
	switch (m_CardDirection)
	{
	case Direction_East:	//东向   右
		{
			//变量定义
			int nCellWidth=g_CardResource.m_ImageTableRight.GetViewWidth();
			int nCellHeight=g_CardResource.m_ImageTableRight.GetViewHeight();
			int nXPos=m_ControlPoint.x+((m_wCardCount-1)/12)*37+18;
			int nYPos=m_ControlPoint.y+((m_wCardCount-1)%12)*18+9;

			return CPoint(nXPos,nYPos);
		}
	case Direction_South:	//南向	下
		{
			//变量定义
			int nCellWidth=g_CardResource.m_ImageTableBottom.GetViewWidth();
			int nCellHeight=g_CardResource.m_ImageTableBottom.GetViewHeight();
			int nXPos=m_ControlPoint.x-((m_wCardCount-1)%12)*24+12;
			int nYPos=m_ControlPoint.y+((m_wCardCount-1)/12)*32+16;

			return CPoint(nXPos,nYPos);
		}
	case Direction_West:	//西向	左
		{
			//变量定义
			int nCellWidth=g_CardResource.m_ImageTableLeft.GetViewWidth();
			int nCellHeight=g_CardResource.m_ImageTableLeft.GetViewHeight();
			int nXPos=m_ControlPoint.x-((m_wCardCount-1)/12)*37+18;
			int nYPos=m_ControlPoint.y-((m_wCardCount-1)%12)*18+9;

			return CPoint(nXPos,nYPos);
		}
	case Direction_North:	//北向	上
		{
			//变量定义
			int nCellWidth=g_CardResource.m_ImageTableTop.GetViewWidth();
			int nCellHeight=g_CardResource.m_ImageTableTop.GetViewHeight();         
			int nXPos=m_ControlPoint.x+((m_wCardCount-1)%12)*24+12;
			int nYPos=m_ControlPoint.y+((-m_wCardCount+1)/12)*32+16;

			return CPoint(nXPos,nYPos);
		}
	}
	return CPoint(0,0);

}

//////////////////////////////////////////////////////////////////////////

//构造函数
CTableCard::CTableCard()
{
	//扑克数据
	m_wCardCount=0;
	ZeroMemory(m_cbCardData,sizeof(m_cbCardData));

	//控制变量
	m_ControlPoint.SetPoint(0,0);
	m_CardDirection=Direction_East;

	return;
}

//析构函数
CTableCard::~CTableCard()
{
}

//绘画扑克
void CTableCard::DrawCardControl(CD3DDevice * pD3DDevice)
{
	//绘画控制
	switch (m_CardDirection)
	{
	case Direction_East:	//东向   右
		{
			//绘画扑克
			for (WORD i=0;i<m_wCardCount;i++)
			{
				int nXPos=m_ControlPoint.x-33;
				int nYPos=m_ControlPoint.y+i*17;
				g_CardResource.m_ImageTableRight.DrawCardItem(pD3DDevice,m_cbCardData[m_wCardCount-i-1],nXPos,nYPos);
			}

			break;
		}
	case Direction_South:	//南向	下
		{
			//绘画扑克
			for (WORD i=0;i<m_wCardCount;i++)
			{
				int nYPos=m_ControlPoint.y;
				int nXPos=m_ControlPoint.x-(m_wCardCount-i)*45;
				g_CardResource.m_TextureUserTableBottom.DrawCardItem(pD3DDevice,m_cbCardData[i],nXPos,nYPos);
			}

			break;
		}
	case Direction_West:	//西向	左
		{
			//绘画扑克
			for (WORD i=0;i<m_wCardCount;i++)
			{
				int nXPos=m_ControlPoint.x;
				int nYPos=m_ControlPoint.y-(m_wCardCount-i)*17;
				g_CardResource.m_ImageTableLeft.DrawCardItem(pD3DDevice,m_cbCardData[i],nXPos,nYPos);
			}

			break;
		}
	case Direction_North:	//北向	上
		{
			//绘画扑克
			for (WORD i=0;i<m_wCardCount;i++)
			{
				int nYPos=m_ControlPoint.y;
				int nXPos=m_ControlPoint.x+i*24;
				g_CardResource.m_ImageTableTop.DrawCardItem(pD3DDevice,m_cbCardData[m_wCardCount-i-1],nXPos,nYPos);
			}

			break;
		}
	}

	return;
}

//设置扑克
bool CTableCard::SetCardData(const BYTE cbCardData[], WORD wCardCount)
{
	//效验大小
	ASSERT(wCardCount<=CountArray(m_cbCardData));
	if (wCardCount>CountArray(m_cbCardData))
		return false;

	//设置扑克
	m_wCardCount=wCardCount;
	CopyMemory(m_cbCardData,cbCardData,sizeof(m_cbCardData[0])*wCardCount);

	return true;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CCardControl::CCardControl()
{
	//状态变量
	m_bPositively=false;
	m_bDisplayItem=false;

	//位置变量
	m_XCollocateMode=enXCenter;
	m_YCollocateMode=enYCenter;
	m_BenchmarkPos.SetPoint(0,0);

	//扑克数据
	m_wCardCount=0;
	m_wHoverItem=INVALID_ITEM;
	ZeroMemory(&m_CurrentCard,sizeof(m_CurrentCard));
	ZeroMemory(&m_CardItemArray,sizeof(m_CardItemArray));

	//加载设置
	m_ControlPoint.SetPoint(0,0);
	m_ControlSize.cy=CARD_HEIGHT+POS_SHOOT;
	m_ControlSize.cx=(CountArray(m_CardItemArray)+1)*CARD_WIDTH+POS_SPACE;

	return;
}

//析构函数
CCardControl::~CCardControl()
{
}

//基准位置
void CCardControl::SetBenchmarkPos(int nXPos, int nYPos, enXCollocateMode XCollocateMode, enYCollocateMode YCollocateMode)
{
	//设置变量
	m_BenchmarkPos.x=nXPos;
	m_BenchmarkPos.y=nYPos;
	m_XCollocateMode=XCollocateMode;
	m_YCollocateMode=YCollocateMode;

	//横向位置
	switch (m_XCollocateMode)
	{
	case enXLeft:	{ m_ControlPoint.x=m_BenchmarkPos.x; break; }
	case enXCenter: { m_ControlPoint.x=m_BenchmarkPos.x-m_ControlSize.cx/2; break; }
	case enXRight:	{ m_ControlPoint.x=m_BenchmarkPos.x-m_ControlSize.cx; break; }
	}

	//竖向位置
	switch (m_YCollocateMode)
	{
	case enYTop:	{ m_ControlPoint.y=m_BenchmarkPos.y; break; }
	case enYCenter: { m_ControlPoint.y=m_BenchmarkPos.y-m_ControlSize.cy/2; break; }
	case enYBottom: { m_ControlPoint.y=m_BenchmarkPos.y-m_ControlSize.cy; break; }
	}

	return;
}

//基准位置
void CCardControl::SetBenchmarkPos(const CPoint & BenchmarkPos, enXCollocateMode XCollocateMode, enYCollocateMode YCollocateMode)
{
	//设置变量
	m_BenchmarkPos=BenchmarkPos;
	m_XCollocateMode=XCollocateMode;
	m_YCollocateMode=YCollocateMode;

	//横向位置
	switch (m_XCollocateMode)
	{
	case enXLeft:	{ m_ControlPoint.x=m_BenchmarkPos.x; break; }
	case enXCenter: { m_ControlPoint.x=m_BenchmarkPos.x-m_ControlSize.cx/2; break; }
	case enXRight:	{ m_ControlPoint.x=m_BenchmarkPos.x-m_ControlSize.cx; break; }
	}

	//竖向位置
	switch (m_YCollocateMode)
	{
	case enYTop:	{ m_ControlPoint.y=m_BenchmarkPos.y; break; }
	case enYCenter: { m_ControlPoint.y=m_BenchmarkPos.y-m_ControlSize.cy/2; break; }
	case enYBottom: { m_ControlPoint.y=m_BenchmarkPos.y-m_ControlSize.cy; break; }
	}

	return;
}

//获取扑克
BYTE CCardControl::GetHoverCard()
{
	//获取扑克
	if (m_wHoverItem!=INVALID_ITEM)
	{
		if (m_wHoverItem==CountArray(m_CardItemArray))
			return m_CurrentCard.cbCardData;
		return m_CardItemArray[m_wHoverItem].cbCardData;
	}

	return 0;
}

//设置扑克
bool CCardControl::SetCurrentCard(BYTE cbCardData)
{
	//设置变量
	m_CurrentCard.bShoot=false;
	m_CurrentCard.cbCardData=cbCardData;

	return true;
}

//设置扑克
bool CCardControl::SetCurrentCard(tagCardItem CardItem)
{
	//设置变量
	m_CurrentCard.bShoot=CardItem.bShoot;
	m_CurrentCard.cbCardData=CardItem.cbCardData;

	return true;
}

//设置扑克
bool CCardControl::SetCardData(const BYTE cbCardData[], WORD wCardCount, BYTE cbCurrentCard)
{
	//效验大小
	ASSERT(wCardCount<=CountArray(m_CardItemArray));
	if (wCardCount>CountArray(m_CardItemArray)) 
		return false;

	//当前扑克
	m_CurrentCard.bShoot=false;
	m_CurrentCard.cbCardData=cbCurrentCard;

	//设置扑克
	m_wCardCount=wCardCount;
	for (WORD i=0;i<m_wCardCount;i++)
	{
		m_CardItemArray[i].bShoot=false;
		m_CardItemArray[i].cbCardData=cbCardData[i];
	}

	return true;
}

//设置扑克
bool CCardControl::SetCardItem(const tagCardItem CardItemArray[], WORD wCardCount)
{
	//效验大小
	ASSERT(wCardCount<=CountArray(m_CardItemArray));
	if (wCardCount>CountArray(m_CardItemArray))
		return false;

	//设置扑克
	m_wCardCount=wCardCount;
	for (WORD i=0;i<m_wCardCount;i++)
	{
		m_CardItemArray[i].bShoot=CardItemArray[i].bShoot;
		m_CardItemArray[i].cbCardData=CardItemArray[i].cbCardData;
	}

	return true;
}

//绘画扑克
void CCardControl::DrawCardControl(CD3DDevice * pD3DDevice)
{
	//绘画准备
	int nXExcursion=m_ControlPoint.x+(CountArray(m_CardItemArray)-m_wCardCount)*CARD_WIDTH;

	//绘画扑克
	for (WORD i=0;i<m_wCardCount;i++)
	{
		//计算位置
		int nXScreenPos=nXExcursion+CARD_WIDTH*i;
		int nYScreenPos=m_ControlPoint.y+(((m_CardItemArray[i].bShoot==false)&&(m_wHoverItem!=i))?POS_SHOOT:0);

		//绘画扑克
		BYTE cbCardData=(m_bDisplayItem==true)?m_CardItemArray[i].cbCardData:0;
		g_CardResource.m_ImageUserBottom.DrawCardItem(pD3DDevice,cbCardData,nXScreenPos,nYScreenPos);
	}

	//当前扑克
	if (m_CurrentCard.cbCardData!=0)
	{
		//计算位置
		int nXScreenPos=m_ControlPoint.x+m_ControlSize.cx-CARD_WIDTH;
		int nYScreenPos=m_ControlPoint.y+(((m_CurrentCard.bShoot==false)&&(m_wHoverItem!=CountArray(m_CardItemArray)))?POS_SHOOT:0);

		//绘画扑克
		BYTE cbCardData=(m_bDisplayItem==true)?m_CurrentCard.cbCardData:0;
		g_CardResource.m_ImageUserBottom.DrawCardItem(pD3DDevice,cbCardData,nXScreenPos,nYScreenPos);
	}

	return;
}

//索引切换
WORD CCardControl::SwitchCardPoint(CPoint & MousePoint)
{
	//基准位置
	int nXPos=MousePoint.x-m_ControlPoint.x;
	int nYPos=MousePoint.y-m_ControlPoint.y;

	//范围判断
	if ((nXPos<0)||(nXPos>m_ControlSize.cx)) 
		return INVALID_ITEM;
	if ((nYPos<POS_SHOOT)||(nYPos>m_ControlSize.cy)) 
		return INVALID_ITEM;

	//牌列子项
	if (nXPos<CARD_WIDTH*CountArray(m_CardItemArray))
	{
		WORD wViewIndex=(WORD)(nXPos/CARD_WIDTH)+m_wCardCount;
		if (wViewIndex>=CountArray(m_CardItemArray))
			return wViewIndex-CountArray(m_CardItemArray);
		return INVALID_ITEM;
	}

	//当前子项
	if ((m_CurrentCard.cbCardData!=0)&&(nXPos>=(m_ControlSize.cx-CARD_WIDTH))) 
		return CountArray(m_CardItemArray);

	return INVALID_ITEM;
}

//光标消息
bool CCardControl::OnEventSetCursor(CPoint Point, bool & bRePaint)
{
	//获取索引
	WORD wHoverItem=SwitchCardPoint(Point);

	//响应判断
	if ((m_bPositively==false)&&(m_wHoverItem!=INVALID_ITEM))
	{
		bRePaint=true;
		m_wHoverItem=INVALID_ITEM;
	}

	//更新判断
	if ((wHoverItem!=m_wHoverItem)&&(m_bPositively==true))
	{
		bRePaint=true;
		m_wHoverItem=wHoverItem;
		SetCursor(LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_CARD_CUR)));
	}

	return (wHoverItem!=INVALID_ITEM);
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//变量声明
CCardResource						g_CardResource;						//扑克资源

//////////////////////////////////////////////////////////////////////////
