//===============================================================
//系统设置

#define		CRYSTAL_FREQUENCE		72000000UL			//Hz

#define		SYS_TICK            	5					//ms
#define		UART1_BAUD_RATE			9600

//===============================================================
//位带操作

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8)  //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8)  //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8)  //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8)  //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8)  //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8)  //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8)  //0x40011E08 

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n) 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n) 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n) 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n) 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n) 

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n) 

//===============================================================
//端口定义


//===============================================================
//定时值

#define		_5ms					5   / SYS_TICK
#define		_10ms					10   / SYS_TICK
#define		_20ms					20   / SYS_TICK
#define		_30ms					30   / SYS_TICK
#define		_40ms					40   / SYS_TICK
#define		_50ms					50   / SYS_TICK
#define		_60ms					60   / SYS_TICK
#define		_70ms					70   / SYS_TICK
#define		_80ms					80   / SYS_TICK
#define		_90ms					90   / SYS_TICK
#define		_100ms					100  / SYS_TICK

#define		_110ms					110  / SYS_TICK
#define		_120ms					120  / SYS_TICK
#define		_130ms					130  / SYS_TICK
#define		_140ms					140  / SYS_TICK
#define		_150ms					150  / SYS_TICK
#define		_160ms					160  / SYS_TICK
#define		_170ms					170  / SYS_TICK
#define		_180ms					180  / SYS_TICK
#define		_190ms					190  / SYS_TICK
#define		_200ms					200  / SYS_TICK
#define		_300ms					300  / SYS_TICK
#define		_400ms					400  / SYS_TICK
#define		_350ms					350  / SYS_TICK
#define		_500ms					500  / SYS_TICK
#define		_600ms					600  / SYS_TICK
#define		_800ms					800  / SYS_TICK
#define		_900ms					900  / SYS_TICK

#define		_1000ms					1000 / SYS_TICK
#define		_2000ms					2000 / SYS_TICK
#define		_3000ms					3000 / SYS_TICK
#define		_4000ms					4000 / SYS_TICK
#define		_5000ms					5000 / SYS_TICK
#define		_6000ms					6000 / SYS_TICK
#define		_7000ms					7000 / SYS_TICK
#define		_8000ms					8000 / SYS_TICK
#define		_9000ms					9000 / SYS_TICK
#define		_10000ms				10000 / SYS_TICK
