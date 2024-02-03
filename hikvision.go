package hikvision

/*
#include <stdio.h>
#include <stdlib.h>
#include "hikvision.h"
#include <string.h>
*/
import "C"
import (
	"fmt"
	"time"
	"unsafe"
)

const (
	devInfoLog = "DevInfo: ByZeroChanNum: %d, ByStartChan: %d, ByChanNum: %d, ByStartDChan: %d, " +
		"ByDChanNum: %d, SSerialNumber: %s, ByDiskNum: %d, ByDVRType: %d\n"

	errUnknownMsg = "unknown error occurred"
)

// goError - конвертирует ошибку из C в Go.
// errCode == -1 - ошибка на стороне собственного cpp кода.
func goError(cError *C.Error) error {
	var (
		errCode = int(cError.errCode)
		errWrap = C.GoString(cError.errWrap)
		errMsg  = C.GoString(cError.errMsg)
	)

	if len(errMsg) == 0 {
		errMsg = errUnknownMsg
	}

	if len(errWrap) == 0 {
		return fmt.Errorf("%s [%d]", errMsg, errCode)
	}

	return fmt.Errorf("%s: %s [%d]", errWrap, errMsg, errCode)
}

// Init - инициализирует ресурсы программы перед остальными вызовами.
func Init() error {
	cError := C.Error{errCode: 0}

	C.Init(&cError)
	if int(cError.errCode) != 0 {
		return goError(&cError)
	}

	return nil
}

// Cleanup - отчищает ресурсы после того, как программа завершилась.
func Cleanup() error {
	cError := C.Error{errCode: 0}

	C.Cleanup(&cError)
	if int(cError.errCode) != 0 {
		return goError(&cError)
	}

	return nil
}

// Logout - производит выход из системы.
func Logout(lUserID int) error {
	var (
		cLUserID = C.int(lUserID)

		cError = C.Error{errCode: 0}
	)

	C.Logout(cLUserID, &cError)
	if int(cError.errCode) != 0 {
		return goError(&cError)
	}

	return nil
}

// Login - производит вход в систему.
func Login(ip string, port int, username string, password string) (int, error) {
	var (
		cIP       = C.CString(ip)
		cPort     = C.int(port)
		cUsername = C.CString(username)
		cPassword = C.CString(password)

		loginParams = C.LoginParams{
			ip:       cIP,
			port:     cPort,
			username: cUsername,
			password: cPassword,
		}

		devInfo = C.DevInfo{byStartChan: 0}

		cError = C.Error{errCode: 0}
	)
	defer func() {
		C.free(unsafe.Pointer(cIP))
		C.free(unsafe.Pointer(cUsername))
		C.free(unsafe.Pointer(cPassword))
	}()

	userID := C.Login(&loginParams, &devInfo, &cError)

	fmt.Printf(
		devInfoLog,
		int(devInfo.byZeroChanNum),
		int(devInfo.byStartChan),
		int(devInfo.byChanNum),
		int(devInfo.byStartDChan),
		int(devInfo.byDChanNum),
		C.GoString(devInfo.sSerialNumber),
		int(devInfo.byDiskNum),
		int(devInfo.byDVRType),
	)

	return int(userID), nil
}

// SaveFileByTime - скачивает видео для заданного channelNumber (камеры) за промежуток времени
// [startTime; endTime] - с погрешностями в границах, и сохраняет файл в dstFile.
func SaveFileByTime(
	lUserID, channelNumber int,
	startTime, endTime time.Time,
	dstFile string,
) error {
	var (
		cLUserID       = C.int(lUserID)
		cChannelNumber = C.int(channelNumber)
		cDstFile       = C.CString(dstFile)

		cStartTime = C.Time{
			year:   C.int(startTime.Year()),
			month:  C.int(startTime.Month()),
			day:    C.int(startTime.Day()),
			hour:   C.int(startTime.Hour()),
			minute: C.int(startTime.Minute()),
			second: C.int(startTime.Second()),
		}

		cEndTime = C.Time{
			year:   C.int(endTime.Year()),
			month:  C.int(endTime.Month()),
			day:    C.int(endTime.Day()),
			hour:   C.int(endTime.Hour()),
			minute: C.int(endTime.Minute()),
			second: C.int(endTime.Second()),
		}

		cError = C.Error{errCode: 0}
	)

	defer C.free(unsafe.Pointer(cDstFile))

	C.SaveFileByTimeV40(cLUserID, cChannelNumber, &cStartTime, &cEndTime, cDstFile, &cError)
	if int(cError.errCode) != 0 {
		return goError(&cError)
	}

	return nil
}

// SetConnectTime - устанавливает таймаут на соединение и количество попыток.
func SetConnectTime(waitTime int, tryTimes int) error {
	var (
		cWaitTime = C.int(waitTime)
		cTryTimes = C.int(tryTimes)

		cError = C.Error{errCode: 0}
	)

	C.SetConnectTime(cWaitTime, cTryTimes, &cError)
	if int(cError.errCode) != 0 {
		return goError(&cError)
	}

	return nil
}

// SetLogLevel - устанавливает логирование при работе SDK.
// Ненадежный метод, потому что в документации не нашел про NET_DVR_SetLogPrintAction.
// Параметры для этого метода лишь из примера.
func SetLogLevel(logLevel int) error {
	var (
		cLogLevel = C.int(logLevel)

		cError = C.Error{errCode: 0}
	)

	C.SetLogLevel(cLogLevel, &cError)
	if int(cError.errCode) != 0 {
		return goError(&cError)
	}

	return nil
}
