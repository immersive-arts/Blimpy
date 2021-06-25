// <<< Use Configuration Wizard in Context Menu >>>

//<h>系统配置
	//<e>调试信息
	#define DEBUG_ENABLE					1
	//</e>

//</h>


//<h>小车控制配置
	//<e>陀螺仪
	#define MPU6xxx_ENABLE					1
		//<c>使用板载IMU
		#define  USE_BOARD_IMU  			1
		//</c>

		//<c>启用Z轴自旋
		//#define  AXLE_Z_RESTRAIN  			0
		//</c>
	//</e>
	
	//<e>使用编码器
	#define ENCODER_ENABLE					1

		//<c>编码器方向 1: 正向，0: 反向
		#define ENCODER_DIRECTION			1
		//</c>
	//</e>

//</h>


//<e>OLED显示控制
//<i>选择是否开启OLED显示
#define  OLED_DISPLAY_ENABLE		0

//</e>

// <<< end of configuration section >>>

