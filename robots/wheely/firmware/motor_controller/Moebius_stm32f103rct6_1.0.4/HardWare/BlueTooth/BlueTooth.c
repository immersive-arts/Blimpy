#include "BlueTooth.h"

/**************************************************************************
作者：墨比斯科技
我的淘宝小店：https://moebius.taobao.com/
**************************************************************************/

#define MAXQSIZE	10

int QueueSize;
LinkQueue *Uart_Queue;

//========================================================================
//创建链表队列
void InitQueue()
{
	Uart_Queue = (LinkQueue*)malloc(sizeof(LinkQueue));
	Uart_Queue->Hand = (QNode*)malloc(sizeof(QNode));
	Uart_Queue->Tail = Uart_Queue->Hand;
	if (!Uart_Queue->Hand)	printf("Er 0001: Init queue error!\r\n");
	Uart_Queue->Hand->next = NULL;
}

//========================================================================
//入队
void InQueue(u8 Data)		//头插法创建链表
{
	if (QueueSize >= MAXQSIZE)
	{
		printf("Er 0002: Queue overflow!\r\n");
		return;
	}
	else
	{
		QNode* NewNode = (QNode*)malloc(sizeof(QNode));
		NewNode->data = Data;
		NewNode->next = Uart_Queue->Hand;
		Uart_Queue->Hand = NewNode;
		QueueSize++;
	}
}

//========================================================================
//出队
u8 OutQueue()
{
	QNode* Temp;
	Temp = Uart_Queue->Hand;
	if (Uart_Queue->Hand == Uart_Queue->Tail)
	{
		printf("Er 0002: Queue empty!\r\n");
		return 0;
	}
	while (Temp->next != Uart_Queue->Tail)
	{
		Temp = Temp->next;
	}
	Temp->next = NULL;
	free(Uart_Queue->Tail);
	Uart_Queue->Tail = Temp;
	QueueSize--;
	return Temp->data;
}

//========================================================================
//检查队列是否为空
u8 InspectQueue()
{
	if(Uart_Queue->Tail == Uart_Queue->Hand)
		return 0;
	else
		return 1;
}

//========================================================================
//输出队列
void PrintQueue()
{
	QNode* Temp;
	Temp = Uart_Queue->Hand;
	while (Temp->next != NULL)
	{
		printf("%c, ",Temp->data);
		Temp = Temp->next;
	}
	printf("Queue size %d \r\n",QueueSize);
}

void USART3_Send_Data(u8 Dat)
{ 
	USART_SendData(USART3,Dat); 
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC) != SET);      
	USART_ClearFlag(USART3,USART_FLAG_TC);
}
/*******************************************************************************
* 函 数 名         : USART1_Init
* 函数功能		   : USART1初始化函数
* 输    入         : bound:波特率
* 输    出         : 无
*******************************************************************************/ 
void USART3_Init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE); 				//Uart3重映射
 
	/*  配置GPIO的模式和IO口 */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//TX				//串口输出PA2
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;			//复用推挽输出
	GPIO_Init(GPIOC,&GPIO_InitStructure);					/* 初始化串口输入IO */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;//RX			//串口输入PA3
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;		//模拟输入
	GPIO_Init(GPIOC,&GPIO_InitStructure);					/* 初始化GPIO */
	
	//USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;						//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;				//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART3, &USART_InitStructure);				//初始化串口1
	  
	
	USART_ClearFlag(USART3, USART_FLAG_TC);
		
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);			//开启相关中断
	USART_Cmd(USART3, ENABLE);								//使能串口1 

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;		//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器
	
	InitQueue();
}

/*******************************************************************************
* 函 数 名         : USART1_IRQHandler
* 函数功能		   : USART1中断函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/ 
void USART3_IRQHandler(void)									//串口1中断服务程序
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)		//接收中断
	{
		InQueue(USART_ReceiveData(USART3));
	} 
} 	

 



