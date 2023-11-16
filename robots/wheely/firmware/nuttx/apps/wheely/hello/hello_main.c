/****************************************************************************
 * apps/examples/hello/hello_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * #include <nuttx/sensors/qencoder.h>
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <nuttx/timers/timer.h>
#include <signal.h>
#include <nuttx/ioexpander/gpio.h>
#include <nuttx/timers/pwm.h>
#include <nuttx/sensors/qencoder.h>
#include <math.h>

#include "perf_counter.h"
#define d_PARAMETER          (0.38f)
#define SQRT3_2				 (0.866f)
#define TICKS_PER_REV	     (660.0f)
#define LOOP_PERIOD          (0.01f)
#define MIN_V				 (0.05f)
#define R					 (0.065f) // wheel radius [m]
#define D					 (0.38f) // wheel location [m]
#define VZ_MAX				 (2.0f) // max rotational speed [rad/s]
#define W2DC_m				 (168.0f) // wheel speed to DC linear map
#define W2DC_c				 (3024.0f) // wheel speed to DC constant map

/****************************************************************************
 * Public Functions
 ****************************************************************************/

static volatile bool daemon_should_run;
static int pid_daemon;
static int fd_CS, fd_CLK, fd_DATA, fd_CMD;
uint8_t Comd[2]={0x01,0x42};
uint8_t Data[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
bool PRINT = false;

#if 0
static void kill_task(FAR struct tcb_s *tcb, FAR void *arg)
{
	kill(tcb->pid, SIGUSR1);
}

void killall(void)
{
//	printf("Sending SIGUSR1 to all processes now\n");

	/* iterate through all tasks and send kill signal */
	sched_foreach(kill_task, NULL);
}
#endif

void write_data(uint8_t CMD)
{
	volatile uint16_t ref=0x01;
	Data[1] = 0;
	for(ref=0x01;ref<0x0100;ref<<=1)
	{
		if(ref&CMD)
		{
			ioctl(fd_CMD, GPIOC_WRITE, (unsigned long)1);
		}
		else
			ioctl(fd_CMD, GPIOC_WRITE, (unsigned long)0);

		bool invalue;
		ioctl(fd_DATA, GPIOC_READ, (unsigned long)((uintptr_t)&invalue));

		if(invalue)
			Data[1] = ref|Data[1];

		ioctl(fd_CLK, GPIOC_WRITE, (unsigned long)1);
		up_udelay(1);
		ioctl(fd_CLK, GPIOC_WRITE, (unsigned long)0);
		up_udelay(1);
		ioctl(fd_CLK, GPIOC_WRITE, (unsigned long)1);

	}
	up_udelay(20);
}

static void read_data(void)
{
	volatile uint8_t byte = 0;
	volatile uint16_t ref = 0x01;

	for(byte = 0; byte < 9; byte++)
		Data[byte] = 0x00;

	ioctl(fd_CS, GPIOC_WRITE, (unsigned long)0);;
	write_data(Comd[0]);
	write_data(Comd[1]);
	for(byte=2;byte<9;byte++)
	{
		for(ref=0x01;ref<0x100;ref<<=1)
		{
			bool invalue;
			ioctl(fd_DATA, GPIOC_READ, (unsigned long)((uintptr_t)&invalue));

			if(invalue)
			  Data[byte] = ref|Data[byte];

			ioctl(fd_CLK, GPIOC_WRITE, (unsigned long)1);
			up_udelay(1);
			ioctl(fd_CLK, GPIOC_WRITE, (unsigned long)0);
			up_udelay(1);
			ioctl(fd_CLK, GPIOC_WRITE, (unsigned long)1);
		}
		up_udelay(20);
	}
	ioctl(fd_CS, GPIOC_WRITE, (unsigned long)1);
}

static int task_spawn_cmd(const char *name, int scheduler, int priority,
		int stack_size, main_t entry, FAR char *const argv[])
{
	int pid;

	sched_lock();

	/* create the task */
	pid = task_create(name, priority, stack_size, entry, argv);

	if (pid > 0)
	{

		/* configure the scheduler */
		struct sched_param param;

		param.sched_priority = priority;
		sched_setscheduler(pid, scheduler, &param);

		/* XXX do any other private task accounting here before the task starts */
	}

	sched_unlock();

	return pid;
}

int16_t int16_t_from_bytes(uint8_t bytes[])
{
	union {
		uint8_t    b[2];
		int16_t    w;
	} u;

	u.b[1] = bytes[0];
	u.b[0] = bytes[1];

	return u.w;
}

void my_usleep(int usecs) {
  #define STEPS_PER_SEC 1
  unsigned int i,s;
  for (s=0; s < usecs; s++) {
    for (i=0; i < STEPS_PER_SEC; i++) {
       // skip CPU cycle or any other statement(s) for making loop
       // untouched by C compiler code optimizations
       asm("nop");
    }
  }
}

/****************************************************************************
 * Name: Daemon
 ****************************************************************************/
static int Daemon(int argc, char *argv[])
{

	printf("Hello, World MAXI1M!!\n");
	printf("PID: %d %d\n", getpid(), pid_daemon);

	/* set up periodic signalling stuff */
	int tmr_id = open("/dev/timer0", O_RDONLY);
	if (tmr_id < 0)
	{
		fprintf(stderr, "ERROR: Failed to open timer: %d\n", errno);
		close(tmr_id);
		return EXIT_FAILURE;
	}

	printf("ID: %d\n", tmr_id);
	ioctl(tmr_id, TCIOC_SETTIMEOUT, 10000);

	struct timer_notify_s notify;

	notify.pid = getpid();
	notify.periodic = true;

	notify.event.sigev_notify = SIGEV_SIGNAL;
	notify.event.sigev_signo = SIGUSR1;
	notify.event.sigev_value.sival_ptr = NULL;

	int ret =ioctl(tmr_id, TCIOC_NOTIFICATION, (unsigned long) ((uintptr_t) &notify));
	if (ret < 0)
	{
		fprintf(stderr, "ERROR: Failed to set the timer handler: %d\n", errno);
		close(tmr_id);
		return EXIT_FAILURE;
	}

	ret = ioctl(tmr_id, TCIOC_START, 0);
	if (ret < 0)
	{
		fprintf(stderr, "ERROR: Failed to start the timer: %d\n", errno);
		close(tmr_id);
		return EXIT_FAILURE;
	}

	sigset_t mask;
	sigaddset(&mask, SIGUSR1);

	perf_counter_t perf_counter_interval;
	perf_counter_interval = perf_alloc (PC_INTERVAL, "fastloop_interval");

	perf_counter_t perf_counter_dt;
	perf_counter_dt = perf_alloc (PC_ELAPSED, "fastloop_dt");

	int fd_IMU = open("/dev/imu0", O_RDONLY);

	fd_DATA = open("/dev/gpio0", O_RDONLY);
	fd_CS = open("/dev/gpio1", O_WRONLY);
	fd_CLK = open("/dev/gpio2", O_WRONLY);
	fd_CMD = open("/dev/gpio3", O_WRONLY);

	int fd_A1 = open("/dev/gpio10", O_WRONLY);
	int fd_A2 = open("/dev/gpio11", O_WRONLY);

	int fd_B1 = open("/dev/gpio4", O_WRONLY);
	int fd_B2 = open("/dev/gpio5", O_WRONLY);

	int fd_C1 = open("/dev/gpio6", O_WRONLY);
	int fd_C2 = open("/dev/gpio7", O_WRONLY);

    int fd_PWM = open("/dev/pwm0", O_RDONLY);

    int fd_QE_A = open("/dev/qe0", O_RDONLY);
    int fd_QE_B = open("/dev/qe1", O_RDONLY);
    int fd_QE_C = open("/dev/qe2", O_RDONLY);

#pragma pack(push, 1)
	struct MPUReport {
		uint8_t		accel_x[2];
		uint8_t		accel_y[2];
		uint8_t		accel_z[2];
		uint8_t		temp[2];
		uint8_t		gyro_x[2];
		uint8_t		gyro_y[2];
		uint8_t		gyro_z[2];
	} mpu_report;
#pragma pack(pop)

	int32_t position_A;
	int32_t position_B;
	int32_t position_C;

	int32_t previous_position_A = 0;
	int32_t previous_position_B = 0;
	int32_t previous_position_C = 0;

	float speed_A = 0.0f;
	float speed_B = 0.0f;
	float speed_C = 0.0f;

	float alpha = LOOP_PERIOD / (LOOP_PERIOD + 0.1f);

	daemon_should_run = true;
	while (daemon_should_run)
	{
		//struct timespec t;
		//clock_gettime(0, &t);
		//printf("Wait: %d %d\n", t.tv_sec, t.tv_nsec);
		siginfo_t info;
		sigwaitinfo(&mask, &info);
		perf_count (perf_counter_interval);
		perf_begin (perf_counter_dt);

		read_data();

		read(fd_IMU, (uint8_t *)&mpu_report, sizeof(mpu_report));

		int16_t accX = int16_t_from_bytes(mpu_report.accel_x);
		int16_t accY = int16_t_from_bytes(mpu_report.accel_y);
		int16_t accZ = int16_t_from_bytes(mpu_report.accel_z);
		int16_t temp = int16_t_from_bytes(mpu_report.temp);
	    int16_t gyroX = int16_t_from_bytes(mpu_report.gyro_x);
	    int16_t gyroY = int16_t_from_bytes(mpu_report.gyro_y);
	    int16_t gyroZ = int16_t_from_bytes(mpu_report.gyro_z);

	    /* Configure the characteristics of the pulse train */
	    struct pwm_info_s pwm_info;
		pwm_info.frequency = 20000;

		float Vx = -(Data[7]-127)/128.0f;
		float Vy = (Data[8]-127)/128.0f;
		float Vz = (Data[5]-127)/128.0f * VZ_MAX;

		if (fabs(Vx) < MIN_V)
			Vx = 0.0f;
		if (fabs(Vy) < MIN_V)
			Vy = 0.0f;
		if (fabs(Vz) < MIN_V)
			Vz = 0.0f;

		float Va = -Vx + Vz*D;
		float Vb = Vx/2 - Vy*SQRT3_2 + Vz*D;
		float Vc = Vx/2 + Vy*SQRT3_2 + Vz*D;

		float wa = Va/R*60.0f/2.0f/3.14f;
		float wb = Vb/R*60.0f/2.0f/3.14f;
		float wc = Vc/R*60.0f/2.0f/3.14f;

		b16_t dc_A = 0;
		b16_t dc_B = 0;
		b16_t dc_C = 0;

		ret = ioctl(fd_QE_A, QEIOC_POSITION, (unsigned long)((uintptr_t)&position_A));
		ret = ioctl(fd_QE_B, QEIOC_POSITION, (unsigned long)((uintptr_t)&position_B));
		ret = ioctl(fd_QE_C, QEIOC_POSITION, (unsigned long)((uintptr_t)&position_C));

		//printf("QE: %li %li %li \n", position_A, position_B, position_C);

		speed_A = (1 - alpha) * speed_A + alpha * (position_A - previous_position_A)/(float)LOOP_PERIOD/TICKS_PER_REV*60.0f;
		speed_B = (1 - alpha) * speed_B + alpha * (position_B - previous_position_B)/(float)LOOP_PERIOD/TICKS_PER_REV*60.0f;
		speed_C = (1 - alpha) * speed_C + alpha * (position_C - previous_position_C)/(float)LOOP_PERIOD/TICKS_PER_REV*60.0f;

		previous_position_A = position_A;
		previous_position_B = position_B;
		previous_position_C = position_C;

		if(Data[1] == 115)
		{
			float kp = 100.0f;

			dc_A = (b16_t)(wa * W2DC_m + W2DC_c + (wa - speed_A) * kp);
			dc_B = (b16_t)(wb * W2DC_m + W2DC_c + (wb - speed_B) * kp);
			dc_C = (b16_t)(wc * W2DC_m + W2DC_c + (wc - speed_C) * kp);
		}

	    if (PRINT)
	    {
			printf("PS2: %d %d %d %d %d %d %d %d %d\n", Data[0], Data[1], Data[2], Data[3], Data[4], Data[5], Data[6], Data[7], Data[8]);
			printf("DC: %ld %ld %ld\n", dc_A, dc_B, dc_C);
			printf("IMU: %d %d %d %d %d %d %d\n", temp, accX, accY, accZ, gyroX, gyroY, gyroZ);
	    }

		if(dc_A > 127)
		{
			ioctl(fd_A1, GPIOC_WRITE, (unsigned long)1);
			ioctl(fd_A2, GPIOC_WRITE, (unsigned long)0);
		}
		else
		{
			ioctl(fd_A1, GPIOC_WRITE, (unsigned long)0);
			ioctl(fd_A2, GPIOC_WRITE, (unsigned long)1);
		}

		if(dc_B > 127)
		{

			ioctl(fd_B1, GPIOC_WRITE, (unsigned long)0);
			ioctl(fd_B2, GPIOC_WRITE, (unsigned long)1);
		}
		else
		{

			ioctl(fd_B1, GPIOC_WRITE, (unsigned long)1);
			ioctl(fd_B2, GPIOC_WRITE, (unsigned long)0);
		}

		if(dc_C > 127)
		{
			ioctl(fd_C1, GPIOC_WRITE, (unsigned long)1);
			ioctl(fd_C2, GPIOC_WRITE, (unsigned long)0);
		}
		else
		{
			ioctl(fd_C1, GPIOC_WRITE, (unsigned long)0);
			ioctl(fd_C2, GPIOC_WRITE, (unsigned long)1);
		}

	    pwm_info.channels[0].channel = 4;
    	pwm_info.channels[0].duty = abs(dc_A);
	    pwm_info.channels[1].channel = 3;
    	pwm_info.channels[1].duty = abs(dc_B);
	    pwm_info.channels[2].channel = 2;
    	pwm_info.channels[2].duty = abs(dc_C);
	    pwm_info.channels[3].channel = 1;
    	pwm_info.channels[3].duty = 0;


	    ret = ioctl(fd_PWM, PWMIOC_SETCHARACTERISTICS, (unsigned long)((uintptr_t)&pwm_info));

	    ret = ioctl(fd_PWM, PWMIOC_START, 0);

		printf("DC: %ld %ld %ld Speed: %.2f (%.2f) %.2f (%.2f) %.2f (%.2f) Vx: %.2f Vy: %.2f Vz: %.2f\n", dc_A, dc_B, dc_C, speed_A, wa, speed_B, wb, speed_C, wc, Vx, Vy, Vz);
		//printf("DC: %ld %ld %ld\n", dc_A, dc_B, dc_C);

		perf_end(perf_counter_dt);

		//clock_gettime(0, &t);
		//printf("Pass: %d %d\n", t.tv_sec, t.tv_nsec);
	}

	ret = ioctl(tmr_id, TCIOC_STOP, 0);

	close(tmr_id);
	close(fd_IMU);
	close(fd_CS);
	close(fd_DATA);
	close(fd_CLK);
	close(fd_CMD);
	close(fd_PWM);
	close(fd_A1);
	close(fd_A2);
	close(fd_B1);
	close(fd_B2);
	close(fd_C1);
	close(fd_C2);
	close(fd_QE_A);
	close(fd_QE_B);
	close(fd_QE_C);

	perf_free(perf_counter_interval);
	perf_free(perf_counter_dt);

	printf("Daemon stopped\n");

	return 0;
}

/****************************************************************************
 * Name: StartDaemon
 ****************************************************************************/
static int StartDaemon(void)
{
	if (daemon_should_run)
	{
		printf("Daemon already running.\n");
		exit(0);
	}

	pid_daemon = task_spawn_cmd("fastloop_daemon", SCHED_RR,
			SCHED_PRIORITY_DEFAULT, 2000, Daemon, NULL);

	return 0;
}

/****************************************************************************
 * Name: StopDaemon
 ****************************************************************************/
static int StopDaemon(void)
{
	if (!daemon_should_run)
		return 0;

	daemon_should_run = false;

	return 0;
}

/****************************************************************************
 * hello_main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{

	if (!strcmp(argv[1], "start"))
		return StartDaemon();
	if (!strcmp(argv[1], "stop"))
		return StopDaemon();

	if (!strcmp(argv[1], "print"))
	{
		perf_print_all();
		PRINT = !PRINT;
		return 0;
	}

	return 0;
}
