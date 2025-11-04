#ifndef PINTER_F_H
#define PINTER_F_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
typedef BOOL (WINAPI *GetDefaultPrinterA_t)(LPSTR, LPDWORD);
typedef HDC  (WINAPI *CreateDCA_t)(LPCSTR, LPCSTR, LPCSTR, const DEVMODEA*);
typedef int  (WINAPI *StartDocA_t)(HDC, const DOCINFOA*);
typedef int  (WINAPI *StartPage_t)(HDC);
typedef int  (WINAPI *EndPage_t)(HDC);
typedef int  (WINAPI *EndDoc_t)(HDC);
typedef BOOL (WINAPI *DeleteDC_t)(HDC);
typedef BOOL (WINAPI *TextOutA_t)(HDC, int, int, LPCSTR, int);
#endif

static void print_to_printer(const char *format, ...) {
    char tmpfile[256];
    FILE *fp;
    va_list args;

#ifdef _WIN32
    va_list args2;
    va_start(args, format);
    va_copy(args2, args);
    int len = _vscprintf(format, args) + 1;
    char *buf = (char*)malloc((size_t)len);
    if (!buf) {
        va_end(args2);
        va_end(args);
        return;
    }
    vsnprintf(buf, (size_t)len, format, args2);
    va_end(args2);
    va_end(args);

    HMODULE hspool = LoadLibraryA("winspool.drv");
    if (!hspool) { free(buf); return; }
    GetDefaultPrinterA_t pGetDefaultPrinterA = (GetDefaultPrinterA_t)GetProcAddress(hspool, "GetDefaultPrinterA");
    if (!pGetDefaultPrinterA) { FreeLibrary(hspool); free(buf); return; }

    DWORD needed = 0;
    pGetDefaultPrinterA(NULL, &needed);
    if (needed == 0) { FreeLibrary(hspool); free(buf); return; }
    char *printerName = (char*)malloc(needed);
    if (!printerName) { FreeLibrary(hspool); free(buf); return; }
    if (!pGetDefaultPrinterA(printerName, &needed)) { free(printerName); FreeLibrary(hspool); free(buf); return; }

    HMODULE hgdi = LoadLibraryA("gdi32.dll");
    if (!hgdi) { free(printerName); FreeLibrary(hspool); free(buf); return; }
    CreateDCA_t  pCreateDCA  = (CreateDCA_t)GetProcAddress(hgdi, "CreateDCA");
    StartDocA_t  pStartDocA  = (StartDocA_t)GetProcAddress(hgdi, "StartDocA");
    StartPage_t  pStartPage  = (StartPage_t)GetProcAddress(hgdi, "StartPage");
    TextOutA_t   pTextOutA   = (TextOutA_t)GetProcAddress(hgdi, "TextOutA");
    EndPage_t    pEndPage    = (EndPage_t)GetProcAddress(hgdi, "EndPage");
    EndDoc_t     pEndDoc     = (EndDoc_t)GetProcAddress(hgdi, "EndDoc");
    DeleteDC_t   pDeleteDC   = (DeleteDC_t)GetProcAddress(hgdi, "DeleteDC");
    if (!pCreateDCA || !pStartDocA || !pStartPage || !pTextOutA || !pEndPage || !pEndDoc || !pDeleteDC) {
        FreeLibrary(hgdi);
        free(printerName);
        FreeLibrary(hspool);
        free(buf);
        return;
    }

    HDC hdc = pCreateDCA("WINSPOOL", printerName, NULL, NULL);
    if (hdc) {
        DOCINFOA di;
        di.cbSize = sizeof(di);
        di.lpszDocName = "printer_f job";
        di.lpszOutput = NULL;
        di.lpszDatatype = NULL;
        di.fwType = 0;
        if (pStartDocA(hdc, &di) > 0) {
            if (pStartPage(hdc) > 0) {
                pTextOutA(hdc, 100, 100, buf, (int)(len - 1));
                pEndPage(hdc);
            }
            pEndDoc(hdc);
        }
        pDeleteDC(hdc);
    }

    FreeLibrary(hgdi);
    free(printerName);
    FreeLibrary(hspool);
    free(buf);

#elif defined(__linux__) || defined(__APPLE__)
    snprintf(tmpfile, sizeof(tmpfile), "/tmp/printXXXXXX");
    int fd = mkstemp(tmpfile);
    if (fd == -1) { perror("mkstemp"); return; }
    fp = fdopen(fd, "w");
    if (!fp) { perror("fdopen"); return; }
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);
    fclose(fp);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "lp '%s' 2>/dev/null", tmpfile);
    system(cmd);
    remove(tmpfile);
#else
    va_start(args, format);
    fputs("Unsupported OS\n", stdout);
    va_end(args);
#endif
}

#define printf(...) print_to_printer(__VA_ARGS__)

#endif
