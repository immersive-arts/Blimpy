#ifndef _Sys_H
#define _Sys_H

//========================================================================
//头文件声明

#include "stm32f10x.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "Systick.h"
#include "define.h"
#include "usart.h"
#include "LED.h"
#include "KEY.h"
#include "oled.h"
#include "show.h"
#include "motor.h"
#include "ioi2c.h"
#include "BlueTooth.h"
#include "control.h"
#include "exti.h"
#include "encoder.h"
#include "adc.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu6050.h"
#include "dmpKey.h"
#include "dmpmap.h"
#include "pstwo.h"
#include "config.h"

extern int Encoder_A,Encoder_B,Encoder_C,Encoder_D;                    //编码器的脉冲计数
extern long int Motor_A,Motor_B,Motor_C,Motor_D;                   //电机PWM变量
extern u8 Flag_Left,Flag_Right,Flag_sudu,Flag_Direction; //蓝牙遥控相关的变量
extern u8 Flag_Stop,Flag_Show;                               //停止标志位和 显示标志位 默认停止 显示打开
extern long int Target_A,Target_B,Target_C,Target_D,Rate_A,Rate_B,Rate_C,Rate_D;                      //电机目标速度
extern  int Voltage,Voltage_Zheng,Voltage_Xiao;                //电池电压采样相关的变量
extern float Angle_Balance,Gyro_Balance,Gyro_Turn;           //平衡倾角 平衡陀螺仪 转向陀螺仪
extern float Show_Data_Mb;                                    //全局显示变量，用于显示需要查看的数据
extern int Temperature;
extern u32 Distance;                                           //超声波测距
extern u8 Bi_zhang,delay_50,delay_flag;
extern float Acceleration_Z;
extern int RC_Velocity,RC_Position;
extern int Encoder_A_EXTI;
extern u8 Run_Flag,PID_Send,Flash_Send,Turn_Flag;
extern u8 rxbuf[8],Urxbuf[8],txbuf[8],txbuf2[8],CAN_ON_Flag,Usart_ON_Flag,Usart_Flag,PS2_ON_Flag;
extern float Pitch,Roll,Yaw,Move_X,Move_Y,Move_Z; 
extern long int Position_A,Position_B,Position_C,Position_D;
extern u16 PID_Parameter[10],Flash_Parameter[10];
extern float	Position_KP,Position_KI,Position_KD;  //位置控制PID参数
extern float Velocity_KP,Velocity_KI;	                    //速度控制PID参数
extern int PS2_LX,PS2_LY,PS2_RX,PS2_RY,PS2_KEY;
extern int Gryo_Z;
extern u8 PS2_BLU;
extern unsigned char Usart1_Buf;						//上位机串口消息

//Ex_NVIC_Config专用定义
#define GPIO_A 0
#define GPIO_B 1
#define GPIO_C 2
#define GPIO_D 3
#define GPIO_E 4
#define GPIO_F 5
#define GPIO_G 6 

#define FTIR   1  //下降沿触发
#define RTIR   2  //上升沿触发

void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset);//设置偏移地址
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group);//设置NVIC分组
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group);//设置中断
void Ex_NVIC_Config(u8 GPIOx,u8 BITx,u8 TRIM);//外部中断配置函数(只对GPIOA~G)

#endif

