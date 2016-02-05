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

/*
	'2014.04.17
	Using the original "rastertopwgc.c" code of the CUPS 1.6.1 to create 
	a new filter which outputs pwgraster.
 */
/*
 * "$Id: rastertopwg.c 10005 2011-09-20 18:36:23Z mike $"
 *
 *   CUPS raster to PWG raster format filter for CUPS.
 *
 *   Copyright 2011 Apple Inc.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Apple Inc. and are protected by Federal copyright law.
 *   Distribution and use rights are outlined in the file "LICENSE.txt"
 *   which should have been included with this file.  If this file is
 *   file is missing or damaged, see the license at "http://www.cups.org/".
 *
 *   This file is subject to the Apple OS-Developed Software exception.
 *
 * Contents:
 *
 *   main() - Main entry for filter.
 */

#include <stdlib.h>
#include <getopt.h>
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "mkpset.h"
#include "cndata_def.h"
#include "com_def.h"

#define DATA_BUF_SIZE (1024 * 1)
#define STR_BUF_SIZE (256 * 1)
#define CNIJPWG_TEMP "/var/tmp/cnijpwgtmpXXXXXX"
#define CNIJRAW_TEMP "/var/tmp/cnijrawtmpXXXXXX"
#define DUPLEX_LONGSIDE "duplexnotumble"

typedef ssize_t (*cups_raster_iocb_t)(void *ctx, unsigned char *buffer, size_t length );

enum {
	OPT_VERSION = 0,
	OPT_PRINTABLE_WIDTH,
	OPT_PRINTABLE_HEIGHT,
	OPT_DUPLEXPRINT,
};

static char *StringToLower ( char *src ) 
{
	char *cur;

	for( cur = src; *cur; cur++ ){
		*cur = tolower( *cur );
	}
	return src;
}

/*
 * Private structures...
 */

typedef struct					/**** Raster stream data ****/
{
	unsigned		sync;		/* Sync word from start of stream */
	void			*ctx;		/* File descriptor */
	cups_raster_iocb_t	iocb;		/* IO callback */
//	cups_mode_t		mode;		/* Read/write mode */
	cups_page_header2_t	header;		/* Raster header for current page */
	int			count,		/* Current row run-length count */
				remaining,	/* Remaining rows in page image */
				bpp;		/* Bytes per pixel/color */
	unsigned char		*pixels,	/* Pixels for current row */
						*pend,		/* End of pixel buffer */
						*pcurrent;	/* Current byte in pixel buffer */
	int			compressed,	/* Non-zero if data is compressed */
				swapped;	/* Non-zero if data is byte-swapped */
	unsigned char		*buffer;	/* Read/write buffer */
//						*bufptr,	/* Current (read) position in buffer */
//						*bufend;	/* End of current (read) buffer */
	size_t		bufsize;	/* Buffer size */
} pwg_raster_s;

typedef struct SizePixelTable {
	char *name;
	long width;
	long height;
} SIZEPIXELTABLE, *LPFIZEPIXELTABLE;


/* Prototypes */
static int ComputeDestinationSize( long in_w, long in_h, long out_w, long out_h, long *dst_w, long *dst_h, long *ofs_w, long *ofs_h );
static int h_extend( unsigned char *in, unsigned char *out, int src_width, int dst_width, int component );
static int WriteWhiteLineToPWG( pwg_raster_s *outras, unsigned char white, long out_buf_size, long line_num  );
static short mirror_raster( unsigned char *buf, long width, short bpp );
static int rawRasterTempOpen( void );
static int InitPWGPageData( pwg_raster_s **outras );
static int CreatePWGPageData( int page, cups_page_header2_t *inheader, cups_raster_t *inras, pwg_raster_s *outras, long printable_width, long printable_height, int is_rotate );
static int OutputPWGPageData( pwg_raster_s *outras, short isNextPgae, short page );
static int DestroyPWGPageData( pwg_raster_s **outras );
static int isRotate( const char *option );


static long GetFileSize( int fd )
{
	struct stat buf;
	long result = -1;

	fstat( fd, &buf );
	result = buf.st_size;
	DEBUG_PRINT2( "DEBUG:[tocnpwg] PWGRaster File Size: %ld\n",result );

	return result;
}

/*
 * 'cups_write_fd()' - Write bytes to a file.
 */
static ssize_t				/* O - Bytes written or -1 */
cups_write_fd(	void          *ctx,	/* I - File descriptor pointer */
				unsigned char *buf,	/* I - Bytes to write */
				size_t        bytes)	/* I - Number of bytes to write */
{
	int		fd = (int)((intptr_t)ctx); /* File descriptor */
	ssize_t	count;			/* Number of bytes written */

	while ((count = write(fd, buf, bytes)) < 0)
	if (errno != EINTR && errno != EAGAIN) {
		DEBUG_PRINT3( "DEBUG:[tocnpwg] Error in cups_write_fd count : %d size : %d\n", count, bytes );
		return (-1);
	}
	return (count);
}


/*
 * 'cups_raster_io()' - Read/write bytes from a context, handling interruptions.
 */

static int				/* O - Bytes read or -1 */
cups_raster_io( pwg_raster_s *r,	/* I - Raster stream */
           unsigned char *buf,		/* I - Buffer for read/write */
           int           bytes)		/* I - Number of bytes to read/write */
{
	ssize_t	count;			/* Number of bytes read/written */
	size_t	total;			/* Total bytes read/written */

	for (total = 0; total < (size_t)bytes; total += count, buf += count) {
		count = (*r->iocb)(r->ctx, buf, bytes - total);
		if (count == 0)
			return (0);
		else if (count < 0)
			return (-1);
	}

	return ((int)total);
}


/*
 * for write
 */
static pwg_raster_s *pwgRasterTempOpen( void )
{
	char tmpName[64];
	pwg_raster_s *r = NULL;
	static int pwgtmp_fd;
	DEBUG_PRINT( "DEBUG:[tocnpwg] pwgRasterTempOpen<1>\n" );

	strncpy( tmpName, CNIJPWG_TEMP, 64 );

	if ((r = calloc(sizeof(pwg_raster_s), 1)) == NULL) {
		goto onErr;
	}

	DEBUG_PRINT( "DEBUG:[tocnpwg] pwgRasterTempOpen<2>\n" );
	//r->ctx  = (void *)((intptr_t)fd);
	pwgtmp_fd = mkstemp( tmpName );
	r->ctx = (void *)((intptr_t)pwgtmp_fd);
	r->iocb = cups_write_fd;

	/* for write */
	r->compressed = 1;
	r->sync       = htonl(CUPS_RASTER_SYNCv2);
	r->swapped    = r->sync != CUPS_RASTER_SYNCv2;

	DEBUG_PRINT( "DEBUG:[tocnpwg] pwgRasterTempOpen<3>\n" );
    if (cups_raster_io(r, (unsigned char *)&(r->sync), sizeof(r->sync)) < sizeof(r->sync)) {
		goto onErr;
	}

	DEBUG_PRINT( "DEBUG:[tocnpwg] pwgRasterTempOpen<4>\n" );
EXIT:
	return (r);
onErr:
	if ( r != NULL ){
		free( r );
	}
	goto EXIT;
}

/*
 * for Dump
 */
static int pwgRasterDump( pwg_raster_s *r )
{
	int read_bytes, write_bytes;
	int result = -1;
	char *p_data_buf = NULL;

	if ( (p_data_buf = malloc( DATA_BUF_SIZE * sizeof(char) )) == NULL ) goto onErr;

	lseek( (int)r->ctx, 0, SEEK_SET );
	while( 1 ) {
		read_bytes = read( (int)r->ctx, p_data_buf, DATA_BUF_SIZE );
		if ( read_bytes > 0 ) {
			char *p_current = p_data_buf;

			do {
				write_bytes = write( 1, p_current, read_bytes );
				if ( write_bytes < 0 ){
					if ( errno ==EINTR ) continue;
					goto onErr;
				}
				read_bytes -= write_bytes;
				p_current += write_bytes;

			} while( read_bytes > 0 );
		}
		else {
			break;
		}
	}

	result = 0;
onErr:
	if ( p_data_buf != NULL ) free( p_data_buf );

	return result;
}

/*
 * for GetFileSize
 */
static long pwgRasterGetFileSize( pwg_raster_s *r )
{
	return ( GetFileSize((int)r->ctx) );
}

static void pwgRasterClose( pwg_raster_s *r )
{
	if ( r != NULL ) {
		if ( r->buffer != NULL ) {
			free( r->buffer );
		}
		if ( r->pixels != NULL) {
			free( r->pixels );
		}
		free( r);
	}
}

static void pwgraster_update( pwg_raster_s *r ) 
{

	/*
	* Set the number of bytes per pixel/color...
	*/
	if (r->header.cupsColorOrder == CUPS_ORDER_CHUNKED){
		r->bpp = (r->header.cupsBitsPerPixel + 7) / 8;
	}
	else{
		r->bpp = (r->header.cupsBitsPerColor + 7) / 8;
	}

	/*
	* Set the number of remaining rows...
	*/
	if (r->header.cupsColorOrder == CUPS_ORDER_PLANAR) {
		r->remaining = r->header.cupsHeight * r->header.cupsNumColors;
	}
	else {
		r->remaining = r->header.cupsHeight;
	}

	/*
	* Allocate the compression buffer...
	*/
	if (r->compressed) {
		if (r->pixels != NULL){
			free(r->pixels);
			r->pixels = NULL;
		}

		r->pixels   = calloc(r->header.cupsBytesPerLine, 1);
		r->pcurrent = r->pixels;
		r->pend     = r->pixels + r->header.cupsBytesPerLine;
		r->count    = 0;
	}

}

static unsigned pwgRasterWriteHeader( 
	pwg_raster_s *r,			/* I - Raster stream */
	cups_page_header2_t *h )	/* I - Raster page header */
{
	unsigned int result = 0;
    cups_page_header2_t	fh;		/* File page header */

	if ( r == NULL ) goto onErr;

  	memcpy(&(r->header), h, sizeof(cups_page_header2_t));

	pwgraster_update( r );

	/* Write Header */
	memset(&fh, 0, sizeof(fh));
	strncpy(fh.MediaClass, "PwgRaster", sizeof(fh.MediaClass));
	strncpy(fh.MediaColor, r->header.MediaColor, sizeof(fh.MediaColor));
	strncpy(fh.MediaType, r->header.MediaType, sizeof(fh.MediaType));
	strncpy(fh.OutputType, r->header.OutputType, sizeof(fh.OutputType));
	strncpy(fh.cupsRenderingIntent, r->header.cupsRenderingIntent, sizeof(fh.cupsRenderingIntent));
	strncpy(fh.cupsPageSizeName, r->header.cupsPageSizeName, sizeof(fh.cupsPageSizeName));

	fh.CutMedia              = htonl(r->header.CutMedia);
	fh.Duplex                = htonl(r->header.Duplex);
	fh.HWResolution[0]       = htonl(r->header.HWResolution[0]);
	fh.HWResolution[1]       = htonl(r->header.HWResolution[1]);
	fh.ImagingBoundingBox[0] = htonl(r->header.ImagingBoundingBox[0]);
	fh.ImagingBoundingBox[1] = htonl(r->header.ImagingBoundingBox[1]);
	fh.ImagingBoundingBox[2] = htonl(r->header.ImagingBoundingBox[2]);
	fh.ImagingBoundingBox[3] = htonl(r->header.ImagingBoundingBox[3]);
	fh.InsertSheet           = htonl(r->header.InsertSheet);
	fh.Jog                   = htonl(r->header.Jog);
	fh.LeadingEdge           = htonl(r->header.LeadingEdge);
	fh.ManualFeed            = htonl(r->header.ManualFeed);
	fh.MediaPosition         = htonl(r->header.MediaPosition);
	fh.MediaWeight           = htonl(r->header.MediaWeight);
	fh.NumCopies             = htonl(r->header.NumCopies);
	fh.Orientation           = htonl(r->header.Orientation);
	fh.PageSize[0]           = htonl(r->header.PageSize[0]);
	fh.PageSize[1]           = htonl(r->header.PageSize[1]);
	fh.Tumble                = htonl(r->header.Tumble);
	fh.cupsWidth             = htonl(r->header.cupsWidth);
	fh.cupsHeight            = htonl(r->header.cupsHeight);
	fh.cupsBitsPerColor      = htonl(r->header.cupsBitsPerColor);
	fh.cupsBitsPerPixel      = htonl(r->header.cupsBitsPerPixel);
	fh.cupsBytesPerLine      = htonl(r->header.cupsBytesPerLine);
	fh.cupsColorOrder        = htonl(r->header.cupsColorOrder);
	//fh.cupsColorSpace        = htonl(r->header.cupsColorSpace);
	// 2013.06.10  CUPS_CSPACE_SRG
	fh.cupsColorSpace        = htonl( 19 );
	fh.cupsNumColors         = htonl(r->header.cupsNumColors);
	//fh.cupsInteger[0]        = htonl(r->header.cupsInteger[0]);
	// TotalPageCount Set 1
	fh.cupsInteger[0]        = htonl( 1 );
	fh.cupsInteger[1]        = htonl(r->header.cupsInteger[1]);
	fh.cupsInteger[2]        = htonl(r->header.cupsInteger[2]);
	fh.cupsInteger[3]        = htonl((unsigned)(r->header.cupsImagingBBox[0] * r->header.HWResolution[0]));
	fh.cupsInteger[4]        = htonl((unsigned)(r->header.cupsImagingBBox[1] * r->header.HWResolution[1]));
	fh.cupsInteger[5]        = htonl((unsigned)(r->header.cupsImagingBBox[2] * r->header.HWResolution[0]));
	fh.cupsInteger[6]        = htonl((unsigned)(r->header.cupsImagingBBox[3] * r->header.HWResolution[1]));
	fh.cupsInteger[7]        = htonl(0xffffff);
	

	if ( cups_raster_io(r, (unsigned char *)&fh, sizeof(fh)) != sizeof(fh) ){
		DEBUG_PRINT( "DEBUG:[tocnpwg] Error in cups_raster_io\n" );
		goto onErr;
	}

	result = 1;
onErr:
	return result;
}

/*
 * 'cups_raster_write()' - Write a row of compressed raster data...
 */
static int				/* O - Number of bytes written */
cups_raster_write(
    pwg_raster_s		*r,		/* I - Raster stream */
    const unsigned char *pixels)	/* I - Pixel data to write */
{
	const unsigned char	*start,		/* Start of sequence */
						*ptr,		/* Current pointer in sequence */
						*pend,		/* End of raster buffer */
						*plast;		/* Pointer to last pixel */
	unsigned char		*wptr;		/* Pointer into write buffer */
	int			bpp,		/* Bytes per pixel */
				count;		/* Count */

	//DEBUG_printf(("cups_raster_write(r=%p, pixels=%p)\n", r, pixels));

	/*
	* Allocate a write buffer as needed...
	*/

	count = r->header.cupsBytesPerLine * 2;
	if ((size_t)count > r->bufsize) {
		if (r->buffer){
			wptr = realloc(r->buffer, count);
		}
		else {
			wptr = malloc(count);
		}

		if (!wptr) {
			return (-1);
		}

		r->buffer  = wptr;
		r->bufsize = count;
	}

	/*
	* Write the row repeat count...
	*/

	bpp     = r->bpp;
	pend    = pixels + r->header.cupsBytesPerLine;
	plast   = pend - bpp;
	wptr    = r->buffer;
	*wptr++ = r->count - 1;

	/*
	* Write using a modified PackBits compression...
	*/

	for (ptr = pixels; ptr < pend;) {
		start = ptr;
		ptr += bpp;

		if (ptr == pend) {
		/*
		* Encode a single pixel at the end...
		*/

			*wptr++ = 0;
			for (count = bpp; count > 0; count --) {
				*wptr++ = *start++;
			}
		}
		else if (!memcmp(start, ptr, bpp)) {
		/*
		* Encode a sequence of repeating pixels...
		*/

			for (count = 2; count < 128 && ptr < plast; count ++, ptr += bpp) {
				if (memcmp(ptr, ptr + bpp, bpp)) break;
			}

			*wptr++ = count - 1;
			for (count = bpp; count > 0; count --){
				*wptr++ = *ptr++;
			}
		}
		else {
		/*
		* Encode a sequence of non-repeating pixels...
		*/

			for (count = 1; count < 128 && ptr < plast; count ++, ptr += bpp) {
				if (!memcmp(ptr, ptr + bpp, bpp)) break;
			}

			if (ptr >= plast && count < 128) {
				count ++;
				ptr += bpp;
			}

			*wptr++ = 257 - count;

			count *= bpp;
			memcpy(wptr, start, count);
			wptr += count;
		}
	}
	return (cups_raster_io(r, r->buffer, (int)(wptr - r->buffer)));
}


/*
 * 'cupsRasterWritePixels()' - Write raster pixels.
 */
static unsigned			/* O - Number of bytes written */
pwgRasterWritePixels(
	pwg_raster_s *r,		/* I - Raster stream */
	unsigned char *p,		/* I - Bytes to write */
	unsigned len )			/* I - Number of bytes to write */
{
	int		bytes;			/* Bytes read */
	unsigned	remaining;		/* Bytes remaining */


	//DEBUG_printf(("cupsRasterWritePixels(r=%p, p=%p, len=%u), remaining=%u\n", r, p, len, r->remaining));

	if (r == NULL || r->remaining == 0) return (0);

	if (!r->compressed) return (0);

	/*
	* Otherwise, compress each line...
	*/

	for (remaining = len; remaining > 0; remaining -= bytes, p += bytes) {
	/*
	* Figure out the number of remaining bytes on the current line...
	*/

		if ((bytes = remaining) > (int)(r->pend - r->pcurrent))
			bytes = (int)(r->pend - r->pcurrent);

		if (r->count > 0) {
		/*
		* Check to see if this line is the same as the previous line...
		*/

			if (memcmp(p, r->pcurrent, bytes)) {
				if (!cups_raster_write(r, r->pixels)) {
					return (0);
				}
				r->count = 0;
			}
			else {
			/*
			* Mark more bytes as the same...
			*/

				r->pcurrent += bytes;

				if (r->pcurrent >= r->pend) {
				/*
		  		* Increase the repeat count...
				*/

					r->count ++;
					r->pcurrent = r->pixels;

				/*
			  	* Flush out this line if it is the last one...
				*/

					r->remaining --;

					if (r->remaining == 0) {
						return (cups_raster_write(r, r->pixels));
					}
					else if (r->count == 256) {
						if (cups_raster_write(r, r->pixels) == 0) {
			  				return (0);
						}
						r->count = 0;
					}
				}
				continue;
			}
		}

		if (r->count == 0) {
		/*
		* Copy the raster data to the buffer...
		*/

			memcpy(r->pcurrent, p, bytes);

			r->pcurrent += bytes;

			if (r->pcurrent >= r->pend) {
			/*
			* Increase the repeat count...
			*/

			r->count ++;
			r->pcurrent = r->pixels;

			/*
			* Flush out this line if it is the last one...
			*/

			r->remaining --;

			if (r->remaining == 0)
				return (cups_raster_write(r, r->pixels));
			}
		}
	}

	return (len);

}

int main( int argc, char *argv[] )
{
	int result = -1;
	int fd;
	pwg_raster_s *outras = NULL;
  	cups_raster_t *inras = NULL;	/* Input raster stream */
	cups_page_header2_t	inheader;	/* Input raster page header */
  	int page = 0;			/* Current page */
	int opt, opt_index;
	int is_rotate = 0;
	struct option long_opt[] = {
		{ "version", required_argument, NULL, OPT_VERSION }, 
		{ "printable_width", required_argument, NULL, OPT_PRINTABLE_WIDTH }, 
		{ "printable_height", required_argument, NULL, OPT_PRINTABLE_HEIGHT }, 
		{ "duplexprint", required_argument, NULL, OPT_DUPLEXPRINT }, 
		{ 0, 0, 0, 0 }, 
	};
	int isPWGExist;
	long printable_width, printable_height;

	printable_width = printable_height = 0;
	while( (opt = getopt_long( argc, argv, "0:", long_opt, &opt_index )) != -1) {
		switch( opt ) {
			case OPT_VERSION:
				break;
			case OPT_PRINTABLE_WIDTH:
				DEBUG_PRINT3( "[tocnpwg] OPTION(%s):VALUE(%s)\n", long_opt[opt_index].name, optarg );
				printable_width = atol( optarg );			
				DEBUG_PRINT2( "[tocnpwg] printable_width : %ld\n", printable_width );
				break;
			case OPT_PRINTABLE_HEIGHT:
				DEBUG_PRINT3( "[tocnpwg] OPTION(%s):VALUE(%s)\n", long_opt[opt_index].name, optarg );
				printable_height = atol( optarg );			
				DEBUG_PRINT2( "[tocnpwg] printable_height : %ld\n", printable_height );
				break;
			case OPT_DUPLEXPRINT:
				DEBUG_PRINT3( "[tocnpwg] OPTION(%s):VALUE(%s)\n", long_opt[opt_index].name, optarg );
				is_rotate = isRotate( optarg ); 
				DEBUG_PRINT2( "[tocnpwg] is_rotate : %d\n", is_rotate );
				break;
		}
	}
	
#ifdef DEBUG_LOG
	int debug_fd = 0;		
#endif

	/* Open CupsRaster */
	fd = 0;
	inras  = cupsRasterOpen(fd, CUPS_RASTER_READ);

	page = 1;
	isPWGExist = 0;
	while (cupsRasterReadHeader2(inras, &inheader)) {

		/* exist next page */
		if ( isPWGExist ) {
			if ( OutputPWGPageData( outras, 1, page) != 0 ) goto onErr;
			DestroyPWGPageData( &outras );
			isPWGExist = 0;
		}

		InitPWGPageData( &outras );
		if ( CreatePWGPageData( page, &inheader, inras, outras, printable_width, printable_height, is_rotate ) != 0 ) goto onErr;
		isPWGExist = 1;

		page++;
	}

	/* not exist next page */
	if ( isPWGExist ) {
		if ( OutputPWGPageData( outras, 0, page) != 0 ) goto onErr;
		DestroyPWGPageData( &outras );
		isPWGExist = 0;
	}

#ifdef DEBUG_LOG
	close( debug_fd );
#endif
	result = 0;
onErr:
	if ( inras != NULL ){
		cupsRasterClose(inras);
	}
	close(fd);
	return result;
}

/*
 * in_w : Input image width
 * in_h : Input image height
 * out_w : Output imge width
 * out_h : Output image height
 * dst_w : Scaled image width
 * dst_h : Scaled image heiht
 * ofs_w : Offset from orignal image to scaled image ( x direction)
 * ofs_h : Offset from orignal image to scaled image ( y direction)
 */
static int ComputeDestinationSize( long in_w, long in_h, long out_w, long out_h, long *dst_w, long *dst_h, long *ofs_w, long *ofs_h )
{
	double coef_w,  coef_h;
	int result = -1;

	if ( (dst_w == NULL) || (dst_h == NULL) ) goto onErr;

	coef_w = (double)out_w / (double)in_w;
	coef_h = (double)out_h / (double)in_h;

	if( coef_w < coef_h ){
		*dst_w = out_w;
		*dst_h = (long)((double)in_h * coef_w + 0.5);
		*ofs_w = 0;
		*ofs_h = (out_h - *dst_h) / 2;
	}
	else {
		*dst_w = (long)((double)in_w * coef_h + 0.5);
		*dst_h = out_h;
		*ofs_w = (out_w - *dst_w) / 2;
		*ofs_h = 0;
	}

	result = 0;
onErr:
	return result;	
}

static int h_extend( unsigned char *in, unsigned char *out, int src_width, int dst_width, int component )
{
	int quotient;
	int	rest;
	int	total_rest;
	int prev_pos;
	int curr_pos;
	int	x;
	unsigned char K,Y,M,C,y,m,c;
	unsigned char * src_in;
	short result = -1;
	

	if ( (src_width < 1) || (dst_width < 0) ) goto onErr;

	if ( dst_width == 0 ) {
		goto EXIT;
	}
	else if ( dst_width == 1 ){
		switch ( component ){
			case 1:
				out[0] = in[0];
				break;
			case 3:
				out[0] = in[0]; out[1] = in[1]; out[2] = in[2];
				break;
			case 4:
				out[0] = in[0]; out[1] = in[1]; out[2] = in[2]; out[3] = in[3];
				break;
			case 6:
				out[0] = in[0]; out[1] = in[1]; out[2] = in[2];
				out[3] = in[3]; out[4] = in[4]; out[5] = in[5];
				break;
			case 7:
				out[0] = in[0]; out[1] = in[1]; out[2] = in[2];
				out[3] = in[3]; out[4] = in[4]; out[5] = in[5]; out[6] = in[6];
				break;
		}
		goto EXIT;
	}

	quotient  = (src_width - 1) / (dst_width - 1);
	rest      = (src_width - 1) % (dst_width - 1);
	total_rest = 0;
	prev_pos   = -1;
	curr_pos   = 0;
	K = Y = M = C = y = m = c = 0;	

	for ( x=0; x<dst_width; x++ ){

		/* calculate remainder */
		if ( total_rest * 2 >= (dst_width -1) ){
			total_rest -= (dst_width -1);
			curr_pos++;
		}

		if ( curr_pos != prev_pos ){
			prev_pos = curr_pos;
			src_in = in + ( component * curr_pos );

			switch ( component ){
				case 1:
					K = src_in[0];
					break;
				case 3:
					Y = src_in[0]; M = src_in[1];
					C = src_in[2];
					break;
				case 4:
					K = src_in[0]; Y = src_in[1];
					M = src_in[2]; C = src_in[3];
					break;
				case 6:
					K = src_in[0]; Y = src_in[1];
					M = src_in[2]; C = src_in[3];
					m = src_in[4]; c = src_in[5];
					break;
				case 7:
					K = src_in[0]; Y = src_in[1];
					M = src_in[2]; C = src_in[3];
					y = src_in[4]; m = src_in[5]; c = src_in[6];
					break;
			}
		}
		
		switch ( component ){
			case 1:
				out[0] = K;
				break;
			case 3:
				out[0] = Y; out[1] = M; out[2] = C;
				break;
			case 4:
				out[0] = K; out[1] = Y; out[2] = M; out[3] = C;
				break;
			case 6:
				out[0] = K; out[1] = Y; out[2] = M;
				out[3] = C; out[4] = m; out[5] = c;
				break;
			case 7:
				out[0] = K; out[1] = Y; out[2] = M;
				out[3] = C; out[4] = y; out[5] = m; out[6] = c;
				break;
		}
		
		out += component;
		
		total_rest += rest;
		curr_pos += quotient;
		
	}

EXIT:	
	result = 0;
onErr:
	return result;
}


static int WriteWhiteLineToPWG( pwg_raster_s *outras, unsigned char white, long out_buf_size, long line_num  )
{
	int result = -1;
	long i;
	unsigned char *ptr = NULL;

	if ( (ptr = malloc( out_buf_size )) == NULL ) goto onErr1;
	memset( ptr, white, out_buf_size );

	for ( i=0; i<line_num; i++ ){
		if (!pwgRasterWritePixels(outras, ptr, out_buf_size) < 0) {
			DEBUG_PRINT( "DEBUG:[tocnpwg] Error in pwgRasterWritePixels\n" );
			goto onErr2;
		}
	}

	result = 0;
onErr2:
	if ( ptr != NULL ){
		free( ptr );
	}
onErr1:
	return result;
}

static int WriteWhiteLineToRAW( int fd, unsigned char white, long out_buf_size, long line_num  )
{
	int result = -1;
	long i;
	unsigned char *ptr = NULL;

	if ( (ptr = malloc( out_buf_size )) == NULL ) goto onErr1;
	memset( ptr, white, out_buf_size );

	for ( i=0; i<line_num; i++ ){
		if ( write( fd, ptr, out_buf_size ) < 0 ){
			DEBUG_PRINT( "DEBUG:[tocnpwg] Error in pwgRasterWritePixels\n" );
			goto onErr2;
		}
	}

	result = 0;
onErr2:
	if ( ptr != NULL ){
		free( ptr );
	}
onErr1:
	return result;
}



static int InitPWGPageData( pwg_raster_s **outras )
{
	int result = -1;

	if ( outras == NULL ) goto onErr;

	*outras = pwgRasterTempOpen();

	result = 0;
onErr:
	return result;
}

static int OutputPWGPageData( pwg_raster_s *outras, short isNextPage, short page )
{
	int result = -1;
	CNDATA CNData;

	if ( outras == NULL )  goto onErr;

	/* Write Page Info */
   	memset( &CNData, 0x00, sizeof(CNDATA) );
	CNData.magic_num = MAGIC_NUMBER_FOR_CNIJPWG;
	CNData.image_size = pwgRasterGetFileSize(outras);
	CNData.next_page = isNextPage;
	CNData.page_num = page;
	write( 1, &CNData, sizeof(CNDATA) );

	/* Output PWG Page Data */
	pwgRasterDump(outras);

	result = 0;
onErr:
	return result;
}

static int DestroyPWGPageData( pwg_raster_s **outras )
{
	int result = -1;
	pwg_raster_s *l_outras = *outras;

	if ( outras == NULL ) goto onErr;

	pwgRasterClose(l_outras);
	*outras = NULL;

	result = 0;
onErr:
	return result;
}

static int CreatePWGPageData( 
		int page, 
		cups_page_header2_t *inheader, 
  		cups_raster_t *inras,
		pwg_raster_s *outras,
		long printable_width, 
		long printable_height, 
		int is_rotate )
{
	//pwg_raster_s *outras = NULL;	/* Ouput raster stream */
	cups_page_header2_t outheader;
	int y;
	unsigned char white;
	long in_w, in_h, out_w, out_h, dst_w, dst_h, ofs_w, ofs_h;
	long quotient, rest, total_rest, curr_pos, prev_pos, cnt, i;
	long in_buf_size, out_buf_size;
	unsigned char *in_ptr = NULL, *out_ptr = NULL;
	int tmp_fd = -1;
	int result = -1;	


	/* Open PWGRaster */
	//outras = pwgRasterTempOpen();

	/*
	* Compute the real raster size...
	*/
	DEBUG_PRINT3( "DEBUG:[tocnpwg] PAGE: %d %d\n", page, inheader->NumCopies);
	switch (inheader->cupsColorSpace)
	{
		case CUPS_CSPACE_W :
		case CUPS_CSPACE_RGB :
			white = 255;
			break;
		case CUPS_CSPACE_K :
		case CUPS_CSPACE_CMYK :
			white = 0;
			break;

		default :
		DEBUG_PRINT3( "DEBUG:[tocnpwg] Unsupported cupsColorSpace %d on page %d.\n", inheader->cupsColorSpace, page);
		goto onErr1;
	}

	DEBUG_PRINT2( "DEBUG:[tocnpwg] inheader->cupsColorOrder: %d\n", inheader->cupsColorOrder);
	DEBUG_PRINT2( "DEBUG:[tocnpwg] inheader->cupsBitsPerPixel: %d\n", inheader->cupsBitsPerPixel);
	DEBUG_PRINT2( "DEBUG:[tocnpwg] inheader->cupsInteger(TotalPageCount): %d\n", htonl(inheader->cupsInteger[0]));

	if (inheader->cupsColorOrder != CUPS_ORDER_CHUNKED) {
		DEBUG_PRINT3( "DEBUG:[tocnpwg] Unsupported cupsColorOrder %d on page %d.\n", inheader->cupsColorOrder, page);
		goto onErr1;
	}

	DEBUG_PRINT2( "DEBUG:[tocnpwg] cupsBitsPerPixel = %d\n", inheader->cupsBitsPerPixel );
	if (inheader->cupsBitsPerPixel != 1 &&
		inheader->cupsBitsPerColor != 8 && inheader->cupsBitsPerColor != 16) {
		DEBUG_PRINT3( "DEBUG:[tocnpwg] Unsupported cupsBitsPerColor %d on page %d.\n", inheader->cupsBitsPerColor, page);
		goto onErr1;
	}

	/* Copy in-header to out-header */
	memcpy(&outheader, inheader, sizeof(outheader));

	/* Compute Destination Size */
	in_w = inheader->cupsWidth;
	in_h = inheader->cupsHeight;
	if ( printable_width != 0 ) {
		out_w = printable_width;
	}
	else {
		out_w = in_w;
	}
	if ( printable_height != 0 ){
		out_h = printable_height;
	}
	else {
		out_h = in_h;
	}
	dst_w = dst_h = ofs_w = ofs_h = 0;

	if ( ComputeDestinationSize( in_w, in_h, out_w, out_h, &dst_w, &dst_h, &ofs_w, &ofs_h ) != 0 ) {
		DEBUG_PRINT( "DEBUG:[tocnpwg] Error ComputeDestinationSize\n");
		goto onErr1;
	}
	DEBUG_PRINT2( "DEBUG:[tocnpwg] in_w: %ld\n",in_w );
	DEBUG_PRINT2( "DEBUG:[tocnpwg] in_h: %ld\n",in_h );
	DEBUG_PRINT2( "DEBUG:[tocnpwg] out_w: %ld\n",out_w );
	DEBUG_PRINT2( "DEBUG:[tocnpwg] out_h: %ld\n",out_h );
	DEBUG_PRINT2( "DEBUG:[tocnpwg] dst_w: %ld\n",dst_w );
	DEBUG_PRINT2( "DEBUG:[tocnpwg] dst_h: %ld\n",dst_h );
	DEBUG_PRINT2( "DEBUG:[tocnpwg] ofs_w: %ld\n",ofs_w );
	DEBUG_PRINT2( "DEBUG:[tocnpwg] ofs_h: %ld\n",ofs_h );


	// ############################################### update outheader
	outheader.cupsInteger[14]  = 0;	/* VendorIdentifier */
	outheader.cupsInteger[15]  = 0;	/* VendorLength */

	/* New code */
	outheader.cupsWidth  = out_w;
	outheader.cupsHeight  = out_h;
	outheader.cupsBytesPerLine = outheader.cupsWidth * outheader.cupsNumColors;
	in_buf_size = inheader->cupsBytesPerLine;
	out_buf_size = outheader.cupsBytesPerLine;

	DEBUG_PRINT2( "DEBUG:[tocnpwg] inheader->cupsPageSizeName[0]: %s\n", inheader->cupsPageSizeName );

	/* Write out-Header */
	if (!pwgRasterWriteHeader(outras, &outheader))
	{
		DEBUG_PRINT2( "DEBUG:[tocnpwg] Unable to write header for page %d.\n", page);
		goto onErr1;
	}

#if 0
	debug_fd = open( "/var/spool/cups/tmp/debug_raw.ppm", O_CREAT | O_WRONLY );
	if ( debug_fd == -1 ){
	  DEBUG_PRINT( "DEBUG:[tocnpwg] Unable to open file\n");
	}
	dprintf( debug_fd, "P6\n#Comment\n%d\n%d\n255\n", outheader.cupsWidth, outheader.cupsHeight ); 
#endif

   	/*
   	 * Copy raster data...
   	*/

   	/* allocate input buffer */
   	if ( (in_ptr = malloc(in_buf_size)) == NULL ){
   		DEBUG_PRINT( "DEBUG:[tocnpwg] Can not allocate in_ptr\n" );
   		goto onErr1;
   	}
   	memset(in_ptr, white, in_buf_size);

   	/* allocate output buffer */
   	if ( (out_ptr = malloc(out_buf_size)) == NULL ){
   		DEBUG_PRINT( "DEBUG:[tocnpwg] Can not allocate in_ptr\n" );
   		goto onErr2;
   	}
   	memset(out_ptr, white, out_buf_size);

   	DEBUG_PRINT2( "DEBUG:[tocnpwg] inheader->cupsHeight: %d\n", inheader->cupsHeight);

	quotient  = (in_h - 1) / (dst_h - 1);
	rest      = (in_h - 1) % (dst_h - 1);
	total_rest = 0;
	curr_pos = 0;
	prev_pos = -1;

	if ( is_rotate && (page%2) ) {	/* Rotate 180 degree */
		DEBUG_PRINT( "DEBUG:[tocnpwg] Rotate 180<1>\n" );
		tmp_fd = rawRasterTempOpen();

		/* Write Temp File ********************************************************************/
		/* Write Top Margine */
		WriteWhiteLineToRAW( tmp_fd, white, out_buf_size, ofs_h );
		DEBUG_PRINT( "DEBUG:[tocnpwg] Rotate 180<2>\n" );

		/* Write Print Data */
		for ( y = 0; y < dst_h; y++ ){
			if ( total_rest * 2 >= (dst_h -1) ){
				total_rest -= (dst_h -1);
				curr_pos++;
			}

			if ( curr_pos != prev_pos ) {
				cnt = curr_pos - prev_pos;
				prev_pos = curr_pos;

				for( i=0; i<cnt; i++ ) {
					if ( cupsRasterReadPixels( inras, in_ptr,in_buf_size ) < 0) {
						DEBUG_PRINT( "DEBUG:[tocnpwg] Error in cupsRasterReadPixels\n" );
						goto onErr3;
					}
				}
			}
			
			/* clear output buffer */	
			memset(out_ptr, white, out_buf_size);

			/* resize output data */
			if ( h_extend( in_ptr, out_ptr + ofs_w * outheader.cupsNumColors, in_w, dst_w, outheader.cupsNumColors ) != 0 ) goto onErr3;

			/* mirror raster */
			if ( mirror_raster( out_ptr, out_buf_size/outheader.cupsNumColors, outheader.cupsNumColors ) != 0 ) goto onErr3;

			/* output data */
			//if (!pwgRasterWritePixels(outras, out_ptr, out_buf_size) < 0) {
			if ( write(tmp_fd, out_ptr, out_buf_size) < 0) {
				DEBUG_PRINT( "DEBUG:[tocnpwg] Error in write\n" );
				goto onErr3;
			}

			total_rest += rest;
			curr_pos += quotient;
		}
		DEBUG_PRINT( "DEBUG:[tocnpwg] Rotate 180<3>\n" );

		/* Write Bottom Margine */
		WriteWhiteLineToRAW( tmp_fd, white, out_buf_size, (out_h - dst_h - ofs_h) );

		/* Write Temp File ********************************************************************/
		for ( y = 0; y < dst_h; y++ ) {
			long read_top;

			read_top = (dst_h - y - 1) * out_buf_size;

			lseek( tmp_fd, read_top, SEEK_SET);
			read( tmp_fd, out_ptr, out_buf_size );

			/* output data */
			if (!pwgRasterWritePixels(outras, out_ptr, out_buf_size) < 0) {
				DEBUG_PRINT( "DEBUG:[tocnpwg] Error in pwgRasterWritePixels\n" );
				goto onErr3;
			}
		}
		close( tmp_fd );
		tmp_fd = -1;
	}
	else { /* Normal Print */
		/* Write Top Margine */
		WriteWhiteLineToPWG( outras, white, out_buf_size, ofs_h );

		/* Write Print Data */
		for ( y = 0; y < dst_h; y++ ){
			if ( total_rest * 2 >= (dst_h -1) ){
				total_rest -= (dst_h -1);
				curr_pos++;
			}

			if ( curr_pos != prev_pos ) {
				cnt = curr_pos - prev_pos;
				prev_pos = curr_pos;

				for( i=0; i<cnt; i++ ) {
					if ( cupsRasterReadPixels( inras, in_ptr,in_buf_size ) < 0) {
						DEBUG_PRINT( "DEBUG:[tocnpwg] Error in cupsRasterReadPixels\n" );
						goto onErr3;
					}
				}
			}
			
			/* clear output buffer */	
			memset(out_ptr, white, out_buf_size);

			/* resize output data */
			if ( h_extend( in_ptr, out_ptr + ofs_w * outheader.cupsNumColors, in_w, dst_w, outheader.cupsNumColors ) != 0 )  goto onErr3;

			/* output data */
			if (!pwgRasterWritePixels(outras, out_ptr, out_buf_size) < 0) {
				DEBUG_PRINT( "DEBUG:[tocnpwg] Error in pwgRasterWritePixels\n" );
				goto onErr3;
			}

			total_rest += rest;
			curr_pos += quotient;
		}

		/* Write Bottom Margine */
		WriteWhiteLineToPWG( outras, white, out_buf_size, (out_h - dst_h - ofs_h) );
	}

	result = 0;
onErr3:
	if ( tmp_fd != -1 ) {
		close( tmp_fd );
	}
	if ( out_ptr != NULL ) {
		free( out_ptr );
	}
onErr2:
	if ( in_ptr != NULL ) {
		free( in_ptr );
	}
onErr1:
	return result;
}


/*
 * for write RAWTEMP
 */
static int rawRasterTempOpen( void )
{
	char tmpName[64];
	int fd;

	strncpy( tmpName, CNIJRAW_TEMP, 64 );
	fd = mkstemp( tmpName );

	return fd;
}

static short mirror_raster( unsigned char *buf, long width, short bpp )
{
	unsigned char 	*topbuf;
	unsigned char	*tailbuf;	
	char	pixel[4];
	long	i,j;
	long	pixcount;
	short	result = -1;

	if ( (buf == NULL) || (bpp > 4) ) goto onErr;

	pixcount = width >> 1;

	topbuf  = buf;
	tailbuf = buf + ( width - 1 ) * bpp;
	
	for ( i=0; i<pixcount; i++ ){
		for( j=0; j<bpp; j++ ){
			pixel[j]    = topbuf[j];
			topbuf[j]   = tailbuf[j];
			tailbuf[j]  = pixel[j]; 
		}
		topbuf  += bpp;
		tailbuf -= bpp;
	}	

	result = 0;
onErr:
	return result;	
}

/*
 * isRotate
 */
static int isRotate( const char *option ){
	int result = 0;
	char srcBuf[STR_BUF_SIZE];

	strncpy( srcBuf, option, STR_BUF_SIZE ); srcBuf[STR_BUF_SIZE-1] = '\0';
	StringToLower( srcBuf );

	if ( !strcmp( srcBuf, DUPLEX_LONGSIDE ) ){
		result = 1;
	}
	return result;
}
