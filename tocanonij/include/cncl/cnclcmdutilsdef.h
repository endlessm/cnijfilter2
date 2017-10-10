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

#ifndef __CNCLCMDUTILSDEF_H__
#define __CNCLCMDUTILSDEF_H__

/*
 * Printer Setting Parameter
 */
 
/* mediatype */
#define CNCL_PSET_MEDIA_PLAIN (1)
#define CNCL_PSET_MEDIA_PHOTO_PAPER_PLUS_GLOSSY_II (2)
#define CNCL_PSET_MEDIA_PHOTO_PAPER_PRO_II (3)
#define CNCL_PSET_MEDIA_PHOTO_PAPER_PRO_PLATINUM (5)
#define CNCL_PSET_MEDIA_PROPHOTO (6)
#define CNCL_PSET_MEDIA_SUPER_PHOTO_PAPER (7)
#define CNCL_PSET_MEDIA_PHOTO_PAPER_PRO_LUSTER (8)
#define CNCL_PSET_MEDIA_PHOTO_PAPER_SG (9)
#define CNCL_PSET_MEDIA_GLOSSY_PAPER (10)
#define CNCL_PSET_MEDIA_MATTE_PAPER (11)
#define CNCL_PSET_MEDIA_PHOTOPAPER (12)
#define CNCL_PSET_MEDIA_ENVELOPE (17)
#define CNCL_PSET_MEDIA_INKJET_HAGAKI (13)
#define CNCL_PSET_MEDIA_HAGAKI (14)
#define CNCL_PSET_MEDIA_LABEL (18)
#define CNCL_PSET_MEDIA_HIGHRES (15)
#define CNCL_PSET_MEDIA_OTHER_PHOTO_PAPER (16)

/* pagesize */
#define CNCL_PSET_SIZE_LETTER (1)
#define CNCL_PSET_SIZE_LEGAL (2)
#define CNCL_PSET_SIZE_EXECUTIVE (22)
#define CNCL_PSET_SIZE_A6 (23)
#define CNCL_PSET_SIZE_A5 (3)
#define CNCL_PSET_SIZE_A4 (4)
#define CNCL_PSET_SIZE_A3 (5)
#define CNCL_PSET_SIZE_A3_PLUS (6)
#define CNCL_PSET_SIZE_B5 (7)
#define CNCL_PSET_SIZE_B4 (8)
#define CNCL_PSET_SIZE_OFICIO (24)
#define CNCL_PSET_SIZE_B_OFICIO (25)
#define CNCL_PSET_SIZE_M_OFICIO (26)
#define CNCL_PSET_SIZE_FOOLSCAP (27)
#define CNCL_PSET_SIZE_LEGAL_INDIA (28)
#define CNCL_PSET_SIZE_4X6 (9)
#define CNCL_PSET_SIZE_5X7 (10)
#define CNCL_PSET_SIZE_6GIRI (11)
#define CNCL_PSET_SIZE_4GIRI (12)
#define CNCL_PSET_SIZE_L (14)
#define CNCL_PSET_SIZE_2L (15)
#define CNCL_PSET_SIZE_POST (16)
#define CNCL_PSET_SIZE_ENV_10 (20)
#define CNCL_PSET_SIZE_ENV_DL (21)
#define CNCL_PSET_SIZE_BUSINESSCARD (18)
#define CNCL_PSET_SIZE_SQUARE_127 (29)

/* colormode */
#define CNCL_PSET_COLORMODE_MONO (2)
#define CNCL_PSET_COLORMODE_COLOR (1)

/* duplex */
#define CNCL_PSET_DUPLEX_OFF (1)
#define CNCL_PSET_DUPLEX_ON (2)
#define CNCL_PSET_DUPLEX_ON (2)

/* etc */
#define CNCL_PSET_BORDERLESS_OFF (1)
#define CNCL_PSET_BORDERLESS_ON (2)
#define CNCL_PSET_NEXTPAGE_OFF (1)
#define CNCL_PSET_NEXTPAGE_ON (2)

	
#endif

