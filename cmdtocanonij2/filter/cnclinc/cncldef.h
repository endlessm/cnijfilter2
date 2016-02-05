/*
 *  Canon Inkjet Printer Driver for Linux
 *  Copyright CANON INC. 2001-2015
 * *
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

#ifndef __CNCLDEF_H__
#define __CNCLDEF_H__

#if defined(__cplusplus)
extern "C"
{
#endif	/* __cplusplus */



/*
 *	Defines
 */

/* Model ID */
enum {
	CND_BJC35V2,
	CND_BJC50V,
	CND_BJC80V,
	CND_BJM70,
	CND_BJC210J,
	CND_BJC240J,
	CND_BJC250J,
	CND_BJC610JW,
	CND_BJC620JW,
	CND_BJC410J,
	CND_BJC420J,
	CND_BJC430J,
	CND_BJC440J,
	CND_BJC455J,
	CND_BJC465J,
	CND_BJF100,
	CND_BJF200,
	CND_BJF300,
	CND_BJF600,
	CND_BJF610,
	CND_BJC50,
	CND_BJC80,
	CND_BJC85,
	CND_BJC210,
	CND_BJC240,
	CND_BJC250,
	CND_BJC610,
	CND_BJC620,
	CND_BJC4100,
	CND_BJC4200,
	CND_BJC4300,
	CND_BJC4400,
	CND_BJC4550,
	CND_BJC4650,
	CND_BJC1000,
	CND_BJC2000,
	CND_BJC3000,
	CND_BJC6000,
	CND_BJC6100,
	CND_BJC210SP,
	CND_BJC255SP,
	CND_BJC265SP,
	CND_BJC4200SP,
	CND_BJC4310SP,
	CND_BJC1000SP,
	CND_BJC2000SP,
	CND_BJF850,
	CND_BJC8200,
	CND_BJH200,
	CND_LR1,
	CND_BJF210,
	CND_BJC2100,
	CND_BJC2100SP,
	CND_BJF620,
	CND_BJC6200,
	CND_BJM40,
	CND_BJC55,

	CND_S800,
	CND_F870,
	CND_F860,
	CND_F870PD,

	CND_NEW_MODELS,
	CND_PRINTER_MODELS_MAX,
} ;


/* MediaType */
enum {
	CND_MEDIA_PLAIN,			/* Plain Paper */
	CND_MEDIA_OHP,				/* OHP */
	CND_MEDIA_BPF,				/* Back Print Film (BF-102) */
	CND_MEDIA_BJCLOTH,			/* BJ Cloth (FS-101) */
	CND_MEDIA_HIGLOSSFILM,		/* High Glossy Film (HG-101) */
	CND_MEDIA_POSTCARD,			/* Postcard */
	CND_MEDIA_ENVELOPE,			/* Envelope */
	CND_MEDIA_GLOSSYPOSTCARD,	/* Glossy Postcard (KH-101) */
	CND_MEDIA_HIRES,			/* High Resolution Paper (HR-101) */
	CND_MEDIA_TSHIRT,			/* T-shirt Transfer */
	CND_MEDIA_THICK,			/* Thick Paper */
	CND_MEDIA_HIGLOSSPHTFILM,	/* High Gloss Photo Film (HG-201) */
	CND_MEDIA_GLOSSYPHOTO,		/* Glossy Photo Paper (GP-201/301) */
	CND_MEDIA_GLOSSYPHOTOCARD,	/* Glossy Photo Cards (FM-101) */
	CND_MEDIA_PROPHOTO,			/* Professional Photo Paper */
	
	CND_MEDIA_INKJET_POST,		/* Inkjet post card */
	
	CND_MEDIA_OTHER,			/* Other Media */
	CND_MEDIA_NA,				/* Not Applicable */
} ;


/* CartridgeType */
enum {
	CND_BJINK_BC10,
	CND_BJINK_BC11,
	CND_BJINK_BC11E,
	CND_BJINK_BC12E,
	CND_BJINK_BC20,
	CND_BJINK_BC21,
	CND_BJINK_BC22,
	CND_BJINK_BC21E,
	CND_BJINK_BC22E,
	CND_BJINK_BC02,
	CND_BJINK_BC05,
	CND_BJINK_BC06,
	CND_BJINK_BC30,
	CND_BJINK_BC30BC31,
	CND_BJINK_BC31BC32,
	CND_BJINK_BC33,
	CND_BJINK_BC34,
	CND_BJINK_BC50,
	CND_BJINK_BJI201,
	CND_BJINK_NOHEAD,		/* No head */
	CND_BJINK_NA,			/* No Applicable */
	CND_BJINK_MAX,			/* Max indicator */
} ;

/*
	If you set cartridge parameter to CNCLLIB,
		use this parameter with printer after BJF870/BJF860/BJF850.
	( If you set cartridge parameter to CNCLUILIB
		use CND_CARTRIDGE_* parameter. )
*/
/* CartridgeType (InkType) */
#define	CND_BJINK_COLOR		(1001)	/* Color Cartridge */
#define	CND_BJINK_BK		(1002)	/* Black Cartridge */
#define	CND_BJINK_PHOTO		(1003)	/* Photo Cartridge */
#define	CND_BJINK_BK_COLOR	(1004)	/* Black and Color Cartridge (2-Cartridge model) */
#define	CND_BJINK_BK_PHOTO	(1005)	/* Black and Photo Cartridge (2-Cartridge model) */
#define	CND_BJINK_PHOTO_COLOR	(1006)	/* Photo and Color Cartridge (2-Cartridge model) */

/* PaperSize */
enum {
	CND_SIZE_A5,			/* A5 (148mm x 210mm) */
	CND_SIZE_A5_L,			/* A5-L (210mm x 148mm) */
	CND_SIZE_A4,			/* A4 (210mm x 297mm) */
	CND_SIZE_A4_L,			/* A4-L (297mm x 210mm) */
	CND_SIZE_A3,			/* A3 (297mm x 420mm) */
	CND_SIZE_A3_L,			/* A3-L (420mm x 297mm) */
	CND_SIZE_A2,			/* A2 (420mm x 594mm) */
	CND_SIZE_B5,			/* B5 (182mm x 257mm) */
	CND_SIZE_B5_L,			/* B5-L (257mm x 182mm) */
	CND_SIZE_B4,			/* B4 (257mm x 364mm) */
	CND_SIZE_B4_L,			/* B4-L (364mm x 257mm) */
	CND_SIZE_B3,			/* B3 (364mm x 515mm) */
	CND_SIZE_LETTER,		/* Letter (8.5x 11inch) */
	CND_SIZE_LETTER_L,		/* Letter-L (11inch x 8.5inch) */
	CND_SIZE_LEGAL,			/* Legal (8.5inch x 14inch) */
	CND_SIZE_LEGAL_L,		/* Legal-L (14inch x 8.5inch) */
	CND_SIZE_LEDGER,		/* Ledger  (11inch x 17inch) */
	CND_SIZE_LEDGER_L,		/* Ledger-L (17inch x 11inch) */
	CND_SIZE_CSIZE,			/* C size (17 x 22inch) */
	CND_SIZE_POST,			/* Postcard (10mm 0x 148mm) */
	CND_SIZE_POST_L,		/* Postcard-L (14mm 8x 100mm) */
	CND_SIZE_ENV_10,		/* #10 Envelope (9.5inch x 4.125inch) */
	CND_SIZE_ENV_DL,		/* DL Envelope (220mm x 110mm) */
	CND_SIZE_FOOLS,			/* Foolscap (13.5inch x 17inch) */
	CND_SIZE_FAN8_11,		/* Fanfold (8.5inch x 11inch) */
	CND_SIZE_FAN9_11,		/* Fanfold (9.5inch x 11inch) */
	CND_SIZE_FAN10_11,		/* Fanfold (10inch x 11inch) */
	CND_SIZE_FAN10_12,		/* Fanfold (10inch x 12inch) */
	CND_SIZE_FAN12_8,		/* Fanfold (12inch x 8.5inch) */
	CND_SIZE_FAN14_11,		/* Fanfold (14.5inch x 11inch) */
	CND_SIZE_FAN15_11,		/* Fanfold (15inch x 11inch) */
	CND_SIZE_FAN15_12,		/* Fanfold (15inch x 12inch) */
	CND_SIZE_FAN_US,		/* Fanfold (14.875inch x 11inch) */
	CND_SIZE_ENV_J4,		/* Envelope-J4 */
	CND_SIZE_ENV_J6,		/* Envelope-J6 */
	CND_SIZE_LETTER_BLEED,	/* Letter full-bleed */
	CND_SIZE_A4_BLEED,		/* A4 full-bleed */
	CND_SIZE_A3_PLUS,		/* A3 plus */
	CND_SIZE_PHOTOCARD,		/* Photocard (FM-101) */
	CND_SIZE_USER,			/* User define */
	CND_SIZE_NA,			/* Not Applicable */
	CND_SIZE_MAX,			/* Max indicator */
} ;


/* MediaSupply */
enum {
	CND_SUPPLY_MANUAL,			/* Manual Feed */
	CND_SUPPLY_ASF,				/* Auto Sheet Feeder */
	CND_SUPPLY_ASF2,			/* Auto Sheet Feeder 2 */
	CND_SUPPLY_CASSETTE,		/* Cassette 1 */
	CND_SUPPLY_CASSETTE2,		/* Cassette 2 */
	CND_SUPPLY_CONSECUTIVE,		/* Consecutive Feeder */
	CND_SUPPLY_CONSECUTIVE2,	/* Consecutive Feeder 2 */
	CND_SUPPLY_TRACTOR,			/* Tractor Feeder */
	CND_SUPPLY_CASSETTE_02,		/* Cassette iP4100 type */
	CND_SUPPLY_PRINTER_SW_SELECT,	/* Paper Feed Switch */
	CND_SUPPLY_CASSETTE_04,		/* Front Feeder */
	CND_SUPPLY_FRONT_FOR_PLAIN,		/* Front for plain(original) */
	CND_SUPPLY_AUTO_CONTINUOUS_FEED,/* Auto Continuous Media Supply */
	CND_SUPPLY_AUTO_SELECT,			/* Auto Select */
	CND_SUPPLY_CASSETTE_06,		/* Front Feeder MG2100 type */
	CND_SUPPLY_CASSETTE_AUTO,	/* Auto Casette Feed */
	CND_SUPPLY_NA,				/* Not Applicable */
	CND_SUPPLY_MAX,				/* Max indicator */
} ;


/* Grayscale */
enum {
	CND_BJGRAYSCALE_ON,		/* Grayscale ON */
	CND_BJGRAYSCALE_OFF,	/* Grayscale OFF */
	CND_BJGRAYSCALE_GRAY,	/* Grayscale is Grayout */
	CND_BJGRAYSCALE_NA,		/* Not Applicable */
	CND_BJGRAYSCALE_MAX,	/* Max indicator */
} ;


/* Banner */
enum {
	CND_BANNER_ON,			/* Banner ON */
	CND_BANNER_OFF,			/* Banner OFF */
	CND_BANNER_NA,			/* Not Applicble */
	CND_BANNER_MAX,			/* Max indicator */
} ;


/* Smoothing (printer) */
enum {
	CND_SMOOTH_ON,			/* Smoothing ON */
	CND_SMOOTH_OFF,			/* Smoothing OFF */
	CND_SMOOTH_GRAY,		/* Smoothing is Gray-out */
	CND_SMOOTH_NA,			/* Not Applicble */
	CND_SMOOTH_MAX,			/* Max indicator */
} ;


/* Print quality */
enum {
	CND_QUALITY_HIGH,		/* Quality position HIGH */
	CND_QUALITY_NORMAL,		/* Quality position NORMAL */
	CND_QUALITY_DRAFT,		/* Quality position DRAFT */
	CND_QUALITY_NA,			/* Quality position Not Applicable */
	CND_QUALITY_MAX,		/* Max indicator */
} ;

/* Absolute quality */
#define CND_QUALITY_ABSOLUTE_SPECIAL	-6		/* Quality position Absolute Special */
#define CND_QUALITY_ABSOLUTE_HQ			-5		/* Quality position Absolute High Quality */
#define CND_QUALITY_ABSOLUTE_MQ			-4		/* Quality position Absolute Medium Quality */
#define CND_QUALITY_ABSOLUTE_NORMAL		-3		/* Quality position Absolute Normal */
#define CND_QUALITY_ABSOLUTE_HS			-2		/* Quality position Absolute High Speed */
#define CND_QUALITY_ABSOLUTE_NA			-7		/* Quality position Absolute High Speed */


/* Halftone */
enum {
	CND_BIN_DITHER,			/* Dither */
	CND_BIN_ED,				/* ED */
	CND_BIN_NA,				/* Not Applicable */
	CND_BIN_MAX,			/* Max indicator */
} ;


/* Intent */
enum {
	CND_INTENT_PHOTO,		/* Photo */
	CND_INTENT_GRAPHICS,	/* Graphics */
	CND_INTENT_MATCHING,	/* Matching */
	CND_INTENT_NATURAL,		/* Natural */
	CND_INTENT_NA,			/* Not Applicable */
	CND_INTENT_MAX,			/* Max indicator */
} ;


/* Gamma */
enum {
	CND_GAMMA_DEFAULT,		/* Default Gamma */
	
	CND_GAMMA_10,			/* Gamma 1.0 */
	
	CND_GAMMA_14,			/* Gamma 1.4 */
	CND_GAMMA_18,			/* Gamma 1.8 */
	CND_GAMMA_22,			/* Gamma 2.2 */
	CND_GAMMA_NA,			/* Not Applicable */
	CND_GAMMA_MAX,			/* Max indicator */
} ;

/* Margin Type */
enum {
	CND_MARGIN_NORMAL,
	CND_MARGIN_MINUS,
	CND_MARGIN_NA,
	CND_MARGIN_MAX,
};

/* Paper Gap */
enum {
	CND_PGAP_CMD_AUTO,
	CND_PGAP_CMD_WIDE,
	CND_PGAP_CMD_NARROW,
	CND_PGAP_CMD_NA,
	CND_PGAP_CMD_MAX,
};

/* Duplex Printing */
enum {
	CND_DUPLEX_OFF,
	CND_DUPLEX_AUTO,
	CND_DUPLEX_NA,
	CND_DUPLEX_MAX,
};

/*
 * Source type values for CNCLData.sourcetype
 */
#define CND_ST_RGB24		0		/* 24-bits RGB data */

/*
 * White Line values for CNCLData.WhiteLine
 */
#define CND_WL_WHITE		0		/* White line */
#define CND_WL_NOTWHITE		1		/* Not white line */
#define CND_WL_UNDEFINE		2		/* Can not determine whether white or not */

/*
 * Time Info values for CNCL_Time.TimeInfo
 */
#define CND_TIME_CORRECT	0		/* Passing the correct time */
#define CND_TIME_UNDEFINE	1		/* Can not determine the current time */



/*
 *   Error Code ( CNCLErr types)
 */
#define CNCL_OK				0			/* Success */
#define CNCL_ERROR			-1			/* Failed */
#define CNCL_NOMEM			-2			/* No memory */
#define CNCL_NOLIB			-3			/* No Library file */
#define CNCL_LIBERR			-4			/* Library internal error. see ierr */
#define CNCL_PARAMERR		-5			/* Invalid parameter is set */
#define CNCL_LIBERR2			-6		/* Library internal error. see ierr2 */


/*
 *   Image Rotate
 */
#define CNCL_IMAGEROTATE_UNDEFINED	(0)
#define CNCL_IMAGEROTATE_0			(1)
#define CNCL_IMAGEROTATE_90			(2)
#define CNCL_IMAGEROTATE_180		(3)
#define CNCL_IMAGEROTATE_270		(4)


/* 2009/04/01 */
#define CNCL_UTILS_NOTCOMPLETE		1		/* Success.Not match. */

#if defined(__cplusplus)
}
#endif	/* __cplusplus */

#endif	/* __CNCLDEF_H__ */
