#include <stdio.h>
#include <unistd.h>
#include "HCNetSDK.h"
#include "hikvision.h"
#include <string.h>
#include "stdlib.h"

void errorWrap(Error* error, const char* wrap) {
    if (error == NULL) {
        return;
    }

    int errCode = NET_DVR_GetLastError();

    char* errMsg = NET_DVR_GetErrorMsg(&errCode);

    error->errCode = errCode;
    error->errMsg = errMsg;
    error->errWrap = wrap;
}

int Login(LoginParams* loginParams, DevInfo* devInfo, Error *error) {
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};

    struLoginInfo.bUseAsynLogin = false;
    struLoginInfo.wPort         = loginParams->port;
    memcpy(struLoginInfo.sDeviceAddress, loginParams->ip, NET_DVR_DEV_ADDRESS_MAX_LEN);
    memcpy(struLoginInfo.sUserName, loginParams->username, NAME_LEN);
    memcpy(struLoginInfo.sPassword, loginParams->password, NAME_LEN);


    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

    int lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0) {
        errorWrap(error, "NET_DVR_Login_V40");

        return 0;
    }

    if (devInfo != NULL) {
        devInfo->byZeroChanNum  = struDeviceInfoV40.struDeviceV30.byZeroChanNum;
        devInfo->byStartChan    = struDeviceInfoV40.struDeviceV30.byStartChan;
        devInfo->byChanNum      = struDeviceInfoV40.struDeviceV30.byChanNum;
        devInfo->byStartDChan   = struDeviceInfoV40.struDeviceV30.byStartDChan;
        devInfo->byDChanNum     = struDeviceInfoV40.struDeviceV30.byHighDChanNum * 256 + struDeviceInfoV40.struDeviceV30.byIPChanNum;
        devInfo->sSerialNumber  = (char *)struDeviceInfoV40.struDeviceV30.sSerialNumber;
        devInfo->byDiskNum      = struDeviceInfoV40.struDeviceV30.byDiskNum;
        devInfo->byDVRType      = struDeviceInfoV40.struDeviceV30.byDVRType;
    }

    return lUserID;
}

void Init(Error* error) {
    if (!NET_DVR_Init()) {
        errorWrap(error, "NET_DVR_Init");
    }

    return;
}

void Cleanup(Error* error) {
    if (!NET_DVR_Cleanup()) {
        errorWrap(error, "NET_DVR_Cleanup");
    }

    return;
}

void Logout(int lUserID, Error* error) {
    if (!NET_DVR_Logout(lUserID)) {
        errorWrap(error, "NET_DVR_Logout");
    }

    return;
}

void SetConnectTime(int dwWaitTime, int dwTryTimes, Error* error) {
    if (!NET_DVR_SetConnectTime(dwWaitTime, dwTryTimes)) {
        errorWrap(error, "NET_DVR_SetConnectTime");
    }

    return;
}

void SaveFileByTimeV40(
  int lUserID, int channelNumber,
  Time* startTime, Time* endTime,
  char* destFile, Error* error
) {
    if (startTime == NULL) {
        error->errCode   = -1;
        error->errMsg    = "no start time for video save";

        return;
    }

    if (endTime == NULL) {
        error->errCode   = -1;
        error->errMsg    = "no end time for video save";

        return;
    }

    NET_DVR_PLAYCOND downloadCond={0};
    downloadCond.dwChannel = channelNumber;

    downloadCond.struStartTime.dwYear   = startTime->year;
    downloadCond.struStartTime.dwMonth  = startTime->month;
    downloadCond.struStartTime.dwDay    = startTime->day;
    downloadCond.struStartTime.dwHour   = startTime->hour;
    downloadCond.struStartTime.dwMinute = startTime->minute;
    downloadCond.struStartTime.dwSecond = startTime->second;

    downloadCond.struStopTime.dwYear    = endTime->year;
    downloadCond.struStopTime.dwMonth   = endTime->month;
    downloadCond.struStopTime.dwDay     = endTime->day;
    downloadCond.struStopTime.dwHour    = endTime->hour;
    downloadCond.struStopTime.dwMinute  = endTime->minute;
    downloadCond.struStopTime.dwSecond  = endTime->second;

    int hPlayback = NET_DVR_GetFileByTime_V40(lUserID, destFile, &downloadCond);
    if(hPlayback < 0) {
        errorWrap(error, "NET_DVR_GetFileByTime_V40");

        return;
     }

    if(!NET_DVR_PlayBackControl_V40(hPlayback, NET_DVR_PLAYSTART, NULL, 0, NULL, NULL)) {
       errorWrap(error, "NET_DVR_PlayBackControl_V40");

       return;
    }

    int pos = 0, lastPos = 0;

    for (pos = 0; pos < 100 && pos >= 0; pos = NET_DVR_GetDownloadPos(hPlayback)) {
       printf("Downloading progress: %d / 100\n", pos);
       usleep(100000); // задаем время сна, чтобы не дергать SDK без надобности.
    }

    if (!NET_DVR_StopGetFile(hPlayback)) {
       errorWrap(error, "NET_DVR_StopGetFile");

       return;
    }

    if (pos < 0 || pos > 100) {
       errorWrap(error, "DownloadPosError");

       return;
    }

    return;
}

void SetLogLevel(int logLevel, Error* error) {
    if (logLevel <= -1) {
        NET_DVR_SetLogPrint(false);

        return;
    }

    if (!NET_DVR_SetLogPrint(true)) {
        errorWrap(error, "NET_DVR_SetLogPrint");
    }

    if (!NET_DVR_SetLogPrintAction(logLevel, 1, 0, 0, 0)) {
        errorWrap(error, "NET_DVR_SetLogPrintAction");
    }

    return;
}