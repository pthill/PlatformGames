#ifndef SCORE_CONTROL_HEAD_FILE
#define SCORE_CONTROL_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "CardControl.h"
#include "GameLogic.h"

//////////////////////////////////////////////////////////////////////////

//积分信息
struct tagScoreInfo
{
	//用户信息
	WORD							wMeChairId;								//当前用户
	BYTE							cbWinOrder;								//胡牌排名
	TCHAR							szUserName[GAME_PLAYER][32];			//用户名字

	//积分信息
	LONG							lGameTax;								//游戏税率
	LONGLONG						lGameScore;								//游戏积分
	LONGLONG						lGangScore;								//详细得分
	WORD							wProvideUser;							//供应用户
	DWORD							dwChiHuRight[MAX_RIGHT_COUNT];			//胡牌类型

	
	BYTE							cbGenCount;								//有根数目
	WORD							wLostFanShu[GAME_PLAYER];				//番数计算
	LONGLONG						lHuaZhuScore[GAME_PLAYER];				//花猪得分
	LONGLONG						lChaJiaoScore[GAME_PLAYER];				//查叫得分
	LONGLONG						lLostHuaZhuScore[GAME_PLAYER];			//花猪得分
	LONGLONG						lLostChaJiaoScore[GAME_PLAYER];			//查叫得分
};

//////////////////////////////////////////////////////////////////////////

//积分控件
class CScoreControl : public CVirtualWindow
{
	//变量定义
protected:
	tagScoreInfo					m_ScoreInfo;							//积分信息
	CChiHuRight						m_ChiHuRight;							//胡牌类型
	CD3DFont						m_FontScore;							//积分字体

	//资源变量
protected:
	CD3DTexture						m_ImageGameWin;							//积分视图
	CD3DTexture						m_ImageGameLost;						//积分视图
	CD3DTexture						m_PngNum;								//积分视图

	//函数定义
public:
	//构造函数
	CScoreControl();
	//析构函数
	virtual ~CScoreControl();

	//系统事件
protected:
	//动画消息
	virtual VOID OnWindowMovie();
	//创建消息
	virtual VOID OnWindowCreate(CD3DDevice * pD3DDevice);
	//销毁消息
	virtual VOID OnWindowDestory(CD3DDevice * pD3DDevice);

	//重载函数
protected:
	//鼠标事件
	virtual VOID OnEventMouse(UINT uMessage, UINT nFlags, INT nXMousePos, INT nYMousePos);
	//按钮事件
	virtual VOID OnEventButton(UINT uButtonID, UINT uMessage, INT nXMousePos, INT nYMousePos);
	//绘画窗口
	virtual VOID OnEventDrawWindow(CD3DDevice * pD3DDevice, INT nXOriginPos, INT nYOriginPos);

	//功能函数
public:
	//隐藏窗口
	VOID CloseControl();
	//设置积分
	void SetScoreInfo(const tagScoreInfo & ScoreInfo);

	//辅助函数
protected:
	//绘画数字
	void DrawNumberString(CD3DDevice * pD3DDevice, CD3DTexture* ImageNumber, TCHAR * szImageNum, SCORE lOutNum, INT nXPos, INT nYPos, UINT uFormat /*= DT_LEFT*/);
};

//////////////////////////////////////////////////////////////////////////

#endif
