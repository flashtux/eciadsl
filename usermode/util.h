#ifndef UTIL_H
#define UTIL_H

#include "gsinterface.h"

struct config_t
{
	unsigned int vid1;
	unsigned int pid1;
	unsigned int vid2;
	unsigned int pid2;
	unsigned int vpi;
	unsigned int vci;
	char* mode;
	char* synch;
	char* firmware;
	eci_device_chiset modem_chipset;
	short int alt_interface_synch;
	short int alt_interface_ppp;

};

void get_unsigned_value(const char*, unsigned int*);
void get_signed_value(const char*, int*);
void get_hexa_value(const char*, unsigned int*);
void read_config_file(void);
void printprogres(void);
char* strDup(char** var, const char* text);

const char * config_filename();

#endif
