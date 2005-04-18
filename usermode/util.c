#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "gsinterface.h"

const char eciadsl_conf[] = CONF_DIR "/eciadsl.conf";

extern struct eci_device_t eci_device;

struct config_t config;

void get_unsigned_value(const char* param, unsigned int* var)
{
	unsigned int value;
	char* chk;

	value = (unsigned int) strtoul(param, &chk, 10);
	if (! *chk)
		*var = value;
}

void get_unsigned_short_value(const char* param, unsigned short int* var)
{
	unsigned short int value;
	char* chk;

	value = (unsigned short int) strtoul(param, &chk, 10);
	if (! *chk)
		*var = value;
}

void get_signed_value(const char* param, int* var)
{
	int value;
	char* chk;

	value = (int)strtol(param, &chk, 10);
	if (!*chk)
		*var = value;
}

void get_signed_short_value(const char* param, short int* var)
{
	short int value;
	char* chk;

	value = (short int) strtoul(param, &chk, 10);
	if (! *chk)
		*var = value;
}

void get_hexa_value(const char* param, unsigned int* var)
{
	unsigned int value;
	char* chk;

	value = strtoul(param, &chk, 0);
	if (!*chk)
		*var = value;
}

char* strDup(char** var, const char* text)
{
	*var=malloc((strlen(text)+1)*sizeof(char));
	if (!*var)
		return(NULL);
	strcpy(*var, text);
	return(*var);
}

#define COPY_HEXA(dest, src) \
	strcpy(dest, "0x"); \
	strncat(dest, src, 4);

void read_config_file(void)
{
	FILE* file;
	char line[8192];
	char* ptr;
	char* tmp;
	char hexa[7];
	unsigned long int num;

	set_eci_modem_chipset("GS7070");
	config.vid1=0;
	config.pid1=0;
	config.vid2=0;
	config.pid2=0;
	config.vpi=0xffffffff;
	config.vci=0xffffffff;
	config.mode=NULL;
	config.synch=NULL;
	config.firmware=NULL;
	config.modem_chipset=ECIDC_GS7070;
	config.alt_interface_ppp=-1;
	config.alt_interface_synch=-1;
	
	file=fopen(eciadsl_conf, "r");
	if (file)
	{
		num=1;
		while (!feof(file))
		{
			fgets(line, 8192, file);
			if (*line)
			{
				/* strtrim */
				ptr=line;
				while ((*ptr) && (*ptr<=' '))
					ptr++;
				tmp=ptr;
				if (*tmp)
				{
					while (*tmp++);
					tmp--;
					while ((tmp>=ptr) && (*tmp<=' '))
						tmp--;
					if (tmp<ptr)
						*ptr=0;
					else
						*(++tmp)=0;
				}
				if (*ptr && (*ptr!='#'))
				{
					tmp=ptr;
					while (*tmp)
					{
						if (*tmp=='=')
						{
							*tmp++=0;
/*
							fputs(ptr, stderr); fputs(" ", stderr);
							fputs(tmp, stderr); fputs("\n", stderr);
*/
						/* new config parameters - kolja */
						
							if (strcmp(ptr, "MODEM_CHIPSET")==0)
							{
								char* chipset;
								strDup(&chipset, tmp);
								set_eci_modem_chipset(chipset);
								config.modem_chipset = eci_device.eci_modem_chipset;
							}
							else						
							if (strcmp(ptr, "SYNCH_ALTIFACE")==0)
							{
								get_signed_short_value(tmp, &config.alt_interface_synch);
							}
							else						
							if (strcmp(ptr, "PPPOECI_ALTIFACE")==0)
							{
								get_signed_short_value(tmp, &config.alt_interface_ppp);
							}
							else						
							if (strcmp(ptr, "VID1")==0)
							{
								COPY_HEXA(hexa, tmp);
								get_hexa_value(hexa, &config.vid1);
							}
							else
							if (strcmp(ptr, "PID1")==0)
							{
								COPY_HEXA(hexa, tmp);
								get_hexa_value(hexa, &config.pid1);
							}
							else
							if (strcmp(ptr, "VID2")==0)
							{
								COPY_HEXA(hexa, tmp);
								get_hexa_value(hexa, &config.vid2);
							}
							else
							if (strcmp(ptr, "PID2")==0)
							{
								COPY_HEXA(hexa, tmp);
								get_hexa_value(hexa, &config.pid2);
							}
							else
							if (strcmp(ptr, "VPI")==0)
								get_unsigned_value(tmp, &config.vpi);
							else
							if (strcmp(ptr, "VCI")==0)
								get_unsigned_value(tmp, &config.vci);
							else
							if (strcmp(ptr, "MODE")==0)
								strDup(&config.mode, tmp);
							else
							if (strcmp(ptr, "SYNCH")==0)
								strDup(&config.synch, tmp);
							else
							if (strcmp(ptr, "FIRMWARE")==0)
								strDup(&config.firmware, tmp);
							else
							if (
								(strcmp(ptr, "PPPD_USER")!=0)
								&& (strcmp(ptr, "PPPD_PASSWD")!=0)
								&& (strcmp(ptr, "STATICIP")!=0)
								&& (strcmp(ptr, "GATEWAY")!=0)
								&& (strcmp(ptr, "USE_STATICIP")!=0)
								&& (strcmp(ptr, "USE_DHCP")!=0)
								&& (strcmp(ptr, "SYNCH_ATTEMPTS")!=0)
								&& (strcmp(ptr, "FIRMWARE_OPTIONS")!=0)
								&& (strcmp(ptr, "SYNCH_OPTIONS")!=0)
								&& (strcmp(ptr, "PPPOECI_OPTIONS")!=0)
								&& (strcmp(ptr, "PREFERRED_USB_HUB")!=0)
								&& (strcmp(ptr, "MODEM")!=0)
								&& (strcmp(ptr, "PROVIDER")!=0)
								&& (strcmp(ptr, "DNS1")!=0)
								&& (strcmp(ptr, "DNS2")!=0)
								)
								fprintf(stderr,
									"config: unrecognized entry '%s' at line %lu\n",
									ptr, num);
						}
						tmp++;
					}
				}
			}
			num++;
		}
		fclose(file);
		if (config.alt_interface_ppp==-1)
			config.alt_interface_ppp= eci_device.alt_interface_ppp;
		if (config.alt_interface_synch==-1)
			config.alt_interface_synch= eci_device.alt_interface_synch;
	}
	else
		fprintf(stderr, "couldn't access %s\n", eciadsl_conf);
/*
	fprintf(stderr, "VID1=%04x\n"
					"PID1=%04x\n"
					"VID2=%04x\n"
					"PID2=%04x\n"
					"VPI=%d\n"
					"VCI=%d\n"
					"MODE=%s\n"
					"SYNCH=%s\n"
					"FIRMWARE=%s\n",
					config.vid1, config.pid1, config.vid2, config.pid2,
					config.vpi, config.vci,
					config.mode,
					config.synch, config.firmware
			);
*/
}

/* function to show a 'progress bar' during synch sequence - kolja */
char* pchars = "-\\|/";
unsigned int ppos = 0;

void printprogres(void){
		printf ("\r Please Wait.. Synchronisation in progress [%c]", (char)(pchars[ppos]));
		ppos==3 ? ppos=0 : ppos++;
}

const char * config_filename()
{
    return eciadsl_conf;
}

