#include "StdAfx.h"
#include "GameLogic.h"

//////////////////////////////////////////////////////////////////////////

//扑克数据
 BYTE CGameLogic::m_cbCardListData[172-4]=
{
	    2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,\
		2,4,6,8,10,12,14,16, 18,20,22,24,26,28,30,32,\
		1,9,17,25,3,11,19,27,3,11,19,27, 5,13,21,29,5,\
		13,21,29,5,13,21,29,7,15,23,31,7,15,23,31,7,15,23,31,7,15,23,31,\
		2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,\
		2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,\
		2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,\
		2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,\
		2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,\
		2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32\
	
};
//////////////////////////////////////////////////////////////////////////

//构造函数
CGameLogic::CGameLogic()
{
}

//析构函数
CGameLogic::~CGameLogic()
{
}

//混乱扑克
void CGameLogic::RandCardList(BYTE cbCardBuffer[], BYTE cbBufferCount)
{

	//混乱准备
	BYTE cbCardData[CountArray(m_cbCardListData)];
	CopyMemory(cbCardData,m_cbCardListData,sizeof(m_cbCardListData));

	//混乱扑克
	BYTE cbRandCount=0,cbPosition=0;
	do
	{
		cbPosition=(BYTE)(rand()+GetTickCount()*2)%(CountArray(cbCardData)-cbRandCount);
		cbCardBuffer[cbRandCount++]=cbCardData[cbPosition];
		cbCardData[cbPosition]=cbCardData[CountArray(cbCardData)-cbRandCount];
	} while (cbRandCount<cbBufferCount);


	return;
}
//获取牌型
BYTE CGameLogic::GetCardType(const BYTE cbCardData, BYTE cbCardCount,BYTE *bcOutCadDataWin )
{
	ASSERT(1==cbCardCount);
	if (!(1==cbCardCount) )return 0;
	memset(bcOutCadDataWin,0,AREA_COUNT+1);
	BYTE bcData =cbCardData;	
//1,13大橘子、2,14,24大铃铛、3小王，4大王、5,11,17,23大苹果、6小苹果、7,19大柠檬、8大西瓜、9小西瓜、10绿LUCK、12小橘子、15小双7、16大双7、18小柠檬、20大星星、21小星星、22红LUCK
	
	if(5==bcData||bcData==11||bcData==17||bcData==23||bcData==6)//苹果
	{
		if(bcData==6)
			bcOutCadDataWin[0]= 2;
		else
			bcOutCadDataWin[0]= 5;
	}
	else if(1==bcData||bcData==13||bcData==12)//橘子
	{
		if(bcData==12)
			bcOutCadDataWin[1]= 2;
		else
			bcOutCadDataWin[1]= 10;
	}
	else if(7==bcData||bcData==19||bcData==18)//柠檬
	{
		if(bcData==18)
			bcOutCadDataWin[2]= 2;
		else
			bcOutCadDataWin[2]= 10;
	}
	else if(2==bcData||bcData==14||bcData==24)//铃铛
	{
		if(bcData==24)
			bcOutCadDataWin[3]= 2;
		else
			bcOutCadDataWin[3]= 10;
	}
	else if(8==bcData||bcData==9)//西瓜
	{
		if(bcData==9)
			bcOutCadDataWin[4]= 2;
		else
			bcOutCadDataWin[4]= 20;
	}
	else if(20==bcData||bcData==21)//星星
	{
		if(bcData==21)
			bcOutCadDataWin[5]= 2;
		else
			bcOutCadDataWin[5]= 20;
	}
	else if(15==bcData||bcData==16)//双7
	{
		if(bcData==15)
			bcOutCadDataWin[6]= 2;
		else
			bcOutCadDataWin[6]= 20;
	}
	else if(3==bcData||bcData==4)//大小王
	{
		if(bcData==3)
			bcOutCadDataWin[7]= 25;
		else
			bcOutCadDataWin[7]= 50;
	}
	else if(bcData==10)
		bcOutCadDataWin[8]=99;
	else if(bcData==22)
		bcOutCadDataWin[8]=100;

	return true;	
}

//获取牌型
BYTE CGameLogic::GetCardTypeEx(const BYTE cbCardData, BYTE cbCardCount,BYTE *bcOutCadDataWin )
{
	ASSERT(1==cbCardCount);
	if (!(1==cbCardCount) )return 0;
	memset(bcOutCadDataWin,0,AREA_COUNT+1);
	BYTE bcData =cbCardData;	
	//1,13大橘子、2,14,24大铃铛、3小王，4大王、5,11,17,23大苹果、6小苹果、7,19大柠檬、8大西瓜、9小西瓜、10绿LUCK、12小橘子、15小双7、16大双7、18小柠檬、20大星星、21小星星、22红LUCK

	if(5==bcData||bcData==11||bcData==17||bcData==23||bcData==6)//苹果
	{
		if(bcData==6)
			bcOutCadDataWin[0]= 2;
		else
			bcOutCadDataWin[0]= 5;
	}
	else if(1==bcData||bcData==13||bcData==12)//橘子
	{
		if(bcData==12)
			bcOutCadDataWin[1]= 2;
		else
			bcOutCadDataWin[1]= 10;

	}
	else if(7==bcData||bcData==19||bcData==18)//柠檬
	{
		if(bcData==18)
			bcOutCadDataWin[2]= 2;
		else
			bcOutCadDataWin[2]= 10;

	}
	else if(2==bcData||bcData==14||bcData==24)//铃铛
	{
		if(bcData==24)
			bcOutCadDataWin[3]= 2;
		else
			bcOutCadDataWin[3]= 10;		
	}
	else if(8==bcData||bcData==9)//西瓜
	{
		if(bcData==9)
			bcOutCadDataWin[4]= 2;
		else
			bcOutCadDataWin[4]= 20;
	}
	else if(20==bcData||bcData==21)//星星
	{
		if(bcData==21)
			bcOutCadDataWin[5]= 2;
		else
			bcOutCadDataWin[5]= 20;
	}
	else if(15==bcData||bcData==16)//双7
	{
		if(bcData==15)
			bcOutCadDataWin[6]= 2;
		else
			bcOutCadDataWin[6]= 20;
	}
	else if(3==bcData||bcData==4)//大小王
	{
		if(bcData==3)
			bcOutCadDataWin[7]= 25;
		else
			bcOutCadDataWin[7]= 50;
	}
	else if(bcData==10)
		bcOutCadDataWin[8]=99;
	else if(bcData==22)
		bcOutCadDataWin[8]=100;

	return true;

}

//大小比较
/*
cbNextCardData>cbFirstCardData  返回1
cbNextCardData<cbFirstCardData  返回-1
cbNextCardData==cbFirstCardData 返回0
*/
//Multiple 比较出来的倍数
int CGameLogic::CompareCard(const BYTE cbFirstCardData[], BYTE cbFirstCardCount,const BYTE cbNextCardData[], BYTE cbNextCardCount,BYTE &Multiple)
{
	//合法判断
	ASSERT(1==cbFirstCardCount && 1==cbNextCardCount);
	if (!(1==cbFirstCardCount && 1==cbNextCardCount)) return 0;

	return true;
}

//获取牌点
BYTE CGameLogic::GetCardListPip(const BYTE cbCardData[], BYTE cbCardCount)
{
	//变量定义
	BYTE cbPipCount=0;

	//获取牌点
	BYTE cbCardValue=0;
	for (BYTE i=0;i<cbCardCount;i++)
	{
		cbCardValue=GetCardValue(cbCardData[i]);
		if(cbCardValue>10)
		{
			cbCardValue = 10;
			
		}
		cbPipCount+=cbCardValue;
	}
	return (cbPipCount%10);
}
BYTE CGameLogic::GetCardNewValue(BYTE cbCardData)
{
	//扑克属性
	BYTE cbCardColor=GetCardColor(cbCardData);
	BYTE cbCardValue=GetCardValue(cbCardData);

	//转换数值
	if (cbCardColor==0x04) return cbCardValue+13+2;
	return cbCardValue;

}
//逻辑大小
BYTE CGameLogic::GetCardLogicValue(BYTE cbCardData)
{
	BYTE cbValue=GetCardValue(cbCardData);

	//获取花色
	BYTE cbColor=GetCardColor(cbCardData);

	if(cbValue>10)
	{
		cbValue = 10;

	}
	if(cbColor==0x4)
	{
		return 11;
	}
	return cbValue;
}

//排列扑克
void CGameLogic::SortCardList(BYTE cbCardData[], BYTE cbCardCount, BYTE cbSortType)
{
	//数目过虑
	if (cbCardCount==0) return;

	//转换数值
	BYTE cbSortValue[CARD_COUNT];
	if (ST_VALUE==cbSortType)
	{
		for (BYTE i=0;i<cbCardCount;i++) cbSortValue[i]=GetCardValue(cbCardData[i]);	
	}
	else 
	{
		if(cbSortType==ST_NEW)
		{
			for (BYTE i=0;i<cbCardCount;i++) cbSortValue[i]=GetCardNewValue(cbCardData[i]);	

		}else
		{
			for (BYTE i=0;i<cbCardCount;i++) cbSortValue[i]=GetCardLogicValue(cbCardData[i]);	

		}
		
	}


	//排序操作
	bool bSorted=true;
	BYTE cbThreeCount,cbLast=cbCardCount-1;
	do
	{
		bSorted=true;
		for (BYTE i=0;i<cbLast;i++)
		{
			if ((cbSortValue[i]<cbSortValue[i+1])||
				((cbSortValue[i]==cbSortValue[i+1])&&(cbCardData[i]<cbCardData[i+1])))
			{
				//交换位置
				cbThreeCount=cbCardData[i];
				cbCardData[i]=cbCardData[i+1];
				cbCardData[i+1]=cbThreeCount;
				cbThreeCount=cbSortValue[i];
				cbSortValue[i]=cbSortValue[i+1];
				cbSortValue[i+1]=cbThreeCount;
				bSorted=false;
			}	
		}
		cbLast--;
	} while(bSorted==false);

	return;
}

//混乱数组
void CGameLogic::ChaosArray( INT nArrayOne[], INT nCountOne, INT nArrayTwo[], INT nCountTwo )
{
	ASSERT( nCountOne == nCountTwo );
	if( nCountTwo != nCountOne )
		return;

	DWORD wTick = GetTickCount();
	for (int i = 1; i < nCountOne; ++i)
	{
		int nTempIndex = (rand()+wTick)%nCountOne;

		int nTempValueOne = nArrayOne[i];
		nArrayOne[i] = nArrayOne[nTempIndex];
		nArrayOne[nTempIndex] = nTempValueOne;

		int nTempValueTwo = nArrayTwo[i];
		nArrayTwo[i] = nArrayTwo[nTempIndex];
		nArrayTwo[nTempIndex] = nTempValueTwo;
	}
}
//////////////////////////////////////////////////////////////////////////

