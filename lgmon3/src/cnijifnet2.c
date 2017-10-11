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

#include <libcnnet3.h>
#include "cnijcomif.h"
#include "cnijlgmon3.h"
#include "cnijifnet2.h"
// #include "cnijutil.h"
#include "./common/libcnnet3_type.h"
#include "./common/libcnnet3_url.h"


#include <dlfcn.h>
#include "./common/libcnnet2.h"


// #define CN_LIB_PATH_LEN 512
#define CNNET2_LIBNAME "libcnbpnet20.so"

/* define WIDSD Library API */
static void * (*pCNNET2_CreateInstance)(void);
static void (*pCNNET2_DestroyInstance)(void *instance);
static CNNET2_ERROR_CODE (*pCNNET2_OptSetting)(void *instance, CNNET2_SETTING_FLAGS settingFlag, unsigned int settingInfo);
static int (*pCNNET2_Search)(void *instance, const char *ipv4Address, search_callback callback, void *arg);
static int (*pCNNET2_SearchByIpv6)(void *instance, const char *ipv6Address, search_callback callback, void *arg);
static void (*pCNNET2_CancelSearch)(void *instance);
static CNNET2_ERROR_CODE (*pCNNET2_EnumSearchInfo)(void *instance, tagSearchPrinterInfo *searchPrinterInfoList, unsigned int *ioSize);
static void callback_CNNET2_Search(void *data, const tagSearchPrinterInfo *printerInfo);

static char *ipAddr;
static NETWORK_DEV	network2dev[NETWORK_DEV_MAX];
static int foundNum;
// static unsigned long sendDataSize;

static HCNNET3 port9100Writer;
static HCNNET3 chmpWriter;

static char model_number[] = "com2";

#ifdef _DEFAULT_PATH_
#define BJLIB_PATH "/usr/lib/bjlib/"
#endif

int (*CNCL_MakeCommand_CancelJob)(char *, char *, unsigned int, unsigned int *);
int (*CNCL_MakeCommand_GetStatus)(char *, unsigned int, unsigned int *);
int (*GET_PROTOCOL)(char *, size_t);

// #define _DEBUG_MODE_


CNNET3_ERROR CNIF_Network2_Open( const char *macAddr )
{
	CNNET3_ERROR ret = CN_NET3_SUCCEEDED;

	port9100Writer = CNNET3_Open();

	if(port9100Writer == NULL){
		ret = CN_NET3_FAILED;
		goto ERROR;
	}

	// Resolve IP Address
	ipAddr = (char *)malloc(LEN_IP_ADDR);

	if(ipAddr == NULL){
		ret = CN_NET3_INVALID_ARG;
		goto ERROR;
	}

	memset( ipAddr, '\0', LEN_IP_ADDR );

	if( CNIF_Network2_ResolveIPAddr( macAddr ) != 0 ){
		ret = CN_NET3_FAILED;
		goto ERROR;
	}

ERROR:
	return ret;
}


CNNET3_ERROR CNIF_Network2_StartSession()
{
	CNNET3_ERROR ret = CN_NET3_SUCCEEDED;

	// Set I/F
	ret = CNNET3_SetIF(port9100Writer, CNNET3_IFTYPE_PORT9100);

	if(ret != CNNET3_ERR_SUCCESS){
		return ret;
	}

	// Set IP Address
	ret = CNNET3_SetIP(port9100Writer, ipAddr);

	if(ret != CNNET3_ERR_SUCCESS){
		return ret;
	}

	return CNNET3_ERR_SUCCESS;
}


CNNET3_ERROR CNIF_Network2_EndSession()
{
	CNNET3_ERROR ret = CN_NET3_SUCCEEDED;

	// pthread_mutex_destroy(&mutex);

	// if(IfType == CNNET3_IFTYPE_PORT9100){
	CNNET3_Close(port9100Writer);
	// }
	// else if(IfType == CNNET3_IFTYPE_HTTP){
		// CNNET3_Close(chmpWriter);
	// }
	// else{
		// ret = CN_NET3_FAILED;
	// }

	if( ipAddr != NULL ){
		free( ipAddr );
	}

	return ret;
}



// CNNET3_ERROR CNIF_Network2_WriteData(unsigned char *sendBuffer, unsigned long bufferSize, int needContinue)
CNNET3_ERROR CNIF_Network2_WriteData( char *sendBuffer, unsigned long bufferSize, int needContinue )
{
	CNNET3_ERROR ret = CN_NET3_SUCCEEDED;

	ret = CNNET3_Write(chmpWriter, (unsigned char *)sendBuffer, bufferSize, needContinue);

	return ret;
}



CNNET3_ERROR CNIF_Network2_SendData( unsigned char *sendBuffer, unsigned long bufferSize, size_t *writtenSize )
{
	int ret = 0;
	int cnt_timeout = 0;

    for (*writtenSize = 0; *writtenSize < bufferSize; ) {
        unsigned long bufSize = bufferSize - *writtenSize;
        unsigned long sentSize = 0;

		if(bufSize > CNMPU2_SEND_DATA_SIZE){
			bufSize = CNMPU2_SEND_DATA_SIZE;
		}

#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [CNNET3_Send] call\n");
#endif

        ret = CNNET3_Send(port9100Writer, (sendBuffer + *writtenSize), bufSize, &sentSize);

#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [CNNET3_Send] ret = %d\n", ret);
#endif

        if (ret == CNNET3_ERR_SUCCESS) {
            // CNIJLOG_MSG(@"SUCCESS!");
            *writtenSize += sentSize;
			cnt_timeout = 0;
        }
        else if (ret == CNNET3_ERR_WRITING_TIMEOUT) {
            // CNIJLOG_MSG(@"!!!!!TIMEOUT!!!!!");
            *writtenSize += sentSize;

			cnt_timeout++;
        }
        else {
            break;
        }

		if( cnt_timeout >= 3 ) {
			ret = CN_NET3_SEND_TIMEOUT;
			break;
		}
    }

	return ret;
}



CNNET3_ERROR CNIF_Network2_Discover(int installer)
{
	char	model[LEN_MODEL_NAME];
	char	model2[LEN_MODEL_NAME];
	char	ipaddr[LEN_IP_ADDR];
	int		i=0, j=0;

	void	*libclss = NULL;
	char	*library_path = NULL;
	int		prot = 0;
	char	model_number[] = "com2";


	library_path = (char *)malloc(CN_LIB_PATH_LEN);

	if(library_path == NULL) {
		return CN_NET3_FAILED;
	}

	memset(library_path, 	'\0', CN_LIB_PATH_LEN);
	snprintf(library_path, CN_LIB_PATH_LEN - 1, CN_CNCL_LIB_PATH, model_number);


	libclss = dlopen(library_path, RTLD_LAZY);

	if(!libclss){
#ifdef _DEBUG_MODE_
		fprintf(stderr, "ERROR: dynamic link error.(%s)", dlerror());
#endif
		return CN_LGMON_DYNAMID_LINK_ERROR;
	}


	GET_PROTOCOL = dlsym(libclss, "CNCL_GetProtocol");

	if(dlerror() != NULL){
#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: cannnot find function4.(%s)", dlerror());
#endif

		return CN_LGMON_DYNAMID_LINK_ERROR;
	}


	if( SearchSnmp() == CNNET2_ERROR_CODE_SUCCESS ){
		int cnt = 0;

		for (j = 0; j < foundNum; j++){

			prot = GET_PROTOCOL( (char *)network2dev[j].deviceId_, sizeof(network2dev[j].deviceId_) );

#ifdef _DEBUG_MODE_
			fprintf(stderr, "DEBUG: [Discover] prot = %d\n", prot);
			fprintf(stderr, "DEBUG: [Discover] network2dev[%d].deviceId_ = %s\n", j, network2dev[j].deviceId_ );
#endif

			if( prot == 2 ){
				memset( model, 0x00, sizeof(model) );
				memset( model2, 0x00, sizeof(model2) );
				memset( ipaddr, 0x00, sizeof(ipaddr) );

				strncpy( ipaddr, network2dev[j].ipAddressStr_, LEN_IP_ADDR );

				strncpy( model, network2dev[j].modelName_, LEN_MODEL_NAME );

				for (i=0; model[i]!='\0'; i++){
					if (model[i] == ' '){
						model2[i]='-';
					} else {
						model2[i]=model[i];
					}
				}

				if (installer != 1){
					printf("network cnijbe2://Canon/?port=net&serial=%c%c-%c%c-%c%c-%c%c-%c%c-%c%c \"%s\" \"%s_%c%c-%c%c-%c%c-%c%c-%c%c-%c%c\"\n",
						network2dev[j].macAddressStr_[0], network2dev[j].macAddressStr_[1], network2dev[j].macAddressStr_[3],
						network2dev[j].macAddressStr_[4], network2dev[j].macAddressStr_[6], network2dev[j].macAddressStr_[7],
						network2dev[j].macAddressStr_[9], network2dev[j].macAddressStr_[10], network2dev[j].macAddressStr_[12],
						network2dev[j].macAddressStr_[13], network2dev[j].macAddressStr_[15], network2dev[j].macAddressStr_[16],
						model,
						model2,
						network2dev[j].macAddressStr_[0], network2dev[j].macAddressStr_[1], network2dev[j].macAddressStr_[3],
						network2dev[j].macAddressStr_[4], network2dev[j].macAddressStr_[6], network2dev[j].macAddressStr_[7],
						network2dev[j].macAddressStr_[9], network2dev[j].macAddressStr_[10], network2dev[j].macAddressStr_[12],
						network2dev[j].macAddressStr_[13], network2dev[j].macAddressStr_[15], network2dev[j].macAddressStr_[16]);
				} else {
					printf("network cnijbe2://Canon/?port=net&serial=%c%c-%c%c-%c%c-%c%c-%c%c-%c%c \"%s\" \"IP:%s\"\n", 
						network2dev[j].macAddressStr_[0], network2dev[j].macAddressStr_[1], network2dev[j].macAddressStr_[3],
						network2dev[j].macAddressStr_[4], network2dev[j].macAddressStr_[6], network2dev[j].macAddressStr_[7],
						network2dev[j].macAddressStr_[9], network2dev[j].macAddressStr_[10], network2dev[j].macAddressStr_[12],
						network2dev[j].macAddressStr_[13], network2dev[j].macAddressStr_[15], network2dev[j].macAddressStr_[16],
						model, ipaddr);
				}

				cnt++;
			}
		}

		foundNum = cnt;
	}

	if( libclss != NULL ){
		dlclose(libclss);
	}

	if( library_path != NULL ){
		free(library_path);
	}

	return CN_LGMON_OK;
}



CNNET3_ERROR CNIF_Network2_CancelPrint( char *jobID ){
    // char			cmdBuffer[2048];
    unsigned int	writtenSize = 0;
	int				result = 0;
	// unsigned char	*data = NULL;
	char			*data = NULL;
	char			*library_path = NULL;
	void			*libclss = NULL;

	// memset( cmdBuffer, 0x00, sizeof(cmdBuffer) );

	data = (char *) malloc(2048);

	if( data == NULL ){
		result = CN_NET3_FAILED;
		goto EXIT;
	}

	memset( data, '\0', 2048 );

	chmpWriter = CNNET3_Open();

	if( chmpWriter == NULL ){
		result = CN_NET3_CREATE_HND_FAIL;
		goto EXIT;
	}

	// Set I/F
	result = CNNET3_SetIF( chmpWriter, CNNET3_IFTYPE_HTTP );

	if(result != CNNET3_ERR_SUCCESS){
		result = CN_NET3_SET_CONFIG_FAIL;
		goto EXIT;
	}

	// Set URL
	result = CNNET3_SetURL( chmpWriter, CNNET3_URL_CONTROL );

	if(result != CNNET3_ERR_SUCCESS){
		result = CN_NET3_SET_CONFIG_FAIL;
		goto EXIT;
	}

	// Set IP Address
	result = CNNET3_SetIP( chmpWriter, ipAddr );

	if(result != CNNET3_ERR_SUCCESS){
		result = CN_NET3_SET_CONFIG_FAIL;
		goto EXIT;
	}

	library_path = (char *)malloc(CN_LIB_PATH_LEN);

	if(library_path == NULL) {
		result = CN_LGMON_OTHER_ERROR;
		goto EXIT;
	}

	memset(library_path, '\0', CN_LIB_PATH_LEN);
	snprintf(library_path, CN_LIB_PATH_LEN - 1, CN_CNCL_LIB_PATH, model_number);

	libclss = dlopen(library_path, RTLD_LAZY);

	if( ! libclss ){
		return CN_LGMON_DYNAMID_LINK_ERROR;
	}

	CNCL_MakeCommand_CancelJob = dlsym(libclss, "CNCL_MakeCommand_CancelJob");

	if(dlerror() != NULL){
		result = CN_LGMON_DYNAMID_LINK_ERROR;
		goto EXIT;
	}

	result = CNCL_MakeCommand_CancelJob( jobID, data, 2048, &writtenSize );

	if ( result != CN_NET3_SUCCEEDED ) {
		result = CN_NET3_MAKE_CMD_FAIL;
		goto EXIT;
	}

	// data = cmdBuffer;

	result = sendData( data );

	if ( result != CNNET3_ERR_SUCCESS){
		// errorLine = __LINE__;
		result = CN_NET3_SEND_DATA_FAIL;
		goto EXIT;
	}


EXIT:
	if( chmpWriter != NULL ){
		CNNET3_Close(chmpWriter);

		chmpWriter = NULL;
	}

	if( data != NULL){
		free( data );
	}

	return result;
}


/* CNNET2 Functions */
CNNET3_ERROR CNIF_Network2_ResolveIPAddr(const char *macAddr)
{
	int i;

	CNNET3_ERROR ret = CN_NET3_SUCCEEDED;

	if( SearchSnmp() != CN_NET3_SUCCEEDED ){
		ret = CN_NET3_FAILED;
		goto onErr1;
	}

	// int arrayNum = sizeof(network2dev) / sizeof(NETWORK_DEV);
	i = 0;

	while( strlen( network2dev[i].modelName_ ) != 0 ) {
		if( strcmp(macAddr, network2dev[i].macAddressStr_) == 0 ){
			memcpy( ipAddr, network2dev[i].ipAddressStr_, LEN_IP_ADDR );

			ret = CN_NET3_SUCCEEDED;
			break;
		}
		else{
			ret = CN_NET3_FAILED;
		}

		i++;
	}

onErr1:
	return ret;
}



int sendData( char *data )
{
	int ret= 0;
	int i = 0;

#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: [sendData] data : %s\n", data);
	fprintf(stderr, "DEBUG: [CNIF_Network2_WriteData] call\n");
#endif

	for( i = 0; i < 10; i++ ){
		ret = CNIF_Network2_WriteData(data, strlen(data), CNNET3_FALSE);

#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [CNIF_Network2_WriteData] ret = %d\n", ret);
#endif

		if ( ret == CNNET3_ERR_SUCCESS ) {
			goto EXIT;
		}
		else{
			continue;
		}
	}


EXIT:
	return ret;
}



CNNET3_ERROR CNIF_Network2_ReadStatusPrint(unsigned char *buffer, unsigned long bufferSize, size_t *readSize)
{
	int						result = CNNET3_FALSE;
	// char					cmdBuffer[ 1024 ];
	char					*cmdBuffer;
	unsigned int			writtenSize;
	// unsigned char*			data = NULL;
	char*					data = NULL;

	// unsigned long			size = 0;
	CNNET3_BOOL				flag = 1;
	// unsigned char			resBuffer[ CNIJPrinterIvecCommandBufSize ];
	// unsigned char*			curPtr;
	unsigned long			total = 0;
	int						err = 0;
	char					*library_path = NULL;
	void					*libclss = NULL;


	cmdBuffer = (char *) malloc(CNMPU2_SEND_DATA_SIZE);

	if( cmdBuffer != NULL ){
		memset(cmdBuffer, '\0', CNMPU2_SEND_DATA_SIZE);
	}
	else{
		result = CN_NET3_SEND_DATA_FAIL;
		goto EXIT;
	}


#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: [CNIF_Network2_ReadStatusPrint] Start\n");
	fprintf(stderr, "DEBUG: [CNNET3_Open] call\n");
#endif

	chmpWriter = CNNET3_Open();

	if( chmpWriter == NULL ){
		result = CN_NET3_CREATE_HND_FAIL;
		goto EXIT;
	}

#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: [CNNET3_SetIF] call\n");
#endif

	// Set I/F
	err = CNNET3_SetIF( chmpWriter, CNNET3_IFTYPE_HTTP );

	if(err != CNNET3_ERR_SUCCESS){
		result = CN_NET3_SET_CONFIG_FAIL;
		goto EXIT;
	}

#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: [CNNET3_SetURL] call\n");
#endif

	// Set URL
	err = CNNET3_SetURL( chmpWriter, CNNET3_URL_CONTROL );

	if(err != CNNET3_ERR_SUCCESS){
		result = CN_NET3_SET_CONFIG_FAIL;
		goto EXIT;
	}

#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: [CNNET3_SetIP] call\n");
#endif

	// Set IP Address
	err = CNNET3_SetIP( chmpWriter, ipAddr );

	if(err != CNNET3_ERR_SUCCESS){
		result = CN_NET3_SET_CONFIG_FAIL;
		goto EXIT;
	}

	library_path = (char *)malloc(CN_LIB_PATH_LEN);

	if(library_path == NULL) {
		result = CN_LGMON_OTHER_ERROR;
		goto EXIT;
	}

	memset(library_path, '\0', CN_LIB_PATH_LEN);
	snprintf(library_path, CN_LIB_PATH_LEN - 1, CN_CNCL_LIB_PATH, model_number);

	libclss = dlopen(library_path, RTLD_LAZY);

	if( ! libclss ){

#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [%s] dlopen failed\n", library_path);
#endif

		return CN_LGMON_DYNAMID_LINK_ERROR;
	}

#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: [CNCL_MakeCommand_GetStatus] dlsym\n");
#endif

	CNCL_MakeCommand_GetStatus = dlsym(libclss, "CNCL_MakeCommand_GetStatusPrint");

	if(dlerror() != NULL){

#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [CNCL_MakeCommand_GetStatus] dlsym failed\n");
#endif
		result = CN_LGMON_DYNAMID_LINK_ERROR;
		goto EXIT;
	}

#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: [CNCL_MakeCommand_GetStatus] call\n");
#endif

	// err = CLSS_MakeCommand_GetStatusPrint(NULL, cmdBuffer, sizeof(cmdBuffer), &writtenSize);
	err = CNCL_MakeCommand_GetStatus( cmdBuffer, CNMPU2_SEND_DATA_SIZE, &writtenSize );

	if ( err != CN_NET3_SUCCEEDED ) {

#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [ERR] [CNCL_MakeCommand_GetStatus] failed\n");
#endif
		result = CN_NET3_MAKE_CMD_FAIL;
		goto EXIT;
	}

	data = cmdBuffer;

#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: [sendData] call\n");
	fprintf(stderr, "DEBUG: [sendData] cmdBuffer = %s\n", cmdBuffer);
	fprintf(stderr, "DEBUG: [sendData] data = %s\n", data);
#endif

	err = sendData( data );

	if ( err != CNNET3_ERR_SUCCESS){

#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [sendData] failed : %d\n", err);
#endif
		// errorLine = __LINE__;
		result = CN_NET3_SEND_DATA_FAIL;
		goto EXIT;
	}

	// size = CNIJPrinterIvecCommandBufSize;
	flag = 1;
	// curPtr = resBuffer;
	total = 0;

	while ( flag != 0 ){

#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [CNNET3_Read] flag : %d\n", flag);
#endif

		err = CNNET3_Read( chmpWriter, buffer, &bufferSize, &flag );

		if ( (err == CNNET3_ERR_SUCCESS) || (err == CNNET3_ERR_READING_TIMEOUT) ) {
			total += bufferSize;
			buffer += bufferSize;
		}
		else {

#ifdef _DEBUG_MODE_
			fprintf(stderr, "DEBUG: [CNNET3_Read] failed : %d\n", err);
#endif
			result = CN_NET3_READ_DATA_FAIL;
			goto EXIT;
		}
	}

	*readSize = total;

	result = CNNET3_ERR_SUCCESS;

EXIT:
	if( chmpWriter != NULL ){

#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: [CNNET3_Close] call\n");
#endif

		err = CNNET3_Close(chmpWriter);

		chmpWriter = NULL;

		if( err < 0 ){
			result = CN_NET3_CLOSE_HND_FAIL;
			// goto EXIT;
		}
	}

	if( cmdBuffer != NULL ){
		free( cmdBuffer );
	}

#ifdef _DEBUG_MODE_
	fprintf(stderr, "DEBUG: [CNIF_Network2_ReadStatusPrint] Exit : %d\n", result);
#endif

	return result;
}


CNNET3_ERROR CNIF_Network2_SendDummyData()
{
	int ret = 0;

	ret = CNNET3_Write(port9100Writer, (unsigned char *)"0x00", 1, CNNET3_FALSE);

    if( ret != CNNET3_ERR_SUCCESS ){
    }

	return ret;
}


void DeleteChar( char *s1, char *s2 )
{
	char *p = s1;
	p = strstr( s1, s2 );

	if( p != NULL ) {
		strcpy( p, p + strlen( s2 ) );
		DeleteChar( p + 1, s2 );
	}
}


int SearchSnmp()
{
	char libPathBuf[CN_LIB_PATH_LEN];
	void *libcnnet2 = NULL;
	void *instance = NULL;
	tagSearchPrinterInfo *infoList = NULL;
	CNNET2_ERROR_CODE err;
	unsigned int size = 0;
	unsigned int i;
	int num = 0;
	int ret = CN_NET3_SUCCEEDED;

	strncpy( libPathBuf, CNNET2_LIBNAME, CN_LIB_PATH_LEN );
	libcnnet2 = dlopen( libPathBuf, RTLD_LAZY );

	if ( !libcnnet2 ) {
		ret = CN_NET3_FAILED;
		goto onErr2;
	}

	/* Load Symbol */
	pCNNET2_CreateInstance = dlsym( libcnnet2, "CNNET2_CreateInstance" );
	if ( dlerror() != NULL ) {
		ret = CN_NET3_FAILED;
		goto onErr2;
	}

	pCNNET2_DestroyInstance = dlsym( libcnnet2, "CNNET2_DestroyInstance" );
	if ( dlerror() != NULL ) {
		ret = CN_NET3_FAILED;
		goto onErr2;
	}

	pCNNET2_OptSetting = dlsym( libcnnet2, "CNNET2_OptSetting" );
	if ( dlerror() != NULL ) {
		ret = CN_NET3_FAILED;
		goto onErr2;
	}

	pCNNET2_Search = dlsym( libcnnet2, "CNNET2_Search" );
	if ( dlerror() != NULL ) {
		ret = CN_NET3_FAILED;
		goto onErr2;
	}

	pCNNET2_SearchByIpv6 = dlsym( libcnnet2, "CNNET2_SearchByIpv6" );
	if ( dlerror() != NULL ) {
		ret = CN_NET3_FAILED;
		goto onErr2;
	}

	pCNNET2_CancelSearch = dlsym( libcnnet2, "CNNET2_CancelSearch" );
	if ( dlerror() != NULL ) {
		ret = CN_NET3_FAILED;
		goto onErr2;
	}

	pCNNET2_EnumSearchInfo = dlsym( libcnnet2, "CNNET2_EnumSearchInfo" );
	if ( dlerror() != NULL ) {
		ret = CN_NET3_FAILED;
		goto onErr2;
	}

	/* Create Instance */
	instance = pCNNET2_CreateInstance();

	/* Settings for Discovery */
	err = pCNNET2_OptSetting( instance, CNNET2_SETTING_FLAG_DISCOVER_PRINTER_TIMEOUT_MILLIS, 2000 );

	if ( err != CNNET2_ERROR_CODE_SUCCESS ) {
		ret = CN_NET3_FAILED;
		goto onErr3;
	}

	/* Search */
	num = pCNNET2_Search( instance, NULL, callback_CNNET2_Search, NULL );

	foundNum = num;

	if ( num < CNNET2_ERROR_CODE_SUCCESS ) {
		ret = CN_NET3_FAILED;
		goto onErr3;
	}

	/* Show Search Result */
	if ( num > 0 ) {
		infoList = (tagSearchPrinterInfo*) malloc( sizeof(tagSearchPrinterInfo) * num );

		if ( infoList == NULL ){
			ret = CN_NET3_FAILED;
			goto onErr3;
		}

		memset(infoList, '\0', sizeof(tagSearchPrinterInfo) * num);
	}
	else{
		ret = CN_NET3_FAILED;
		goto onErr3;
	}

	size = sizeof(tagSearchPrinterInfo) * num;
	// err = pCNNET2_EnumSearchInfo( instance, list, &size );
	err = pCNNET2_EnumSearchInfo( instance, infoList, &size );


	if ( err != CNNET2_ERROR_CODE_SUCCESS ) {
		ret = CN_NET3_FAILED;
		goto onErr3;
	}


	if ( num > NETWORK_DEV_MAX ){
		num = NETWORK_DEV_MAX;
	}

	for ( i = 0; i < num; i++ ) {
		strncpy( network2dev[i].modelName_, infoList[i].modelName_, LEN_MODEL_NAME );
		strncpy( network2dev[i].ipAddressStr_, infoList[i].ipAddressStr_, LEN_IP_ADDR );
		snprintf( network2dev[i].macAddressStr_, LEN_MAC_ADDR, "%c%c-%c%c-%c%c-%c%c-%c%c-%c%c",
				infoList[i].MacAddressStr_[0], infoList[i].MacAddressStr_[1], infoList[i].MacAddressStr_[2], infoList[i].MacAddressStr_[3],
				infoList[i].MacAddressStr_[4], infoList[i].MacAddressStr_[5], infoList[i].MacAddressStr_[6], infoList[i].MacAddressStr_[7],
				infoList[i].MacAddressStr_[8], infoList[i].MacAddressStr_[9], infoList[i].MacAddressStr_[10], infoList[i].MacAddressStr_[11] );
		strncpy( network2dev[i].deviceId_, infoList[i].deviceId_, LEN_DEVICE_ID );

		char	*cnt_cn = NULL;
		char	*cnt_ser = NULL;

		cnt_cn = strstr( network2dev[i].modelName_, MODEL_CANON );
		cnt_ser = strstr( network2dev[i].modelName_, MODEL_SERIES );

		if( cnt_cn == NULL || cnt_ser == NULL ){
			char *cnt_des;
			char *cnt_colon;

			cnt_des = strstr( infoList[i].deviceId_, MODEL_DES );

			if( cnt_des == NULL ){
				ret = CN_NET3_FAILED;
				goto onErr3;
			}
			else{
				cnt_colon = strstr( cnt_des, MODEL_SEMICLN );

				if( cnt_colon == NULL ){
					ret = CN_NET3_FAILED;
					goto onErr3;
				}
				else{
					strncpy( network2dev[i].modelName_, cnt_des + sizeof( MODEL_DES ) - 1, strlen( cnt_des ) - strlen( cnt_colon ) - sizeof( MODEL_DES ) + 1 );
				}
			}
		}
	}


onErr3:
	pCNNET2_DestroyInstance( instance );

onErr2:
	if ( infoList != NULL ) free( infoList );

	return ret;
}

static void callback_CNNET2_Search(void *data, const tagSearchPrinterInfo *printerInfo)
{
}

