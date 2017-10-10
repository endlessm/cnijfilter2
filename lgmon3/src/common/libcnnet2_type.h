/*
 *  Canon Inkjet Printer Driver for Linux
 *  Copyright CANON INC. 2014-2016
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

#ifndef __INC_CNNET2_TYPE__
#define __INC_CNNET2_TYPE__

typedef enum {
  CNNET2_ERROR_CODE_SUCCESS        =  0,/*成功*/
  CNNET2_ERROR_CODE_PARAM          = -1,/*パラメーターエラー*/
  CNNET2_ERROR_CODE_MEMORY         = -2,/*メモリーエラー*/
  CNNET2_ERROR_CODE_OTHER          = -3,/*その他のエラー*/
  CNNET2_ERROR_CODE_SOCKET         = -4,/*ソケットエラー*/
  CNNET2_ERROR_CODE_PACKET         = -5,/*パケットデータエラー*/
  CNNET2_ERROR_CODE_RECV_TIMEOUT   = -6,/*受信タイムアウトエラー*/
  CNNET2_ERROR_CODE_NIC            = -7,/*Nic情報取得エラー*/
} CNNET2_ERROR_CODE;

typedef enum {
  CNNET2_SETTING_FLAG_GET_PRINTER_INFO_TIMEOUT_MILLIS = 1, /*ユニキャスト時の返信待ちタイムアウトの指定(ソケット指定タイムアウト時間)*/
  CNNET2_SETTING_FLAG_IS_INCLUDE_NOT_GET_UNICAST      = 3, /*ユニキャスト通信できないものを結果に含めるかの指定*/
  CNNET2_SETTING_FLAG_DISCOVER_PRINTER_TIMEOUT_MILLIS,     /*検索時の返信待ちタイムアウトの指定*/
  CNNET2_SETTING_FLAG_COMM_TIMEOUT_RETRY_COUNT,            /*通信タイムアウト時のリトライ回数(実行回数)*/
  CNNET2_SETTING_FLAG_COMM_TIMEOUT_RETRY_WAIT_MILLIS,      /*通信タイムアウトリトライ時の待ち時間*/
  CNNET2_SETTING_FLAG_DISCOVER_PACKET_COUNT,               /*検索パケット送信回数*/
  CNNET2_SETTING_FLAG_DISCOVER_PACKET_WAIT_MILLIS,         /*検索パケット複数送信時の待ち時間*/
} CNNET2_SETTING_FLAGS;

#ifndef __cplusplus
typedef char bool;
#endif

typedef struct {
  int nicIndex_;
  char ipAddressStr_[46];
  char MacAddressStr_[13];
  char serialNumberStr_[65];
  char modelName_[256];
  int deviceType_;
  int currentConnectMode_;//canWiredLANWirelessCurrentConnectMode :INTEGER wiredLAN(3)/ssdot11CablelessSetup(4)/wirelessdot11Infra(5)/wirelessdot11APmode(6)/wirelessdot11WifiDirect(7)/wirelessdot11WifiDirectService(8)
  char deviceId_[1024];
  char bonjourName_[256];
  bool isUnicast_;
  bool isSameSegment_;
} tagSearchPrinterInfo;

#define CNNET2_DEFAULT_GET_PRINTER_INFO_TIMEOUT_MILLIS 2000
#define CNNET2_DEFAULT_TTL                             1/*1:同一セグメント内*/
#define CNNET2_DEFAULT_IS_INCLUDE_NOT_GET_UNICAST      0/*0:false 0!:true*/
#define CNNET2_DEFAULT_DISCOVER_PRINTER_TIMEOUT_MILLIS 10000
#define CNNET2_DEFAULT_COMM_TIMEOUT_RETRY_COUNT        2
#define CNNET2_DEFAULT_COMM_TIMEOUT_RETRY_WAIT_MILLIS  0
#define CNNET2_DEFAULT_DISCOVER_PACKET_COUNT           2
#define CNNET2_DEFAULT_DISCOVER_PACKET_WAIT_MILLIS     0

#define CNNET2_MIN_GET_PRINTER_INFO_TIMEOUT_MILLIS     0
#define CNNET2_MIN_TTL                                 1
#define CNNET2_MIN_DISCOVER_PRINTER_TIMEOUT_MILLIS     0
#define CNNET2_MIN_COMM_TIMEOUT_RETRY_COUNT            0
#define CNNET2_MIN_COMM_TIMEOUT_RETRY_WAIT_MILLIS      0
#define CNNET2_MIN_DISCOVER_PACKET_COUNT               0
#define CNNET2_MIN_DISCOVER_PACKET_WAIT_MILLIS         0
#define CNNET2_MAX_GET_PRINTER_INFO_TIMEOUT_MILLIS     2147483/*INT_MAX/1000*/
#define CNNET2_MAX_TTL                                 127/*SCHAR_MAX*/
#define CNNET2_MAX_DISCOVER_PRINTER_TIMEOUT_MILLIS     2147483647/*INT_MAX*/
#define CNNET2_MAX_COMM_TIMEOUT_RETRY_COUNT            2147483647/*INT_MAX*/
#define CNNET2_MAX_COMM_TIMEOUT_RETRY_WAIT_MILLIS      2147483647/*INT_MAX*/
#define CNNET2_MAX_DISCOVER_PACKET_COUNT               2147483647/*INT_MAX*/
#define CNNET2_MAX_DISCOVER_PACKET_WAIT_MILLIS         2147483647/*INT_MAX*/

#endif
