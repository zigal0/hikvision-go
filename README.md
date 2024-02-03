# hikvision-go

Прослойка между hikvision SDK & go.

## Как пользоваться

* Код для linux систем. 
* Для запуска требуется добавить sdk в папки include (заголовочные файлы) и lib (общие библиотеки).
* Необходимые файлы можно найти [тут](https://www.hikvision.com/us-en/support/download/sdk/). 
* Запуск требуется производить командой:
```LD_LIBRARY_PATH="absolute_path_to_lib" CGO_CXXFLAGS="-I absolute_path_to_include" CGO_LDFLAGS="-L. absolute_path_to_lib -lhcnetsdk" go run .```

Пример для заданной конфигурации:

![img.png](img/img.png)

```LD_LIBRARY_PATH="./lib" CGO_CXXFLAGS="-I./include" CGO_LDFLAGS="-L./lib -lhcnetsdk" go run .test/main.go```

```go
package main

import (
	"fmt"
	"time"

	"gitlab.ozon.ru/serefimov/hikvision-go"
)

// Flow:
// Init;
// defer Cleanup;
// SetConnectTime, SetLogLevel - если требуется;
// Login;
// Defer Logout;
// Some logic (e.g. SaveFileByTime).
func main() {
	err := hikvision.Init()
	if err != nil {
		fmt.Println(err)
	}
	defer func() {
		err = hikvision.Cleanup()
		if err != nil {
			fmt.Println(err)
		}
	}()

	// Задаем параметры для логина.
	var (
		ip       = "some_ip"
		port     = 0
		username = "username"
		password = "password"
	)

	userID, err := hikvision.Login(ip, port, username, password)
	if err != nil {
		fmt.Println(err)
	}
	defer func() {
		err = hikvision.Logout(userID)
		if err != nil {
			fmt.Println(err)
		}
	}()

	// Задаем параметры для скачивания видео.
	var (
		timeStart     = time.Now().Add(-time.Hour)
		timeFinish    = timeStart.Add(120 * time.Second)
		channelNumber = 0
		destFile      = "video.mp4"
	)

	err = hikvision.SaveFileByTime(userID, channelNumber, timeStart, timeFinish, destFile)
	if err != nil {
		fmt.Println(err)
	}
}
```

## Полезные ссылки
Примеры работы на Go с SDK:
* https://github.com/seamory/go-hikvision/blob/main/internal/hikvision/hikvision.go
* https://github.com/sarjsheff/hiklib
* https://github.com/lkqm/hcnetsdk-go/blob/main/hcnetsdk.go
