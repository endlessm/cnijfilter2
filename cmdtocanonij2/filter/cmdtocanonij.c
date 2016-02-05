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

//#include <cups/cups.h>
//#include <cups/ppd.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

//#include "./cnclinc/cncl.h"
#include "./cnclinc/cncldef.h"
#include "./cnclinc/cnclcmdutils.h"

#if HAVE_CONFIG_H
#include <config.h>
#endif

#define SAFE_FREE(p)	{if((p)!=NULL)free(p);(p)=NULL;}

#define	DATA_BUF_SIZE	(1024)
#define	PARAM_BUF_SIZE	(256)

#define IS_RETURN(c)	(c == '\r' || c == '\n')
#define COUNTOF(list, type)		(sizeof(list)/sizeof(type))

#define PROC_FAILED		(1)
#define PROC_SUCCEEDED	(0)

#define CN_START_JOBID			("00000001")
#define CN_START_JOBID_LEN (9)

int g_signal_received = 0;

//////////////////////////////////////////////////////////////////////////////


static int read_line(int fd, char *p_buf, int bytes)
{
	static char read_buf[DATA_BUF_SIZE];
	static int buf_bytes = 0;
	static int buf_pos = 0;
	int read_bytes = 0;
	
	memset(p_buf, 0, bytes);
	while( bytes > 0 )
	{
		if( buf_bytes == 0 && fd != -1 )
		{
			buf_bytes = read(fd, read_buf, DATA_BUF_SIZE);
			if( buf_bytes > 0 ) buf_pos = 0;
		}

		if( buf_bytes > 0 )
		{
			*p_buf = read_buf[buf_pos++];
			bytes--;
			buf_bytes--;
			read_bytes++;

			if( IS_RETURN(*p_buf) ) break;
			p_buf++;
		}
		else if( buf_bytes < 0 )
		{
			if( errno == EINTR ) continue;
		}
		else
			break;
	}

	return read_bytes; 
}


static int get_printer_command(const char *cups_command, size_t command_sz, char *canon_command, size_t buffer_sz)
{
	int ret = PROC_FAILED;
	
	FILE *fp=NULL;
	if ((fp = fopen(getenv("PPD"), "r")) == NULL)
	{
		fprintf(stderr, "DEBUG: [cmdtocanonij] cannot open ppd file : %s\n", getenv("PPD"));
		return PROC_FAILED;
	}
	
	char c[3];
	char line_buffer[DATA_BUF_SIZE];
	char line_ppd_option[PARAM_BUF_SIZE];
	char line_cups_cmd[PARAM_BUF_SIZE];
	char line_canon_cmd[PARAM_BUF_SIZE];
	
	while (feof(fp) == 0){
		memset(line_buffer, 0, sizeof(line_buffer));
		memset(line_ppd_option, 0, sizeof(line_ppd_option));
		memset(line_cups_cmd, 0, sizeof(line_cups_cmd));
		memset(line_canon_cmd, 0, sizeof(line_canon_cmd));
		
		fgets(&line_buffer[0], sizeof(line_buffer)-1, fp);
		if (sscanf(line_buffer, "%c%c%255[0-9a-zA-Z]:%1[ ]%255[0-9a-zA-Z.@_]=\"%255[0-9a-zA-Z.@=_]\"", &c[0], &c[1], &line_ppd_option[0], &c[3], &line_cups_cmd[0], &line_canon_cmd[0]) > 2)
		{
			if (c[0]!='*' || c[1]!='%') continue;
			if (strncmp(line_ppd_option, "CNMaintenanceCommand", PARAM_BUF_SIZE) != 0) continue;
			
			int i=0;
			for (i=0; line_cups_cmd[i]!='\0'; i++) if(line_cups_cmd[i]=='_')line_cups_cmd[i]=' ';
			
			
			//fprintf(stderr, "DEBUG: [cmdtocanonij] cups_command:  [%s]\n", cups_command);
			//fprintf(stderr, "DEBUG: [cmdtocanonij] line_cups_cmd: [%s]\n", line_cups_cmd);
			
			line_cups_cmd[PARAM_BUF_SIZE-1] = '\0';
			size_t cmdSize = strlen(line_cups_cmd);

			if (strncmp(cups_command, line_cups_cmd, cmdSize) == 0)
			{
				//fprintf(stderr, "DEBUG: [cmdtocanonij] command found: [%s]->[%s]\n", line_cups_cmd, line_canon_cmd);
		
				memset(canon_command, 0, sizeof(buffer_sz));
				strncpy(canon_command, line_canon_cmd, buffer_sz);
				ret = PROC_SUCCEEDED;
				goto err;
			}
		}
	}

err:
	fclose(fp);
	return ret;
}


static int write_buffer(const uint8_t *buffer, const size_t size)
{
	size_t writtenSize = 0;
	for (writtenSize = 0; writtenSize < size; )
	{
		size_t w = write(1, &buffer[writtenSize], size-writtenSize);
		if (w < 0) return PROC_FAILED;
		writtenSize += w;
	}
	fsync(1);
	return PROC_SUCCEEDED;
}

static int write_command(const char *command)
{
	char jobID[CN_START_JOBID_LEN];
	long writtenSize;
	strncpy( jobID, CN_START_JOBID, sizeof(CN_START_JOBID) );

	if (command != NULL)
	{
		// StartJob
		{
			uint8_t command_buffer[1024];
#if 0
			/* poweron */
			memset(command_buffer, 0x00, sizeof(command_buffer));
			if (CNCL_MakePrintCommand(CNCL_COMMAND_POWERON, command_buffer, sizeof(command_buffer), NULL, "0") != CNCL_OK) return PROC_FAILED;
			if (write_buffer(command_buffer, strlen(command_buffer)) != PROC_SUCCEEDED) return PROC_FAILED;
#endif	
			/* start1 */
			memset(command_buffer, 0x00, sizeof(command_buffer));
			if (CNCL_GetPrintCommand((char*)command_buffer, sizeof(command_buffer), &writtenSize, jobID, CNCL_COMMAND_START1 ) != CNCL_OK) return PROC_FAILED;
			if (write_buffer(command_buffer, writtenSize) != PROC_SUCCEEDED) return PROC_FAILED;
		
			/* start2 */
			memset(command_buffer, 0x00, sizeof(command_buffer));
			if (CNCL_GetPrintCommand((char*)command_buffer, sizeof(command_buffer), &writtenSize, jobID, CNCL_COMMAND_START2 ) != CNCL_OK) return PROC_FAILED;
			if (write_buffer(command_buffer, writtenSize) != PROC_SUCCEEDED) return PROC_FAILED;
		}
	
		// SetTime
		{
			uint8_t setTimeBuffer[256];
			size_t setTimeBufSize = 0;
			memset(setTimeBuffer, 0, sizeof(setTimeBuffer));
			//if (CNCL_MakeBJLSetTimeJob(setTimeBuffer, sizeof(setTimeBuffer), &setTimeBufSize) != CNCL_OK) return PROC_FAILED;
			if (write_buffer(setTimeBuffer, setTimeBufSize) != PROC_SUCCEEDED) return PROC_FAILED;
		}
		
		// Maintenance Job
		{
			uint8_t start_cmd[] = {0x1B,0x5B,0x4B,0x02,0x00,0x00,0x1F,'B','J','L','S','T','A','R','T',0x0A};
			if (write_buffer(start_cmd, sizeof(start_cmd)) != PROC_SUCCEEDED) return PROC_FAILED;
		
			size_t cmdSize = strlen(command);
			//fprintf(stderr, "DEBUG: [cmdtocanonij] cmdSize = %d, Command=[%s]\n", cmdSize, command);
			if (write_buffer((uint8_t *)command, cmdSize) != PROC_SUCCEEDED) return PROC_FAILED;
		
			uint8_t end_cmd[] = {0x0A,'B','J','L','E','N','D',0x0A};
			if (write_buffer(end_cmd, sizeof(end_cmd)) != PROC_SUCCEEDED) return PROC_FAILED;
		}
		
		// EndJob
		{
			uint8_t command_buffer[1024];
			memset(command_buffer, 0x00, sizeof(command_buffer));
			if (CNCL_GetPrintCommand((char*)command_buffer, sizeof(command_buffer), &writtenSize, jobID, CNCL_COMMAND_END ) != CNCL_OK) return PROC_FAILED;
			if (write_buffer(command_buffer, writtenSize) != PROC_SUCCEEDED) return PROC_FAILED;
		}
	}
	return PROC_SUCCEEDED;
}


static int send_maintenance_command(int ifd)
{
	char read_buf[DATA_BUF_SIZE];
	int read_bytes;
	
	memset(read_buf, 0, sizeof(read_buf));
	
	int lines = 0;
	for (lines=0; (read_bytes = read_line(ifd, read_buf, DATA_BUF_SIZE - 1)) > 0; lines++ )
	{
		// check cups command file suffix.
		if (lines == 0) 
		{
			if (strncmp(read_buf, "#CUPS-COMMAND", read_bytes-1) != 0)
			{
				fprintf(stderr, "DEBUG: [cmdtocanonij] command file data are invalid.\n");
				return PROC_FAILED;
			}
			else
				continue;
		}
		
		// get device command
		char mnt_command[PARAM_BUF_SIZE];
		if (get_printer_command(&read_buf[0], read_bytes, &mnt_command[0], PARAM_BUF_SIZE) != PROC_SUCCEEDED)
		{
			fprintf(stderr, "DEBUG: [cmdtocanonij] get_printer_command failed.\n");
			return PROC_FAILED;
		}
		
		// perform only first command.
		return write_command(mnt_command);
	}
	
	fprintf(stderr, "DEBUG: [cmdtocanonij] cannot detect command.\n");
	return PROC_FAILED;
}

static void sigterm_handler(int sigcode)
{
	g_signal_received = 1;
}

int main(int argc, char *argv[])
{
	int retVal = 0;
	
	int ifd = 0;
	struct sigaction sigact;

	setbuf(stderr, NULL);
	fprintf(stderr, "DEBUG: [cmdtocanonij] started. \n");

	memset(&sigact, 0, sizeof(sigact));
	sigact.sa_handler = sigterm_handler;

	if( sigaction(SIGTERM, &sigact, NULL) )
	{
		fputs("ERROR: [cmdtocanonij] can not register signal hander.\n", stderr);
		retVal = 1;
		goto error_exit;
	}

	if( argc < 6 || argc > 7 )
	{
		fputs("ERROR: [cmdtocanonij] illegal parameter number.\n", stderr);
		retVal = 1;
		goto error_exit;
	}
	
	if( argc == 7 )
	{
		if( (ifd = open(argv[6], O_RDONLY)) == -1 )
		{
			fputs("ERROR: [cmdtocanonij] cannot open file.\n", stderr);
			retVal = 1;
			goto error_exit;
		}
	}
	
	// send command from command file.
	retVal = send_maintenance_command(ifd);
	
error_exit:
	fprintf(stderr, "DEBUG: [cmdtocanonij] exited with code %d.\n", retVal);
	return 0;
}

