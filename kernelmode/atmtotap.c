
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <asm/fcntl.h>
#include <atm.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>


/*
	From usermode/util.c
*/
void get_unsigned_value(const char* param, unsigned int* var)
{
        unsigned int value;
        char* chk;

        value = (unsigned int) strtoul(param, &chk, 10);
        if (! *chk)
                *var = value;
}


struct atmtap_datas
{
	int fdtap;
	int fdatm;
};

int tap_open(char *dev,char *path)
{
    struct ifreq ifr;
    int fd, err;
	
    if( (fd = open(path, O_RDWR | O_SYNC)) < 0 )
	{
       return -1;
	}
    if(memset(&ifr, 0, sizeof(ifr))==NULL)
	{
		fprintf(stderr,"memset error in tap_open function\n");
		exit(-1);
	}
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if( *dev )
	{
       strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}
    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 )
	{
       close(fd);
       return err;
    }
    /* strcpy(dev, ifr.ifr_name); */
    return fd;
}


static int open_atmdevice(char * cp)
{
	int fd;
	struct atm_qos qos;
	struct sockaddr_atmpvc addr;

	if(memset(&addr, 0, sizeof addr)==NULL)
	{
		fprintf(stderr,"memset error in open_atmdevice\n");
		exit(-1);
	}
	if (text2atm(cp,(struct sockaddr *)&addr,sizeof(addr),T2A_PVC | T2A_NAME)<0)
	{
		fprintf(stderr,"error openning atm socket\n");
		return 0;
	}

/*	info("open device pppoatm\n"); */
	
	if((fd = socket(AF_ATMPVC, SOCK_DGRAM , 0))<0)
	{
		fprintf(stderr,"failed to create socket");
		return(0);
	}
	if(memset(&qos, 0, sizeof qos)==NULL)
	{
		fprintf(stderr,"second memset error in open_atmdevice\n");
		exit(-1);
	}
	qos.txtp.traffic_class = qos.rxtp.traffic_class = ATM_UBR;

/*
	if (qosstr != NULL)
		if (text2qos(qosstr, &qos, 0))
			fprintf(stderr,"can't parse QoS: ");
*/
	qos.txtp.max_sdu = 1534 + 10; 
	qos.rxtp.max_sdu = 1534 + 10;
	qos.aal = ATM_AAL5;
	if (setsockopt(fd, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0)
		fprintf(stderr,"setsockopt(SO_ATMQOS)");

	if (connect(fd, (struct sockaddr *) &addr,
	    sizeof(struct sockaddr_atmpvc)))
	{
		fprintf(stderr,"socket connection error");
		return 0;
	}
	return fd;
}

void *read_on_tap(void *datas)
{
	struct atmtap_datas *d;
	char  tmpbuf[64*1024];
	int r,errno;
	
	d = (struct atmtap_datas *) datas;
	for(;;)
	{
		do {
			r = read(d->fdtap, tmpbuf, sizeof(tmpbuf));
 		} while(r < 0 && errno == EINTR);	
		if(write(d->fdatm, tmpbuf, r +10)==-1)
		{
			fprintf(stderr,"write error in read_on_tap\n");
			exit(-1);
		}
	}
}

void *write_on_tap(void *datas)
{
	struct atmtap_datas *d;
	char  tmpbuf[64*1024];
	int r,errno;

	d = (struct atmtap_datas *) datas;	
	for(;;)
	{
		do {
			r = read(d->fdatm, tmpbuf, sizeof(tmpbuf));
 		} while(r < 0 && errno == EINTR);
	/*	if(r>10) */
		if(write(d->fdtap, tmpbuf +10 , r-10)==-1)
		{
			fprintf(stderr,"write error in write_on_tap\n");
			exit(-1);
		}
	}
}


void version(const int full)
{
	fprintf(stdout,"Kernel Mode for Globespan based USB ADSL modems - Version 0.x\n");
	exit(full);
}

void usage(const int ret)
{
	fprintf(stdout,	"Usage:\n"
					"       atmtotap [<switch>] [-vpi num -vci num -d device]\n");
	fprintf(stdout,	"Switches:\n");
	fprintf(stdout,	"	-h or --help		display this message then exit\n"
					"	-V or --version     display the version number then exit\n"
					"\n");
	fprintf(stdout,	"The vpi and vci are numerical values. They define the vpi.vci used\n"
					"by provider. For instance: 8.35 for France, 0.38 for UK.\n");
	fprintf(stdout,	"The device is the path to your tun device\n"
					"For instance: /dev/net/tun should be ok for most distribution.\n"
					"\n");
	exit(ret);
}

int main(int argc, char **argv)
{
	struct atmtap_datas datas;
	pthread_t th_id;
	pthread_attr_t attr;
	int i;
	
	/* parse command line options */
	unsigned int my_vpi;
	unsigned int my_vci;
	char vpi_vci[12];
	char path_to_dev[20];
	int arg=0;
	size_t tmp;
	short no_vpi=1;
	short no_vci=1;
	short no_dev=1;
	
	for (i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i], "-vpi") == 0) && (i + 1 < argc))
		{
			get_unsigned_value(argv[++i], &my_vpi);
			no_vpi=0;
		}
		else
		{
			if ((strcmp(argv[i], "-vci") == 0) && (i + 1 < argc))
			{
				get_unsigned_value(argv[++i], &my_vci);
				no_vci=0;
			}
			else
			{
				if ((strcmp(argv[i], "-d") == 0) && ( i+1 < argc))
				{
					tmp=strlen(argv[i+1]);
					if((tmp>0) && (tmp<21))
					{
						strcpy(path_to_dev,argv[++i]);
						arg++;
						no_dev=0;
					}
					else
					{
						fprintf(stderr,"path to device is too long.\n");
						exit(-1);
					}
				}
				else
				{
					if ((strcmp(argv[i], "--version") == 0) ||
						(strcmp(argv[i], "-V") == 0))
					{
						version(0);
					}
					else
					{
						if ((strcmp(argv[i], "--help") == 0) ||
							(strcmp(argv[i], "-h") == 0))
						{
							usage(0);
						}
						else
						{
							usage(-1);
						}
					}
				}
			}
		}
	}
	if(no_vpi)
	{
		/* default value - same as in france */
		my_vpi=8;
	}
	if(no_vci)
	{
		/* default value - same as in france */
		my_vci=35;
	}
	if(no_dev)
	{
		/* default value - should be ok for most distributions */
		strcpy(path_to_dev,"/dev/net/tun");
	}
	if(!(datas.fdtap =  tap_open("tap0",path_to_dev)))
	{
		fprintf(stderr,"can't open tap device\n");
		exit(5);
	}
	sprintf(vpi_vci,"%d.%d",my_vpi ,my_vci);
	if((datas.fdatm = open_atmdevice(vpi_vci))==0)
	{
		fprintf(stderr," can't open atm device\n");
		exit(5);
	}

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(&th_id, &attr, read_on_tap , &datas) != 0)
	{
		fprintf(stderr,"error creating thread\n");
	}
	write_on_tap(&datas);
	return(0);
}
