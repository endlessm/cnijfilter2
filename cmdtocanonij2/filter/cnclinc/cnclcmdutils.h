/*
 *  Canon Inkjet Printer Driver for Linux
 *  Copyright CANON INC. 2001-2015
 * *
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



#ifndef __CNCLCMDUTILS_H__
#define __CNCLCMDUTILS_H__

/*
 * Printer Settings
 */
typedef struct {
	short version;
	short papersize;
	short mediatype;
	short borderlessprint;
	short colormode;
	short duplexprint;
} CNCL_P_SETTINGS, *CNCL_P_SETTINGSPTR;


/*
 *   Parameter Id
 */
#define CNCL_COMMAND_START1		(1)
#define CNCL_COMMAND_START2		(2)
#define CNCL_COMMAND_END		(3)

/* decode */
#define CNCL_DECODE_EXEC	(1)
#define CNCL_DECODE_NONE	(0)


/* file tags */
#define CNCL_FILE_TAG_VERSION		"CNIJ-TOOL-VERSION"
#define CNCL_FILE_TAG_CAPABILITY	"CNIJ-IVEC-CAPABILITY"
#define CNCL_FILE_TAG_DEVICEID		"CNIJ-DEVCE-INFO"

/*
 *   prototypes
 */
extern unsigned short CNCL_GetInfoResponse(char *data, int readed_data, unsigned short *oprationId, char *jobId, unsigned short **responseDetail);
extern int CNCL_GetPrintCommand(char *cmdBuffer, long cmdBufferSize, long *writtenSize, char *jobId, long opration_id);
extern int CNCL_GetStatus(char *data, int readed_data, int *statusId, int *statusDetail, char *supportId);
extern int CNCL_EncodeToString(const uint8_t *buffer, const size_t buffersize, char *strbuffer, const size_t strsize);
extern int CNCL_DecodeFromString(const char *strbuffer, const size_t strsize, uint8_t *buffer, const size_t buffersize);
extern int CNCL_GetStringWithTagFromFile(const char* fileName, const char* tagName, int decode, uint8_t** resBuffer);
extern int CNCL_GetSetConfigurationCommand( CNCL_P_SETTINGSPTR pSettings, char *jobID,long cmdBufferSize,void *xmlBuffer,long xmlBufferSize, char *cmdBuffer,long *writtenSize);
extern int CNCL_GetSendDataPWGRasterCommand( char *jobID,long data_size,long cmdBufferSize,char *cmdBuffer,long *writtenSize);
extern int CNCL_GetSetPageConfigurationCommand( char *jobID, unsigned short nextpage, void *cmdBuffer, long cmdBufferSize,long *writtenSize );
extern int CNCL_MakeBJLSetTimeJob(void *cmdBuffer, size_t cmdBufferSize, size_t *writtenSize );
extern int CNCL_MakeGetCapabilityCommand(char *cmdBuffer, unsigned int cmdBufferSize, unsigned int *writtenSize, int service_type );

#endif  /*  __CNCLCMDUTILS_H__ */
