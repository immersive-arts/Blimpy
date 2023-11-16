/****************************************************************************
 * apps/examples/hello/hello_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obt perf_counter.cain a copy of the License at
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
#include "../../wheely/motor/perf_counter.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

volatile bool daemon_should_run;
static int pid_daemon;
static int missed;

static uint32_t timing[] =
{ 22555, 9354, 7186, 6065, 5349, 4842, 4457, 4153, 3905, 3698, 3521, 3369, 3235,
		3116, 3010, 2915, 2828, 2749, 2677, 2611, 2549, 2492, 2439, 2389, 2343,
		2299, 2258, 2219, 2182, 2148, 2115, 2083, 2054, 2025, 1998, 1972, 1947,
		1924, 1901, 1879, 1858, 1838, 1818, 1799, 1781, 1764, 1747, 1730, 1715,
		1699, 1685, 1670, 1656, 1643, 1630, 1617, 1605, 1593, 1581, 1570, 1559,
		1548, 1538, 1528, 1518, 1508, 1499, 1490, 1481, 1472, 1463, 1455, 1447,
		1439, 1431, 1424, 1416, 1409, 1402, 1395, 1388, 1382, 1375, 1369, 1362,
		1356, 1350, 1344, 1339, 1333, 1328, 1322, 1317, 1312, 1307, 1302, 1297,
		1292, 1287, 1283, 1278, 1274, 1269, 1265, 1261, 1257, 1252, 1248, 1245,
		1241, 1237, 1233, 1230, 1226, 1222, 1219, 1216, 1212, 1209, 1206, 1203,
		1200, 1196, 1193, 1191, 1188, 1185, 1182, 1179, 1177, 1174, 1171, 1169,
		1166, 1164, 1161, 1159, 1157, 1154, 1152, 1150, 1148, 1146, 1143, 1141,
		1139, 1137, 1135, 1133, 1132, 1130, 1128, 1126, 1124, 1123, 1121, 1119,
		1118, 1116, 1115, 1113, 1112, 1110, 1109, 1107, 1106, 1105, 1103, 1102,
		1101, 1100, 1098, 1097, 1096, 1095, 1094, 1093, 1092, 1091, 1090, 1089,
		1088, 1087, 1086, 1085, 1084, 1083, 1083, 1082, 1081, 1080, 1080, 1079,
		1078, 1078, 1077, 1077, 1076, 1076, 1075, 1075, 1074, 1074, 1073, 1073,
		1072, 1072, 1072, 1072, 1071, 1071, 1071, 1071, 1070, 1070, 1070, 1070,
		1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070,
		1070, 1070, 1071, 1071, 1071, 1071, 1072, 1072, 1072, 1072, 1073, 1073,
		1074, 1074, 1075, 1075, 1076, 1076, 1077, 1077, 1078, 1078, 1079, 1080,
		1080, 1081, 1082, 1083, 1083, 1084, 1085, 1086, 1087, 1088, 1089, 1090,
		1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1100, 1101, 1102, 1103,
		1105, 1106, 1107, 1109, 1110, 1112, 1113, 1115, 1116, 1118, 1119, 1121,
		1123, 1124, 1126, 1128, 1130, 1132, 1133, 1135, 1137, 1139, 1141, 1143,
		1146, 1148, 1150, 1152, 1154, 1157, 1159, 1161, 1164, 1166, 1169, 1171,
		1174, 1177, 1179, 1182, 1185, 1188, 1191, 1193, 1196, 1200, 1203, 1206,
		1209, 1212, 1216, 1219, 1222, 1226, 1230, 1233, 1237, 1241, 1245, 1248,
		1252, 1257, 1261, 1265, 1269, 1274, 1278, 1283, 1287, 1292, 1297, 1302,
		1307, 1312, 1317, 1322, 1328, 1333, 1339, 1344, 1350, 1356, 1362, 1369,
		1375, 1382, 1388, 1395, 1402, 1409, 1416, 1424, 1431, 1439, 1447, 1455,
		1463, 1472, 1481, 1490, 1499, 1508, 1518, 1528, 1538, 1548, 1559, 1570,
		1581, 1593, 1605, 1617, 1630, 1643, 1656, 1670, 1685, 1699, 1715, 1730,
		1747, 1764, 1781, 1799, 1818, 1838, 1858, 1879, 1901, 1924, 1947, 1972,
		1998, 2025, 2054, 2083, 2115, 2148, 2182, 2219, 2258, 2299, 2343, 2389,
		2439, 2492, 2549, 2611, 2677, 2749, 2828, 2915, 3010, 3116, 3235, 3369,
		3521, 3698, 3905, 4153, 4457, 4842, 5349, 6065, 7186, 9354, 22555 };

static int N = 444;

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

struct timespec rel;
uint32_t debug_timeout;
static void my_usleep(int fd, uint32_t timeout)
{
	ioctl(fd, TCIOC_STOP, 0);
	ioctl(fd, TCIOC_SETTIMEOUT, timeout);
	ioctl(fd, TCIOC_START, 0);

	//printf("Hello\n");
	siginfo_t info;
	sigset_t mask;
	sigaddset(&mask, SIGUSR1);
	struct timespec t1;
	clock_gettime(0, &t1);
	sigwaitinfo(&mask, &info);
	struct timespec t2;
	clock_gettime(0, &t2);

	clock_timespec_subtract(&t2, &t1, &rel);
	debug_timeout = timeout;

	//printf("Time: %d %d\n", rel.tv_nsec*1000, timeout);

}

static int open_pin(char *file_path)
{
	int fd = open(file_path, O_RDWR);
	if (fd < 0)
	{
		int errcode = errno;
		fprintf(stderr, "ERROR: Failed to open %s: %d\n", file_path, errcode);
		return EXIT_FAILURE;
	}

	int ret = ioctl(fd, GPIOC_SETPINTYPE, (unsigned long) 3);
	if (ret < 0)
	{
		int errcode = errno;
		fprintf(stderr, "ERROR: Failed to set pintype on %s: %d\n", file_path,
				errcode);
		close(fd);
		return EXIT_FAILURE;
	}

	return fd;

}

static void set_pin(int file_descriptor, bool high_low, char *file_path)
{
	int ret = ioctl(file_descriptor, GPIOC_WRITE, (unsigned long) high_low);
	if (ret < 0)
	{
		int errcode = errno;
		fprintf(stderr, "ERROR: Failed to write value %s from %d: %d\n",
				file_path, (unsigned int) high_low, errcode);
		close(file_descriptor);
		return EXIT_FAILURE;
	}
}

int task_spawn_cmd(const char *name, int scheduler, int priority,
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

/****************************************************************************
 * Name: Daemon
 ****************************************************************************/
int Daemon(int argc, char *argv[])
{

	printf("motor!!\n");
	printf("PID: %d %d\n", getpid(), pid_daemon);

	bool direction = false;
	int i = 0;

	char enable_pin[] = "/dev/gpio0";
	char dir_pin[] = "/dev/gpio1";
	char step_pin[] = "/dev/gpio2";

	/* Open the pin driver */
	int fd_enable = open_pin(enable_pin);
	int fd_dir = open_pin(dir_pin);
	int fd_step = open_pin(step_pin);
	/* Enable motor (0: enabled, 1: disabled) */
	set_pin(fd_enable, 0, enable_pin);

	perf_counter_t perf_counter_interval;
	perf_counter_interval = perf_alloc(PC_INTERVAL, "motor_interval");

	perf_counter_t perf_counter_dt;
	perf_counter_dt = perf_alloc (PC_ELAPSED, "motor_dt");

	struct timespec now;

	uint32_t sum = 0;
	uint32_t usleep_samples = 10000;
	uint32_t usleep_time = 100;
	for(int k=0; k < usleep_samples; k++)
	{
		struct timespec t1;
		clock_gettime(0, &t1);
		usleep(usleep_time);
		struct timespec t2;
		clock_gettime(0, &t2);

		clock_timespec_subtract(&t2, &t1, &rel);

		sum += rel.tv_nsec/1000;
	}

	printf("Delay: %u\n", sum/usleep_samples);

	uint32_t delay = sum/usleep_samples - usleep_time + 17;

	printf("Delay: %d\n", delay);

	sum = 0;
	uint32_t sum2 = 0;

	for(int k=0; k < N; k++)
	{
		struct timespec t1;
		clock_gettime(0, &t1);
		usleep(timing[k]);
		struct timespec t2;
		clock_gettime(0, &t2);

		clock_timespec_subtract(&t2, &t1, &rel);

		sum2 += timing[k];
		sum += rel.tv_nsec/1000;
	}

	printf("Delay: %d\n", sum/N - sum2/N);


	daemon_should_run = true;
	while (daemon_should_run)
	{
		if (direction)
			set_pin(fd_dir, 1, dir_pin);
		else
			set_pin(fd_dir, 0, dir_pin);

		set_pin(fd_step, 1, step_pin);
		if (timing[i] - delay > (uint32_t) 100)
		{
			usleep(timing[i++] - delay - 100);
		}

		set_pin(fd_step, 0, step_pin);
		perf_begin(perf_counter_dt);
		usleep(100 - delay);
		perf_end(perf_counter_dt);

		if (i == N)
		{
			perf_count(perf_counter_interval);
			i = 0;
			direction = !direction;
		}
	}
	set_pin(fd_enable, 1, enable_pin);

	perf_free(perf_counter_interval);
	perf_free(perf_counter_dt);

	close(fd_enable);
	close(fd_step);
	close(fd_dir);

	printf("Daemon stopped\n");

	return 0;
}

/****************************************************************************
 * Name: StartDaemon
 ****************************************************************************/
int StartDaemon(void)
{
	if (daemon_should_run)
	{
		printf("Daemon already running.\n");
		exit(0);
	}

	pid_daemon = task_spawn_cmd("motor_daemon", SCHED_RR,
	200, 6000, Daemon, NULL);

	return 0;
}

/****************************************************************************
 * Name: StopDaemon
 ****************************************************************************/
int StopDaemon(void)
{
	if (!daemon_should_run)
		return 0;

	daemon_should_run = false;

	perf_reset_all();

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
		printf("Missed: %d\n", missed);
		perf_print_all();
		return 0;
	}

	return 0;
}
