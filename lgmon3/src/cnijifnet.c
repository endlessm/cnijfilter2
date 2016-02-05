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
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <dlfcn.h>

#include <libcnnet.h>
#include "cnijcomif.h"
#include "cnijlgmon3.h"
#include "cnijifnet.h"

#ifdef _DEFAULT_PATH_
#define BJLIB_PATH "/usr/lib/bjlib/"
#endif

static CNNLHANDLE    hnd = NULL;
static int isCanceled = 0;
//static unsigned long packet_size= 4*1024;


int CNIF_Network_Open(const char *_deviceID, CNIF_INFO *_if_info)
{
	unsigned long latest_version = CNNL_LIB_LATEST_VERSION;
	unsigned long regacy_version = 100;
	unsigned long param_size     = sizeof(unsigned long);
	char ipaddr[STRING_SHORT];
	
	fprintf(stderr, "DEBUG: CNIF_Network_Open DeviceID=%s\n", _deviceID);
	
	/* Init */
	if (CNNL_Init(&hnd) != CNNL_RET_SUCCESS){
		goto error_init;
	}
	if (CNNL_Config(hnd, CNNL_CONFIG_SET_VERSION, &latest_version, &param_size) != CNNL_RET_SUCCESS){
		goto error_open;
	}
		
	/* ---------------------------------------------------------------------
	    detect specified printers
	*/
	{
		int found=0;
		memset(ipaddr, 0x00, STRING_SHORT);
		if (CNNL_GetIPAddressEx(hnd, CACHE_PATH, _deviceID, ipaddr, STRING_SHORT, CNNET_SEARCH_AUTO, 1, 5000) == CNNL_RET_SUCCESS){
			found=1;
		}
		
		/* not found */
		if (found == 0) goto error_open;
		
		/* cancelled */
		if (isCanceled) goto error_open;
	}
	
	/* ---------------------------------------------------------------------
	    open printer 
	*/

	if (CNNL_OpenEx(hnd, ipaddr, CNNET_TYPE_PRINTER, 10, 60000) != CNNL_RET_SUCCESS){
		goto error_open;
	}
	
	// check wether the printer exists or not
	if (CNNL_CheckVersion(hnd, 10, 60000) != CNNL_RET_SUCCESS){
		goto error_close;
	}
	
	// check command type
	int command = CNNL_COMMAND_SUPPORT;
	if (CNNL_GetCommandType(hnd, &command, 10, 60000) != CNNL_RET_SUCCESS){
		goto error_close;
	}
	_if_info->isIVEC = (command == CNNL_COMMAND_SUPPORT)? 1 : 0;
		
	// switch functions
	if (command == CNNL_COMMAND_SUPPORT){
		// support ivec commands
	} else {
		param_size = sizeof(unsigned long);
		if (CNNL_Config(hnd, CNNL_CONFIG_SET_VERSION, &regacy_version, &param_size) != CNNL_RET_SUCCESS){
			goto error_close;
		}
	}
	return CN_LGMON_OK;
	
error_close:
	CNNL_Close(hnd);
	
error_open:
	CNNL_Terminate(&hnd);
		
error_init:
	return CN_LGMON_ERROR;
}

int CNIF_Network_StartSession()
{
	int ret = 0;
	
	// request to start session
	if ((ret=CNNL_SessionStart(hnd, "Linux", "Linux", "Linux", 3, 9000)) == CNNL_RET_SUCCESS){
		// succeed to start session
			
		/* set timeout */
		CNNL_SetTimeout(hnd, SESSION_TIMEOUT, 3, 15000);
		return CN_LGMON_OK;
	}
	
	// failed to start session
	if (ret == CNNL_RET_BUSY){
		// device busy
		return CN_LGMON_BUSY;
	}
	else
	{
		// fatal error
		return CN_LGMON_ERROR;
	}
}

int CNIF_Network_Close()
{
	CNNL_SessionEnd(hnd, 5, 30000);
	
	if (hnd != NULL){
		CNNL_Close(hnd);
	}
	CNNL_Terminate(&hnd);
	return CN_LGMON_OK;
}

int CNIF_Network_Reset()
{
	CNNL_SoftReset(hnd, 5, 30000);
	return CN_LGMON_OK;
}

int CNIF_Network_Read(uint8_t *buffer, size_t bufferSize, size_t *readSize)
{
	int i=0;
	for (i=0; i<20; i++)
	{
		int result = CNNL_DataRead(hnd, buffer, (long unsigned int *)readSize, bufferSize, 3, 9000);
		
		if (result == CNNL_RET_SUCCESS){
			//fprintf(stderr, "DEBUG: ---CNNL_DataRead From---------------------------------------------\n");
			//fprintf(stderr, "%s\n", buffer);
			//fprintf(stderr, "DEBUG: ---CNNL_DataRead To  ---------------------------------------------\n");
			return CN_LGMON_OK;
		} else if (result == CNNL_RET_BUSY_RESPONSE){
			sleep(1);
			continue;
		} else {
			return CN_LGMON_ERROR;
		}
	}
	return CN_LGMON_ERROR;
}

int CNIF_Network_Write(uint8_t *buffer, size_t bufferSize, size_t *writtenSize)
{
	int result = CNNL_DataWrite(hnd, buffer, bufferSize, (long unsigned int *)writtenSize, 5, 30000);
			
	if (result == CNNL_RET_SUCCESS){
		return CN_LGMON_OK;
	} else if (result == CNNL_RET_FAILURE){
		return CN_LGMON_ERROR;
	} else {
		// busy
		usleep(40*1000);
		return CN_LGMON_OK;
	}
}

int CNIF_Network_GetDeviceID(uint8_t *buffer, size_t bufferSize, size_t *readSize)
{
	if (CNNL_GetDeviceID(hnd, buffer, (long unsigned int *)readSize, bufferSize, 3, 9000) != CNNL_RET_SUCCESS){
		return CN_LGMON_ERROR;
	} else {
		return CN_LGMON_OK;
	}
}

int CNIF_Network_Cancel()
{
	CNNL_Abort(hnd);
	isCanceled = 1;
	return CN_LGMON_OK;
}

int CNIF_Network_Discover(int installer)
{
	CNNLHANDLE    hmdl           = NULL;
	unsigned long latest_version = CNNL_LIB_LATEST_VERSION;
	unsigned long param_size     = sizeof(unsigned long);
	CNNLNICINFO   *nic;
	char          model[STRING_SHORT];
	char          model2[STRING_SHORT];
	char          ipaddr[STRING_SHORT];
	int           i=0, j=0, max = MAX_COUNT_OF_PRINTERS, found=0;
	
	if (CNNL_Init(&hmdl) != CNNL_RET_SUCCESS) goto error;
	
	if (CNNL_Config(hmdl, CNNL_CONFIG_SET_VERSION, &latest_version, &param_size) != CNNL_RET_SUCCESS){
		goto error;
	}
	
	// find printers
	nic = (CNNLNICINFO *)malloc(sizeof(CNNLNICINFO)*max);
	memset(nic, 0x00, sizeof(CNNLNICINFO)*max);
	
	if (CNNL_SearchPrintersEx(hmdl, nic, CACHE_PATH, max, &found, CNNET_SEARCH_BROADCAST, 1, (installer==1)? 5000:3000) == CNNL_RET_SUCCESS){
		
		for (j=0; j<found; j++){
			
			memset(model, 0x00, sizeof(model));
			memset(model2, 0x00, sizeof(model2));
			
			memset(ipaddr, 0x00, STRING_SHORT);
			snprintf(ipaddr, STRING_SHORT-1, "%d.%d.%d.%d", 
					nic[j].ipaddr[0],nic[j].ipaddr[1],nic[j].ipaddr[2],nic[j].ipaddr[3]);
			
			if (CNNL_OpenEx(hmdl, ipaddr, CNNET_TYPE_PRINTER, 1, 3000) == CNNL_RET_SUCCESS){
				if (CNNL_GetModelName(hmdl, model, STRING_SHORT-1, 1, 3000) == CNNL_RET_SUCCESS){
					
					for (i=0; model[i]!='\0'; i++){
						if (model[i] == ' '){
							model2[i]='-';
						} else {
							model2[i]=model[i];
						}
					}
					
					if (installer != 1){
						/* cups list device mode */
						printf("network cnijbe2://Canon/?port=net&serial=%02X-%02X-%02X-%02X-%02X-%02X \"%s\" \"%s_%02X-%02X-%02X-%02X-%02X-%02X\"\n", 
							nic[j].macaddr[0],nic[j].macaddr[1],nic[j].macaddr[2],
							nic[j].macaddr[3],nic[j].macaddr[4],nic[j].macaddr[5],
							model,
							model2,
							nic[j].macaddr[0],nic[j].macaddr[1],nic[j].macaddr[2],
							nic[j].macaddr[3],nic[j].macaddr[4],nic[j].macaddr[5]);
					} else {
						/* installer mode */
						printf("network cnijbe2://Canon/?port=net&serial=%02X-%02X-%02X-%02X-%02X-%02X \"%s\" \"IP:%s\"\n", 
							nic[j].macaddr[0],nic[j].macaddr[1],nic[j].macaddr[2],
							nic[j].macaddr[3],nic[j].macaddr[4],nic[j].macaddr[5],
							model, ipaddr);
					}
				}
				CNNL_Close(hmdl);
			}
		}
	} else {
		free(nic);
		goto error;
	}
	
	free(nic);
	CNNL_Terminate(&hmdl);
	return CN_LGMON_OK;
	
error:
	if (hmdl!= NULL) CNNL_Terminate(&hmdl);

	return CN_LGMON_OK;
}
