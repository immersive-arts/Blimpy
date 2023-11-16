/****************************************************************************
 * apps/system/cu/cu_main.c
 *
 *   Copyright (C) 2014 sysmocom - s.f.m.c. GmbH. All rights reserved.
 *   Author: Harald Welte <hwelte@sysmocom.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <debug.h>

#include "system/readline.h"

#include "cu.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

enum parity_mode
{
  PARITY_NONE,
  PARITY_EVEN,
  PARITY_ODD,
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct cu_globals_s g_cu;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: cu_listener
 *
 * Description:
 *   Entry point for the listener thread.
 *
 ****************************************************************************/

static FAR void *cu_listener(FAR void *parameter)
{
  FAR struct cu_globals_s *cu = (FAR struct cu_globals_s *)parameter;

  for (; ; )
    {
      int rc;
      char ch;

      rc = read(cu->infd, &ch, 1);
      if (rc <= 0)
        {
          break;
        }

      fputc(ch, stdout);
      fflush(stdout);
    }

  /* Won't get here */

  return NULL;
}

static void sigint(int sig)
{
  g_cu.force_exit = true;
}

#ifdef CONFIG_SERIAL_TERMIOS
static int set_termios(FAR struct cu_globals_s *cu, int rate,
                       enum parity_mode parity, int rtscts, int nocrlf)
#else
static int set_termios(FAR struct cu_globals_s *cu, int nocrlf)
#endif
{
  int rc = 0;
  int ret;
  struct termios tio;

  tio = cu->devtio;

#ifdef CONFIG_SERIAL_TERMIOS
  tio.c_cflag &= ~(PARENB | PARODD | CRTSCTS);

  switch (parity)
    {
      case PARITY_EVEN:
        tio.c_cflag |= PARENB;
        break;

      case PARITY_ODD:
        tio.c_cflag |= PARENB | PARODD;
        break;

      case PARITY_NONE:
        break;
    }

  /* set baudrate */

  if (rate != 0)
    {
      cfsetspeed(&tio, rate);
    }

  if (rtscts)
    {
      tio.c_cflag |= CRTS_IFLOW | CCTS_OFLOW;
    }
#endif

  tio.c_oflag = OPOST;

  /* enable or disable \n -> \r\n conversion during write */

  if (nocrlf == 0)
    {
      tio.c_oflag |= ONLCR;
    }

  ret = tcsetattr(cu->outfd, TCSANOW, &tio);
  if (ret)
    {
      fprintf(stderr, "set_termios: ERROR during tcsetattr(): %d\n", errno);
      rc = -1;
      goto errout;
    }

  /* Let the remote machine to handle all crlf/echo except Ctrl-C */

  if (cu->stdfd >= 0)
  {
    tio = cu->stdtio;

    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_lflag &= ~ECHO;

    ret = tcsetattr(cu->stdfd, TCSANOW, &tio);
    if (ret)
      {
        fprintf(stderr, "set_termios: ERROR during tcsetattr(): %d\n",
                errno);
        rc = -1;
      }
  }

errout:
  return rc;
}

static void retrieve_termios(FAR struct cu_globals_s *cu)
{
  tcsetattr(cu->outfd, TCSANOW, &cu->devtio);
  if (cu->stdfd >= 0)
    {
      tcsetattr(cu->stdfd, TCSANOW, &cu->stdtio);
    }
}

static void print_help(void)
{
  printf("Usage: cu [options]\n"
         " -l: Use named device (default %s)\n"
#ifdef CONFIG_SERIAL_TERMIOS
         " -e: Set even parity\n"
         " -o: Set odd parity\n"
         " -s: Use given speed (default %d)\n"
         " -r: Disable RTS/CTS flow control (default: on)\n"
#endif
         " -c: Disable lf -> crlf conversion (default: off)\n"
         " -f: Enable endless mode without escape sequence (default: off)\n"
         " -?: This help\n",
         CONFIG_SYSTEM_CUTERM_DEFAULT_DEVICE
#ifdef CONFIG_SERIAL_TERMIOS
         , CONFIG_SYSTEM_CUTERM_DEFAULT_BAUD
#endif
        );
}

static void print_escape_help(void)
{
  printf("[Escape sequences]\n"
         "[~. hangup]\n"
         );
}

static int cu_cmd(char bcmd)
{
  switch (bcmd)
    {
    case '?':
      print_escape_help();
      break;

    case '.':
      return 1;

    /* FIXME: implement other commands such as send/receive file */
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: cu_main
 *
 * Description:
 *   Main entry point for the serial terminal example.
 *
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  pthread_attr_t attr;
  struct sigaction sa;
  FAR const char *devname = CONFIG_SYSTEM_CUTERM_DEFAULT_DEVICE;
  FAR struct cu_globals_s *cu = &g_cu;
#ifdef CONFIG_SERIAL_TERMIOS
  int baudrate = CONFIG_SYSTEM_CUTERM_DEFAULT_BAUD;
  enum parity_mode parity = PARITY_NONE;
  int rtscts = 1;
#endif
  int nocrlf = 0;
  int nobreak = 0;
  int option;
  int ret;
  int bcmd;
  int start_of_line = 1;
  int exitval = EXIT_FAILURE;
  bool badarg = false;

  /* Initialize global data */

  memset(cu, 0, sizeof(*cu));

  /* Install signal handlers */

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigint;
  sigaction(SIGINT, &sa, NULL);

  optind = 0;   /* Global that needs to be reset in FLAT mode */
  while ((option = getopt(argc, argv, "l:s:cefhor?")) != ERROR)
    {
      switch (option)
        {
          case 'l':
            devname = optarg;
            break;

#ifdef CONFIG_SERIAL_TERMIOS
          case 's':
            baudrate = atoi(optarg);
            break;

          case 'e':
            parity = PARITY_EVEN;
            break;

          case 'o':
            parity = PARITY_ODD;
            break;

          case 'r':
            rtscts = 0;
            break;
#endif

          case 'c':
            nocrlf = 1;
            break;

          case 'f':
            nobreak = 1;
            break;

          case 'h':
          case '?':
            print_help();
            exitval = EXIT_SUCCESS;

            /* Go through */

          default:
            badarg = true;
            break;
        }
    }

  if (badarg)
    {
      return exitval;
    }

  /* Open the serial device for writing */

  cu->outfd = open(devname, O_WRONLY);
  if (cu->outfd < 0)
    {
      fprintf(stderr, "cu_main: ERROR: Failed to open %s for writing: %d\n",
              devname, errno);
      goto errout_with_devinit;
    }

  /* Remember serial device termios attributes */

  ret = tcgetattr(cu->outfd, &cu->devtio);
  if (ret)
    {
      fprintf(stderr, "cu_main: ERROR during tcgetattr(): %d\n", errno);
      goto errout_with_outfd;
    }

  /* Remember std termios attributes if it is a tty. Try to select
   * right descriptor that is used to refer to tty
   */

  if (isatty(fileno(stderr)))
    {
      cu->stdfd = fileno(stderr);
    }
  else if (isatty(fileno(stdout)))
    {
      cu->stdfd = fileno(stdout);
    }
  else if (isatty(fileno(stdin)))
    {
      cu->stdfd = fileno(stdin);
    }
  else
    {
      cu->stdfd = -1;
    }

  if (cu->stdfd >= 0)
    {
      tcgetattr(cu->stdfd, &cu->stdtio);
    }

#ifdef CONFIG_SERIAL_TERMIOS
  if (set_termios(cu, baudrate, parity, rtscts, nocrlf) != 0)
#else
  if (set_termios(cu, nocrlf) != 0)
#endif
    {
      goto errout_with_outfd_retrieve;
    }

  /* Open the serial device for reading.  Since we are already connected,
   * this should not fail.
   */

  cu->infd = open(devname, O_RDONLY);
  if (cu->infd < 0)
    {
      fprintf(stderr, "cu_main: ERROR: Failed to open %s for reading: %d\n",
             devname, errno);
      goto errout_with_outfd;
    }

  /* Start the serial receiver thread */

  ret = pthread_attr_init(&attr);
  if (ret != OK)
    {
      fprintf(stderr, "cu_main: pthread_attr_init failed: %d\n", ret);
      goto errout_with_fds;
    }

  /* Set priority of listener to configured value */

  attr.priority = CONFIG_SYSTEM_CUTERM_PRIORITY;

  ret = pthread_create(&cu->listener, &attr, cu_listener, cu);
  pthread_attr_destroy(&attr);
  if (ret != 0)
    {
      fprintf(stderr, "cu_main: Error in thread creation: %d\n", ret);
      goto errout_with_fds;
    }

  /* Send messages and get responses -- forever */

  while (!cu->force_exit)
    {
      int ch = getc(stdin);

      if (ch < 0)
        {
          continue;
        }

      if (nobreak == 1)
        {
          write(cu->outfd, &ch, 1);
          continue;
        }

      if (start_of_line == 1 && ch == '~')
        {
          /* We've seen and escape (~) character, echo it to local
           * terminal and read the next char from serial
           */

          fputc(ch, stdout);
          bcmd = getc(stdin);
          if (bcmd == ch)
            {
              /* Escaping a tilde: handle like normal char */

              write(cu->outfd, &ch, 1);
              continue;
            }
          else
            {
              if (cu_cmd(bcmd) == 1)
                {
                  break;
                }
            }
        }

      /* Normal character */

      write(cu->outfd, &ch, 1);

      /* Determine if we are now at the start of a new line or not */

      if (ch == '\n' || ch == '\r')
        {
          start_of_line = 1;
        }
      else
        {
          start_of_line = 0;
        }
    }

  pthread_cancel(cu->listener);
  exitval = EXIT_SUCCESS;

  /* Error exits */

errout_with_fds:
  close(cu->infd);
errout_with_outfd_retrieve:
  retrieve_termios(cu);
errout_with_outfd:
  close(cu->outfd);
errout_with_devinit:
  return exitval;
}
