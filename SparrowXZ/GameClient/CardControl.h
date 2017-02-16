#ifndef CARD_CONTROL_HEAD_FILE
#define CARD_CONTROL_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "Resource.h"
#include "GameLogic.h"
#include "cassert"

//////////////////////////////////////////////////////////////////////////

//X 排列方式
enum enXCollocateMode 
{ 
	enXLeft,						//左对齐
	enXCenter,						//中对齐
	enXRight,						//右对齐
};

//Y 排列方式
enum enYCollocateMode 
{ 
	enYTop,							//上对齐
	enYCenter,						//中对齐
	enYBottom,						//下对齐
};

//方向枚举
enum enDirection
{
	Direction_East,					//东向   右
	Direction_South,				//南向
	Direction_West,					//西向
	Direction_North,				//北向
};

//////////////////////////////////////////////////////////////////////////

//扑克图片
class CCardListImage
{
	//位置变量
protected:
	int								m_nItemWidth;						//子项高度
	int								m_nItemHeight;						//子项宽度
	int								m_nViewWidth;						//子项高度
	int								m_nViewHeight;						//子项宽度

	//资源变量
protected:
	CD3DTexture						m_CardListImage;					//图片资源
					

	//函数定义
public:
	//构造函数
	CCardListImage();
	//析构函数
	virtual ~CCardListImage();

	//信息函数
public:
	//获取宽度
	int GetViewWidth() { return m_nViewWidth; }
	//获取高度
	int GetViewHeight() { return m_nViewHeight; }

	//资源管理
public:
	//加载资源
	bool LoadResource(CD3DDevice * pD3DDevice, LPCTSTR uResourceID, int nViewWidth, int nViewHeight);
	//释放资源
	bool DestroyResource();

	//功能函数
public:
	//获取位置
	inline int GetImageIndex(BYTE cbCardData);
	//绘画扑克
	inline bool DrawCardItem(CD3DDevice * pD3DDevice, BYTE cbCardData, int xDest, int yDest);
};

//////////////////////////////////////////////////////////////////////////

//扑克资源
class CCardResource
{
	//用户扑克
public:
	CD3DTexture						m_ImageUserTop;						//图片资源
	CCardListImage					m_ImageUserBottom;					//图片资源

	CD3DTexture						m_ImageUserLeft;					//图片资源
	CD3DTexture						m_ImageUserRight;					//图片资源

	//桌子扑克
public:
	CCardListImage					m_ImageTableTop;					//图片资源
	CCardListImage					m_ImageTableLeft;					//图片资源
	CCardListImage					m_ImageTableRight;					//图片资源
	CCardListImage					m_ImageTableBottom;					//图片资源
	CCardListImage					m_TextureUserTableBottom;			//图片资源

	//牌堆扑克
public:
	CD3DTexture						m_ImageHeapDoubleLV;				//左边的堆牌
	CD3DTexture						m_ImageHeapSingleLV;				//左边的堆牌

	CD3DTexture						m_ImageHeapDoubleRV;				//右边的堆牌
	CD3DTexture						m_ImageHeapSingleRV;				//右边的堆牌

	CD3DTexture						m_ImageHeapDoubleTV;				//上边的堆牌
	CD3DTexture						m_ImageHeapSingleTV;				//上边的堆牌

	CD3DTexture						m_ImageHeapDoubleBV;				//下边的堆牌
	CD3DTexture						m_ImageHeapSingleBV;				//下边的堆牌


	//函数定义
public:
	//构造函数
	CCardResource();
	//析构函数
	virtual ~CCardResource();

	//功能函数
public:
	//加载资源
	bool LoadResource(CD3DDevice * pD3DDevice);
	//消耗资源
	bool DestroyResource();
};

//////////////////////////////////////////////////////////////////////////

//堆立扑克
class CHeapCard
{
	//控制变量
protected:
	CPoint							m_ControlPoint;						//基准位置
	enDirection						m_CardDirection;					//扑克方向

	//扑克变量
protected:
	WORD							m_wFullCount;						//全满数目
	WORD							m_wMinusHeadCount;					//头部空缺
	WORD							m_wMinusLastCount;					//尾部空缺

	//函数定义
public:
	//构造函数
	CHeapCard();
	//析构函数
	virtual ~CHeapCard();

	//功能函数
public:
	//绘画扑克
	void DrawCardControl(CD3DDevice * pD3DDevice);
	//设置扑克
	bool SetCardData(WORD wMinusHeadCount, WORD wMinusLastCount, WORD wFullCount);

	//控件控制
public:
	//设置方向
	void SetDirection(enDirection Direction) { m_CardDirection=Direction; }
	//基准位置
	void SetControlPoint(int nXPos, int nYPos) { m_ControlPoint.SetPoint(nXPos,nYPos); }
};

//////////////////////////////////////////////////////////////////////////

//组合扑克
class CWeaveCard
{
	//控制变量
protected:
	bool							m_bDisplayItem;						//显示标志
	CPoint							m_ControlPoint;						//基准位置
	enDirection						m_CardDirection;					//扑克方向
	BYTE							m_cbDirectionCardPos;				//方向扑克
	bool							m_bGameEnd;							//

	//扑克数据
protected:
	WORD							m_wCardCount;						//扑克数目
	BYTE							m_cbCardData[4];					//扑克数据

	//函数定义
public:
	//构造函数
	CWeaveCard();
	//析构函数
	virtual ~CWeaveCard();

	//功能函数
public:
	//绘画扑克
	void DrawCardControl(CD3DDevice * pD3DDevice);
	//绘画扑克
	void DrawCardControl(CD3DDevice * pD3DDevice, int nXPos, int nYPos);
	//设置扑克
	bool SetCardData(const BYTE cbCardData[], WORD wCardCount);

	//控件控制
public:
	//设置显示
	void SetDisplayItem(bool bDisplayItem) { m_bDisplayItem=bDisplayItem; }
	//设置方向
	void SetDirection(enDirection Direction) { m_CardDirection=Direction; }
	//基准位置
	void SetControlPoint(int nXPos, int nYPos) { m_ControlPoint.SetPoint(nXPos,nYPos); }
	//方向牌
	void SetDirectionCardPos(BYTE cbPos) { assert(cbPos <= 3); m_cbDirectionCardPos = cbPos;}
	//
	void SetGameEnd( bool bGameEnd ) { m_bGameEnd = bGameEnd; }

	//内部函数
private:
	//获取扑克
	inline BYTE GetCardData(WORD wIndex);
};

//////////////////////////////////////////////////////////////////////////

//用户扑克
class CUserCard
{
	//扑克数据
protected:
	WORD							m_wCardCount;						//扑克数目
	bool							m_bCurrentCard;						//当前标志

	//控制变量
protected:
	CPoint							m_ControlPoint;						//基准位置
	enDirection						m_CardDirection;					//扑克方向

	//函数定义
public:
	//构造函数
	CUserCard();
	//析构函数
	virtual ~CUserCard();

	//功能函数
public:
	//绘画扑克
	void DrawCardControl(CD3DDevice * pD3DDevice);
	//设置扑克
	bool SetCurrentCard(bool bCurrentCard);
	//设置扑克
	bool SetCardData(WORD wCardCount, bool bCurrentCard);

	//控件控制
public:
	//设置方向
	void SetDirection(enDirection Direction) { m_CardDirection=Direction; }
	//基准位置
	void SetControlPoint(int nXPos, int nYPos) { m_ControlPoint.SetPoint(nXPos,nYPos); }
};

//////////////////////////////////////////////////////////////////////////

//丢弃扑克
class CDiscardCard
{
	//扑克数据
protected:
	WORD							m_wCardCount;						//扑克数目
	BYTE							m_cbCardData[24];					//扑克数据

	//控制变量
protected:
	CPoint							m_ControlPoint;						//基准位置
	enDirection						m_CardDirection;					//扑克方向

	//函数定义
public:
	//构造函数
	CDiscardCard();
	//析构函数
	virtual ~CDiscardCard();

	//功能函数
public:
	//绘画扑克
	void DrawCardControl(CD3DDevice * pD3DDevice);
	//增加扑克
	bool AddCardItem(BYTE cbCardData);
	//设置扑克
	bool SetCardData(const BYTE cbCardData[], WORD wCardCount);
	//获取位置
	CPoint GetLastCardPosition();

	//控件控制
public:
	//设置方向
	void SetDirection(enDirection Direction) { m_CardDirection=Direction; }
	//基准位置
	void SetControlPoint(int nXPos, int nYPos) { m_ControlPoint.SetPoint(nXPos,nYPos); }
};

//////////////////////////////////////////////////////////////////////////

//桌面扑克
class CTableCard
{
	//扑克数据
protected:
	WORD							m_wCardCount;						//扑克数目
	BYTE							m_cbCardData[14];					//扑克数据

	//控制变量
protected:
	CPoint							m_ControlPoint;						//基准位置
	enDirection						m_CardDirection;					//扑克方向

	//函数定义
public:
	//构造函数
	CTableCard();
	//析构函数
	virtual ~CTableCard();

	//功能函数
public:
	//绘画扑克
	void DrawCardControl(CD3DDevice * pD3DDevice);
	//设置扑克
	bool SetCardData(const BYTE cbCardData[], WORD wCardCount);

	//控件控制
public:
	//设置方向
	void SetDirection(enDirection Direction) { m_CardDirection=Direction; }
	//基准位置
	void SetControlPoint(int nXPos, int nYPos) { m_ControlPoint.SetPoint(nXPos,nYPos); }
};

//////////////////////////////////////////////////////////////////////////

//扑克结构
struct tagCardItem
{
	bool							bShoot;								//弹起标志
	BYTE							cbCardData;							//扑克数据
};

//扑克控件
class CCardControl
{
	//状态变量
protected:
	bool							m_bPositively;						//响应标志
	bool							m_bDisplayItem;						//显示标志

	//位置变量
protected:
	CPoint							m_BenchmarkPos;						//基准位置
	enXCollocateMode				m_XCollocateMode;					//显示模式
	enYCollocateMode				m_YCollocateMode;					//显示模式

	//扑克数据
protected:
	WORD							m_wCardCount;						//扑克数目
	WORD							m_wHoverItem;						//盘旋子项
	tagCardItem						m_CurrentCard;						//当前扑克
	tagCardItem						m_CardItemArray[13];				//扑克数据

	//资源变量
protected:
	CSize							m_ControlSize;						//控件大小
	CPoint							m_ControlPoint;						//控件位置

	//函数定义
public:
	//构造函数
	CCardControl();
	//析构函数
	virtual ~CCardControl();

	//查询函数
public:
	//获取扑克
	BYTE GetHoverCard();
	//获取扑克
	BYTE GetCurrentCard() { return m_CurrentCard.cbCardData; }

	//扑克控制
public:
	//设置扑克
	bool SetCurrentCard(BYTE cbCardData);
	//设置扑克
	bool SetCurrentCard(tagCardItem CardItem);
	//设置扑克
	bool SetCardData(const BYTE cbCardData[], WORD wCardCount, BYTE cbCurrentCard);
	//设置扑克
	bool SetCardItem(const tagCardItem CardItemArray[], WORD wCardCount);

	//控件控制
public:
	//设置响应
	void SetPositively(bool bPositively) { m_bPositively=bPositively; }
	//设置显示
	void SetDisplayItem(bool bDisplayItem) { m_bDisplayItem=bDisplayItem; }
	//基准位置
	void SetBenchmarkPos(int nXPos, int nYPos, enXCollocateMode XCollocateMode, enYCollocateMode YCollocateMode);
	//基准位置
	void SetBenchmarkPos(const CPoint & BenchmarkPos, enXCollocateMode XCollocateMode, enYCollocateMode YCollocateMode);

	//事件控制
public:
	//绘画扑克
	void DrawCardControl(CD3DDevice * pD3DDevice);
	//光标消息
	bool OnEventSetCursor(CPoint Point, bool & bRePaint);

	//内部函数
private:
	//索引切换
	WORD SwitchCardPoint(CPoint & MousePoint);
};

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

//变量声明
//导出变量
extern CCardResource				g_CardResource;						//扑克资源

//////////////////////////////////////////////////////////////////////////

#endif
