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
#include <string.h>
#include <getopt.h>
//#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>


enum {
	OPT_VERSION = 0,
	OPT_PAPERSIZE,
	OPT_MEDIATYPE,
	OPT_BORDERLESSPRINT,
	OPT_COLORMODE,
	OPT_DUPLEXPRINT,
};


static int IsBorderless( const char *str )
{
	int result = 0;

	if ( str == NULL ) goto onErr;
	
	if ( strstr( str, ".tbl" ) != NULL ) {
		result = 1;
	}

onErr:
	return result;
}

int CreateJobSettings( int argc, char *argv[] , void **pData )
{
	int result = -1;
	int rc;
	int opt, opt_index;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	struct option long_opt[] = {
		{ "version", required_argument, NULL, OPT_VERSION }, 
		{ "papersize", required_argument, NULL, OPT_PAPERSIZE }, 
		{ "mediatype", required_argument, NULL, OPT_MEDIATYPE }, 
		{ "borderlessprint", required_argument, NULL, OPT_BORDERLESSPRINT }, 
		{ "colormode", required_argument, NULL, OPT_COLORMODE }, 
		{ "duplexprint", required_argument, NULL, OPT_DUPLEXPRINT }, 
		{ 0, 0, 0, 0 }, 
	};

	if ( (argv == NULL) || (pData == NULL) ) goto onErr1;

	buf = xmlBufferCreate();
	if ( buf == NULL ) {
		fprintf( stderr, "Error at xmlBufferCreate\n" );
		goto onErr1;
	}

	writer = xmlNewTextWriterMemory( buf, 0 );
	if ( writer == NULL ) {
		fprintf( stderr, "Error at xmlNewTextWriterMemory\n" );
		goto onErr2;
	}

	/* Start the document with the xml default for the version */
	rc = xmlTextWriterStartDocument( writer, NULL, "UTF-8", NULL );
	if ( rc < 0 ) {
		fprintf( stderr, "Error at xmlTextWriterStartDocument\n" );
		goto onErr3;
	}

	/* Start an element name "JOBINFO" */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "JOBINFO");
    if (rc < 0) {
		fprintf( stderr, "Error at xmlTextWriterStartElement\n" );
        goto onErr3;
    }
	

	/* ADD JOB SETTINGS */
	while( (opt = getopt_long( argc, argv, "0:", long_opt, &opt_index )) != -1) {
		switch( opt ) {
			case OPT_VERSION:
				break;
			case OPT_PAPERSIZE:
    			xmlTextWriterStartElement(writer, BAD_CAST "PAPERSIZE" );
    			xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "VALUE", "%s", optarg);
    			rc = xmlTextWriterEndElement(writer );
				if ( rc < 0 ){
					fprintf( stderr, "Error at xmlTextWriterWriteFormatElement\n" );
					goto onErr3;
				}
				break;
			case OPT_MEDIATYPE:
    			xmlTextWriterStartElement(writer, BAD_CAST "MEDIATYPE" );
    			xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "VALUE", "%s", optarg);
    			rc = xmlTextWriterEndElement(writer);
				if ( rc < 0 ){
					fprintf( stderr, "Error at xmlTextWriterWriteFormatElement\n" );
					goto onErr3;
				}
				break;
			case OPT_BORDERLESSPRINT:
				if ( IsBorderless( optarg ) ){
    				xmlTextWriterStartElement(writer, BAD_CAST "BORDERLESSPRINT" );
    				xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "VALUE", "on" );
    				rc = xmlTextWriterEndElement(writer);
				}
				else {
    				xmlTextWriterStartElement(writer, BAD_CAST "BORDERLESSPRINT" );
    				xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "VALUE", "off" );
    				rc = xmlTextWriterEndElement(writer);
				}
				if ( rc < 0 ){
					fprintf( stderr, "Error at xmlTextWriterWriteFormatElement\n" );
					goto onErr3;
				}
				break;
			case OPT_COLORMODE:
    			//rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "COLORMODE", "");
				break;
			case OPT_DUPLEXPRINT:
    			//rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "DUPLEXPRINT", "");
				break;
			case '?':
				fprintf( stderr, "Error: invalid option %c:\n", optopt);
				break;
			default:
				break;
		}
	}

	/* End element */
    rc = xmlTextWriterEndElement(writer);
	if ( rc < 0 ){
		fprintf( stderr, "Error at xmlTextWriterWriteFormatElement\n" );
		goto onErr3;
	}

    xmlFreeTextWriter(writer);

	*pData = buf;
	result = 0;
EXIT:
	return result;
onErr3:
    xmlFreeTextWriter(writer);
onErr2:
	xmlBufferFree( buf );	
onErr1:
	goto EXIT;
}


int CreatePageSettings( int dataSize, void **pData )
{
	int result = -1;
	int rc;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;

	if ( pData == NULL ) goto onErr1;

	buf = xmlBufferCreate();
	if ( buf == NULL ) {
		fprintf( stderr, "Error at xmlBufferCreate\n" );
		goto onErr1;
	}

	writer = xmlNewTextWriterMemory( buf, 0 );
	if ( writer == NULL ) {
		fprintf( stderr, "Error at xmlNewTextWriterMemory\n" );
		goto onErr2;
	}

	/* Start the document with the xml default for the version */
	rc = xmlTextWriterStartDocument( writer, NULL, "UTF-8", NULL );
	if ( rc < 0 ) {
		fprintf( stderr, "Error at xmlTextWriterStartDocument\n" );
		goto onErr3;
	}

	/* Start an element name "PAGEINFO" */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "PAGEINFO");
    if (rc < 0) {
		fprintf( stderr, "Error at xmlTextWriterStartElement\n" );
        goto onErr3;
    }

	rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "VALUE", "%d", dataSize);
	if ( rc < 0 ){
		fprintf( stderr, "Error at xmlTextWriterWriteFormatElement\n" );
		goto onErr3;
	}

	/* End element */
    rc = xmlTextWriterEndElement(writer);
	if ( rc < 0 ){
		fprintf( stderr, "Error at xmlTextWriterWriteFormatElement\n" );
		goto onErr3;
	}

    xmlFreeTextWriter(writer);

	*pData = buf;
	result = 0;
EXIT:
	return result;
onErr3:
    xmlFreeTextWriter(writer);
onErr2:
	xmlBufferFree( buf );	
onErr1:
	goto EXIT;	
}

int GetSettingsDataSize( void *pData )
{
	xmlBufferPtr buf = pData;
	int size = -1;

	if ( pData == NULL ) return size;

	size = buf->use;
	return size;	
}

char * GetSettingsStr( void *pData )
{
	xmlBufferPtr buf = pData;
	char * result = NULL;

	if ( pData == NULL ) goto onErr;

	result = (char *)buf->content;
onErr:
	return result;
}

void FreeSettings( void *pData )
{
	xmlBufferPtr buf = pData;

	if ( buf != NULL ) {
		xmlBufferFree( buf );
	}
}
