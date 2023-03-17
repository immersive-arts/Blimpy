#include "control.h"	
#include "filter.h"
#include "config.h"

  /**************************************************************************
作者：墨比斯科技
我的淘宝小店：https://moebius.taobao.com/
**************************************************************************/
u8 Target_Z;
u8 Flag_Target,Flag_Change;				//相关标志位
u8 PS2_BLU;
u8 temp1;								//临时变量
float Voltage_Count,Voltage_All;		//电压采样相关变量
float Gyro_K=-0.6;						//陀螺仪比例系数
int Gyro_Bias;
int j;
unsigned int TimClk = 200;
#define a_PARAMETER          (0.311f)               
#define b_PARAMETER          (0.3075f)   
#define d_PARAMETER          (0.38f)
#define SQRT3_2							 (0.866f)
#define MAX_PWM 10000
#define OMNIWHEEL_3_WHEEL
/**************************************************************************
函数功能：小车运动数学模型
入口参数：X Y Z 三轴速度或者位置
返回  值：无
**************************************************************************/
void Kinematic_Analysis(float Vx,float Vy,float Vz)
{

#if	AXLE_Z_RESTRAIN
	int temp;
	if(!KEY1)	Gyro_Bias = Yaw;
	temp = Yaw - Gyro_Bias;
	if (temp > 180)
		temp = 360-temp;
	if (temp < -180)
		temp = 360 + temp;
	if(temp > 1 || temp < -1)
		Vz += Gyro_K * temp;
#endif
	//Target_A   = -Vx+Vy+Vz*(a_PARAMETER+b_PARAMETER);
	//Target_B   = +Vx+Vy-Vz*(a_PARAMETER+b_PARAMETER);
	//Target_C   = -Vx+Vy-Vz*(a_PARAMETER+b_PARAMETER);
	//Target_D   = +Vx+Vy+Vz*(a_PARAMETER+b_PARAMETER);
#if defined(MECHANUM_4_WHEEL)
	Target_A   = -Vx+Vz*(a_PARAMETER+b_PARAMETER);
	Target_B   = Vy-Vz*(a_PARAMETER+b_PARAMETER);
	Target_C   = -Vx-Vz*(a_PARAMETER+b_PARAMETER);
	Target_D   = Vy+Vz*(a_PARAMETER+b_PARAMETER);
#elif defined(OMNIWHEEL_3_WHEEL)
	Target_A   = -Vx + Vz*d_PARAMETER;
	Target_B   = -Vx/2 + Vy*SQRT3_2	- Vz*d_PARAMETER;
	Target_C   = -Vx/2 - Vy*SQRT3_2 - Vz*d_PARAMETER;
#endif
}
/**************************************************************************
函数功能：所有的控制代码都在这里面
         5ms定时中断由MPU6050的INT引脚触发
         严格保证采样和数据处理的时间同步				 
**************************************************************************/
int EXTI15_10_IRQHandler(void) 
{    
	int LX,LY,RX,RY;

	int Yuzhi=20;
	if(INT==0)		
	{     
		EXTI->PR=1<<15;                                                      //清除LINE5上的中断标志位
		if(TimClk)
		{
			TimClk--;
			if(TimClk == 0)
			{
				TimClk = 200;
				LED = ~LED;
			}
		}
		Flag_Target=!Flag_Target;
		if(delay_flag==1)
		{
			if(++delay_50==10)	 delay_50=0,delay_flag=0;                     //给主函数提供50ms的精准延时
		}
																					//===10ms控制一次，为了保证M法测速的时间基准，首先读取编码器数据
#if   ENCODER_DIRECTION
		Encoder_A	=	-Read_Encoder(2);                                          //===读取编码器的值
		Position_A	+=	Encoder_A;                                                 //===积分得到位置 
		Encoder_B	=	+Read_Encoder(3);                                          //===读取编码器的值
		Position_B	+=	Encoder_B;                                                 //===积分得到位置 
		Encoder_C	=	+Read_Encoder(4);                                         //===读取编码器的值
		Position_C	+=	Encoder_C;                                                 //===积分得到位置  
		Encoder_D	=	-Read_Encoder(5);                                       //===读取编码器的值
		Position_D	+=	Encoder_D;                                                 //===积分得到位置    
#else
		Encoder_A	=	+Read_Encoder(2);                                          //===读取编码器的值
		Position_A	+=	Encoder_A;                                                 //===积分得到位置 
		Encoder_B	=	-Read_Encoder(3);                                          //===读取编码器的值
		Position_B	+=	Encoder_B;                                                 //===积分得到位置 
		Encoder_C	=	-Read_Encoder(4);                                         //===读取编码器的值
		Position_C	+=	Encoder_C;                                                 //===积分得到位置  
		Encoder_D	=	+Read_Encoder(5);                                       //===读取编码器的值
		Position_D	+=	Encoder_D;                                                 //===积分得到位置    
#endif

		Read_DMP();                                                            //===更新姿态	
		Voltage_All+=Get_battery_volt();                                       //多次采样累积
		if(++Voltage_Count==100) Voltage=Voltage_All/100,Voltage_All=0,Voltage_Count=0;//求平均值 获取电池电压	       

#if		ENCODER_ENABLE
		Motor_A=Incremental_PI_A(Encoder_A,Target_A);                         //===速度闭环控制计算电机A最终PWM
		Motor_B=Incremental_PI_B(Encoder_B,Target_B);                         //===速度闭环控制计算电机B最终PWM
		Motor_C=Incremental_PI_C(Encoder_C,Target_C);                         //===速度闭环控制计算电机C最终PWM
		Motor_D=Incremental_PI_D(Encoder_D,Target_D);                         //===速度闭环控制计算电机C最终PWM
#else
		Motor_A = Target_A;                         //===速度闭环控制计算电机A最终PWM
		Motor_B = Target_B;                         //===速度闭环控制计算电机B最终PWM
		Motor_C = Target_C;                         //===速度闭环控制计算电机C最终PWM
		Motor_D = Target_D;                         //===速度闭环控制计算电机C最终PWM
#endif
		if(InspectQueue())
		{
			Flag_Direction=OutQueue();  
		}
		else
		{
			if((PS2_LX > 250 && PS2_LY > 250 &&PS2_RX > 250 &&PS2_RY > 250)
				|| (PS2_LX == 0 && PS2_LY == 0 &&PS2_RX == 0 &&PS2_RY == 0))
			{
				PS2_LX = 128;
				PS2_LY = 128;
				PS2_RX = 128;
				PS2_RY = 128;
			}
			LX=PS2_LX-128;
			LY=PS2_LY-128; 
			RX=PS2_RX-128;
			RY=PS2_RY-128;		
			if(LX>-Yuzhi&&LX<Yuzhi)LX=0;
			if(LY>-Yuzhi&&LY<Yuzhi)LY=0;
			if(RX>-Yuzhi&&RX<Yuzhi)RX=0;
			if(RY>-Yuzhi&&RY<Yuzhi)RY=0;
			
			Move_X=LX*RC_Velocity/(400 + RY);
			Move_Y=-LY*RC_Velocity/(400 + RY);	
			if(RX != 0)	Gyro_Bias = Yaw;
			Move_Z=-RX*RC_Velocity/(400 + RY);
		}

		Get_RC(0);
		
		Xianfu_Pwm(MAX_PWM);                     //===PWM限幅
		Set_Pwm(Motor_A,Motor_B,Motor_C,Motor_D);     //===赋值给PWM寄存器  
	}
	return 0;	 
} 


/**************************************************************************
函数功能：赋值给PWM寄存器
入口参数：PWM
返回  值：无
**************************************************************************/
void Set_Pwm(int motor_a,int motor_b,int motor_c,int motor_d)
{
	if(motor_a<0)		INA2=1,			INA1=0;
	else				INA2=0,			INA1=1;
	PWMA=myabs(motor_a);

	if(motor_b<0)		INB2=1,			INB1=0;
	else				INB2=0,			INB1=1;
	PWMB=myabs(motor_b);

	if(motor_c>0)		INC2=1,			INC1=0;
	else				INC2=0,			INC1=1;
	PWMC=myabs(motor_c);

	if(motor_d>0)		IND2=1,			IND1=0;
	else				IND2=0,			IND1=1;
	PWMD=myabs(motor_d);
}

/**************************************************************************
函数功能：限制PWM赋值 
入口参数：幅值
返回  值：无
**************************************************************************/
void Xianfu_Pwm(int amplitude)
{	
	if(Motor_A<-amplitude) Motor_A=-amplitude;	
	if(Motor_A>amplitude)  Motor_A=amplitude;	
	if(Motor_B<-amplitude) Motor_B=-amplitude;	
	if(Motor_B>amplitude)  Motor_B=amplitude;		
	if(Motor_C<-amplitude) Motor_C=-amplitude;	
	if(Motor_C>amplitude)  Motor_C=amplitude;		
	if(Motor_D<-amplitude) Motor_D=-amplitude;	
	if(Motor_D>amplitude)  Motor_D=amplitude;		
}

/**************************************************************************
函数功能：异常关闭电机
入口参数：电压
返回  值：1：异常  0：正常
**************************************************************************/
u8 Turn_Off( int voltage)
{
	u8 temp;
	if(voltage<2000||EN==0)//电池电压低于22.2V关闭电机
	{	                                                
		temp=1;      
		PWMA=0;
		PWMB=0;
		PWMC=0;
		PWMD=0;							
	}
	else
		temp=0;
	return temp;			
}

/**************************************************************************
函数功能：绝对值函数
入口参数：long int
返回  值：unsigned int
**************************************************************************/
u32 myabs(long int a)
{ 		   
	u32 temp;
		if(a<0)  temp=-a;  
	else temp=a;
	return temp;
}
/**************************************************************************
函数功能：增量PI控制器
入口参数：编码器测量值，目标速度
返回  值：电机PWM
根据增量式离散PID公式 
pwm+=Kp[e（k）-e(k-1)]+Ki*e(k)+Kd[e(k)-2e(k-1)+e(k-2)]
e(k)代表本次偏差 
e(k-1)代表上一次的偏差  以此类推 
pwm代表增量输出
在我们的速度控制闭环系统里面，只使用PI控制
pwm+=Kp[e（k）-e(k-1)]+Ki*e(k)
**************************************************************************/
int Incremental_PI_A (int Encoder,int Target)
{ 	
	 static int Bias,Pwm,Last_bias;
	 Bias=Encoder-Target;                //计算偏差
	 Pwm+=Velocity_KP*(Bias-Last_bias)+Velocity_KI*Bias;   //增量式PI控制器
	 if(Pwm>MAX_PWM)	Pwm=MAX_PWM;
	 if(Pwm<-MAX_PWM)	Pwm=-MAX_PWM;
	 Last_bias=Bias;	                   //保存上一次偏差 
	 return Pwm;                         //增量输出
}
int Incremental_PI_B (int Encoder,int Target)
{ 	
	 static int Bias,Pwm,Last_bias;
	 Bias=Encoder-Target;                //计算偏差
	 Pwm+=Velocity_KP*(Bias-Last_bias)+Velocity_KI*Bias;   //增量式PI控制器
	 if(Pwm>MAX_PWM)	Pwm=MAX_PWM;
	 if(Pwm<-MAX_PWM)	Pwm=-MAX_PWM;
	 Last_bias=Bias;	                   //保存上一次偏差 
	 return Pwm;                         //增量输出
}
int Incremental_PI_C (int Encoder,int Target)
{ 	
	 static int Bias,Pwm,Last_bias;
	 Bias=Encoder-Target;                                  //计算偏差
	 Pwm+=Velocity_KP*(Bias-Last_bias)+Velocity_KI*Bias;   //增量式PI控制器
	 if(Pwm>MAX_PWM)	Pwm=MAX_PWM;
	 if(Pwm<-MAX_PWM)	Pwm=-MAX_PWM;
	 Last_bias=Bias;	                   //保存上一次偏差 
	 return Pwm;                         //增量输出
}
int Incremental_PI_D (int Encoder,int Target)
{ 	
	 static int Bias,Pwm,Last_bias;
	 Bias=Encoder-Target;                                  //计算偏差
	 Pwm+=Velocity_KP*(Bias-Last_bias)+Velocity_KI*Bias;   //增量式PI控制器
	 if(Pwm>MAX_PWM)	Pwm=MAX_PWM;
	 if(Pwm<-MAX_PWM)	Pwm=-MAX_PWM;
	 Last_bias=Bias;	                   //保存上一次偏差 
	 return Pwm;                         //增量输出
}
/**************************************************************************
函数功能：通过串口指令对小车进行遥控
入口参数：串口指令
返回  值：无
**************************************************************************/
void Get_RC(u8 mode)
{
	float step=0.25;  //设置速度控制步进值。
	u8 Flag_Move=1;

	switch(Flag_Direction)   //方向控制
	{
		case 'A':	Move_X=0;		Move_Y+=step;				Flag_Move=1;	break;
		case 'B':	Move_X+=step;	Move_Y+=step;				Flag_Move=1;	break;
		case 'C':	Move_X+=step;	Move_Y=0;					Flag_Move=1;	break;
		case 'D':	Move_X+=step;	Move_Y-=step;				Flag_Move=1;	break;
		case 'E':	Move_X=0;		Move_Y-=step;				Flag_Move=1;	break;
		case 'F':	Move_X-=step;	Move_Y-=step;				Flag_Move=1;	break;
		case 'G':	Move_X-=step;	Move_Y=0;					Flag_Move=1;	break;
		case 'H':	Move_X-=step;	Move_Y+=step;				Flag_Move=1;	break; 
		case 'Z':	Move_X = 0;		Move_Y=0;		Move_Z=0;					break;
		case 'L':	RC_Velocity = 30;											break;
		case 'M':	RC_Velocity = 10;											break;
		case 'a':	break;
		case 'b':	Move_Z-=step;		Gyro_Bias = Yaw;	break;
		case 'c':	break;
		case 'd':	Move_Z+=step;		Gyro_Bias = Yaw;	break;

		case 'O':	break;	//控制头部 
		case 'N': 	break;	//控制转向
		case 'I': 	break;	//遥控
		case 'J': 	break;	//跟随
		case 'K': 	break;	//避障
		case 'P': 	break;	//LED_on
		case 'p': 	break;	//LED_off
		default: Flag_Move=0;        Move_X=Move_X/1.04;	Move_Y=Move_Y/1.04;	  break;	 
	}
	if(Flag_Move==1)		Flag_Left=0,Flag_Right=0;//Move_Z=0;
	if(Move_X<-RC_Velocity)	Move_X=-RC_Velocity;	   //速度控制限幅
	if(Move_X>RC_Velocity)	Move_X=RC_Velocity;	     
	if(Move_Y<-RC_Velocity)	Move_Y=-RC_Velocity;	
	if(Move_Y>RC_Velocity)	Move_Y=RC_Velocity;	 
	if(Move_Z<-RC_Velocity)	Move_Z=-RC_Velocity;	
	if(Move_Z>RC_Velocity)	Move_Z=RC_Velocity;	 
	
	Kinematic_Analysis(Move_X,Move_Y,Move_Z);//得到控制目标值，进行运动学分析
}
