/*
  Author: Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation: 01/02/2002

  Goal: test if the HDLC support is ok and return status 0 in this case
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main()
{
	int fd;
	int disc = N_HDLC;

	fd = open("/dev/ptmx",O_RDWR);
	if (fd < 0)
	{
		perror("/dev/ptmx");
		return -1;
	}

	if (ioctl(fd,TIOCSETD,&disc) < 0)
	{
		/* HDLC support is not ok! */
		return -1;
	}

	close (fd);

	return 0;
}
	
