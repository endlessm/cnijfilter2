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

#include <cups/cups.h>
#include <cups/ppd.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


#include "canonopt.h"
#include "com_def.h"


char *ppd_to_opt_key(PpdToOptKey *p_table, char *ppd_key)
{
	while( p_table->ppd_key != NULL )
	{
		if( !strcmp(p_table->ppd_key, ppd_key) )
			return p_table->opt_key;

		p_table++;
	}
	return NULL;
}

static
int read_line(FILE *fp, char *buf, int buf_size)
{
	int cc = EOF;
	int i = 0;

	if( buf != NULL && buf_size > 0 )
	{
		while( (cc = fgetc(fp)) != EOF )
		{
			if( cc == 0x0d || cc == 0x0a )
				break;
			if( i < buf_size - 1 )
				buf[i++] = cc;
		}
		buf[i] = 0;
	}

	return (cc == EOF)? -1 : i;
}


#define	LINE_BUF_SIZE	1024


PpdToOptKey *alloc_opt_key_table(char *ppd_name)
{
	FILE *fp = fopen(ppd_name, "r");
	PpdToOptKey *p_key_table = NULL;
	PpdToOptKey *p_table;
	char line_buf[LINE_BUF_SIZE];

	if( fp != NULL )
	{
		int opt_num = 0;

		while( read_line(fp, line_buf, LINE_BUF_SIZE) >= 0 )
		{
			if( line_buf[0] == '*' && line_buf[1] == '%' )
			{
				if( !strcmp(strtok(line_buf, " "), "*%CNPpdToOptKey") )
					opt_num++;
			}
		}

		p_key_table = p_table = (PpdToOptKey*)malloc(sizeof(PpdToOptKey) * (opt_num + 1));

		if( p_key_table != NULL )
		{
			rewind(fp);

			while( read_line(fp, line_buf, LINE_BUF_SIZE) >= 0 )
			{
				if( line_buf[0] == '*' && line_buf[1] == '%' )
				{
					if( !strcmp(strtok(line_buf, " "), "*%CNPpdToOptKey") )
					{
						char *ppd_key = strtok(NULL, " ");
						char *opt_key= strtok(NULL, " ");

						if( ppd_key != NULL && opt_key != NULL )
						{
							int ppd_len = strlen(ppd_key) + 1;
							int opt_len = strlen(opt_key) + 1;
							int total_size = ppd_len + opt_len;

							p_table->ppd_key = (char*)malloc(total_size);

							if( p_table->ppd_key != NULL )
							{
								p_table->opt_key = p_table->ppd_key + ppd_len;

 								strncpy(p_table->ppd_key, ppd_key, ppd_len);
								strncpy(p_table->opt_key, opt_key, opt_len);
								p_table->ppd_key[total_size -1] = '\0';
								p_table++;
							}
						}
					}
				}
			}
 			p_table->ppd_key = NULL;
			p_table->opt_key = NULL;
		}
		fclose(fp);
	}
	return p_key_table;
}

void free_opt_key_table(PpdToOptKey *p_opt_key_table)
{
	if( p_opt_key_table != NULL )
	{
		PpdToOptKey *p_table = p_opt_key_table;

		while( p_table->ppd_key != NULL )
		{
			free(p_table->ppd_key);
			p_table++;
		}
		free(p_opt_key_table);
	}
}


int size_to_print_area_table(SizeToPrintArea *p_table_top, char *size_key, long *w, long *h)
{
	SizeToPrintArea *current = p_table_top;
	int result = -1;

	if ( (p_table_top == NULL) || (size_key == NULL) || (w == NULL) || (h == NULL)) goto onErr;

	while( current->size_key != NULL )
	{
		if( !strcmp(current->size_key, size_key) ){
			*w = current->width;
			*h = current->height;
			break;
		}
		current++;
	}

	result = 0;
onErr:
	return result;
}


SizeToPrintArea *alloc_size_to_print_area_table(char *ppd_name)
{
	FILE *fp = fopen(ppd_name, "r");
	SizeToPrintArea *p_table_top = NULL;
	SizeToPrintArea *p_table;
	long mem_size = 0;
	char line_buf[LINE_BUF_SIZE];

	if( fp != NULL )
	{
		int opt_num = 0;

		while( read_line(fp, line_buf, LINE_BUF_SIZE) >= 0 )
		{
			if( line_buf[0] == '*' && line_buf[1] == '%' )
			{
				if( !strcmp(strtok(line_buf, " "), "*%CNSizeToPrintArea") )
					opt_num++;
			}
		}
		DEBUG_PRINT2( "DEBUG:[rastertocanonij] opt_num : opt_num:%d\n",opt_num );

		mem_size = sizeof(SizeToPrintArea) * (opt_num + 1);
		p_table_top = p_table = (SizeToPrintArea*)malloc( mem_size );
		memset( p_table, 0x00, mem_size );

		if( p_table_top != NULL )
		{
			rewind(fp);

			while( read_line(fp, line_buf, LINE_BUF_SIZE) >= 0 )
			{
				if( line_buf[0] == '*' && line_buf[1] == '%' )
				{
					if( !strcmp(strtok(line_buf, " "), "*%CNSizeToPrintArea") )
					{
						char *size_key = strtok(NULL, " ");
						long w = atol( strtok(NULL, " ") );
						long h = atol( strtok(NULL, " ") );

						if( (size_key != NULL) && (w != 0) && (h != 0) )
						{
							int size_len = strlen(size_key) + 1;

							p_table->size_key = (char*)malloc(size_len);

							if( p_table->size_key != NULL )
							{
 								//strcpy(p_table->size_key, size_key);
 								strncpy(p_table->size_key, size_key, size_len);
 								p_table->size_key[size_len -1] = '\0';
								p_table->width = w;
								p_table->height = h;
								p_table++;
							}
						}
					}
				}
			}
 			p_table->size_key = NULL;
			p_table->width = 0;
			p_table->height = 0;
		}
		fclose(fp);
	}
	return p_table_top;
}


void free_size_to_print_area_table(SizeToPrintArea *p_table_top)
{
	if( p_table_top != NULL )
	{
		SizeToPrintArea *p_table = p_table_top;
		while( p_table->size_key != NULL ){
			free( p_table->size_key );
			p_table++;
		}
		free( p_table_top );
	}
}


