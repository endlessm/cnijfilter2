/*  Canon Inkjet Printer Driver for Linux
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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

//#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libxml/parser.h>	/* Ver.2.80 */
#include <string.h>

#include "keytext.h"

#define CNIJLGMON2_RESFILE "cnb_cnijlgmon2.res"

static const char* g_keytext_dir = NULL;
static KeyTextList *g_key_keytext_list = NULL;


static char* CheckFileExists(const char *dirname, const char *filename)
{
	char *full_filename;
	struct stat s;
	int status;
	int full_filename_len;

	full_filename_len = strlen (dirname) + 1 + strlen (filename) + 1;
	full_filename = (char*)malloc( full_filename_len );
	memset( full_filename, 0,  full_filename_len );

	strncpy(full_filename, dirname, full_filename_len); /* Ver.3.60 */
	full_filename[ full_filename_len -1 ] = '\0';
	strcat(full_filename, "/");
	strcat(full_filename, filename);

	status = stat(full_filename, &s);
	if( status == 0 && S_ISREG(s.st_mode) ) {
		return full_filename;
	}

	free(full_filename);
	return NULL;
}

static short NewStringList( STRINGLISTCOMP **strListComp, char *key, char *text )
{
	LPSTRINGLISTCOMP lpStringListComp;
	short len = 0;
	short result = -1;

	if ( strListComp == NULL ) goto Err1;
	if ( strListComp == NULL ) goto Err1;
	if ( (lpStringListComp = (LPSTRINGLISTCOMP)malloc(sizeof(STRINGLISTCOMP))) == NULL ) goto Err1;

	len = strlen(key) + 1;
	if ( (lpStringListComp->key = (char *)malloc(len)) == NULL ) goto Err1;
	strncpy( lpStringListComp->key, key, len );
	lpStringListComp->key[len-1] = '\0';

	len = strlen(text) + 1;
	if ( (lpStringListComp->text = (char *)malloc(len)) == NULL ) goto Err2;
	strncpy( lpStringListComp->text, text, len );
	lpStringListComp->text[len-1] = '\0';

	lpStringListComp->next = NULL;
	*strListComp = lpStringListComp;


	result = 0;
	return result;

	if ( lpStringListComp->text != NULL )  free( lpStringListComp->text );
Err2:
	if ( lpStringListComp->key != NULL )  free( lpStringListComp->key );
Err1:
	return result;
}

static short AppendStringList( STRINGLISTCOMP *root, STRINGLISTCOMP *append )
{
	short result = -1;
	STRINGLISTCOMP *current;

	if ( (root == NULL) || (append == NULL) ) goto Err;

	current = root;
	while( current->next != NULL ){
		current = current->next;
	}
	current->next = append;

	result = 0;
Err:
	return result;
}

static short FreeStringListComp( STRINGLISTCOMP *current )
{
	short result = -1;
	if ( current == NULL ) goto Err;

	if ( current->key != NULL ) {
		free( current->key );
		current->key = NULL;
	}
	if ( current->text != NULL ){
		free( current->text);
		current->text = NULL;
	}

	result = 0;
Err:
	return result;
}

static short FreeStringList( STRINGLISTCOMP *list )
{
	STRINGLISTCOMP *current, *prev = NULL;
	short  result = -1;

	if ( list == NULL ) goto Err;
	current = list;

	while( 1 ) {
		current = list;
		if ( current->next == NULL ) {
			if ( FreeStringListComp( current ) != 0 ) goto Err;
			break;
		}

		while( current->next != NULL ) {
			prev = current;
			current = current->next;
		}
		if ( FreeStringListComp( current ) != 0 ) goto Err;
		prev->next = NULL;
	}

	result = 0;
Err:
	return result;
}

static short LookupTextFromStringList( STRINGLISTCOMP *list, char *key, char **text )
{
	STRINGLISTCOMP *current = NULL;
	short result = -1;

	if ( list == NULL ) goto Err;
	current = list;

	while ( current != NULL ){
		if ( !strcmp( current->key, key ) ){
			*text = current->text;
			result = 0;
			break;
		}
		current = current->next;
	}

	result = 0;
Err:
	return result;	
}



//static GTree* CreateTree()
static STRINGLISTCOMP* CreateTree()
{
	STRINGLISTCOMP *list = NULL;
	
	if ( NewStringList( &list, "", "" ) != 0 ){
		list = NULL;
		goto Err;
	}
Err:
	return list;
}

static void AddKeyAndTextToTree(xmlNodePtr xmlnode, STRINGLISTCOMP* tree)
{
	char *key, *text;
	STRINGLISTCOMP *list = NULL;

	//if( !xmlnode->name || g_strcasecmp((const gchar *)xmlnode->name,"Item") != 0 )
	if( !xmlnode->name )
		return;

	key  = (char *)xmlGetProp(xmlnode,(const xmlChar *)"key");
	text = (char *)xmlNodeGetContent(xmlnode);

	if( key != NULL && text != NULL )
	{
		//g_tree_insert(tree, (gpointer)key, (gpointer)text);
		NewStringList( &list, key, text );
		AppendStringList( tree, list );
	}
}

static void ParseXMLDoc(xmlDocPtr doc, STRINGLISTCOMP* tree)
{
	xmlNodePtr node;

/*	for( node = doc->root->childs ; node != NULL ; node = node->next ) */
	for( node = doc->xmlRootNode->xmlChildrenNode ; node != NULL ; node = node->next )
	{
		AddKeyAndTextToTree(node, tree);
	}
}

static short ReadXMLFile(char *fname, STRINGLISTCOMP* tree)
{
	short result = -1;
	xmlDocPtr doc;

	if( (doc = xmlParseFile(fname)) == NULL ) goto Err;

	if( doc->xmlRootNode == NULL
	 || doc->xmlRootNode->name == NULL )
	 //|| g_strcasecmp((const gchar *)doc->xmlRootNode->name, "KeyTextList") != 0)
	{
		xmlFreeDoc(doc);
		goto Err;
	}

	ParseXMLDoc(doc, tree);
	xmlFreeDoc(doc);


	result = 0;
Err:
	return result;
}

void SetKeyTextDir(const char* dirname)
{
	if( g_keytext_dir != NULL ) {
		//free(g_keytext_dir);
		g_keytext_dir = NULL;
	}

	//g_keytext_dir = g_strdup(dirname);
	g_keytext_dir = dirname;
}

short LoadKeyTextList( void )
{
	KeyTextList* list;
	char* pathname = NULL;
	short  result = -1;

	if( !g_keytext_dir  ) goto Err1; 

	if( (pathname = CheckFileExists(g_keytext_dir, CNIJLGMON2_RESFILE)) == NULL ){
		if( (pathname = CheckFileExists("keytext", CNIJLGMON2_RESFILE)) == NULL )  goto Err1;
	}

	if ( (list = (KeyTextList*)malloc(sizeof(KeyTextList))) == NULL ) goto Err1;

	if( (list->tree = CreateTree()) == NULL ) goto Err2;

	if( ReadXMLFile(pathname, list->tree) != 0 ) goto Err3;

	g_key_keytext_list = list;

	result = 0;
	return result;
Err3:
	FreeStringList( list->tree );
Err2:
	free( list );
Err1:
	return result;
}

void FreeKeyTextList( void )
{
	if ( g_key_keytext_list == NULL ) return;

	FreeStringList( g_key_keytext_list->tree );
	free( g_key_keytext_list );
	g_key_keytext_list = NULL;
	
}

void FreeKeyTextDir( void )
{
	if ( g_keytext_dir != NULL ){
		g_keytext_dir = NULL;
	}
}

char* LookupText(char* key)
{
	char *text = NULL;

	if( g_key_keytext_list == NULL || g_key_keytext_list->tree == NULL || key == NULL ) return NULL;

	if ( LookupTextFromStringList( g_key_keytext_list->tree, key, &text ) != 0 ) return NULL;

	return (char*)dgettext(PACKAGE,text);
}

