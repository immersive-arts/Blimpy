#include "Sys.h"

/**************************************************************************
作者：墨比斯科技
我的淘宝小店：https://moebius.taobao.com/
**************************************************************************/ 

/*************************************************
Function: pinMode(unsigned char GPIOx, unsigned char Pin, unsigned char Mode)
Description: 使用函数操作将管脚初始化封装为类似arduino方式
Input: GPIOx: PA-PG	Pin: 0-15	Mode: INPUT/OUTPUT
Output: void
Return: void
*************************************************/
void pinMode(unsigned char GPIOx, unsigned char Pin, unsigned char Mode)
{
	RCC->APB2ENR |= 0x01 << GPIOx;					//使能端口时钟
	if(Pin > 7)
	{
		Pin -= 8;
		((GPIO_TypeDef *)(GPIOx * 0x400 + APB2PERIPH_BASE))->CRH &= ~(0x0000000F << (Pin * 4));
		((GPIO_TypeDef *)(GPIOx * 0x400 + APB2PERIPH_BASE))->CRH |= ((0x0000000F & Mode) << (Pin * 4));
	}
	else
	{
		((GPIO_TypeDef *)(GPIOx * 0x400 + APB2PERIPH_BASE))->CRL &= ~(0x0000000F << (Pin * 4));
		((GPIO_TypeDef *)(GPIOx * 0x400 + APB2PERIPH_BASE))->CRL |= ((0x0000000F & Mode) << (Pin * 4));
	}
	
}
