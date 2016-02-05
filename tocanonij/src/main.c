/*
 *  CUPS add-on module for Canon Inkjet Printer.
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <dlfcn.h>
#include <errno.h>
//#include "cncl.h"
#include "cnclcmdutils.h"
#include "cnclcmdutilsdef.h"
#include "cndata_def.h"
#include "com_def.h"
#include "cncl_paramtbl.h"

#define CN_LIB_PATH_LEN 512
#define CN_CNCL_LIBNAME "libcnbpcnclapicom2.so"
#define TMP_BUF_SIZE 256
#define IS_NUMBER(c)	(c >= '0' && c <= '9')


enum {
	OPT_VERSION = 0,
	OPT_FILTERPATH,
	OPT_PAPERSIZE,
	OPT_MEDIATYPE,
	OPT_BORDERLESSPRINT,
	OPT_COLORMODE,
	OPT_DUPLEXPRINT,
};

static int is_size_X(char *str)
{
	int is_size = 1;

	while( *str && is_size )
	{
		if( *str == '.' ) break;	/* Ver.2.90 */

		switch( is_size )
		{
		case 1:
			if( IS_NUMBER(*str) )
				is_size = 2;
			else
				is_size = 0;
			break;
		case 2:
			if( *str == 'X' )
				is_size = 3;
			else if( !IS_NUMBER(*str) )
				is_size = 0;
			break;
		case 3:
		case 4:
			if( IS_NUMBER(*str) )
				is_size = 4;
			else
				is_size = 0;
			break;

		}
		str++;
	}

	return (is_size == 4)? 1 : 0;
}

static void to_lower_except_size_X(char *str)
{
	if( !is_size_X(str) )
	{
		while( *str )
		{
			if( *str >= 'A' && *str <= 'Z' )
				*str = *str - 'A' + 'a';
			str++;
		}
	}
}

static long ConvertStrToID( const char *str, const MapTbl *tbl )
{
	int result = -1;
	const MapTbl *cur = tbl;
	char srcBuf[TMP_BUF_SIZE];
	char dstBuf[TMP_BUF_SIZE];

	if ( cur == NULL ) goto onErr;

	while( cur->optNum != -1 ){
		strncpy( srcBuf, str, TMP_BUF_SIZE ); srcBuf[TMP_BUF_SIZE-1] = '\0';
		to_lower_except_size_X(srcBuf);

		strncpy( dstBuf, cur->optName, TMP_BUF_SIZE ); dstBuf[TMP_BUF_SIZE-1] = '\0';
		to_lower_except_size_X(dstBuf);

		if ( !strcmp( dstBuf, srcBuf ) ){
			result = cur->optNum;
			break;
		}
		cur++;
	}

onErr:
	return result;
}

static int IsBorderless( const char *str )
{
	int result = 0;

	if ( str == NULL ) goto onErr;
	
	if ( strstr( str, ".bl" ) != NULL ) {
		result = 1;
	}

onErr:
	return result;
}



static void InitpSettings( CNCL_P_SETTINGS *pSettings )
{
	if ( pSettings == NULL ) return;

	pSettings->version 			= 0;
	pSettings->papersize		= -1;
	pSettings->mediatype		= -1;
	pSettings->borderlessprint	= -1;
	pSettings->colormode		= -1;
	pSettings->duplexprint		= -1;
}

static int DumpSettings( CNCL_P_SETTINGS *pSettings )
{
	int result = -1;

	DEBUG_PRINT( "[tocanonij] !!!----------------------!!!\n" );
	DEBUG_PRINT2( "[tocanonij] papersize : %d\n", pSettings->papersize );
	DEBUG_PRINT2( "[tocanonij] mediatype : %d\n", pSettings->mediatype );
	DEBUG_PRINT2( "[tocanonij] borderlessprint : %d\n", pSettings->borderlessprint );
	DEBUG_PRINT2( "[tocanonij] colormode : %d\n", pSettings->colormode );
	DEBUG_PRINT2( "[tocanonij] duplexprint : %d\n", pSettings->duplexprint );
	DEBUG_PRINT( "[tocanonij] !!!----------------------!!!\n" );

	result = 0;
	return result;
}

static int CheckSettings( CNCL_P_SETTINGS *pSettings )
{
	int result = -1;

	if ( (pSettings->papersize == -1) ||
		 (pSettings->mediatype == -1) ||
		 (pSettings->borderlessprint == -1) ||
		 (pSettings->colormode == -1) ||
		 (pSettings->duplexprint == -1)  ){
		goto onErr;
	}

	result = 0;
onErr:
	return result;
}


/* define CNCL API */
static int (*GETSETCONFIGURATIONCOMMAND)( CNCL_P_SETTINGSPTR, char *, long ,void *, long, char *, long * );
static int (*GETSENDDATAPWGRASTERCOMMAND)( char *, long, long, char *, long * );
static int (*GETPRINTCOMMAND)( char *, long, long *, char *, long );
static int (*GETSTRINGWITHTAGFROMFILE)( const char* , const char* , int , uint8_t**  );
static int (*GETSETPAGECONFIGUARTIONCOMMAND)( const char* , unsigned short , void * , long, long *  );
static int (*MAKEBJLSETTIMEJOB)( void*, size_t, size_t* );


/* CN_START_JOBID */
#define CN_BUFSIZE				(1024 * 256)
#define CN_START_JOBID			("00000001")
#define CN_START_JOBID_LEN		(9)


int OutputSetTime( int fd, char *jobID )
{
	long bufSize, retSize, writtenSize;
	char *bufTop = NULL;
	int result  = -1;

	/* Allocate Buffer */
	bufSize = sizeof(char) * CN_BUFSIZE;
	if ( (bufTop = malloc( bufSize )) == NULL ) goto onErr;

	/* StartJob1 */
	if ( GETPRINTCOMMAND == NULL ) goto onErr;
	if ( GETPRINTCOMMAND( bufTop, bufSize, &writtenSize, jobID, CNCL_COMMAND_START1 ) != 0 ) {
		fprintf( stderr, "Error in OutputSetTime\n" );
		goto onErr;
	}
	if ( (retSize = write( fd, bufTop, writtenSize )) != writtenSize ) goto onErr;

	/* StartJob2 */
	if ( GETPRINTCOMMAND( bufTop, bufSize, &writtenSize, jobID, CNCL_COMMAND_START2 ) != 0 ) {
		fprintf( stderr, "Error in OutputSetTime\n" );
		goto onErr;
	}
	if ( (retSize = write( fd, bufTop, writtenSize )) != writtenSize ) goto onErr;

	/* SetTime */
	if ( MAKEBJLSETTIMEJOB == NULL ) goto onErr;
	if ( MAKEBJLSETTIMEJOB( bufTop, (size_t)bufSize, (size_t *)&writtenSize ) != 0 ) {
		fprintf( stderr, "Error in OutputSetTime\n" );
		goto onErr;
	}
	if ( (retSize = write( fd, bufTop, writtenSize )) != writtenSize ) goto onErr;

	/* EndJob */
	if ( GETPRINTCOMMAND( bufTop, bufSize, &writtenSize, jobID, CNCL_COMMAND_END ) != 0 ) {
		fprintf( stderr, "Error in OutputSetTime\n" );
		goto onErr;
	}
	if ( (retSize = write( fd, bufTop, writtenSize )) != writtenSize ) goto onErr;

	result = 0;
onErr:
	if ( bufTop != NULL ) {
		free( bufTop );
	}
	return result;
}

int main( int argc, char *argv[] )
{
	int fd = 0;
	int opt, opt_index;
	int result = -1;
	long bufSize = 0; 
	long writtenSize;
	char *bufTop = NULL;
	CNCL_P_SETTINGS Settings;
	char jobID[CN_START_JOBID_LEN];
	char libPathBuf[CN_LIB_PATH_LEN];
	void *libclss = NULL;
	struct option long_opt[] = {
		{ "version", required_argument, NULL, OPT_VERSION }, 
		{ "filterpath", required_argument, NULL, OPT_FILTERPATH }, 
		{ "papersize", required_argument, NULL, OPT_PAPERSIZE }, 
		{ "mediatype", required_argument, NULL, OPT_MEDIATYPE }, 
		{ "grayscale", required_argument, NULL, OPT_COLORMODE }, 
		{ "duplexprint", required_argument, NULL, OPT_DUPLEXPRINT }, 
		{ 0, 0, 0, 0 }, 
	};
	const char *p_ppd_name = getenv("PPD");
	uint8_t *xmlBuf = NULL;
	int xmlBufSize;
	int retSize;

	DEBUG_PRINT( "[tocanonij] start tocanonij\n" );

	/* init CNCL API */
	GETSETCONFIGURATIONCOMMAND = NULL;
	GETSENDDATAPWGRASTERCOMMAND = NULL;
	GETPRINTCOMMAND = NULL;
	GETSTRINGWITHTAGFROMFILE = NULL;
	GETSETPAGECONFIGUARTIONCOMMAND = NULL;
	MAKEBJLSETTIMEJOB = NULL;

	/* Init Settings */
	memset( &Settings, 0x00, sizeof(CNCL_P_SETTINGS) );
	InitpSettings( &Settings );

	while( (opt = getopt_long( argc, argv, "0:", long_opt, &opt_index )) != -1) {
		switch( opt ) {
			case OPT_VERSION:
				DEBUG_PRINT3( "[tocanonij] OPTION(%s):VALUE(%s)\n", long_opt[opt_index].name, optarg );
				break;
			case OPT_FILTERPATH:
				DEBUG_PRINT3( "[tocanonij] OPTION(%s):VALUE(%s)\n", long_opt[opt_index].name, optarg );
				snprintf( libPathBuf, CN_LIB_PATH_LEN, "%s%s", optarg, CN_CNCL_LIBNAME );
				break;
			case OPT_PAPERSIZE:
				DEBUG_PRINT3( "[tocanonij] OPTION(%s):VALUE(%s)\n", long_opt[opt_index].name, optarg );
				Settings.papersize = ConvertStrToID( optarg, papersizeTbl );
				if ( IsBorderless( optarg ) ){
					Settings.borderlessprint = CNCL_PSET_BORDERLESS_ON;
				}
				else {
					Settings.borderlessprint = CNCL_PSET_BORDERLESS_OFF;
				}
				break;
			case OPT_MEDIATYPE:
				DEBUG_PRINT3( "[tocanonij] OPTION(%s):VALUE(%s)\n", long_opt[opt_index].name, optarg );
				Settings.mediatype = ConvertStrToID( optarg, mediatypeTbl );
				DEBUG_PRINT2( "[tocanonij] media : %d\n", Settings.mediatype );
				break;
#if 0
			case OPT_BORDERLESSPRINT:
				DEBUG_PRINT3( "[tocanonij] OPTION(%s):VALUE(%s)\n", long_opt[opt_index].name, optarg );
				if ( IsBorderless( optarg ) ){
					Settings.borderlessprint = CNCL_PSET_BORDERLESS_ON;
				}
				else {
					Settings.borderlessprint = CNCL_PSET_BORDERLESS_OFF;
				}
				break;
#endif
			case OPT_COLORMODE:
				DEBUG_PRINT3( "[tocanonij] OPTION(%s):VALUE(%s)\n", long_opt[opt_index].name, optarg );
				Settings.colormode = ConvertStrToID( optarg, colormodeTbl );
				break;
			case OPT_DUPLEXPRINT:
				DEBUG_PRINT3( "[tocanonij] OPTION(%s):VALUE(%s)\n", long_opt[opt_index].name, optarg );
				//Settings.duplexprint = CNCL_PSET_DUPLEX_OFF;
				Settings.duplexprint = ConvertStrToID( optarg, duplexprintTbl);
				break;
			case '?':
				fprintf( stderr, "Error: invalid option %c:\n", optopt);
				break;
			default:
				break;
		}
	}
	/* dlopen */
	/* Make progamname with path of execute progname. */
	//snprintf( libPathBuf, CN_LIB_PATH_LEN, "%s%s", GetExecProgPath(), CN_CNCL_LIBNAME );
	DEBUG_PRINT2( "[tocanonij] libPath : %s\n", libPathBuf );
	if ( access( libPathBuf, R_OK ) ){
		strncpy( libPathBuf, CN_CNCL_LIBNAME, CN_LIB_PATH_LEN );
	}
	DEBUG_PRINT2( "[tocanonij] libPath : %s\n", libPathBuf );


	libclss = dlopen( libPathBuf, RTLD_LAZY );
	if ( !libclss ) {
		fprintf( stderr, "Error in dlopen\n" );
		goto onErr1;
	}

	GETSETCONFIGURATIONCOMMAND = dlsym( libclss, "CNCL_GetSetConfigurationCommand" );
	if ( dlerror() != NULL ) {
		fprintf( stderr, "Error in CNCL_GetSetConfigurationCommand. API not Found.\n" );
		goto onErr2;
	}
	GETSENDDATAPWGRASTERCOMMAND = dlsym( libclss, "CNCL_GetSendDataPWGRasterCommand" );
	if ( dlerror() != NULL ) {
		fprintf( stderr, "Error in CNCL_GetSendDataPWGRasterCommand\n" );
		goto onErr2;
	}
	GETPRINTCOMMAND = dlsym( libclss, "CNCL_GetPrintCommand" );
	if ( dlerror() != NULL ) {
		fprintf( stderr, "Error in CNCL_GetPrintCommand\n" );
		goto onErr2;
	}
	GETSTRINGWITHTAGFROMFILE = dlsym( libclss, "CNCL_GetStringWithTagFromFile" );
	if ( dlerror() != NULL ) {
		fprintf( stderr, "Error in CNCL_GetStringWithTagFromFile\n" );
		goto onErr2;
	}
	GETSETPAGECONFIGUARTIONCOMMAND = dlsym( libclss, "CNCL_GetSetPageConfigurationCommand" );
	if ( dlerror() != NULL ) {
		fprintf( stderr, "Load Error in CNCL_GetSetPageConfigurationCommand\n" );
		goto onErr2;
	}
	MAKEBJLSETTIMEJOB = dlsym( libclss, "CNCL_MakeBJLSetTimeJob" );
	if ( dlerror() != NULL ) {
		fprintf( stderr, "Load Error in CNCL_MakeBJLSetTimeJob\n" );
		goto onErr2;
	}

	/* Check Settings */
	if ( CheckSettings( &Settings ) != 0 ) goto onErr2;


#if 1
	/* Dump Settings */
	DumpSettings( &Settings );
#endif


	/* Set JobID */
	strncpy( jobID, CN_START_JOBID, sizeof(CN_START_JOBID) );

	/* OutputSetTime */
	if ( OutputSetTime( 1, jobID ) != 0 ) goto onErr2;


	/* Allocate Buffer */
	bufSize = sizeof(char) * CN_BUFSIZE;
	if ( (bufTop = malloc( bufSize )) == NULL ) goto onErr2;

	/* Write StartJob Command */
	if ( GETPRINTCOMMAND( bufTop, bufSize, &writtenSize, jobID, CNCL_COMMAND_START1 ) != 0 ) {
		fprintf( stderr, "Error in CNCL_GetPrintCommand\n" );
		goto onErr2;

	}
	/* WriteData */
	if ( (retSize = write( 1, bufTop, writtenSize )) != writtenSize ) goto onErr2;

	/* Write SetConfiguration Command */
	if ( (xmlBufSize = GETSTRINGWITHTAGFROMFILE( p_ppd_name, CNCL_FILE_TAG_CAPABILITY, CNCL_DECODE_EXEC, &xmlBuf )) < 0 ){
		DEBUG_PRINT2( "[tocanonij] p_ppd_name : %s\n", p_ppd_name );
		DEBUG_PRINT2( "[tocanonij] xmlBufSize : %d\n", xmlBufSize );
		fprintf( stderr, "Error in CNCL_GetStringWithTagFromFile\n" );
		goto onErr3;
	}

	if ( GETSETCONFIGURATIONCOMMAND( &Settings, jobID, bufSize, (void *)xmlBuf, xmlBufSize, bufTop, &writtenSize ) != 0 ){
		fprintf( stderr, "Error in CNCL_GetSetConfigurationCommand\n" );
		goto onErr3;
	}
	/* WriteData */
	retSize = write( 1, bufTop, writtenSize );

	/* Write Page Data */
	while ( 1 ) {
		int readBytes = 0;
		int writeBytes;
		char *pCurrent;
		CNDATA CNData;
		long readSize = 0;
		unsigned short next_page;

		memset( &CNData, 0, sizeof(CNDATA) );

		/* read magic number */
		readBytes = read( fd, &CNData, sizeof(CNDATA) );
		if ( readBytes > 0 ){
			if ( CNData.magic_num != MAGIC_NUMBER_FOR_CNIJPWG ){
				fprintf( stderr, "Error illeagal MagicNumber\n" );
				goto onErr3;
			}
			if ( CNData.image_size < 0 ){
				fprintf( stderr, "Error illeagal dataSize\n" );
				goto onErr3;
			}
		}
		else if ( readBytes < 0 ){
			if ( errno == EINTR ) continue;
			fprintf( stderr, "DEBUG:[tocanonij] tocnij read error, %d\n", errno );
			goto onErr3;
		}
		else {
			DEBUG_PRINT( "DEBUG:[tocanonij] !!!DATA END!!!\n" );
			break; /* data end */
		}

		/* Write Next Page Info */
		if ( CNData.next_page ) {
			next_page = CNCL_PSET_NEXTPAGE_ON;
		}
		else {
			next_page = CNCL_PSET_NEXTPAGE_OFF;
		}
		if ( GETSETPAGECONFIGUARTIONCOMMAND( jobID, next_page, bufTop, bufSize, &writtenSize ) != 0 ) {
			fprintf( stderr, "Error in CNCL_GetPrintCommand\n" );
			goto onErr3;
		}
		/* WriteData */
		if ( (retSize = write( 1, bufTop, writtenSize )) != writtenSize ) goto onErr3;

		/* Write SendData Command */
		memset(	bufTop, 0x00, bufSize );
		readSize = CNData.image_size;
		if ( GETSENDDATAPWGRASTERCOMMAND( jobID, readSize, bufSize, bufTop, &writtenSize ) != 0 ) {
			DEBUG_PRINT( "Error in CNCL_GetSendDataJPEGCommand\n" );
			goto onErr3;
		}
		/* WriteData */
		retSize = write( 1, bufTop, writtenSize );

		while( readSize ){
			pCurrent = bufTop;
	
			if ( readSize - bufSize > 0 ){
				readBytes = read( fd, bufTop, bufSize );
				DEBUG_PRINT2( "[tocanonij] PASS tocanonij READ1<%d>\n", readBytes );
				if ( readBytes < 0 ) {
					if ( errno == EINTR ) continue;
				}
				readSize -= readBytes;
			}
			else {
				readBytes = read( fd, bufTop, readSize );
				DEBUG_PRINT2( "[tocanonij] PASS tocanonij READ2<%d>\n", readBytes );
				if ( readBytes < 0 ) {
					if ( errno == EINTR ) continue;
				}
				readSize -= readBytes;
			}

			do {
				writeBytes = write( 1, pCurrent, readBytes );
				DEBUG_PRINT2( "[tocanonij] PASS tocanonij WRITE<%d>\n", writeBytes );
				if( writeBytes < 0){
					if ( errno == EINTR ) continue;
					goto onErr3;
				}
				readBytes -= writeBytes;
				pCurrent += writeBytes;
			} while( writeBytes > 0 );

		}
	}

	/* CNCL_GetPrintCommand */
	if ( GETPRINTCOMMAND( bufTop, bufSize, &writtenSize, jobID, CNCL_COMMAND_END ) != 0 ) {
		DEBUG_PRINT( "Error in CNCL_GetPrintCommand\n" );
		goto onErr3;

	}
	/* WriteData */
	retSize = write( 1, bufTop, writtenSize );
	DEBUG_PRINT( "[tocanonij] to_cnijf <end>\n" );

	result = 0;
onErr3:
	if ( xmlBuf != NULL ){
		free( xmlBuf );
	}

onErr2:
	if ( bufTop != NULL ){
		free( bufTop );
	}
	
onErr1:
	if ( libclss != NULL ) {
		dlclose( libclss );
	}
	return result;
}


