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

#ifndef _CNIJLGMONN3_H_
#define _CNIJLGMONN3_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <libusb.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <cups/ppd.h>
#include "cnijcomif.h"

/*-------Magic Number------------------*/
#define CN_PRINTER_CLASS 		(7)
#define CN_WRITE_TIMEOUT 		(10 * 1000)
#define CN_SIGINT 				(SIGINT)
#define CN_SIGTERM 				(SIGTERM)
#define CN_START_JOBID 			("00000001")
#define CN_CMD_INSTALL_USB		("--installer_usb")
#define CN_CMD_INSTALL_NET		("--installer_net")
#define CN_CNCL_LIB_PATH		("libcnbpcnclapi%s.so")

#define CN_WRITE_SIZE 			(4096)
#define CN_READ_SIZE 			(4096)
#define CN_LIB_PATH_LEN			(512)
#define CN_SERIAL_NUM_LEN		(256)
#define CN_DEVICE_URI_LEN		(256)
#define CN_MODEL_ID_LEN			(64)
#define CN_STAT_MSG_LEN			(64)
#define CN_DEVICE_ID_LEN		(32)
#define CN_IVEC_SUPPORT_CODE_LENGTH	(10)
#define CN_IVEC_JOBID_LEN 		(9)

/*-------Response opreation id-------*/
#define CN_IVEC_START_RESPONSE 	(8)
#define CN_IVEC_END_RESPONSE 	(10)
#define CN_IVEC_VENDER_RESPONSE (22)
#define CN_IVEC_STATUS_RESPONSE (14)

/*-------Response detail id----------*/
#define CN_IVEC_NOT_START		(5)

/*-------Parameter Id----------------*/
#define CN_IVEC_STARTJOB 	(1)
#define CN_IVEC_VENDERSHIFT (2)
#define CN_IVEC_ENDJOB 		(3)
#define CN_IVEC_STATUS 		(4)
#define CN_IVEC_MEDIA_JAM	(5)

/*-------Status Id----------------*/
#define CN_IVEC_STATUS_IDLE						(1)
#define CN_IVEC_STATUS_PROCESSING				(2)
#define CN_IVEC_STATUS_STOPPED					(3)
#define CN_IVEC_STATUS_NOTREADY					(4)
#define CN_IVEC_STATUS_CANCELING				(6)

/*------Status Detail Id----------*/
#define CN_IVEC_STATUS_DETAIL_MEDIA_JAM			(2)
#define CN_IVEC_STATUS_DETAIL_DOOR_OPEN			(3)
#define CN_IVEC_STATUS_DETAIL_MEDIA_EMPTY		(4)
#define CN_IVEC_STATUS_DETAIL_BUSYING			(6)
#define CN_IVEC_ITEM_NONE						(65535)

/*-------Error Number----------------*/
#define CN_LGMON_OK								(0)
#define CN_LGMON_ERROR							(-1)
#define CN_LGMON_CMD_ERROR						(-2)
#define CN_LGMON_PDD_FILE_ERROR					(-3)
#define CN_LGMON_DYNAMID_LINK_ERROR				(-4)
#define CN_LGMON_OPEN_ERROR						(-5)
#define CN_LGMON_RESPONSE_ERROR					(-6)
#define CN_LGMON_WRITEDATA_ERROR				(-7)
#define CN_LGMON_BUSY							(-8)
#define CN_LGMON_OTHER_ERROR					(-65534)


void CNIF_SetSignal(int sign);
void CNIF_SigCatch(int sign);
int CNIF_GetResponse(CNIF_INFO *if_info, char *jobId, int *response_detail);

#endif

