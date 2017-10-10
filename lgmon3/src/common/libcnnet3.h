/*

	linux copyright.

*/

#ifndef __INC_CNNET3_HEADER__
#define __INC_CNNET3_HEADER__

#include "libcnnet3_type.h"

#ifdef __cplusplus
extern "C" {
#endif

HCNNET3 CNNET3_Open();
CNNET3_ERROR CNNET3_Close(HCNNET3 clientHandle);

CNNET3_ERROR CNNET3_SetIF(HCNNET3 handle, CNNET3_IFTYPE ifType);
CNNET3_ERROR CNNET3_SetIP(HCNNET3 handle, const char* address);
CNNET3_ERROR CNNET3_SetPrinterName(HCNNET3 handle, const char* printerName);
CNNET3_ERROR CNNET3_SetURL(HCNNET3 handle, CNNET3_URL url);
CNNET3_ERROR CNNET3_SetTimeout(HCNNET3 handle, CNNET3_TOSETTING toSetting, unsigned int second);
CNNET3_ERROR CNNET3_SetMasterPortOption(HCNNET3 handle, CNNET3_OPTIONTYPE optionType);

CNNET3_ERROR CNNET3_Write(HCNNET3 handle, unsigned char* sendBuffer, unsigned long bufferSize, CNNET3_BOOL needContinue);
CNNET3_ERROR CNNET3_Read(HCNNET3 handle, unsigned char* recvBuffer, unsigned long* bufferSize, CNNET3_BOOL* needContinue);

CNNET3_ERROR CNNET3_SetEventType(HCNNET3 handle, CNNET3_EVENTTYPE eventtype);
CNNET3_ERROR CNNET3_SetCommandType(HCNNET3 handle, CNNET3_COMMANDTYPE commandtype);
CNNET3_ERROR CNNET3_SetEventBufferSize(HCNNET3 handle, unsigned int size);
CNNET3_ERROR CNNET3_Send(HCNNET3 handle,unsigned char* sendBuffer,unsigned long bufferSize,unsigned long* sentSize);

#ifdef __cplusplus
}
#endif

#endif
