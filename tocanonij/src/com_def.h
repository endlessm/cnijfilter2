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


#ifndef _COM_DEF_H_
#define _COM_DEF_H_

#ifdef DEBUG_LOG
#define DEBUG_PRINT( message ) fprintf( stderr, message )
#define DEBUG_PRINT2( f, a ) fprintf( stderr, f, a )
#define DEBUG_PRINT3( f, a, b ) fprintf( stderr, f, a, b )
#else
#define DEBUG_PRINT( message )
#define DEBUG_PRINT2( f, a )
#define DEBUG_PRINT3( f, a, b )
#endif

#endif
