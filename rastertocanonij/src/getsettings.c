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
 */

#include <stdio.h>
#include <cups/cups.h>
#include <cups/ppd.h>
#include "paramlist.h"
#include "canonopt.h"

#include "common.h"
#include "com_def.h"

#define BUF_SIZE 256

int GetPrinterSettings( cups_option_t *p_cups_opt, int num_opt, ParamList **p_list, int *listNum )
{
	char *p_ppd_name = getenv("PPD");
	ppd_file_t *p_ppd;
	ppd_choice_t *p_choice;
	int result = -1;
	PpdToOptKey *p_opt_key_table = alloc_opt_key_table(p_ppd_name);
	PpdToOptKey *p_table = p_opt_key_table;
	int numCnt = 0;
	char choice[BUF_SIZE];
	ParamList *local_p_list = NULL;
	static const char *libpath;

	if ( (p_cups_opt == NULL) || (listNum == NULL) ) goto onErr;
	*listNum = 0;
	

	DEBUG_PRINT2( "DEBUG:[rastertocanonij] PPD Path(%s)\n", p_ppd_name );
	if ( (p_ppd = ppdOpenFile( p_ppd_name )) == NULL ) goto onErr;

	ppdMarkDefaults(p_ppd);
	cupsMarkOptions(p_ppd, num_opt, p_cups_opt);

	/* add libpath */
	libpath = GetExecProgPath();
	if ( libpath != NULL ){
		param_list_add( &local_p_list, "--filterpath", libpath, strlen(libpath) + 1 );
	}

	while( p_table->ppd_key != NULL ){
		p_choice = ppdFindMarkedChoice( p_ppd, p_table->ppd_key );
		if ( p_choice ) {
			DEBUG_PRINT3( "DEBUG:[rastertocanonij] OPTION(%s) / VALUE(%s)\n", p_table->ppd_key, p_choice->choice );
		}
		else {
			DEBUG_PRINT2( "DEBUG:[rastertocanonij] OPTION(%s) / VALUE(novalue)\n", p_table->ppd_key );
		}

		if ( p_choice != NULL ){
			//strcpy( choice, p_choice->choice );
			strncpy( choice, p_choice->choice, BUF_SIZE );
			choice[BUF_SIZE -1] = '\0';
			//to_lower_except_size_X(choice);
			param_list_add( &local_p_list, p_table->opt_key, choice, strlen(choice) + 1 );
			numCnt++;
		}
		p_table++;
	}

	*listNum = numCnt;
	*p_list = local_p_list;
	result = 0;
onErr:
	return result;
}


int GetPrinterableAreaOptionFromPPD( const char *size_str, ParamList **p_list, int *listNum )
{
	SizeToPrintArea *table_top = NULL;
	long width, height;
	int result = -1;
	char num_buf[BUF_SIZE];
	ParamList *local_p_list = NULL;
	int numCnt = 0;

	if ( (size_str == NULL) || (listNum == NULL) ) goto onErr;

	width = 0;
	height = 0;
	table_top = alloc_size_to_print_area_table( getenv("PPD") );
	if ( size_to_print_area_table( table_top, (char *)size_str, &width, &height ) < 0 ){
		fprintf( stderr, "DEBUG:[rastertocanonij] size_to_print_area_table in Error\n" );
		goto onErr;
	}

	memset( num_buf,  0x00,  sizeof(num_buf) );
	snprintf( num_buf, BUF_SIZE, "%ld", width );
	param_list_add( &local_p_list, "--printable_width", num_buf, strlen(num_buf) + 1 );
	numCnt++;

	memset( num_buf,  0x00, sizeof(num_buf) );
	snprintf( num_buf, BUF_SIZE, "%ld", height );
	param_list_add( &local_p_list, "--printable_height", num_buf, strlen(num_buf) + 1 );
	numCnt++;

	*listNum = numCnt;
	*p_list = local_p_list;
	result = 0;
onErr:
	free_size_to_print_area_table( table_top );
	return result;
}

