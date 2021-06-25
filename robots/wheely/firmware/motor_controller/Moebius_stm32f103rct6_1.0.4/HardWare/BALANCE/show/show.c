#include "show.h"
#include "sys.h"
#include "config.h"

/**************************************************************************
���ߣ�ī��˹�Ƽ�
�ҵ��Ա�С�꣺https://moebius.taobao.com/
**************************************************************************/
#if OLED_DISPLAY_ENABLE
/**************************************************************************
�������ܣ�OLED��ʾ
��ڲ�������
����  ֵ����
**************************************************************************/
void OLED_ShowInt(u8 x,u8 y,long int num,u8 len)
{
	if(num < 0)
	{
		num *= -1;
		OLED_ShowString(x, y, (u8*)"-");
	}
	else
	{
		OLED_ShowString(x, y, (u8*)"+");
	}
	OLED_ShowNumber(x + 15, y ,num, len, 12);
}

void oled_show(void)
{			
//	//=============��1����ʾ3��Ƕ�===============//	
	OLED_ShowString(0,0,(u8*)"X:");
	if(Pitch<0)		OLED_ShowNumber(15,0,Pitch+360,3,12);
	else			OLED_ShowNumber(15,0,Pitch,3,12);	

	OLED_ShowString(40,0,(u8*)"Y:");
	if(Roll<0)		OLED_ShowNumber(55,0,Roll+360,3,12);
	else			OLED_ShowNumber(55,0,Roll,3,12);	

	OLED_ShowString(80,0,(u8*)"Z:");
	if(Yaw<0)		OLED_ShowNumber(95,0,Yaw+360,3,12);
	else			OLED_ShowNumber(95,0,Yaw,3,12);		

	//=============��ʾ���״̬=======================//	
	OLED_ShowInt(0,  10, Target_A, 5);
	OLED_ShowInt(80, 10, Encoder_A, 5);
	OLED_ShowInt(0,  20, Target_B, 5);
	OLED_ShowInt(80, 20, Encoder_B, 5);
	OLED_ShowInt(0,  30, Target_C, 5);
	OLED_ShowInt(80, 30, Encoder_C, 5);
	OLED_ShowInt(0,  40, Target_D, 5);
	OLED_ShowInt(80, 40, Encoder_D, 5);

	//=============��ʾ��ѹ=======================//
	OLED_ShowString(00,50,(u8*)"VELOCITY");
	OLED_ShowString(88,50,(u8*)".");
	OLED_ShowString(110,50,(u8*)"V");
	OLED_ShowNumber(75,50,Voltage/100,2,12);
	OLED_ShowNumber(98,50,Voltage%100,2,12);
	if(Voltage%100<10) 	OLED_ShowNumber(92,50,0,2,12);
	//=============ˢ��=======================//
	OLED_Refresh_Gram();	
}


#endif

