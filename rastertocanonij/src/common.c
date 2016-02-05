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

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define BUF_SIZE  1024

/*-------------------------------------------------------------------------------
 * GetExecProgPath
 * Get execute program path
-------------------------------------------------------------------------------*/
char *GetExecProgPath( void )
{
    static char buf[BUF_SIZE]={};
    char path[BUF_SIZE];
	int len = 0;
	char *result = NULL;

    snprintf( path, BUF_SIZE, "/proc/%d/exe", getpid() );
    readlink( path, buf, sizeof(buf)-1 );

	len = strlen(buf);
	while( len > 0 ){
		if ( '/' == buf[len] ){
			buf[len + 1] = '\0';
			break;
		}
		else {
			len--;
		}
	}

	if ( len != 0 ){
		result = buf;
	}
	else {
		result = NULL;
	}

    return buf;
}
