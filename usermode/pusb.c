/*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
 Licence	:	GPL
*********************************************************************/

/* Simple portable USB interface */

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include "pusb-bsd.c"
#elif defined(__linux__)
#include "pusb-linux.c"
#else
#error Unknown operating system
#endif
