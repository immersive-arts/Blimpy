#ifndef __MOTOR_H
#define __MOTOR_H
#include "sys.h"	 
  /**************************************************************************
作者：墨比斯科技
我的淘宝小店：https://moebius.taobao.com/
**************************************************************************/

//#define PWMA   TIM8->CCR3 
//#define PWMB   TIM8->CCR4 
//#define PWMC   TIM8->CCR1
//#define PWMD   TIM8->CCR2  

//#define Set_MotorPWMA(x)	TIM_SetCompare3(TIM8, x);
//#define INA1   PBout(4)  
//#define INA2   PBout(5)  

//#define Set_MotorPWMB(x)	TIM_SetCompare4(TIM8, x);
//#define INB2   PDout(2)  
//#define INB1   PCout(12) 

//#define Set_MotorPWMC(x)	TIM_SetCompare1(TIM8, x);
//#define INC1   PBout(0)  
//#define INC2   PBout(1) 

//#define Set_MotorPWMD(x)	TIM_SetCompare2(TIM8, x);
//#define IND2   PCout(5)  
//#define IND1   PCout(4)  


#define PWMA   TIM8->CCR4 
#define PWMB   TIM8->CCR3 
#define PWMC   TIM8->CCR2
#define PWMD   TIM8->CCR1  

#define Set_MotorPWMA(x)	TIM_SetCompare3(TIM8, x);
#define INB1   PBout(4)  
#define INB2   PBout(5)  

#define Set_MotorPWMB(x)	TIM_SetCompare4(TIM8, x);
#define INA2   PDout(2)  
#define INA1   PCout(12) 

#define Set_MotorPWMC(x)	TIM_SetCompare1(TIM8, x);
#define IND1   PBout(0)  
#define IND2   PBout(1) 

#define Set_MotorPWMD(x)	TIM_SetCompare2(TIM8, x);
#define INC2   PCout(5)  
#define INC1   PCout(4)  

#define EN     PAin(12)  

void MiniBalance_PWM_Init(u16 arr,u16 psc);
void MiniBalance_Motor_Init(void);
#endif
