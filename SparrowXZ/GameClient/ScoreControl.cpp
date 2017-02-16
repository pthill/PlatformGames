#include "StdAfx.h"
#include "ScoreControl.h"

//////////////////////////////////////////////////////////////////////////


//构造函数
CScoreControl::CScoreControl()
{
	//设置变量
	m_ChiHuRight.SetEmpty();
	ZeroMemory(&m_ScoreInfo,sizeof(m_ScoreInfo));

	return;
}

//析构函数
CScoreControl::~CScoreControl()
{
}

//动画消息
VOID CScoreControl::OnWindowMovie()
{
	return;
}

//创建消息
VOID CScoreControl::OnWindowCreate(CD3DDevice * pD3DDevice)
{
	//创建字体
	m_FontScore.CreateFont(90,TEXT("宋体"),0L);

	//加载资源
	HINSTANCE hResInstance=AfxGetInstanceHandle();
	m_PngNum.LoadImage(pD3DDevice, hResInstance, TEXT("NUM_ORDER"), TEXT("PNG"));
	m_ImageGameWin.LoadImage(pD3DDevice, hResInstance, TEXT("GAME_WIN"), TEXT("PNG"));
	m_ImageGameLost.LoadImage(pD3DDevice, hResInstance, TEXT("GAME_LOST"), TEXT("PNG"));

	return;
}

//销毁消息
VOID CScoreControl::OnWindowDestory(CD3DDevice * pD3DDevice)
{
	//删除字体
	m_FontScore.DeleteFont();

	//销户资源
	if (m_PngNum.IsNull()==false) m_PngNum.Destory();
	if (m_ImageGameWin.IsNull()==false) m_ImageGameWin.Destory();
	if (m_ImageGameLost.IsNull()==false) m_ImageGameLost.Destory();

	return;
}

//鼠标事件
VOID CScoreControl::OnEventMouse(UINT uMessage, UINT nFlags, INT nXMousePos, INT nYMousePos)
{
	return;
}

//按钮事件
VOID CScoreControl::OnEventButton(UINT uButtonID, UINT uMessage, INT nXMousePos, INT nYMousePos)
{
	return;
}

//绘画窗口
VOID CScoreControl::OnEventDrawWindow(CD3DDevice * pD3DDevice, INT nXOriginPos, INT nYOriginPos)
{
	int nXPos = nXOriginPos;
	int nYPos = nYOriginPos;
	D3DCOLOR crColor=D3DCOLOR_XRGB(255, 245, 163);

	//绘画背景
	if( m_ScoreInfo.lGameScore > 0 )
		m_ImageGameWin.DrawImage(pD3DDevice, nXPos, nYPos);
	else m_ImageGameLost.DrawImage(pD3DDevice, nXPos, nYPos);

	nXPos += 60;
	nYPos += 80;

	//胡牌名次
	if( m_ScoreInfo.cbWinOrder > 0 )
	{
		DrawNumberString(pD3DDevice, &m_PngNum, TEXT("0123456789"), m_ScoreInfo.cbWinOrder, nXOriginPos+20,nYOriginPos+100, DT_LEFT);
	}

	//胡牌信息
	TCHAR szBuffer[256] = TEXT("");
	if( !m_ChiHuRight.IsEmpty() )
	{
		if(m_ScoreInfo.wMeChairId == m_ScoreInfo.wProvideUser)
			lstrcpy(szBuffer, TEXT("自摸"));
		else 
		{
			lstrcpy(szBuffer, TEXT("赢："));
			TCHAR szName[10] = TEXT("");
			lstrcpyn(szName, m_ScoreInfo.szUserName[m_ScoreInfo.wProvideUser], sizeof(szName)/sizeof(TCHAR));
			lstrcat(szBuffer,szName);
			if (lstrlen(m_ScoreInfo.szUserName[m_ScoreInfo.wProvideUser]) > 8)
				lstrcat(szBuffer, TEXT("..."));
			lstrcat(szBuffer, TEXT(" 放炮"));
		}
		m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
		nYPos += 19;

		DWORD dwRight[] = {
			CHR_SHU_FAN,CHR_TIAN_HU,CHR_DI_HU,
			CHR_DA_DUI_ZI,CHR_QING_YI_SE,CHR_QI_XIAO_DUI,CHR_DAI_YAO,CHR_JIANG_DUI,CHR_QING_DUI,
			CHR_LONG_QI_DUI,CHR_QING_QI_DUI,CHR_QING_YAO_JIU,CHR_QING_LONG_QI_DUI,
			CHR_GANG_SHANG_PAO,CHR_GANG_KAI
		};
		LPCTSTR pszRight[] = {
			TEXT("素番 1番 "),TEXT("天胡 6番 "),TEXT("地胡 6番 "),
			TEXT("大对子 2番 "),TEXT("清一色 3番 "),TEXT("七小对 3番 "),TEXT("带幺 3番 "),TEXT("将对 4番 "),
			TEXT("清对 4番 "),TEXT("龙七对 5番 "),TEXT("清七对 5番 "),TEXT("清幺九 5番 "),TEXT("清龙七对 6番 "),
			TEXT("杠上炮 1翻 "),TEXT("杠上花 1翻 ")
		};
		szBuffer[0] = 0;
		for( BYTE j = 0; j < CountArray(pszRight); j++ )
		{
			if( !(m_ChiHuRight&dwRight[j]).IsEmpty() )
				lstrcat( szBuffer,pszRight[j] );
		}
		//根
		if( m_ScoreInfo.cbGenCount > 0 )
		{
			TCHAR szGen[8];
			_sntprintf( szGen,CountArray(szGen),TEXT("根 %d番"),m_ScoreInfo.cbGenCount );
			lstrcat( szBuffer,szGen );
		}
		m_FontScore.DrawText(pD3DDevice, szBuffer,nXPos,nYPos,DT_LEFT|DT_BOTTOM ,crColor);
		nYPos += 19;
	}

	//输给玩家
	szBuffer[0] = 0;
	bool bFirstDraw = true;
	for (WORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_ScoreInfo.wMeChairId) continue;

		if (m_ScoreInfo.wLostFanShu[i] > 0)
		{
			if (bFirstDraw)
			{
				bFirstDraw = false;
				lstrcpy(szBuffer, TEXT("输给 "));
			}
			else lstrcpy(szBuffer, TEXT("     "));

			TCHAR szName[10] = TEXT("");
			lstrcpyn(szName, m_ScoreInfo.szUserName[i], sizeof(szName)/sizeof(TCHAR));
			lstrcat(szBuffer, szName );
			if (lstrlen(m_ScoreInfo.szUserName[i]) > 5)
			{
				lstrcat(szBuffer, TEXT("..."));
			}
			_sntprintf(szBuffer, CountArray(szBuffer), TEXT("%s -%d"), szBuffer, m_ScoreInfo.wLostFanShu[i]);
			m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
			nYPos += 19;
		}
	}

	//查花猪赢
	szBuffer[0] = 0;
	bFirstDraw = true;
	for (WORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_ScoreInfo.wMeChairId) continue;

		if (m_ScoreInfo.lLostHuaZhuScore[i] > 0)
		{
			if (bFirstDraw)
			{
				bFirstDraw = false;
				lstrcpy(szBuffer, TEXT("查花猪："));
				m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
				nYPos += 19;
				lstrcpy(szBuffer, TEXT("    赢 "));
			}
			else lstrcpy(szBuffer, TEXT("       "));

			TCHAR szName[10] = TEXT("");
			lstrcpyn(szName, m_ScoreInfo.szUserName[i], sizeof(szName)/sizeof(TCHAR));
			lstrcat(szBuffer, szName);
			if (lstrlen(m_ScoreInfo.szUserName[i]) > 5)
			{
				lstrcat(szBuffer, TEXT("..."));
			}
			_sntprintf(szBuffer, CountArray(szBuffer), TEXT("%s %d"), szBuffer, m_ScoreInfo.lLostHuaZhuScore[i]);
			m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
			nYPos += 19;
		}
	}

	//花猪输给
	szBuffer[0] = 0;
	bFirstDraw = true;
	for (WORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_ScoreInfo.wMeChairId) continue;

		if (m_ScoreInfo.lHuaZhuScore[i] > 0)
		{
			if (bFirstDraw)
			{
				bFirstDraw = false;
				lstrcpy(szBuffer, TEXT("花猪："));
				m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
				nYPos += 19;
				lstrcpy(szBuffer, TEXT("输给 "));
			}
			else lstrcpy(szBuffer, TEXT("     "));

			TCHAR szName[10] = TEXT("");
			lstrcpyn(szName, m_ScoreInfo.szUserName[i], sizeof(szName)/sizeof(TCHAR));
			lstrcat(szBuffer, szName);
			if (lstrlen(m_ScoreInfo.szUserName[i]) > 5)
			{
				lstrcat(szBuffer, TEXT("..."));
			}
			_sntprintf(szBuffer, CountArray(szBuffer), TEXT("%s -%d"), szBuffer, m_ScoreInfo.lHuaZhuScore[i]);
			m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
			nYPos += 19;
		}
	}

	//查大叫赢
	szBuffer[0] = 0;
	bFirstDraw = true;
	for (WORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_ScoreInfo.wMeChairId) continue;

		if (m_ScoreInfo.lLostChaJiaoScore[i] > 0)
		{
			if (bFirstDraw)
			{
				bFirstDraw = false;
				lstrcpy(szBuffer, TEXT("查大叫："));
				m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
				nYPos += 19;
				lstrcpy(szBuffer, TEXT("    赢 "));
			}
			else lstrcpy(szBuffer, TEXT("       "));

			TCHAR szName[10] = TEXT("");
			lstrcpyn(szName, m_ScoreInfo.szUserName[i], sizeof(szName)/sizeof(TCHAR));
			lstrcat(szBuffer, szName);
			if (lstrlen(m_ScoreInfo.szUserName[i]) > 5)
			{
				lstrcat(szBuffer, TEXT("..."));
			}
			_sntprintf(szBuffer, CountArray(szBuffer), TEXT("%s %d"), szBuffer, m_ScoreInfo.lLostChaJiaoScore[i]);
			m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
			nYPos += 19;
		}
	}

	//查大输给
	szBuffer[0] = 0;
	bFirstDraw = true;
	for (WORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_ScoreInfo.wMeChairId) continue;

		if (m_ScoreInfo.lChaJiaoScore[i] > 0)
		{
			if (bFirstDraw)
			{
				bFirstDraw = false;
				lstrcpy(szBuffer, TEXT("未听牌："));
				m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
				nYPos += 19;
				lstrcpy(szBuffer, TEXT("  输给 "));

			}
			else lstrcpy(szBuffer, TEXT("       "));

			TCHAR szName[10] = TEXT("");
			lstrcpyn(szName, m_ScoreInfo.szUserName[i], sizeof(szName)/sizeof(TCHAR));
			lstrcat(szBuffer, szName);
			if (lstrlen(m_ScoreInfo.szUserName[i]) > 5)
			{
				lstrcat(szBuffer, TEXT("..."));
			}
			_sntprintf(szBuffer, CountArray(szBuffer), TEXT("%s -%d"), szBuffer, m_ScoreInfo.lChaJiaoScore[i]);
			m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
			nYPos += 19;
		}
	}

	//刮风下雨
	if( m_ScoreInfo.lGangScore != 0 )
	{
		_sntprintf(szBuffer, CountArray(szBuffer), TEXT("刮风下雨：%I64d"), m_ScoreInfo.lGangScore );
		m_FontScore.DrawText(pD3DDevice, szBuffer, nXPos, nYPos, DT_LEFT|DT_BOTTOM, crColor);
	}

	//总计
	_sntprintf(szBuffer, CountArray(szBuffer), TEXT("%I64d"), m_ScoreInfo.lGameScore );
	m_FontScore.DrawText(pD3DDevice, szBuffer, nXOriginPos+170, nYOriginPos+60, DT_LEFT|DT_BOTTOM, crColor);

	return;
}

//隐藏窗口
VOID CScoreControl::CloseControl()
{
	//关闭窗口
	if (IsWindowActive()==true)
	{
		//设置数据
		ZeroMemory(&m_ScoreInfo,sizeof(m_ScoreInfo));

		//关闭窗口
		DeleteWindow();
	}

	return;
}

//设置积分
void CScoreControl::SetScoreInfo(const tagScoreInfo & ScoreInfo)
{
	//设置变量
	m_ScoreInfo=ScoreInfo;

	m_ChiHuRight.SetRightData( ScoreInfo.dwChiHuRight,MAX_RIGHT_COUNT );

	return;
}

//绘画数字
void CScoreControl::DrawNumberString(CD3DDevice * pD3DDevice, CD3DTexture* ImageNumber, TCHAR * szImageNum, SCORE lOutNum, INT nXPos, INT nYPos, UINT uFormat /*= DT_LEFT*/)
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
