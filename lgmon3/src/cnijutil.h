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

#include <stdint.h>

/*-------UUID Info----------------*/
#define UUID_PTN	"job-uuid=urn:uuid:"
#define UUID_LEN	(36)

#define MAKEPPD_SUCCEEDED	(1)
#define MAKEPPD_FAILED		(0)

typedef struct _CAPABILITY_DATA{
  uint8_t *deviceID;
  size_t deviceIDLength;
  uint8_t *responseBuffer;
  size_t responseSize;
} CAPABILITY_DATA;


int GetUUID( char *arg , char *uuid );
int GetCapabilityFromPPDFile(const char *ppdFileName, CAPABILITY_DATA *_data);
