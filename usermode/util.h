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

inline void get_unsigned_value(const char*, unsigned int*);
inline void get_signed_value(const char*, int*);
inline void get_hexa_value(const char*, unsigned int*);
inline void read_config_file(void);
inline void printprogres(void);
inline char* strDup(char** var, const char* text);

inline const char * config_filename();

#endif
