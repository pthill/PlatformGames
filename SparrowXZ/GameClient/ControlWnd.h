#ifndef CONTROL_WND_HEAD_FILE
#define CONTROL_WND_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "GameLogic.h"

//////////////////////////////////////////////////////////////////////////////////

//消息定义
//消息定义
#define IDM_LISTEN_CARD				(WM_USER+500)						//扑克操作
#define IDM_CARD_OPERATE			(WM_USER+501)						//扑克操作

//////////////////////////////////////////////////////////////////////////////////

//积分控件
class CControlWnd : public CVirtualWindow
{
	//灰度动画
protected:
	BYTE							m_cbAlphaIndex;						//透明索引
	CLapseCount						m_AlphaLapseCount;					//流逝计数

	//配置变量
protected:
	BYTE							m_cbActionMask;						//类型掩码
	BYTE							m_cbCenterCard;						//中心扑克
	BYTE							m_cbGangCard[16];					//杠牌数据
	CPoint							m_PointBenchmark;					//基准位置
	CSize							m_ControlSize;

	//状态变量
protected:
	BYTE							m_cbItemCount;						//子项数目
	BYTE							m_cbCurrentItem;					//当前子项

	//控件变量
protected:
	CVirtualButton					m_btChiHu;							//吃胡按钮
	CVirtualButton					m_btListen;							//听牌按钮
	CVirtualButton					m_btGiveUp;							//放弃按钮

	//资源变量
protected:
	CD3DTexture						m_TextureControlTop;				//资源图片
	CD3DTexture						m_TextureControlMid;				//资源图片
	CD3DTexture						m_TextureControlButtom;				//资源图片
	CD3DTexture						m_TextureActionExplain;				//动作解释
	CD3DTexture						m_TextureItemFrame;					//选中边框

	//函数定义
public:
	//构造函数
	CControlWnd();
	//析构函数
	virtual ~CControlWnd();

	//控件控制
public:
	//基准位置
	void SetBenchmarkPos(int nXPos, int nYPos);
	//设置扑克
	void SetControlInfo(BYTE cbCenterCard, BYTE cbActionMask, tagGangCardResult & GangCardResult);
	//隐藏窗口
	VOID CloseControl();

	//内部函数
protected:
	//调整控件
	void RectifyControl();

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
};

//////////////////////////////////////////////////////////////////////////////////

#endif