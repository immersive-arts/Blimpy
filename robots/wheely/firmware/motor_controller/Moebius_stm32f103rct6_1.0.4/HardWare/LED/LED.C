#include "led.h"
/**************************************************************************
作者：墨比斯科技
我的淘宝小店：https://moebius.taobao.com/
**************************************************************************/ 

/**************************************************************************
函数功能：LED接口初始化
入口参数：无 
返回  值：无
**************************************************************************/
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//使能端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//50M
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOB
}
