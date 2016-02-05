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
 */

//
//  '2004.05.25
//  Using the original "usb.c" code of the CUPS 1.1.14 to create a new
//  backend for writing the printing data into a named pipe.
//

/*
 *
 *   USB port backend for the Common UNIX Printing System (CUPS).
 *
 *   Copyright 1997-2002 by Easy Software Products, all rights reserved.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "LICENSE" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: CUPS Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9603
 *       EMail: cups-info@cups.org
 *         WWW: http://www.cups.org
 *
 * Contents:
 *
 *   main()         - Send a file to the specified USB port.
 *   list_devices() - List all USB devices.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#include <cups/cups.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include "cnijbe.h"

#if defined(WIN32) || defined(__EMX__)
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif /* WIN32 || __EMX__ */

#define CNIJ_LGMON2_PATH	PROG_PATH "/cnijlgmon3"
//#define _DEBUG_MODE_

pid_t pid = -1;

/*
 * 'main()' - Send a file to the specified USB port.
 *
 * Usage:
 *
 *    printer-uri job-id user title copies options [file]
 */

int			/* O - Exit status */
main(int  argc,		/* I - Number of command-line arguments (6 or 7) */
     char *argv[])	/* I - Command-line arguments */
{
	//FILE *fp;
	//char	resource[1024],			/* Resource info (device and options) */
	//		*options;				/* Pointer to options 				*/
	char copies[2];					/* Number of copies to print 		*/
	char printer_uri[64];
	int status = 0;
	//char discover[2] = "0";

	CNIF_SetSignal(SIGTERM);
 /*
  * Make sure status messages are not buffered...
  */

	setbuf(stderr, NULL);
	memset(printer_uri, 0, sizeof(printer_uri));
	/*
	* Check command-line...
	*/
	if (argc == 1) {
		//strncpy(discover, "1", sizeof(discover));
	} else if (argc < 6 || argc > 7){
		fputs("ERROR: Illegal argument.\n", stderr);
		fputs("Usage: cnijbe\n", stderr);
		return (1);
	} else if(strstr(argv[0], "port") == NULL) {
		fputs("ERROR: Illegal argument.\n", stderr);
		fputs("Usage: cnijbe\n", stderr);
		return (1);
	}
	
	strncpy(printer_uri, argv[0], sizeof(printer_uri));

	/*
	* If we have 7 arguments, print the file named on the command-line.
	* Otherwise, send stdin instead...
	*/

	if (argc == 6) {
		strncpy(copies, "1", sizeof(copies));
	} else if (argc == 7) {
		/*
		* Try to open the print file...
		*/

		if ((stdin = fopen(argv[6], "rb")) == NULL)
		{
#ifdef _DEBUG_MODE_
			perror("ERROR: unable to open print file\n");
#endif
			return (1);
		}

		strncpy(copies, argv[4], sizeof(copies));
	}
	
	int err = 0;
	/* Divid Process */
	pid = fork();
	
	if (pid < 0){
#ifdef _DEBUG_MODE_
		fprintf(stderr, "ERROR: cnjibe fork error\n");
#endif
		exit(1);
	}
	
	if (pid > 0){
		close(STDIN_FILENO);
		waitpid(pid, &status, 0);
#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: -------------Write process end\n");
#endif
		if (WIFEXITED(status)){
			err = 0;
			err = WEXITSTATUS(status);
			if (err == 0) {
				
#ifdef _DEBUG_MODE_
				fprintf(stderr, "DEBUG: cnijlgmon3 normal exit\n");
#endif
			} else {
#ifdef _DEBUG_MODE_
				fprintf(stderr, "DEBUG: cnijlgmon3 exit error.(%d)\n", err);
#endif
			}
		} else if (WIFSIGNALED(status)){
#ifdef _DEBUG_MODE_
			fprintf(stderr, "DEBUG: cnijlgmon3 signal, sig = %d\n", WTERMSIG(status));
#endif
		} else {
#ifdef _DEBUG_MODE_
			fprintf(stderr, "DEBUG: cnijlgmon3 other err\n");
#endif
		}
	} else {
#ifdef _DEBUG_MODE_
		fprintf(stderr, "DEBUG: Write process start--------------\n");
#endif
		if (argc == 1) {
			execl(CNIJ_LGMON2_PATH, "cnijlgmon3", NULL);
		} else {
			execl(CNIJ_LGMON2_PATH, "cnijlgmon3", printer_uri, copies, NULL);
		}
	}
	
	return 0;

}

/* set signal */
void CNIF_SetSignal(int sign)
{
	
#if defined(HAVE_SIGACTION) && !defined(HAVE_SIGSET)
    struct  sigaction action;			/* Actions for POSIX signals        */
#endif /* HAVE_SIGACTION && !HAVE_SIGSET */

#ifdef HAVE_SIGSET
	//sigset
	sigset(SIGPIPE, SIG_IGN);
	sigset(sign, CNIF_SigCatch);
	
#elif defined(HAVE_SIGACTION)
	//sigaction
	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, NULL);
	
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGTERM);
	
	action.sa_handler = CNIF_SigCatch;
	sigaction(sign, &action, NULL);
	
	//signal
#else
	signal(SIGPIPE, SIG_IGN);
	if (SIG_ERR == signal(sign, CNIF_SigCatch)) {        
		fprintf(stderr, "DEBUG: failed to set signal handler\n"); 
		exit(1); 
	}
#endif

}

//signal catch
void CNIF_SigCatch(int sign)
{
	fprintf(stderr, "DEBUG: cnijbe_process catch the interrupt signal %d\n", sign);//DEBUG_MSG
	if( pid > 0 ) {
		kill(pid, SIGTERM);
	}
}

