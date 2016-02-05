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

#define  CNNL_LIB_LATEST_VERSION			(110)
#define  CACHE_PATH       BJLIB_PATH "/cnnet.ini"

#define WAITTIME_FIND_PRINTERS   (30)
#define WAITTIME_START_SESSION   (5)
#define WAITTIME_RETRY_SEARCH    (30)

#define MAX_COUNT_OF_PRINTERS   (64)
#define STRING_SHORT            (32)
#define SESSION_TIMEOUT         (60)

int CNIF_Network_Open(const char *_deviceID, CNIF_INFO *_if_info);
int CNIF_Network_StartSession();
int CNIF_Network_Close();
int CNIF_Network_Reset();
int CNIF_Network_Read(uint8_t *buffer, size_t bufferSize, size_t *readSize);
int CNIF_Network_Write(uint8_t *buffer, size_t bufferSize, size_t *writtenSize);
int CNIF_Network_GetDeviceID(uint8_t *buffer, size_t bufferSize, size_t *readSize);
int CNIF_Network_Cancel();
int CNIF_Network_Discover(int installer);
