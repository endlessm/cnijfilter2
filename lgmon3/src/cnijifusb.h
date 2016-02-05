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

#ifndef _CNIJIFUSB_H_
#define _CNIJIFUSB_H_

#define CN_USB_VENDERID (0x04a9)	//Canon

/*-------Error Code------------------*/
#define CN_USB_WRITE_OK (0)
#define CN_USB_WRITE_END (0)
#define CN_USB_WRITE_ERROR (-1)
#define CN_USB_WRITE_EP_SEARCH_ERROR (-101)
#define CN_USB_READ_EP_SEARCH_ERROR (-102)
#define CN_USB_READ_DATA_ERROR (-103)
#define CN_USB_WRITE_DATA_ERROR (-104)
#define CN_USB_FILE_READ_ERROR (-105)

/*-------Parameter Id----------------*/
#define CN_USB_PRINTER_WRITE_EP (1)
#define CN_USB_PRINTER_READ_EP (2)

int CNIF_USB_Open(const char *device_id, CNIF_INFO *if_info);
int CNIF_USB_Close();
int CNIF_USB_OneDeviceClose();
int CNIF_USB_Reset();
int CNIF_USB_Read(uint8_t *buffer, size_t bufferSize, size_t *readSize);
int CNIF_USB_Write(uint8_t *buffer, size_t bufferSize, size_t *writtenSize);
int CNIF_USB_Cancel();
int CNIF_USB_Discover();
int CNIF_USB_SearchEndpoint(int type);
int CNIF_USB_IsIvec(char *info);
int CNIF_USB_IsPrinter(char *info);
int CNIF_USB_CmpSerialNum(const char *serial);
int CNIF_USB_MakeDeviceUri(uint8_t *uri_buffer, size_t buf_size);

#endif

