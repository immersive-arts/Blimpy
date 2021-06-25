#include "sys.h"
/**************************************************************************
作者：墨比斯科技
我的淘宝小店：https://moebius.taobao.com/
**************************************************************************/ 
u8 Flag_Left, Flag_Right, Flag_Direction = 0;		//蓝牙遥控相关的变量
u8 Flag_Stop = 1, Flag_Show = 0;					//停止标志位和 显示标志位 默认停止 显示打开
int Encoder_A, Encoder_B, Encoder_C, Encoder_D;		//编码器的脉冲计数
long int Position_A, Position_B, Position_C, Position_D, Rate_A, Rate_B, Rate_C, Rate_D; //PID控制相关变量
int Encoder_A_EXTI;									//通过外部中断读取的编码器数据                       
long int Motor_A, Motor_B, Motor_C, Motor_D;		//电机PWM变量
long int Target_A = 6, Target_B = 6, Target_C = 6, Target_D = 6;	//电机目标值
int Voltage;										//电池电压采样相关的变量
float Show_Data_Mb;									//全局显示变量，用于显示需要查看的数据                         
u8 delay_50, delay_flag;							//延时相关变量
u8 Run_Flag = 0;  									//蓝牙遥控相关变量和运行状态标志位
u8 rxbuf[8], Urxbuf[8], CAN_ON_Flag = 0, Usart_ON_Flag = 0, PS2_ON_Flag = 0, Usart_Flag, PID_Send, Flash_Send;  //CAN和串口控制相关变量
u8 txbuf[8], txbuf2[8], Turn_Flag;					//CAN发送相关变量
float Pitch, Roll, Yaw, Move_X, Move_Y, Move_Z;		//三轴角度和XYZ轴目标速度
u16 PID_Parameter[10], Flash_Parameter[10];			//Flash相关数组
float	Position_KP = 6, Position_KI = 0, Position_KD = 3;	//位置控制PID参数
float Velocity_KP = 10, Velocity_KI = 10;			//速度控制PID参数
int RC_Velocity = 30, RC_Position = 1000;			//设置遥控的速度和位置值
int PS2_LX, PS2_LY, PS2_RX, PS2_RY, PS2_KEY;
int Gryo_Z;
/*************************************************
Function: Peripheral_Init()
Description: Init system peripheral
Input:  void
Output: void
Return: void
*************************************************/
void Peripheral_Init()
{
	LED_Init();						//=====初始化与 LED 连接的硬件接口
	KEY_Init();						//=====按键初始化

	USART1_Init(9600);				//=====串口初始化
	USART3_Init(9600);				//=====蓝牙串口
	
	MiniBalance_PWM_Init(7199,0);	//=====电机驱动

	Adc_Init();
	
	PS2_Init();

#if MPU6xxx_ENABLE
	IIC_Init();
	MPU6050_initialize();           //=====MPU6050初始化	
	DMP_Init();                     //=====初始化DMP   
#endif
	
#if OLED_DISPLAY_ENABLE
	OLED_Init();					//=====OLED初始化
#endif
	
#if ENCODER_ENABLE					//=====编码器初始化
	Encoder_Init_TIM2();
	Encoder_Init_TIM3();
	Encoder_Init_TIM4();
	Encoder_Init_TIM5();
#endif
	EXTI15_Init();
}

/*************************************************
Function: main()
Description: Main function entry
Input:  void
Output: void
Return: void
*************************************************/
int main(void)
{
	uint8_t PS2_Hold;
	Peripheral_Init();
	while (1)
	{	
		if(PS2_KEY)
		{
			do
			{
				if(PS2_Hold)	break;
				PS2_Hold = 1;
				printf("PS2 Key = %d",PS2_KEY);	
				switch(PS2_KEY)		//PS2按键处理
				{
					case 1:						break;		//select
					case 2:						break;		//L摇杆按键
					case 3:						break;		//R摇杆按键
					case 4:	PS2_BLU = 0;		break;		//start
					case 5:						break;		//L前
					case 6:						break;		//L右
					case 7:						break;		//L后
					case 8:						break;		//L左
					case 9:						break;		//L2键
					case 10:					break;		//R2键
					case 11:					break;		//L1键
					case 12:					break;		//R1键
					case 13:	if(PS2_Hold && RC_Velocity < 25) RC_Velocity += 5;		break;		//R上
					case 14:					break;		//R右
					case 15:	if(PS2_Hold && RC_Velocity > 5) RC_Velocity -= 5;		break;		//R下
					case 16:					break;		//R左
				}
			}while(0);
		}
		else	PS2_Hold = 0;
		PS2_Receive();
        delay_ms(10);
		#if OLED_DISPLAY_ENABLE
		oled_show();
		#endif
	}
}
