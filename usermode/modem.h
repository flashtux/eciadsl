/*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
 Licence	:	GPL
*********************************************************************/

/*
  15/02/2002 Benoit PAPILLAULT
    Change the string for the device name. Since the 2 chips are used in
	several modems of differents brand names, we choose EZUSB and GlobeSpan
	for their description.
*/

#ifndef MODEM_H
#define MODEM_H

#include "config.h"

#ifndef PRODUCT_ID
#error "PRODUCT_ID must be defined"
#endif

#ifndef PRODUCT_NAME
#error "PRODUCT_NAME must be defined"
#endif

#ifndef PRODUCT_VERSION
#error "PRODUCT_VERSION must be defined"
#endif

#ifndef ETC_PATH
#error "ETC_PATH must be defined"
#endif

#ifndef ETC_DIR
#error "ETC_DIR must be defined"
#endif

/*
#define AND &&
#define OR ||
#define NOT !
#define NOP {;}
#define true 1
#define false 0
#define MAX(X,Y) ((X)>(Y)?(X):(Y))
#define MIN(X,Y) ((X)<(Y)?(X):(Y))
*/

/* Vendor/ProdID for "ECI Telecom USB ADSL Loader" */
#define EZUSB_NAME    "EZUSB USB ADSL Loader"
#define EZUSB_VENDOR  0x0547 /* 0x0547 */
#define EZUSB_PRODUCT 0x2131 /* 0x2131 */

/* Vendor/ProdID for "ECI Telecom USB ADSL WAN Modem" */
#define GS_NAME       "GlobeSpan USB ADSL WAN Modem"
#define GS_VENDOR     0x0915 /* 0x0915 */
#define GS_PRODUCT    0x8000 /* 0x8000 */

/* endpoint numbers */

#define EP_INT      0x86

#define EP_DATA_IN  0x88
#define EP_DATA_OUT 0x02

#endif
