/*
  Author : kolja <gava@bergamoblog.it>
  Creation : 29/11/2003
  Licence : GPL

*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
*********************************************************************
  GlobeSpan Chipset Common Interface
     cantain all the information related to device 
     this interface will load infos and methods from the right chipset depending on
     compile option (see ./configure --help | --with-chipset option)
*/

#include <string.h>
#include "gsinterface.h"

struct eci_device_t eci_device;

#include "gs7470.c"
#include "gs7070.c"

int (*pF_gsSetControl)(unsigned char*);
void (*pF_gsGetResponse)(unsigned char*);
void (*pF_allocateGSint)(void);
void (*pF_deallocateGSint)(void);
void (*pF_getAal5HeaderStructure)(const unsigned char*,
                                  struct aal5_header_st*);

/* set eci modem chipset - kolja */
void set_eci_modem_chipset(char* chipset){
	if (memcmp(chipset, "GS7470", 6)==0){
		gs7470InitParams();
		pF_gsSetControl  = gs7470SetControl;
		pF_gsGetResponse = gs7470GetResponse;
		pF_allocateGSint = allocateGS7470int;
		pF_deallocateGSint = deallocateGS7470int;
		pF_getAal5HeaderStructure = getAal5HeaderStructure7470;				
	}else{
		gs7070InitParams();
		pF_gsSetControl  = gs7070SetControl;
		pF_gsGetResponse = gs7070GetResponse;
		pF_allocateGSint = allocateGS7070int;
		pF_deallocateGSint = deallocateGS7070int;
		pF_getAal5HeaderStructure = getAal5HeaderStructure7070;			
	}
}


const char * get_chipset_descr(eci_device_chiset eci_chipset){
	if (eci_chipset == ECIDC_GS7470)
		return("GS7470");
	else
		return("GS7070");
}

int  gsSetControl(unsigned char* buffer){
	return(pF_gsSetControl(buffer));
}

void gsGetResponse(unsigned char* buffer){
	pF_gsGetResponse(buffer);
}

void allocateGSint(void){
	pF_allocateGSint();
}

void deallocateGSint(void){
	pF_deallocateGSint();
}

void getAal5HeaderStructure(const unsigned char* aal5Header,
                            struct aal5_header_st* aal5HeaderOut)
{
	pF_getAal5HeaderStructure(aal5Header, aal5HeaderOut);
}
