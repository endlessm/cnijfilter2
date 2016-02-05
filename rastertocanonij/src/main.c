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

#if HAVE_CONFIG_H
#include    <config.h>
#endif  // HAVE_CONFIG_H

#include <cups/cups.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include "paramlist.h"
#include "common.h"
#include "com_def.h"
#include "canonopt.h"
#include "getsettings.h"

#define	DATA_BUF_SIZE	(1024 * 256)
#define SHELL_PATH     "/bin"
#define SHELL_NAME     "sh"

#define CMD_BUF_SIZE (1024*3)

#define TOPWG_PATH PROG_PATH
//#define TOPWG_PATH  "/usr/local/bin/tocnpwg"
#define TOPWG_BIN	"tocnpwg"

#define TOJPEG_PATH  PROG_PATH
//#define TOJPEG_PATH  "/usr/local/bin/tocnjpeg"
#define TOJPEG_BIN	 "tocnjpeg"

#define TOCNIJ_PATH	PROG_PATH
//#define TOCNIJ_PATH	"/usr/local/bin"
#define TOCNIJ_BIN	"tocanonij"
//#define TOCNIJ_PATH	"/usr/local/bin/tocanonij"

#define CUSTOM_PAGESIZE_STR "PageSize=Custom"

int g_filter_pid = -1;
int g_signal_received = 0;

static void sigterm_handler( int sigcode )
{
	if ( g_filter_pid != -1 ){
		kill( -g_filter_pid, SIGTERM );
	}
	g_signal_received = 1;
}

static int exec_filter(char *cmd_buf, int ofd, int fds[2])
{
	int status = 0;
	int	child_pid = -1;
	char *filter_param[4];
	char shell_buf[256];
	int size;

	if( pipe(fds) >= 0 )
	{
		child_pid = fork();

		if( child_pid == 0 )
		{

			setpgid(0, 0);
			
			close(0);
			dup2(fds[0], 0);
			close(fds[0]);
			close(fds[1]);

			if( ofd != 1 )
			{
				close(1);
				dup2(ofd, 1);
				close(ofd);
			}

			{
				size = 256;
				strncpy(shell_buf, SHELL_PATH, size); size -= strlen(SHELL_PATH);
				strncat(shell_buf, "/", size); size -= 1;
				strncat(shell_buf, SHELL_NAME, size);

				filter_param[0] = shell_buf;
				filter_param[1] = "-c";
				filter_param[2] = cmd_buf;
				filter_param[3] = NULL;

				execv(shell_buf, filter_param);
						
				fprintf(stderr, "execl() error\n");
				status = -1;
			}
		}
		else if( child_pid != -1 )
		{
			close(fds[0]);
		}
	}
	return child_pid;
}

static int GetOptionBufSize( ParamList *p_list )
{
	int result = -1;
	int key_len, val_len;
	int total_len = 0;

	if ( p_list == NULL ) goto onErr;

	/* topwg */
	total_len += strlen( TOPWG_PATH ) + 1 + strlen( TOPWG_BIN ) + 1;

	/* pipe */
	total_len += 2;

	/* tocnij */
	total_len += strlen( TOCNIJ_PATH ) + 1 + strlen( TOCNIJ_BIN ) + 1;
	while( p_list != NULL ) {
		key_len = strlen( p_list->key );
		val_len = strlen( p_list->value );
	
		DEBUG_PRINT2( "DEBUG:[rastertocanonij] key : %s\n", p_list->key );
		DEBUG_PRINT2( "DEBUG:[rastertocanonij] val : %s\n", p_list->value );

		total_len += (key_len + 1 + val_len + 1);
		p_list = p_list->next;
	}
	/* teminate */
	total_len += 1;

	/* toriaezu */
	total_len *= 2;

	result = total_len;
	DEBUG_PRINT2( "DEBUG:[rastertocanonij] total_len  %d\n", total_len );
onErr:
	return result;
}

static int MakeExecCommand( char *cmd_buf, int cmd_buf_size, ParamList *p_list, ParamList *p_list2 )
{
	int result = -1;
	int i;
	char tmp_buf[CMD_BUF_SIZE];
	ParamList *list_array[2];
	ParamList *p_cur;

	/* Set tocnpwg path */
	snprintf( tmp_buf, CMD_BUF_SIZE, "%s%s", GetExecProgPath(), TOPWG_BIN );
	if ( access( tmp_buf, R_OK ) ){
		snprintf( tmp_buf, CMD_BUF_SIZE, "%s/%s", TOPWG_PATH, TOPWG_BIN );
		if ( access( tmp_buf, R_OK ) ){
			fprintf( stderr, "Not found tocnpwg\n" );
			goto onErr;
		}
	}

	strncpy( cmd_buf, tmp_buf, cmd_buf_size );
	cmd_buf[cmd_buf_size -1] = '\0';
	//strcpy( cmd_buf, tmp_buf );

	/* Set tocnpwg option */
	list_array[0] = p_list;
	list_array[1] = p_list2;
	for ( i=0; i<2; i++ ){	
		p_cur = list_array[i];
		while ( p_cur != NULL ) {
			strcat( cmd_buf, " " );
			strcat( cmd_buf, p_cur->key );
			strcat( cmd_buf, " " );
			strcat( cmd_buf, p_cur->value );

			p_cur = p_cur->next;
		}
	}

	/* Set tocnij path */
	snprintf( tmp_buf, CMD_BUF_SIZE, "%s%s", GetExecProgPath(), TOCNIJ_BIN );
	if ( access( tmp_buf, R_OK ) ){
		snprintf( tmp_buf, CMD_BUF_SIZE, "%s/%s", TOCNIJ_PATH, TOCNIJ_BIN );
		if ( access( tmp_buf, R_OK ) ){
			fprintf( stderr, "Not exist tocanonij\n" );
			goto onErr;
		}
	}

	strcat( cmd_buf, " | " );
	strcat( cmd_buf, tmp_buf );

	/* Set tocnij option */
	p_cur = list_array[0];
	while ( p_cur != NULL ) {
		strcat( cmd_buf, " " );
		strcat( cmd_buf, p_cur->key );
		strcat( cmd_buf, " " );
		strcat( cmd_buf, p_cur->value );

		p_cur = p_cur->next;
	}

	DEBUG_PRINT2( "DEBUG:[rastertocanonij] cmd_buf%s",cmd_buf );

	result = 0;
onErr:
	return result;
}

int main(int argc, char *argv[])
{
	cups_option_t *p_cups_opt = NULL;
	ParamList *p_list = NULL;
	ParamList *p_list2 = NULL;
	char *p_data_buf = NULL;
	char *cmd_buf = NULL;
	int cmd_buf_size = 0;
	struct sigaction sigact;
	int num_opt = 0,out_num = 0;
	int ifd = 0;
	int fds[2];
	int result = -1;

#if defined(HAVE_SIGACTION) && !defined(HAVE_SIGSET)
	struct sigaction action;
#endif

#ifdef HAVE_SIGSET
	sigset(SIGPIPE, SIG_IGN);
#elif defined(HAVE_SIGACTION)
	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, NULL);
#else
	signal(SIGPIPE, SIG_IGN);
#endif /* HAVE_SIGSET */
	/* register signal */
	memset(&sigact, 0, sizeof(sigact));
	sigact.sa_handler = sigterm_handler;

	if( sigaction(SIGTERM, &sigact, NULL) )
	{
		fputs("DEBUG:[rastertocanonij] pstocanonij can't register signal hander.\n", stderr);
		goto onErr1;
	}

	if( argc < 6 || argc > 7 ) {
		fputs("DEBUG:[rastertocanonij] pstocanonij illegal parameter number.\n", stderr);
		goto onErr1;
	}

	if( argv[5] != NULL ) {
		if ( strstr( argv[5], CUSTOM_PAGESIZE_STR ) == NULL ){
			num_opt = cupsParseOptions(argv[5], 0, &p_cups_opt);
			if( num_opt < 0 ) {
				fputs("DEBUG:[rastertocanonij] illegal option.\n", stderr);
				goto onErr1;
			}
		}
		else {
			/* If PageSize is CustomSize, exit as error. */
			goto onErr1;
		}
	}

	if( g_signal_received ) goto onErr1;


	/* Allocate buffer to communicate */
	if( (p_data_buf = (char*)malloc(DATA_BUF_SIZE)) == NULL ) goto onErr1;

	/* Parse Option, and get option list for tocanonij filter */
	if ( GetPrinterSettings( p_cups_opt, num_opt, &p_list, &out_num ) != 0 ){
		fprintf( stderr, "DEBUG:Error in GetPrinterSettings\n" );
		goto onErr2;
	}

	/* Parse PPD, and get option list for tocnpwg filter */
	if ( GetPrinterableAreaOptionFromPPD( ref_value_from_list(p_list, "--papersize"), &p_list2, &out_num ) != 0 ){
		fprintf( stderr, "DEBUG:Error in GetPrinterableAreaOptionFromPPD\n" );
		goto onErr3;
	}

	DEBUG_PRINT( "DEBUG:[rastertocanonij] rastertocanonij main <0>\n" );
	/* Get BufSize */
	if ( (cmd_buf_size = GetOptionBufSize( p_list )) < 0 ){
		fprintf( stderr, "DEBUG:[rastertocanonij] GetOptionBufSize in Error\n" );
		goto onErr4;
	}

	/* Allocate Command Buffer */
	cmd_buf_size = CMD_BUF_SIZE;
	if ( (cmd_buf = (char *)malloc( cmd_buf_size )) == NULL ) goto onErr4;

	DEBUG_PRINT( "DEBUG:[rastertocanonij] rastertocanonij main <1>\n" );
	/* Make execute command */
	if ( MakeExecCommand( cmd_buf, cmd_buf_size, p_list, p_list2 ) != 0 ){
		fprintf( stderr, "DEBUG:[rastertocanonij] MakeExecCommand in Error\n" );
		goto onErr5;
	}
	
	DEBUG_PRINT( "DEBUG:[rastertocanonij] rastertocanonij main <2>\n" );
	/*	set cmd_buf here .....*/
	DEBUG_PRINT2( "DEBUG:[rastertocanonij] cmd_buf : %s\n", cmd_buf );
	if ( (g_filter_pid = exec_filter(cmd_buf, 1, fds)) == -1 ){
		fprintf( stderr, "DEBUG:[rastertocanonij] exec_filter in Error\n" );
		goto onErr5;
	}

	DEBUG_PRINT( "DEBUG:[rastertocanonij] rastertocanonij main <3>\n" );
	/* WriteData */
	while( !g_signal_received ) {
		int read_bytes = read(ifd, p_data_buf, DATA_BUF_SIZE);
		if ( read_bytes > 0 ) {
			int write_bytes;
			char *p_data = p_data_buf;
	
			do {
				write_bytes = write(fds[1], p_data, read_bytes);
				if( write_bytes < 0 ) {
					if( errno == EINTR ) continue;
					fprintf(stderr, "DEBUG:[rastertocanonij] pstocanonij write error,%d.\n", errno);
					goto onErr6;
				}
				read_bytes -= write_bytes;
				p_data += write_bytes;
			} while( !g_signal_received && read_bytes > 0 );
		}
		else if( read_bytes < 0 ) {
			if( errno == EINTR ) continue;
			fprintf(stderr, "DEBUG:[rastertocanonij] pstocanonij read error,%d.\n", errno);
			goto onErr6;
		}
		else {
			DEBUG_PRINT( "DEBUG:[rastertocanonij] rastertocanonij main <break>\n" );
			break;
		}
	}

	DEBUG_PRINT( "DEBUG:[rastertocanonij] rastertocanonij main <4>\n" );
	result = 0;
onErr6:
	close( fds[1] );

	/* wait process */	
	if ( g_filter_pid != -1 ){
		waitpid( g_filter_pid, NULL, 0 );
	}	
onErr5:
	if( cmd_buf != NULL ){
		free( cmd_buf );
	}
onErr4:
	param_list_free( p_list2 );
onErr3:
	param_list_free( p_list );
onErr2:
	if( p_data_buf != NULL ){
		free( p_data_buf );
	}
onErr1:
	return result;
}
