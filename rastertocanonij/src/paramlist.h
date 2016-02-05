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

#ifndef _paramlist_
#define _paramlist_


#define	MAX_KEY_LEN		255
#define	MAX_VALUE_LEN	255


typedef struct param_list_s ParamList;

struct param_list_s {
  ParamList *next;
  char *key;
  char *value;
  int value_size;
};

ParamList *param_list_find(ParamList *pl, const char *key);
void param_list_delete(ParamList **root, const char *key);
void param_list_add_multi(ParamList **root,
		const char *key, const char *value, int value_size, int multi);
void param_list_free(ParamList *pl);
int param_list_num(ParamList *pl);
void param_list_print(ParamList *pl);
char *ref_value_from_list( ParamList *root, const char *key );

#define param_list_add(root, key, value, value_size) \
                param_list_add_multi(root, key, value, value_size, 0);

#endif	// _paramlist_

