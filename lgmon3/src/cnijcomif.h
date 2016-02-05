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

#ifndef _CNIJCOMIF_H_
#define _CNIJCOMIF_H_

#include <stdint.h>
#include <stddef.h>

typedef enum {
	CNIF_TYPE_USB,
	CNIF_TYPE_NET,
} CNIF_TYPE;

typedef struct _CNIF_INFO
{
	CNIF_TYPE ifType;
	int isIVEC;
} CNIF_INFO;

int CNIF_Open(const char *_deviceID, CNIF_INFO *_if_info);
int CNIF_StartSession(CNIF_INFO *_if_info);
int CNIF_Close(CNIF_INFO *_if_info);
int CNIF_Reset(CNIF_INFO *_if_info);
int CNIF_Read(CNIF_INFO *_if_info, uint8_t *buffer, size_t bufferSize, size_t *readSize);
int CNIF_Write(CNIF_INFO *_if_info, uint8_t *buffer, size_t bufferSize, size_t *writtenSize);
int CNIF_Cancel(CNIF_INFO *_if_info);
int CNIF_Discover(CNIF_INFO *_if_info, int installer);
int CNIF_GetSerialNum(CNIF_INFO *if_info, uint8_t *buffer, size_t bufferSize, char *serial);

int CNIF_MakeDeviceUri(CNIF_INFO *if_info, uint8_t *uri_buffer, size_t size);

#endif

