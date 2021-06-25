#include "Sys.h"

//unsigned char bSysTick;

//void TIM3_Init(u8 MilliSecond)
//{
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
//	NVIC_InitTypeDef NVIC_InitStruct;
//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
//	
////	((1+TIM_Prescaler )/72000000)*(1+TIM_Period );
//	
//	TIM_TimeBaseInitStruct.TIM_Period 			=		(MilliSecond * 10) - 1;
//	TIM_TimeBaseInitStruct.TIM_Prescaler 		=		7199;
//	TIM_TimeBaseInitStruct.TIM_ClockDivision	= 		TIM_CKD_DIV1;
//	TIM_TimeBaseInitStruct.TIM_CounterMode 		=		TIM_CounterMode_Up;
//	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);
//	
//	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
//	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
//	
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
//	
//	NVIC_InitStruct.NVIC_IRQChannel 			=		TIM3_IRQn;
//	NVIC_InitStruct.NVIC_IRQChannelCmd 			=		ENABLE;
//	NVIC_InitStruct.NVIC_IRQChannelSubPriority 	=		0;				//从优先级
//	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority =	3;				//先占优先级
//	NVIC_Init(&NVIC_InitStruct);
//	
//	TIM_Cmd(TIM3,ENABLE);
//}

//void TIM3_IRQHandler()
//{
//	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==1)
//	{
//		bSysTick = 1;		//5ms定时器溢出标志
//	}
//	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
//}


