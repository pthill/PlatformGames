#ifndef CMD_DZSHOWHAND_HEAD_FILE
#define CMD_DZSHOWHAND_HEAD_FILE

//////////////////////////////////////////////////////////////////////////
//�����궨��

#define KIND_ID							3									//��Ϸ I D
#define GAME_PLAYER						8									//��Ϸ����
#define GAME_NAME						TEXT("�����˿�")					//��Ϸ����
#define GAME_GENRE						(GAME_GENRE_GOLD|GAME_GENRE_MATCH)	//��Ϸ����

//�汾��Ϣ
#define VERSION_SERVER					PROCESS_VERSION(7,0,1)				//����汾
#define VERSION_CLIENT					PROCESS_VERSION(7,0,1)				//����汾

//����ԭ��
#define GER_NO_PLAYER					0x10								//û�����

//��Ŀ����
#define FULL_COUNT					    52									//ȫ����Ŀ
#define MAX_COUNT						2									//�����Ŀ
#define MAX_CENTERCOUNT					5									//�����Ŀ

//��ֵ����
#define	CARD_MASK_COLOR				0xF0									//��ɫ����
#define	CARD_MASK_VALUE				0x0F									//��ֵ����

#define  SMALL_CARD_WIDTH			25
#define  SMALL_CARD_HEIGHT			33

//X ���з�ʽ
enum enXCollocateMode 
{
	enXLeft,						//�����
	enXCenter,						//�ж���
	enXRight,						//�Ҷ���
};

//Y ���з�ʽ
enum enYCollocateMode 
{ 
	enYTop,							//�϶���
	enYCenter,						//�ж���
	enYBottom,						//�¶���
};

//�˿˽ṹ
struct tagCardItem
{
	bool							bEffect;							//��Ч��־
	bool							bMy;								//�Լ���־
	bool							bShoot;								//�����־
	BYTE							cbCardData;							//�˿�����
};



//���Զ���
#define MAX_CARD_COUNT				5									//�˿���Ŀ
#define SPACE_CARD_DATA				255									//����˿�


//��ඨ��
#define DEF_X_DISTANCE				19									//Ĭ�ϼ��
#define DEF_Y_DISTANCE				17									//Ĭ�ϼ��
#define DEF_SHOOT_DISTANCE			20									//Ĭ�ϼ��


//////////////////////////////////////////////////////////////////////////
//����������ṹ
#define SUB_S_GAME_START				100									//��Ϸ��ʼ
#define SUB_S_ADD_SCORE					101									//��ע���
#define SUB_S_GIVE_UP					102									//������ע	
#define SUB_S_SEND_CARD					103									//������Ϣ
#define SUB_S_GAME_END					104									//��Ϸ����
#define SUB_S_SIT_DOWN					105									//�û�����
#define SUB_S_OPEN_CARD					106									//�û�����
#define SUB_S_SP_USER					107									//�����û�

//��Ϸ״̬
struct CMD_S_StatusFree
{
	LONGLONG							lCellMinScore;						//��С��ע
	LONGLONG							lCellMaxScore;						//�����ע
};

//��Ϸ״̬
struct CMD_S_StatusPlay
{
	//��ע��Ϣ
	LONGLONG							lCellScore;							//��Ԫ��ע
	LONGLONG							lTurnMaxScore;						//�����ע
	LONGLONG							lTurnLessScore;						//��С��ע
	LONGLONG							lCellMaxScore;						//�����ע
	LONGLONG							lAddLessScore;						//����Сע
	LONGLONG							lTableScore[GAME_PLAYER];			//��ע��Ŀ
	LONGLONG							lTotalScore[GAME_PLAYER];			//�ۼ���ע
	LONGLONG							lCenterScore;						//���ĳ���

	//״̬��Ϣ
	WORD								wDUser;								//D���
	WORD								wMinChipInUser;						//Сäע�û�
	WORD				 				wCurrentUser;						//��ǰ���
	BYTE								cbPlayStatus[GAME_PLAYER];			//��Ϸ״̬
	BYTE							    cbBalanceCount;						//ƽ�����

	//�˿���Ϣ
	BYTE								cbCenterCardData[MAX_CENTERCOUNT];	//�˿���Ŀ
	BYTE								cbHandCardData[MAX_COUNT];			//�����˿�
};

//�����˿�
struct CMD_S_GameStart
{
	WORD				 				wCurrentUser;						//��ǰ���
	WORD								wDUser;								//D���
	WORD								wMinChipInUser;						//Сäע�û�
	WORD								wMaxChipInUser;						//��äע���	
	LONGLONG							lCellScore;							//��Ԫ��ע
	LONGLONG							lTurnMaxScore;						//�����ע
	LONGLONG							lTurnLessScore;						//��С��ע
	LONGLONG							lAddLessScore;						//����Сע
	BYTE								cbCardData[GAME_PLAYER][MAX_COUNT];	//�û��˿�
};

//�����˿�
struct CMD_S_SpUser
{
	BYTE								cbAllData[GAME_PLAYER][MAX_COUNT];	//�û��˿�
	BYTE								cbCenterCardData[MAX_CENTERCOUNT];	//�����˿�
};

//�û���ע
struct CMD_S_AddScore
{
	WORD								wCurrentUser;						//��ǰ�û�
	WORD								wAddScoreUser;						//��ע�û�
	LONGLONG							lAddScoreCount;						//��ע��Ŀ
	LONGLONG							lTurnLessScore;						//���ټ�ע
	LONGLONG							lTurnMaxScore;						//�����ע
	LONGLONG							lAddLessScore;						//����Сע
	BYTE								cbShowHand[GAME_PLAYER];			//����û�
};

//�û�����
struct CMD_S_GiveUp
{
	WORD								wGiveUpUser;						//�����û�
	LONGLONG							lLost;								//������	
};

//�������ݰ�
struct CMD_S_SendCard
{
	BYTE								cbPublic;							//�Ƿ���
	WORD								wCurrentUser;						//��ǰ�û�
	BYTE								cbSendCardCount;					//������Ŀ
	BYTE								cbCenterCardData[MAX_CENTERCOUNT];	//�����˿�	
};

//��Ϸ����
struct CMD_S_GameEnd
{
	BYTE								cbTotalEnd;							//ǿ�˱�־
	LONGLONG							lGameTax[GAME_PLAYER];				//��Ϸ˰��
	LONGLONG							lGameScore[GAME_PLAYER];			//��Ϸ�÷�
	BYTE								cbCardData[GAME_PLAYER][MAX_COUNT];	//�û��˿�
	BYTE								cbLastCenterCardData[GAME_PLAYER][MAX_CENTERCOUNT];//����˿�
	BYTE								cbDelayOverGame;
};

//��Ϸ����
struct CMD_S_OpenCard
{
	WORD								wWinUser;							//�û�����
};


//////////////////////////////////////////////////////////////////////////
//�ͻ�������ṹ
#define SUB_C_ADD_SCORE					1									//�û���ע
#define SUB_C_GIVE_UP					2									//������Ϣ
#define SUB_C_OPEN_CARD					3									//������Ϣ

//�û���ע
struct CMD_C_AddScore
{
	LONGLONG							lScore;								//��ע��Ŀ
};

//////////////////////////////////////////////////////////////////////////

#endif