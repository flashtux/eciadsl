/*
  Author: Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation: 01/02/2002

  Goal: test if the HDLC support is ok and if the HDLC bug is corrected or not.

  Note: pppd has the slave side of /dev/ptmx. pppoeci has the master side of
  /dev/ptmx (ie /dev/ptmx itself). The bug is that closing the slave side
  has no effect on the master side.

  03/02/2002: added a signal handler for SIGALRM
*/

#define _XOPEN_SOURCE /* for grantpt in <stdlib.h> */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

void sigalrm()
{
	exit (-1);
}

int main()
{
	int fd_master, fd_slave;
	int disc = N_HDLC;
	const char * pts;
	int r;
	char buf[10];

	/* handle the SIGALARM signal */
	signal(SIGALRM, sigalrm);

	/* open the master side */
	fd_master = open("/dev/ptmx",O_RDWR);
	if (fd_master < 0)
	{
		perror("/dev/ptmx");
		return -1;
	}

	/* open the slave side */
	grantpt(fd_master);
	unlockpt(fd_master);
	pts = ptsname(fd_master);

	fd_slave = open(pts,O_RDWR|O_NOCTTY);
	if (fd_slave < 0)
	{
		perror(pts);
		return -1;
	}

	/* install the HDLC line discipline */
	if (ioctl(fd_master,TIOCSETD,&disc) < 0)
	{
		/* HDLC support is not ok! */
		perror("N_HDLC");
		return -1;
	}

	/* we close the slave side */
	close(fd_slave);

	/* we try to read on the master side */
	alarm(1);

	r = read(fd_master, buf, sizeof(buf));
	/* we don't care about errors returned by read().
	   we only test if read() returns or blocks */

	close (fd_master);

	return 0;
}
	
