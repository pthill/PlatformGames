#pragma once

#include "Stdafx.h"
#include "ControlWnd.h"
#include "CardControl.h"
#include "ScoreControl.h"
#include "HistoryScore.h"
#include "DrawSiceWnd.h"

//////////////////////////////////////////////////////////////////////////

#define FRAME_HEIGHT					38								//框架头高
#define FRAME_CAPTION					30								//框架标题

//////////////////////////////////////////////////////////////////////////
//消息定义

#define IDM_START					(WM_USER+100)						//开始消息
#define IDM_CALL_CARD				(WM_USER+101)						//选缺消息
#define IDM_OUT_CARD				(WM_USER+102)						//出牌消息
#define IDM_TRUSTEE_CONTROL			(WM_USER+103)						//托管控制
#define WM_SET_CAPTION				(WM_USER+229)						//窗口消息
#define IDM_LBUTTONDOWN_VIEW		(WM_USER+220)						//左键视图

#define IDI_DISC_EFFECT				102								//丢弃效果

//////////////////////////////////////////////////////////////////////////

//游戏视图
class CGameClientView : public CGameFrameViewD3D
{
	//标志变量
protected:
	bool							m_bOutCard;							//出牌标志
	bool							m_bWaitOther;						//等待标志
	bool							m_bTrustee[GAME_PLAYER];			//是否托管
	LONGLONG						m_lGangScore[GAME_PLAYER];			//杠牌计数
	bool							m_bShowSelectTip;					//显示定缺
	bool							m_bShowWaitChooseTip;				//等待定缺
	BYTE							m_cbVoidShowing[GAME_PLAYER];		//缺门标志

	//游戏属性
protected:
	LONGLONG						m_lCellScore;						//基础积分
	WORD							m_wBankerUser;						//庄家用户
	WORD							m_wCurrentUser;						//当前用户
	BYTE							m_cbWinOrder[GAME_PLAYER];			//胡牌顺序

	//历史积分
public:
	bool							m_bShowScore;						//成绩标志
	tagHistoryScore	*				m_pHistoryScore[GAME_PLAYER];		//成绩信息

	//动作动画
protected:
	bool							m_bBombEffect;						//动作效果
	BYTE							m_cbBombFrameIndex;					//帧数索引

	//丢弃效果
	WORD							m_wDiscUser;						//丢弃用户
	BYTE							m_cbDiscFrameIndex;					//帧数索引

	//杠牌动画
	int								m_nYGangStep;						//动画步数
	BYTE							m_cbXiaYu;							//刮风下雨
	WORD							m_wGangChairId;						//引杠视图
	BYTE							m_cbGangIndex;						//引杠索引
	CLapseCount						m_CartoonGangLapseCount;			//流逝计数

	//用户状态
protected:
	BYTE							m_cbCardData;						//出牌扑克
	WORD							m_wOutCardUser;						//出牌用户
	BYTE							m_cbUserAction[GAME_PLAYER];		//用户动作

	//位置变量
protected:
	CPoint							m_ptUserFrame[GAME_PLAYER];			//框架位置
	CPoint							m_UserFlagPos[GAME_PLAYER];			//标志位置
	CPoint							m_PointTrustee[GAME_PLAYER];		//托管位置
	CPoint							m_ptWinOrder[GAME_PLAYER];			//顺序位置
	CPoint							m_ptGangScore[GAME_PLAYER];			//杠分位置
	CPoint							m_ptVoidShowPos[GAME_PLAYER];		//缺门位置

	//图片资源
protected:
	CD3DTexture						m_ImageBack;						//背景图案
	CD3DTexture						m_ImageCenter;						//LOGO  图
	CD3DTexture						m_ImageWait;						//等待提示
	CD3DTexture						m_ImageOutCard;						//出牌提示
	CD3DTexture						m_ImageUserFlag;					//用户标志
	CD3DTexture						m_ImageActionBack;					//动作背景
	CD3DTexture						m_ImageTrustee;						//托管标志
	CD3DTexture						m_ImageActionAni;					//吃牌动画资源
	CD3DTexture						m_ImageDisc;						//丢弃效果
	CD3DTexture						m_ImageArrow;						//定时器箭头	
	CD3DTexture						m_ImageWinOrder;					//胡牌顺序
	CD3DTexture						m_ImageCellScore;					//图片资源
	CD3DTexture						m_PngNumWin;						//图片资源
	CD3DTexture						m_PngNumLost;						//图片资源
	CD3DTexture						m_PngXiaYu;							//图片资源
	CD3DTexture						m_PngGuaFeng;						//图片资源
	CD3DTexture						m_TextureUserFrameH;				//用户框架
	CD3DTexture						m_TextureUserFrameV;				//用户框架
	CD3DTexture						m_TextureVoidShowing;				//缺门标志
	CD3DTexture						m_TextureVoidShowingTip;			//缺门标志
	CD3DTexture						m_TextureWaitChooseTip;				//定缺等待
	CD3DTexture						m_TextureHistoryScore;				//历史成绩

	//缺门按钮
public:
	CVirtualButton					m_btCallCard1;						//缺门按钮
	CVirtualButton					m_btCallCard2;						//缺门按钮
	CVirtualButton					m_btCallCard3;						//缺门按钮
	CVirtualButton					m_btOpenHistoryScore;				//积分按钮
	CVirtualButton					m_btCloseHistoryScore;				//积分按钮

	//扑克控件
public:
	CHeapCard						m_HeapCard[GAME_PLAYER];			//堆立扑克
	CUserCard						m_UserCard[GAME_PLAYER];			//用户扑克
	CTableCard						m_TableCard[GAME_PLAYER];			//桌面扑克
	CWeaveCard						m_WeaveCard[GAME_PLAYER][4];		//组合扑克
	CDiscardCard					m_DiscardCard[GAME_PLAYER];			//丢弃扑克
	CCardControl					m_HandCardControl;					//手上扑克

	//控件变量
public:
	CVirtualButton					m_btStart;							//开始按钮
	CVirtualButton					m_btStustee;						//拖管控制
	CVirtualButton					m_btCancelStustee;					//拖管控制
	CControlWnd						m_ControlWnd;						//控制窗口
	CScoreControl					m_ScoreControl[GAME_PLAYER];		//积分控件
	CDrawSiceWnd					m_DrawSiceWnd;						//色子控件

protected:
	CString							m_StringCaption;					//标题
	CDFontEx						m_FontCaption;						//标题字体
	CD3DTexture						m_ImageGameFrame[8];				//游戏框架
	
	//控件变量
protected:
	CSkinButton						m_btMin;							//最小化按钮
	CSkinButton						m_btClose;							//关闭按钮
	CSkinButton						m_btBank;							//银行按钮

	//函数定义
public:
	//构造函数
	CGameClientView();
	//析构函数
	virtual ~CGameClientView();

	//重载函数
private:
	//重置界面
	virtual VOID ResetGameView();
	//调整控件
	virtual VOID RectifyControl(INT nWidth, INT nHeight);

	//界面函数
protected:
	//动画驱动
	virtual VOID CartoonMovie();
	//配置设备
	virtual VOID InitGameView(CD3DDevice * pD3DDevice, INT nWidth, INT nHeight);
	//绘画界面
	virtual VOID DrawGameView(CD3DDevice * pD3DDevice, INT nWidth, INT nHeight);
	//绘画框架
	virtual VOID DrawGameFrame(CD3DDevice *pD3DDevice, int nWidth, int nHeight);

	//功能函数
public:
	//基础积分
	void SetCellScore(LONGLONG lCellScore);
	//庄家用户
	void SetBankerUser(WORD wBankerUser);
	//状态标志
	void SetStatusFlag(bool bOutCard, bool bWaitOther);
	//出牌信息
	void SetOutCardInfo(WORD wViewChairID, BYTE cbCardData);
	//动作信息
	void SetUserAction(WORD wViewChairID, BYTE bUserAction);
	//设置动作
	bool SetBombEffect(bool bBombEffect);
	//丢弃用户
	void SetDiscUser(WORD wDiscUser);
	//定时玩家
	void SetCurrentUser(WORD wCurrentUser);
	//设置托管
	void SetTrustee(WORD wTrusteeUser,bool bTrustee);
	//设置顺序
	void SetWinOrder( WORD wChairId, BYTE cbWinOrder );
	//设置杠分
	void SetGangScore( WORD wChairId, BYTE cbXiaYu, LONGLONG lGangScore[GAME_PLAYER] );
	//设置缺门
	void SetVoidShowing(WORD wChairID, BYTE cbShowIndex);
	//设置积分
	VOID SetHistoryScore(WORD wChairID, tagHistoryScore * pHistoryScore);
	//显示积分
	VOID ShowGameScoreInfo();
	//显示定缺
	void ShowSelectTip(bool bShow);
	//显示定缺
	void ShowWaitChooseTip(bool bShow);

	//辅助函数
protected:
	//绘画数字
	void DrawNumberString(CD3DDevice * pD3DDevice, CD3DTexture* ImageNumber, TCHAR * szImageNum, SCORE lOutNum, INT nXPos, INT nYPos, UINT uFormat /*= DT_LEFT*/);
	//绘画小数点
	void DrawDecimalString(CD3DDevice * pD3DDevice, CD3DTexture* ImageNumber, TCHAR * szImageNum, SCORE lOutNum, INT nXPos, INT nYPos, UINT uFormat /*= DT_LEFT*/);
	// 添加小数点
	CString AddDecimal( LONGLONG lScore ,  bool bComma = true, bool bPlus = false );

	//消息映射
protected:
	//开始按钮
	afx_msg void OnStart();
	//定缺按钮
	afx_msg void OnCallCard1();
	//定缺按钮
	afx_msg void OnCallCard2();
	//定缺按钮
	afx_msg void OnCallCard3();
	//拖管控制
	afx_msg void OnStusteeControl();
	//积分按钮
	afx_msg void OnBnClickedOpenHistory();
	//积分按钮
	afx_msg void OnBnClickedCloseHistory();
	//建立函数
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//鼠标消息
	afx_msg void OnLButtonDown(UINT nFlags, CPoint Point);
	//光标消息
	afx_msg BOOL OnSetCursor(CWnd * pWnd, UINT nHitTest, UINT uMessage);
	//标题
	afx_msg LRESULT OnSetCaption(WPARAM wParam, LPARAM lParam);
	//最小按钮	
	afx_msg void OnButtonMin();
	//关闭按钮	
	afx_msg void OnButtonClose();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnTimer(UINT nIDEvent);
};

//////////////////////////////////////////////////////////////////////////
