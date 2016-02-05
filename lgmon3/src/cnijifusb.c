/*
 *  Canon Inkjet Printer Driver for Linux
 *  Copyright CANON INC. 2001-2015
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * NOTE:
 *  - As a special exception, this program is permissible to link with the
 *    libraries released as the binary modules.
 *  - If you write modifications of your own for these programs, it is your
 *    choice whether to permit this exception to apply to your modifications.
 *    If you do not wish that, delete this exception.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

#include <libusb.h>
#include "cnijcomif.h"
#include "cnijlgmon3.h"
#include "cnijifusb.h"

//#define _DEBUG_MODE_
#define PRINTER_ALTERNATIVE_NUM 0

static struct libusb_device **g_list = NULL;				/* device list */
static struct libusb_device *g_device = NULL;				/* usb device */
static struct libusb_device_handle *g_dh = NULL;			/* device handler */
static struct libusb_config_descriptor *g_cptr = NULL;		/* configuration descriptor */
static struct libusb_context *g_context = NULL;				/* libusb context */
static uint8_t g_num[4];									/* [0].configuration; [1].interface [2].rep; [3].wep */
int g_claimed_interface[16] = {0};
int g_usblp_attached = 0;									/* number of detached interface*/

int CNIF_USB_Open(const char *device_id, CNIF_INFO *if_info)
{
	/*-------------define and init parameter --------------*/
	ssize_t numdev;		/* number of device */
	int err = 0;		/* error number */
	int i;
	int ep = 0;
	
	struct libusb_device_descriptor devdesc;					/* device descriptor */
	const struct libusb_interface *iptr = NULL;					/* Array of interface descriptors */
	const struct libusb_interface_descriptor *altptr = NULL;	/* interface descriptor */
	uint8_t cNum,												/* configuration number */
			iNum,												/* interface number */		
			altNum;	
	char info[CN_READ_SIZE];											/* info of printer */
	
	err = libusb_init(&g_context);
	if (err < 0){
		
#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: libusb_init was failed\n");
#endif
		
		err = CN_USB_WRITE_ERROR;
		goto onErr;
	}
	/*-------------search canon printer--------------*/
	numdev = libusb_get_device_list(NULL, &g_list);
	if((int)numdev == 0) {
		err = CN_USB_WRITE_ERROR;
		goto onErr;
	}
	
#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: libusb_get_device_list = %d\n", (int)numdev);
#endif

	
	for(i = 0; i < numdev; i++) {
		g_device = g_list[i];
		err = libusb_get_device_descriptor(g_device, &devdesc);
		if(err < 0) {
			
#ifdef _DEBUG_MODE_
			fprintf(stderr, "ERROR: get device descriptor was failed\n");
#endif
			
			goto onErr;
		}
		
		if(devdesc.idVendor == CN_USB_VENDERID) {
#ifdef _DEBUG_MODE_
			fprintf(stderr, "DEBUG: canon device found\n");
#endif
			
			/*-----------------device open------------------*/
			err = libusb_open(g_device, &g_dh);
			if(err < 0) {
#ifdef _DEBUG_MODE_
				fprintf(stderr, "ERROR: [discover]usb_open was failed.\n");//ERROR_MSG
#endif
				err = CN_USB_WRITE_ERROR;
				goto onErr;
			}
			
			/*-----------------serch interface--------------*/
			for(cNum = 0; cNum < devdesc.bNumConfigurations; cNum++) {
				if(libusb_get_config_descriptor(g_device, cNum, &g_cptr) < 0){
					continue;
				}
				for(iNum = 0; iNum < g_cptr->bNumInterfaces; iNum++) {
					iptr = g_cptr->interface + iNum;
					for(altNum = 0; altNum < iptr->num_altsetting; altNum++){
						altptr = iptr->altsetting + altNum;
						if(altptr->bInterfaceClass != LIBUSB_CLASS_PRINTER){
							continue;
						}
						
					    g_num[1] = altptr->bInterfaceNumber;
					    g_claimed_interface[g_usblp_attached] = altptr->bInterfaceNumber;
						g_usblp_attached++;
						
						/*-----------------kernel setting--------------*/
						if (libusb_kernel_driver_active(g_dh, iNum) == 1) {
#ifdef _DEBUG_MODE_
							fprintf(stderr, "DEBUG: kernel driver active\n");
#endif
							if(libusb_detach_kernel_driver(g_dh, iNum) == 0) {
#ifdef _DEBUG_MODE_
							     fprintf(stderr, "DEBUG: kernel driver detached!\n");
#endif
							} else {
								err = CN_USB_WRITE_ERROR;
								goto onErr;
							}
					    }
					    
					    /*-----------------claim interface-------------*/
						while((err = libusb_claim_interface(g_dh, g_num[1])) < 0) {
							if (err != LIBUSB_ERROR_BUSY) {
#ifdef _DEBUG_MODE_
							fprintf(stderr, "ERROR: [discover]libusb_claim_interface Error.%d (%d)\n", g_num[1], err);
#endif
							err = CN_USB_WRITE_ERROR;
							goto onErr;
								
							}
						}
#ifdef _DEBUG_MODE_
						fprintf(stderr, "DEBUG: End of claim interface %d \n", g_num[1]);
#endif
						
						/*-----------------get deviceId--------------*/
						memset(info, '\0', sizeof(info));
						
						err = libusb_control_transfer(g_dh, 0xA1, 0x00, 0x0000, iNum, (unsigned char *)info, sizeof(info), 0);
#ifdef _DEBUG_MODE_
						fprintf(stderr, "DEBUG: info = %s\n", &info[2]);
#endif
						if(err < 0) {
#ifdef _DEBUG_MODE_
							fprintf(stderr, "ERROR: libusb_control_transfer Error.(%d)\n", err);
#endif
							err = CN_USB_WRITE_ERROR;
							goto onErr;
						}

						int num1 = 0x000000ff & ((int)info[1]);
						int num2 = (0x000000ff & ((int)info[0])) << 8;
						if(err != num2 + num1) {
#ifdef _DEBUG_MODE_
							fprintf(stderr, "ERROR: read data number error\n");
#endif
							err = CN_USB_WRITE_ERROR;
							goto onErr;
						}
						//check wheather it is a printer
						if(CNIF_USB_IsPrinter(&info[2]) == 0)
						{
							/*--------------compare serial number------------------*/
							err = CNIF_USB_CmpSerialNum(device_id);
							if(err < 0) {
#ifdef _DEBUG_MODE_
								fprintf(stderr, "DEBUG: goto next interface\n");
#endif
								continue;
							}
							
							/*--------------is support ivec-------------------------*/
							if_info->isIVEC = CNIF_USB_IsIvec(&info[2]);
							
							g_num[0] = cNum;
							g_num[1] = iNum;
							
							/*---------------search read_endpoint-------------------*/
							ep = CNIF_USB_SearchEndpoint(CN_USB_PRINTER_READ_EP);
							if(ep < 0){
#ifdef _DEBUG_MODE_
								fprintf(stderr, "ERROR: cannot search the read_endpoint of the printer\n");//ERROR_MSG
#endif
								err = CN_USB_READ_EP_SEARCH_ERROR;
								goto onErr;
							}
							g_num[2] = ep;
							/*---------------search write_endpoint-------------------*/
							ep = CNIF_USB_SearchEndpoint(CN_USB_PRINTER_WRITE_EP);
							if(ep < 0){
#ifdef _DEBUG_MODE_
								fprintf(stderr, "ERROR: cannot search the write_endpoint of the printer\n");//ERROR_MSG
#endif
								err = CN_USB_WRITE_EP_SEARCH_ERROR;
								goto onErr;
							}
							g_num[3] = ep;
							err = 0;
							goto onErr;
						}
					}
				}
			}
			CNIF_USB_OneDeviceClose();
		}
	}
	
#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: cannot find the printer\n");
#endif

	err = CN_USB_WRITE_ERROR;	//cannot find device
onErr:
	
	if (err < 0) {
		CNIF_USB_Close();
	}
	
	return err;
}

int CNIF_USB_Close()
{	
	int i = 0;
	int e = 0;
	
	/*----------free configuration----------- */
	if(g_cptr != NULL){
		libusb_free_config_descriptor(g_cptr);
		g_cptr = NULL;
	}
	
	/*----------release interface/attache kernel----------- */
	for(; i < g_usblp_attached; i++) {
		if( (e = libusb_release_interface(g_dh, g_claimed_interface[i])) < 0) {
#ifdef _DEBUG_MODE_
		fprintf(stderr, "ERROR: libusb_release_interface Error.%d (%d)\n", g_claimed_interface[i], e);
#endif
		}
#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: End of release interface %d \n", g_claimed_interface[i]);
#endif

		e = libusb_attach_kernel_driver(g_dh, g_claimed_interface[i]);
#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: End of attache %d ,%d \n", g_claimed_interface[i], e);
#endif
		g_claimed_interface[i] = 0;
	}
	g_usblp_attached = 0;
	
	/*----------close device----------- */
	if(g_dh != NULL) {
		libusb_close(g_dh);
		g_dh = NULL;
	}
	
	/*----------free libusb context----------- */
	if(g_context != NULL) {
		libusb_exit(g_context);
		g_context = NULL;
	}
	
	return 0;
}

int CNIF_USB_OneDeviceClose()
{	
	int i = 0;
	int e = 0;
	
	/*----------free configuration----------- */
	if(g_cptr != NULL){
		libusb_free_config_descriptor(g_cptr);
		g_cptr = NULL;
	}
	
	/*----------release interface/attache kernel----------- */
	for(; i < g_usblp_attached; i++) {
		if( (e = libusb_release_interface(g_dh, g_claimed_interface[i])) < 0) {
#ifdef _DEBUG_MODE_
		fprintf(stderr, "ERROR: [discover]libusb_release_interface Error.%d (%d)\n", g_claimed_interface[i], e);
#endif
		}
#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [discover]End of release interface %d (%d)\n", g_claimed_interface[i], e);
#endif

		e = libusb_attach_kernel_driver(g_dh, g_claimed_interface[i]);
#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [discover]End of attache %d ,%d \n", g_claimed_interface[i], e);
#endif
		g_claimed_interface[i] = 0;
	}
	g_usblp_attached = 0;
	
	/*----------close device----------- */
	if(g_dh != NULL) {
		libusb_close(g_dh);
		g_dh = NULL;
	}
	
	return 0;
}

int CNIF_USB_Reset()
{
	int err = 0;
	if (g_dh != NULL) {
		err = libusb_control_transfer(g_dh, 0x21, 0x02, 0x0000, g_num[1], NULL, 0x0000, 0);
		if(err < 0) {
#ifdef _DEBUG_MODE_
			fprintf(stderr, "DEBUG: printer reset error.(%d)\n", err);//ERROR_MSG
#endif
		}
	}
	
	return err;
}

int CNIF_USB_Read(uint8_t *buffer, size_t buffer_size, size_t *read_size)
{
	int err = 0;
	int i = 0;
	/* get printer status via USB */

#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: read data start\n");
#endif
	
	for (i = 0; i < 3; i++) {
		err = libusb_bulk_transfer(g_dh, g_num[2], (unsigned char *)buffer, buffer_size - 1, (int *)read_size, CN_WRITE_TIMEOUT);
		if(err < 0) {
	        if (err == LIBUSB_ERROR_OVERFLOW) {
#ifdef _DEBUG_MODE_
	        	fprintf(stderr, "DEBUG: read overflow error(%d)\n", err);//ERROR_MSG
#endif
	        	break;
			} else if (err == LIBUSB_ERROR_TIMEOUT) {
#ifdef _DEBUG_MODE_
			fprintf(stderr, "DEBUG: read timeout error(%d)\n", err);//ERROR_MSG
#endif

			} else {
#ifdef _DEBUG_MODE_
	        	fprintf(stderr, "DEBUG: usb_bulk_read error(%d)\n", err);//ERROR_MSG
#endif
	        }
	        if(i == 2) {
				goto onErr;
			}
	    }
	    break;
    }
#ifdef _DEBUG_MODE_
	fprintf(stderr, "read data = %s, size = %d\n", buffer, (int)*read_size);
#endif
	
onErr:
	return err;
}

int CNIF_USB_Write(uint8_t *buffer, size_t buffer_size, size_t *written_size)
{
	int err = 0;
	err = libusb_bulk_transfer(g_dh, g_num[3], (unsigned char *)buffer, (int)buffer_size, (int *)written_size, CN_WRITE_TIMEOUT);
	
	return err;
}
int CNIF_USB_Cancel()
{
	CNIF_USB_Reset();
	return 0;
}

int CNIF_USB_Discover()
{
	int err = 0;
	int length = 0;
	int n = 0;
	
	ssize_t numdev = 0;
	char serial[CN_DEVICE_ID_LEN];
	char dev_uri[CN_DEVICE_URI_LEN];
	char info[CN_READ_SIZE];
	char mdl[] = "DES";
	char key[4];
	char value[CN_SERIAL_NUM_LEN];
	char *tp;
	struct libusb_device_descriptor devdesc;
	const struct libusb_interface *iptr = NULL;					/* Array of interface descriptors */
	const struct libusb_interface_descriptor *altptr = NULL;	/* interface descriptor */
	int i, cNum, iNum, altNum;
	
	/*---------------get device info---------------------------*/
	g_usblp_attached = 0;
	
	err = libusb_init(&g_context);
	if (err < 0){
#ifdef _DEBUG_MODE_
		fprintf(stderr, "ERROR: [discover]libusb_init was failed\n");
#endif
		err = CN_USB_WRITE_ERROR;
		goto onErr;
	}
	/*-------------search canon printer--------------*/
	numdev = libusb_get_device_list(NULL, &g_list);
	if((int)numdev == 0) {
		err = CN_USB_WRITE_ERROR;
		goto onErr;
	}
	
	for(i = 0; i < numdev; i++) {
		g_device = g_list[i];
		err = libusb_get_device_descriptor(g_device, &devdesc);
		if(err < 0) {
#ifdef _DEBUG_MODE_
			fprintf(stderr, "ERROR: [discover]get device descriptor was failed\n");
#endif
			err = CN_USB_WRITE_ERROR;
			goto onErr;
		}
		if(devdesc.idVendor == CN_USB_VENDERID) {
#ifdef _DEBUG_MODE_
			fprintf(stderr, "DEBUG: [discover]canon device found\n");
#endif
			/*-----------------device open------------------*/
			err = libusb_open(g_device, &g_dh);
			if(err < 0) {
#ifdef _DEBUG_MODE_
				fprintf(stderr, "ERROR: [discover]usb_open was failed.\n");//ERROR_MSG
#endif
				err = CN_USB_WRITE_ERROR;
				goto onErr;
			}
			
			/*-----------------serch interface--------------*/
			for(cNum = 0; cNum < devdesc.bNumConfigurations; cNum++) {
				if(libusb_get_config_descriptor(g_device, cNum, &g_cptr) < 0){
					continue;
				}
				for(iNum = 0; iNum < g_cptr->bNumInterfaces; iNum++) {
					iptr = g_cptr->interface + iNum;
					for(altNum = 0; altNum < iptr->num_altsetting; altNum++){
						altptr = iptr->altsetting + altNum;
					    
						if(altptr->bInterfaceClass == LIBUSB_CLASS_PRINTER && altptr->bAlternateSetting == PRINTER_ALTERNATIVE_NUM){
							
						    g_num[1] = altptr->bInterfaceNumber;
						    g_claimed_interface[g_usblp_attached] = altptr->bInterfaceNumber;
							g_usblp_attached++;
							
							/*-----------------kernel setting--------------*/
							if (libusb_kernel_driver_active(g_dh, iNum) == 1) {
#ifdef _DEBUG_MODE_
								fprintf(stderr, "DEBUG: [discover]kernel driver active\n");
#endif
								if(libusb_detach_kernel_driver(g_dh, iNum) == 0) {
#ifdef _DEBUG_MODE_
								     fprintf(stderr, "DEBUG: [discover]kernel driver detached!\n");
#endif
								} else {
									err = CN_USB_WRITE_ERROR;
									goto onErr;
								}
							}
							
							/*-----------------claim interface-------------*/
							while((err = libusb_claim_interface(g_dh, g_num[1])) < 0) {
								if (err != LIBUSB_ERROR_BUSY) {
#ifdef _DEBUG_MODE_
								fprintf(stderr, "ERROR: [discover]libusb_claim_interface Error.%d (%d)\n", g_num[1], err);
#endif
								err = CN_USB_WRITE_ERROR;
								goto onErr;
									
								}
							}
							
#ifdef _DEBUG_MODE_
							fprintf(stderr, "DEBUG: [discover]End of claim interface %d \n", g_num[1]);
#endif
							
							/*-----------------get device MDL--------------*/
							memset(info, 0, sizeof(info));
							memset(key, 0, sizeof(key));
							memset(value, 0, sizeof(value));
							
							err = libusb_control_transfer(g_dh, 0xA1, 0x00, 0x0000, iNum, (unsigned char *)info, sizeof(info), 0);
							//fprintf(stderr, "info = %s\n", &info[2]);
							if(err < 0) {
#ifdef _DEBUG_MODE_
								fprintf(stderr, "ERROR: libusb_control_transfer Error.(%d)\n", err);
#endif
								err = CN_USB_WRITE_ERROR;
								goto onErr;
							}
							
							tp = strtok(&info[2], ";");
							sscanf(tp, "%3[A-Z]:%255[0-9a-zA-Z, ]", key, value);
							if(!strncmp(key, mdl, sizeof(key))){
								tp = NULL;
							}
							while(tp != NULL){
								tp = strtok(NULL, ";");
								memset(key, 0, sizeof(key));
								memset(value, 0, sizeof(value));
								if(tp != NULL) {
									sscanf(tp, "%3[A-Z]:%255[0-9a-zA-Z, ]", key, value);
									if(!strncmp(key, mdl, sizeof(key))){
										break;
									}
								}
							}
							
							/*-----------------get serial number--------------*/
							memset(serial, 0, sizeof(serial));
							length = libusb_get_string_descriptor_ascii(g_dh, devdesc.iSerialNumber,
																	 (unsigned char *)serial,
																	 sizeof(serial) - 1);
							if(length > 0) {
								serial[length] = '\0';
							}
							/* make device uri */
							memset(dev_uri, 0, sizeof(dev_uri));
							snprintf(dev_uri, sizeof(dev_uri), "direct cnijbe2://Canon/?port=usb&serial=%s \"%s\" \"USB Printer #%d with status readback for Canon IJ\"", serial, value, ++n);

							fprintf(stdout, "%s\n", dev_uri);
						}
					}
				}
			}
			CNIF_USB_OneDeviceClose();
		}
	}
onErr:
	CNIF_USB_Close();
	return err;
}

int CNIF_USB_SearchEndpoint(int type)
{
	int e;
	uint8_t end = 0;
	
	int e_max = (g_cptr -> interface + g_num[1] ) -> altsetting -> bNumEndpoints;
	for (e = 0; e < e_max; ++e) {
		end = ((g_cptr -> interface + g_num[1] ) -> altsetting -> endpoint + e) -> bEndpointAddress;
		if(type == CN_USB_PRINTER_WRITE_EP) {
			if((end & 0xf0) == 0x00){
				return end;
			}
		}
		if(type == CN_USB_PRINTER_READ_EP) {
			if((end & 0xf0) == 0x80){
				return end;
			}
		}
	}
	return CN_USB_WRITE_ERROR;
}

int CNIF_USB_IsIvec(char *info)
{
	int result = 0;
	char cmd[] = "CMD";
	char ivec[] = "IVEC";
	char key[4];
	char *value = NULL;
	char *buffer = NULL;
	char *tp;
	
	value = (char *)malloc(CN_SERIAL_NUM_LEN);
	if (value == NULL) {
		return CN_USB_WRITE_ERROR;
	}
	
	buffer = (char *)malloc(CN_READ_SIZE);
	if (buffer == NULL) {
		return CN_USB_WRITE_ERROR;
	}
	
	strncpy(buffer, info, CN_READ_SIZE);
	memset(key, 0, sizeof(key));
	memset(value, 0, CN_SERIAL_NUM_LEN);
	
	tp = strtok(buffer, ";");
	sscanf(tp, "%3[A-Z]:%255[0-9a-zA-Z, ]", key, value);
	
	if(!strncmp(key, cmd, sizeof(key))){
		if(strstr(value, ivec) != NULL) {
			result = 1;
			goto onErr;
		}
	}
	while(tp != NULL){
		tp = strtok(NULL, ";");
		memset(key, 0, sizeof(key));
		memset(value, 0, CN_SERIAL_NUM_LEN);
		if(tp != NULL) {
			sscanf(tp, "%3[A-Z]:%255[0-9a-zA-Z, ]", key, value);
			if(!strncmp(key, cmd, sizeof(key))){
				if(strstr(value, ivec) != NULL) {
					result = 1;
					goto onErr;
				}
			}
		}
	}
	
onErr:
	free(value);
	free(buffer);
	
	return result;
}

int CNIF_USB_IsPrinter(char *info)
{
	char mdl[] = "FAX";
	char buffer[CN_READ_SIZE];
	char *tp;
	
	memset(buffer, '\0', sizeof(buffer));
	strncpy(buffer, info, sizeof(buffer));
	
	tp = strtok(buffer, ";");
	while(tp != NULL){
		tp = strtok(NULL, ";");
		if(tp != NULL) {
			if(strstr(tp, mdl) != NULL) {
#ifdef _DEBUG_MODE_
				printf("DEBUG: this is a endpoint of FAX!\n");
#endif
				return 1;
			}
		}
	}
	
	return 0;
}

int CNIF_USB_CmpSerialNum(const char *serial)
{
	int ret = 0;
	int length = 0;
	char tmp_serial[CN_DEVICE_ID_LEN];
	struct libusb_device_descriptor devdesc;
	
	ret = libusb_get_device_descriptor(g_device, &devdesc);
	if(ret < 0) {
#ifdef _DEBUG_MODE_
		fprintf(stderr, "ERROR: get device descriptor was failed\n");
#endif
		return CN_USB_WRITE_ERROR;
	}
	length = libusb_get_string_descriptor_ascii(g_dh, devdesc.iSerialNumber,
											 (unsigned char *)tmp_serial,
											 sizeof(tmp_serial) - 1);
	if(length > 0) {
		tmp_serial[length] = '\0';
	}
	
#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: get_serialNum = %s, know serialnum = %s\n", tmp_serial, serial);
#endif

	if(strncmp(tmp_serial, serial, length)){
		return CN_USB_WRITE_ERROR;
	}
#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: the same serial number\n");
#endif

	return 0;
}

