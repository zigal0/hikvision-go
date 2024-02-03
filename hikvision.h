#ifndef hikvision_h
#define hikvision_h

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct LoginParams {
        char*   ip;
        int     port;
        char*   username;
        char*   password;
    } LoginParams;

    typedef struct Error {
        int         errCode;
        const char* errMsg;
        const char* errWrap;
    } Error;

    typedef struct DevInfo {
        char* sSerialNumber;
        int   byDiskNum;
        int   byDVRType;
        int   byZeroChanNum;
        int   byStartChan;
        int   byChanNum;
        int   byStartDChan;
        int   byDChanNum;
    } DevInfo;

    typedef struct Time {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
    } Time;

    // system
    void Init(Error* error);
    int Login(LoginParams* loginParams, DevInfo* devInfo, Error *error);
    void Cleanup(Error* error);
    void Logout(int lUserID, Error* error);
    void SetConnectTime(int dwWaitTime, int dwTryTimes, Error* error);
    void SetLogLevel(int logLevel, Error* error);

    // logic
    void SaveFileByTimeV40(int lUserID, int channelNumber, Time* startTime, Time* endTime, char* destFile, Error* error);

#ifdef __cplusplus
}
#endif

#endif