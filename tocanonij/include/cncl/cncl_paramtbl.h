/*
 *  Canon Inkjet Printer Driver for Linux
 *  Copyright CANON INC. 2014
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

#ifndef __CNCL_PARAMTBL_H__
#define __CNCL_PARAMTBL_H__

typedef struct {
    char *optName;
    long optNum;
}MapTbl, *MapTblPtr;

static const MapTbl mediatypeTbl[] = {
	{ "plain", CNCL_PSET_MEDIA_PLAIN },
	{ "glossygold", CNCL_PSET_MEDIA_PHOTO_PAPER_PLUS_GLOSSY_II },
	{ "photopaperpro2", CNCL_PSET_MEDIA_PHOTO_PAPER_PRO_II },
	{ "proplatinum", CNCL_PSET_MEDIA_PHOTO_PAPER_PRO_PLATINUM },
	{ "photopaperpro", CNCL_PSET_MEDIA_PROPHOTO },
	{ "superphoto", CNCL_PSET_MEDIA_SUPER_PHOTO_PAPER },
	{ "luster", CNCL_PSET_MEDIA_PHOTO_PAPER_PRO_LUSTER },
	{ "semigloss", CNCL_PSET_MEDIA_PHOTO_PAPER_SG },
	{ "glossypaper", CNCL_PSET_MEDIA_GLOSSY_PAPER },
	{ "matte", CNCL_PSET_MEDIA_MATTE_PAPER },
	{ "photopaper", CNCL_PSET_MEDIA_PHOTOPAPER },
	{ "envelope", CNCL_PSET_MEDIA_ENVELOPE },
	{ "ijpostcard", CNCL_PSET_MEDIA_INKJET_HAGAKI },
	{ "postcard", CNCL_PSET_MEDIA_HAGAKI },
	{ "label", CNCL_PSET_MEDIA_LABEL },
	{ "highres", CNCL_PSET_MEDIA_HIGHRES },
	{ "photo", CNCL_PSET_MEDIA_OTHER_PHOTO_PAPER },
	{ NULL, -1 },
};

static const MapTbl papersizeTbl[] = {
	{ "Letter", CNCL_PSET_SIZE_LETTER },
	{ "Letter.bl", CNCL_PSET_SIZE_LETTER },
	{ "legal", CNCL_PSET_SIZE_LEGAL },
	{ "executive", CNCL_PSET_SIZE_EXECUTIVE },
	{ "a6", CNCL_PSET_SIZE_A6 },
	{ "A5", CNCL_PSET_SIZE_A5 },
	{ "A4", CNCL_PSET_SIZE_A4 },
	{ "A4.bl", CNCL_PSET_SIZE_A4 },
	{ "A3", CNCL_PSET_SIZE_A3 },
	{ "A3.bl", CNCL_PSET_SIZE_A3 },
	{ "a3plus", CNCL_PSET_SIZE_A3_PLUS },
	{ "a3plus.bl", CNCL_PSET_SIZE_A3_PLUS },
	{ "B5", CNCL_PSET_SIZE_B5 },
	{ "B4", CNCL_PSET_SIZE_B4 },
	{ "oficio", CNCL_PSET_SIZE_OFICIO },
	{ "boficio", CNCL_PSET_SIZE_B_OFICIO },
	{ "moficio", CNCL_PSET_SIZE_M_OFICIO },
	{ "foolscap", CNCL_PSET_SIZE_FOOLSCAP },
	{ "legalindia", CNCL_PSET_SIZE_LEGAL_INDIA },
	{ "4x6", CNCL_PSET_SIZE_4X6 },
	{ "4x6.bl", CNCL_PSET_SIZE_4X6 },
	{ "5x7", CNCL_PSET_SIZE_5X7 },
	{ "5x7.bl", CNCL_PSET_SIZE_5X7 },
	{ "8x10", CNCL_PSET_SIZE_6GIRI },
	{ "8x10.bl", CNCL_PSET_SIZE_6GIRI },
	{ "10x12", CNCL_PSET_SIZE_4GIRI },
	{ "10x12.bl", CNCL_PSET_SIZE_4GIRI },
	{ "l", CNCL_PSET_SIZE_L },
	{ "l.bl", CNCL_PSET_SIZE_L },
	{ "2l", CNCL_PSET_SIZE_2L },
	{ "2l.bl", CNCL_PSET_SIZE_2L },
	{ "Postcard", CNCL_PSET_SIZE_POST },
	{ "Postcard.bl", CNCL_PSET_SIZE_POST },
	{ "envelop10p", CNCL_PSET_SIZE_ENV_10 },
	{ "envelopdlp", CNCL_PSET_SIZE_ENV_DL },
	{ "businesscard", CNCL_PSET_SIZE_BUSINESSCARD },
	{ "businesscard.bl", CNCL_PSET_SIZE_BUSINESSCARD },
	{ "square127", CNCL_PSET_SIZE_SQUARE_127 },
	{ "square127.bl", CNCL_PSET_SIZE_SQUARE_127 },
	{ NULL, -1 },
};

static const MapTbl colormodeTbl[] = {
	{ "true", CNCL_PSET_COLORMODE_MONO },
	{ "false", CNCL_PSET_COLORMODE_COLOR },
	{ NULL, -1 },
};

static const MapTbl duplexprintTbl[] = {
	{ "none", CNCL_PSET_DUPLEX_OFF },
	{ "duplextumble", CNCL_PSET_DUPLEX_ON },
	{ "duplexnotumble", CNCL_PSET_DUPLEX_ON },
	{ NULL, -1 },
};

	
#endif

