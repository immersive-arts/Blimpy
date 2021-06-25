#include "key.h"
/**************************************************************************
函数功能：按键初始化
入口参数：无
返回  值：无 
**************************************************************************/
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能PA端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//根据设定参数初始化GPIOB
}

