#include <Windows.h>
#include <iostream>
#include <cwchar>
#include <atlimage.h>
#include <vector>

#include "screenshot.h"
#include "gdiplus.h"

bool save_png_memory(HBITMAP hbitmap, std::vector<BYTE> &data);

BITMAPFILEHEADER bfHeader;
BITMAPINFOHEADER biHeader;
BITMAPINFO bInfo;
HGDIOBJ hTempBitmap;
HBITMAP hBitmap;
BITMAP bAllDesktops;
HDC hDC, hMemDC;
LONG lWidth, lHeight;
BYTE* bBits = nullptr;
[[maybe_unused]] HANDLE hHeap = GetProcessHeap();
DWORD cbBits, dwWritten = 0;
HANDLE hFile;

BOOL WINAPI CapScreen2Bmp() {
    CImage CapturedScreenshot;
    INT x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    INT y = GetSystemMetrics(SM_YVIRTUALSCREEN);

    ZeroMemory(&bfHeader, sizeof(BITMAPFILEHEADER));
    ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
    ZeroMemory(&bInfo, sizeof(BITMAPINFO));
    ZeroMemory(&bAllDesktops, sizeof(BITMAP));

    hDC = GetDC(nullptr);
    hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
    GetObjectW(hTempBitmap, sizeof(BITMAP), &bAllDesktops);

    lWidth = bAllDesktops.bmWidth;
    lHeight = bAllDesktops.bmHeight;

    DeleteObject(hTempBitmap);

    bfHeader.bfType = (WORD)('B' | ('M' << 8));
    bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    biHeader.biSize = sizeof(BITMAPINFOHEADER);
    biHeader.biBitCount = 24;
    biHeader.biCompression = BI_RGB;
    biHeader.biPlanes = 1;
    biHeader.biWidth = lWidth;
    biHeader.biHeight = lHeight;

    bInfo.bmiHeader = biHeader;

    cbBits = (((24 * lWidth + 31) & ~31) / 8) * lHeight;

    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID**)&bBits, nullptr, 0);
    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, lWidth, lHeight, hDC, x, y, SRCCOPY);

    ULONG_PTR token;
    Gdiplus::GdiplusStartupInput tmp;
    Gdiplus::GdiplusStartup(&token, &tmp, nullptr);

    std::vector<BYTE> data;
    if(save_png_memory(hBitmap, data)) {
        screenshotData.assign(data.begin(), data.end());
        return false;
    }

    DeleteObject(hBitmap);

    Gdiplus::GdiplusShutdown(token);
    CoUninitialize();

    DeleteDC(hMemDC);
    ReleaseDC(nullptr, hDC);
    DeleteObject(hBitmap);

    return true;
}

bool save_png_memory(HBITMAP hbitmap, std::vector<BYTE> &data) {
    Gdiplus::Bitmap bmp(hbitmap, nullptr);

    //write to IStream
    IStream* istream = nullptr;
    CreateStreamOnHGlobal(nullptr, true, &istream);

    CLSID clsid_png;
    CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &clsid_png);
    Gdiplus::Status status = bmp.Save(istream, &clsid_png);
    if(status != Gdiplus::Status::Ok)
        return false;

    //get memory handle associated with istream
    HGLOBAL hg = nullptr;
    GetHGlobalFromStream(istream, &hg);

    //copy IStream to buffer
    int bufsize = GlobalSize(hg);
    data.resize(bufsize);

    //lock & unlock memory
    LPVOID pimage = GlobalLock(hg);
    memcpy(&data[0], pimage, bufsize);
    GlobalUnlock(hg);

    istream->Release();
    return true;
}

// const wchar_t *path = L"file.bmp";
// CapImg2Bmp((wchar_t*)path);