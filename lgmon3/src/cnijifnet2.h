/*
 *  Canon Inkjet Printer Driver for Linux
 *  Copyright CANON INC. 2001-2016
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

#include "./common/libcnnet3_type.h"


#define CN_NET3_SUCCEEDED		(0)
#define CN_NET3_FAILED			(-1)
#define CN_NET3_INVALID_ARG		(-2)
#define CN_NET3_CREATE_HND_FAIL (-3)
#define CN_NET3_SET_CONFIG_FAIL	(-4)
#define CN_NET3_MAKE_CMD_FAIL	(-5)
#define CN_NET3_SEND_DATA_FAIL	(-6)
#define CN_NET3_READ_DATA_FAIL	(-7)
#define CN_NET3_CLOSE_HND_FAIL	(-8)
#define CN_NET3_SEND_TIMEOUT	(-9)

#define CNMPU2_SEND_DATA_SIZE	(4096)
// #define CNMPU2_READ_BUFFER_SIZE (4096)
#define CNMPU2_READ_BUFFER_SIZE (1024 * 16)

#define CNMPU2_TRUE			(1)
#define CNMPU2_FALSE		(0)

#define CNNL_LIB_LATEST_VERSION	(110)
#define CACHE_PATH       BJLIB_PATH "/cnnet.ini"

#define WAITTIME_FIND_PRINTERS	(30)
#define WAITTIME_START_SESSION	(5)
#define WAITTIME_RETRY_SEARCH	(30)

#define MAX_COUNT_OF_PRINTERS   (64)
#define STRING_SHORT            (32)
#define SESSION_TIMEOUT         (60)

#define NETWORK_DEV_MAX			(64)

#define LEN_MODEL_NAME			(256)
#define LEN_IP_ADDR				(46)
#define LEN_MAC_ADDR			(18)
#define LEN_DEVICE_ID			(1024)

#define CNIJPrinterIvecCommandBufSize	(8192)
#define CNIJPrinterWriteDataSize		(1024 * 4)

#define CNIJCnmpu2PrintLockStatusLocked (0)
#define CNIJCnmpu2PrintLockStatusUnlocked (1)

// #define CNCL_DECODE_NONE	(0)
// #define CNCL_DECODE_EXEC	(1)
// #define CNCL_FILE_TAG_VERSION		"CNIJ-TOOL-VERSION"
// #define CNCL_FILE_TAG_CAPABILITY	"CNIJ-IVEC-CAPABILITY"
#define CNCL_FILE_TAG_DEVICEID		"CNIJ-DEVCE-INFO"
// #define PPD_TOOL_VERSION		"1"

#define MODEL_CANON				"Canon"
#define MODEL_SERIES			"series"
#define MODEL_DES				"DES:"
#define MODEL_SEMICLN			";"

#define SVC_MAINTENANCE			"application/vnd.cups-command"
#define SVC_PRINT				"application/vnd.cups-raster"

typedef struct {
	char		modelName_[LEN_MODEL_NAME];
	char		ipAddressStr_[LEN_IP_ADDR];
	char		macAddressStr_[LEN_MAC_ADDR];
	char		deviceId_[LEN_DEVICE_ID];
	// CNNLNICINFO	nic;
} NETWORK_DEV;


CNNET3_ERROR CNIF_Network2_Open( const char *_deviceID );
CNNET3_ERROR CNIF_Network2_StartSession();
CNNET3_ERROR CNIF_Network2_EndSession();
// CNNET3_ERROR CNIF_Network2_WriteData( unsigned char *sendBuffer, unsigned long bufferSize, int needContinue );
CNNET3_ERROR CNIF_Network2_WriteData( char *sendBuffer, unsigned long bufferSize, int needContinue );
CNNET3_ERROR CNIF_Network2_SendData( unsigned char *sendBuffer, unsigned long bufferSize, size_t *writtenSize );
// CNNET3_ERROR CNIF_Network2_ReadData(unsigned char **buffer, unsigned long *bufferSize);
CNNET3_ERROR CNIF_Network2_Discover( int installer );
CNNET3_ERROR CNIF_Network2_ResolveIPAddr( const char *macAddr );
// CNNET3_ERROR CNIF_ReadStatusPrint(unsigned char **buffer, unsigned long bufferSize, unsigned long *readSize);
CNNET3_ERROR CNIF_Network2_ReadStatusPrint( unsigned char *buffer, unsigned long bufferSize, size_t *readSize );
// CNNET3_ERROR CNIF_ReadStatusPrint(unsigned char **buffer, unsigned long *readSize);
CNNET3_ERROR CNIF_Network2_CancelPrint( char *jobID );
CNNET3_ERROR CNIF_Network2_SendDummyData();
int SearchSnmp();
// int sendData(unsigned char *data);
int sendData( char *data );

