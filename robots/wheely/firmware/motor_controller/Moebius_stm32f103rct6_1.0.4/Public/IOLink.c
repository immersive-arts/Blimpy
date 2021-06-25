#include "Sys.h"

/**************************************************************************
���ߣ�ī��˹�Ƽ�
�ҵ��Ա�С�꣺https://moebius.taobao.com/
**************************************************************************/ 

/*************************************************
Function: pinMode(unsigned char GPIOx, unsigned char Pin, unsigned char Mode)
Description: ʹ�ú����������ܽų�ʼ����װΪ����arduino��ʽ
Input: GPIOx: PA-PG	Pin: 0-15	Mode: INPUT/OUTPUT
Output: void
Return: void
*************************************************/
void pinMode(unsigned char GPIOx, unsigned char Pin, unsigned char Mode)
{
	RCC->APB2ENR |= 0x01 << GPIOx;					//ʹ�ܶ˿�ʱ��
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
