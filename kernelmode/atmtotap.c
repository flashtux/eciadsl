
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_tun.h>

#include <asm/fcntl.h>

#include <atm.h>

#include <errno.h>

#include <pthread.h>

struct atmtap_datas
{
	int fdtap;
	int fdatm;
};





int tap_open(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tun", O_RDWR | O_SYNC)) < 0 )
       return -1;

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if( *dev )
       strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
       close(fd);
       return err;
    }
    //strcpy(dev, ifr.ifr_name);
    return fd;
}


static int open_atmdevice(char * cp)
{
	int fd;
	struct atm_qos qos;
	struct sockaddr_atmpvc addr;

	memset(&addr, 0, sizeof addr);
	if (text2atm(cp, (struct sockaddr *) &addr, sizeof(addr),
	    T2A_PVC | T2A_NAME) < 0)
	{
		printf("erreur openning atm socket\n");
		return 0;
	}

//	info("open device pppoatm\n");
	
	fd = socket(AF_ATMPVC, SOCK_DGRAM , 0);
	if (fd < 0)
		printf("failed to create socket");
	memset(&qos, 0, sizeof qos);
	qos.txtp.traffic_class = qos.rxtp.traffic_class = ATM_UBR;

/*
	if (qosstr != NULL)
		if (text2qos(qosstr, &qos, 0))
			printf("Can't parse QoS: ");
*/
	qos.txtp.max_sdu = 1534 + 10; 
	qos.rxtp.max_sdu = 1534 + 10;
	qos.aal = ATM_AAL5;
	if (setsockopt(fd, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0)
		printf("setsockopt(SO_ATMQOS): ");

	if (connect(fd, (struct sockaddr *) &addr,
	    sizeof(struct sockaddr_atmpvc)))
	{
		printf("connect error");
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
			tmpbuf[0]=0xaa;
			tmpbuf[1]=0xaa;
			tmpbuf[2]=0x03;
			tmpbuf[3]=0x00;
			tmpbuf[4]=0x80;
			tmpbuf[5]=0xc2;
			tmpbuf[6]=0x00;
			tmpbuf[7]=0x07;
			tmpbuf[8]=0x00;
			tmpbuf[9]=0x00;
			r = read(d->fdtap, tmpbuf  + 10, sizeof(tmpbuf) - 10);
 		} while(r < 0 && errno == EINTR);	
		write(d->fdatm, tmpbuf, r +10);
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
	//	if(r>10)
			write(d->fdtap, tmpbuf +10 , r-10);
	}
}


int main(int argc, char **argv)
{
	struct atmtap_datas datas;
	pthread_t th_id;
	pthread_attr_t attr;

	if(!(datas.fdtap =  tap_open("tap0")))
	{
		printf("Can't open tap device\n");
		exit(5);
	}
	datas.fdatm = open_atmdevice("8.35");

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(&th_id, &attr, read_on_tap , &datas) != 0)
	{
		fprintf(stderr,"error creating thread\n");
	}
	write_on_tap(&datas);
}
