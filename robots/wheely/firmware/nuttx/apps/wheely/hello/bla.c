/****************************************************************************
 * Private Data
 ****************************************************************************/
volatile bool threadRunning;
uint8_t ID;
Onboard::FastLoopLogic* logic;
/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: Thread
 ****************************************************************************/
int
Daemon (int argc, char *argv[])
{

	perf_counter_t perf_counter_interval;
	perf_counter_interval = perf_alloc (PC_INTERVAL, "fastloop_interval");

	perf_counter_t perf_counter_dt;
	perf_counter_dt = perf_alloc (PC_ELAPSED, "fastloop_dt");

	/* set up periodic signalling stuff */
	int tmr_id = open ("/dev/timer6", O_RDONLY);
	ioctl (tmr_id, TCIOC_SETTIMEOUT, 2000);
	struct timer_notify_s notify =
		{ .arg = NULL, .pid = getpid (), .signo = SIGUSR1, };
	ioctl (tmr_id, TCIOC_NOTIFICATION, (unsigned long) ((uintptr_t) &notify));
	ioctl (tmr_id, TCIOC_START, 0);
	logic = new Onboard::FastLoopLogic (ID);

	threadRunning = true;
	uint64_t lastTime = clock_systimer();

	while (threadRunning)
		{
			sigwaitinfo ( NULL, NULL);
			perf_count (perf_counter_interval);
			idsc_setled (LED_BLUE, 1);
			perf_begin (perf_counter_dt);
			uint64_t now = clock_systimer ();
			uint32_t microSecondsPassed = (uint32_t) (now - lastTime);
			lastTime = now;
			logic->Run (microSecondsPassed);
			perf_end (perf_counter_dt);
			idsc_setled (LED_BLUE, 0);
		}

	threadRunning = false;

	close(tmr_id);
	delete logic;
	perf_free (perf_counter_interval);
	perf_free (perf_counter_dt);
	return 0;
}

/****************************************************************************
 * Name: StartDaemon
 ****************************************************************************/
int
StartDaemon (void)
{
	if (threadRunning)
		{
			printf ("Daemon already running.\n");
			exit (0);
		}

	task_spawn_cmd ("fastloop_daemon", SCHED_DEFAULT, SCHED_PRIORITY_MAX, 2000,
									Daemon, NULL);

	return 0;
}

/****************************************************************************
 * Name: StopDaemon
 ****************************************************************************/
int
StopDaemon (void)
{
	if (!threadRunning)
		return 0;

	threadRunning = false;

	return 0;
}

/****************************************************************************
 * Name: Echo
 ****************************************************************************/
int
Echo (void)
{
	while (true )
		{
			clearscreen ();
			movecursor ();
			logic->Print ();
			echosleep ();
		}
	return 0;
}

/****************************************************************************
 * Name: fastloop_main
 ****************************************************************************/
int
fastloop_main (int argc, char *argv[])
{
	if (argc > 2)
		ID = (uint8_t) atoi(argv[2]);

	if (!strcmp (argv[1], "start"))
		return StartDaemon ();
	if (!strcmp (argv[1], "stop"))
		return StopDaemon ();
	if (!strcmp (argv[1], "echo"))
		return Echo ();

	return 0;
}
