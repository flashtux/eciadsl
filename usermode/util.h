#ifndef UTIL_H
#define UTIL_H


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
};

void get_unsigned_value(const char*, unsigned int*);
void get_signed_value(const char*, int*);
void get_hexa_value(const char*, unsigned int*);
void read_config_file(void);

#endif
