#ifndef __FILTER_H
#define __FILTER_H
  /**************************************************************************
作者：墨比斯科技
我的淘宝小店：https://moebius.taobao.com/
**************************************************************************/
extern float angle, angle_dot; 	
void Kalman_Filter(float Accel,float Gyro);		
void Yijielvbo(float angle_m, float gyro_m);
#endif
