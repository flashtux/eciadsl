#ifndef UTIL_H
#define UTIL_H


struct config_t
{
	unsigned short vid1;
	unsigned short pid1;
	unsigned short vid2;
	unsigned short pid2;
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
