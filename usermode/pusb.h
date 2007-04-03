/*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
*********************************************************************/

#ifndef PUSB_H
#define PUSB_H

/* Simple portable USB interface */

typedef struct pusb_device_t *pusb_device_t;
typedef struct pusb_endpoint_t *pusb_endpoint_t;
typedef struct pusb_urb_t *pusb_urb_t;

pusb_device_t pusb_search_open(int vendorID, int productID);
pusb_device_t pusb_open(const char *path);
int pusb_close(pusb_device_t dev);

inline int pusb_control_msg(pusb_device_t dev,
		     int request_type, int request,
		     int value, int index, 
		     unsigned char *buf, int size, int timeout);
int pusb_set_configuration(pusb_device_t dev, int config);
int pusb_set_interface(pusb_device_t dev, int interface, int alternate);

int pusb_claim_interface(pusb_device_t dev,int interface);
int pusb_release_interface(pusb_device_t dev,int interface);

pusb_endpoint_t pusb_endpoint_open(pusb_device_t dev, int epnum, int flags);
inline int pusb_endpoint_read(pusb_endpoint_t ep, 
		       unsigned char *buf, int size, int timeout);
inline int pusb_endpoint_read_int(pusb_endpoint_t ep,
                                  unsigned char *buf, int size);
inline int pusb_endpoint_write(pusb_endpoint_t ep, 
			const unsigned char *buf, int size, int timeout);

inline int pusb_endpoint_submit_read (pusb_endpoint_t ep, unsigned char * buf,
							   int size, int signr);

inline int pusb_endpoint_submit_write(pusb_endpoint_t ep, unsigned char * buf,
							   int size, int signr);

inline int pusb_endpoint_submit_int_read(pusb_endpoint_t ep, unsigned char * buf,
							   int size, int signr);

/* buf is at least pkt_size*pkt_nb bytes */
inline int pusb_endpoint_submit_iso_read(pusb_endpoint_t ep, unsigned char * buf,
								  int pkt_size, int pkt_nb, int signr);

/* should be better to pass pusb_device_t instead of pusb_endpoint_t */
inline pusb_urb_t pusb_device_get_urb(pusb_device_t dev);

int pusb_endpoint_reset(pusb_endpoint_t ep);

int pusb_endpoint_close(pusb_endpoint_t ep);

/* function to manipulate URBs */

int pusb_urb_get_epnum(pusb_urb_t urb);

inline int pusb_urb_buffer_first(pusb_urb_t urb,
						  unsigned char ** pbuf, int * psize, int * idx);

inline int pusb_urb_buffer_next(pusb_urb_t urb,
						 unsigned char ** pbuf, int * psize, int * idx);

/* function returning usb status - kolja */
int pusb_get_urb_status(pusb_urb_t urb);

int init_pusb();
void deinit_pusb();

#endif
