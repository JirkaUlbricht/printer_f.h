#ifndef PRINTER_F_H
#define PRINTER_F_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
typedef BOOL (WINAPI *GetDefaultPrinterA_t)(LPSTR, LPDWORD);
typedef HDC  (WINAPI *CreateDCA_t)(LPCSTR, LPCSTR, LPCSTR, const DEVMODEA*);
typedef int  (WINAPI *StartDocA_t)(HDC, const DOCINFOA*);
typedef int  (WINAPI *StartPage_t)(HDC);
typedef int  (WINAPI *EndPage_t)(HDC);
typedef int  (WINAPI *EndDoc_t)(HDC);
typedef int  (WINAPI *AbortDoc_t)(HDC);
typedef BOOL (WINAPI *DeleteDC_t)(HDC);
typedef BOOL (WINAPI *TextOutW_t)(HDC, int, int, LPCWSTR, int);
typedef BOOL (WINAPI *GetTextMetricsW_t)(HDC, LPTEXTMETRICW);
typedef int  (WINAPI *GetDeviceCaps_t)(HDC, int);
#endif

static char *printer_f_buffer = NULL;
static size_t printer_f_size = 0;
static size_t printer_f_cap = 0;
static int printer_f_registered = 0;

static void printer_f_flush(void);

static int printer_f_reserve(size_t add) {
    if (printer_f_size + add > printer_f_cap) {
        size_t ncap = printer_f_cap ? printer_f_cap : 1024;
        while (ncap < printer_f_size + add) {
            ncap *= 2;
        }
        char *nbuf = (char*)realloc(printer_f_buffer, ncap);
        if (!nbuf) {
            return 0;
        }
        printer_f_buffer = nbuf;
        printer_f_cap = ncap;
    }
    return 1;
}

static void print_to_printer(const char *format, ...) {
    va_list args;
    va_start(args, format);
    va_list args2;
    va_copy(args2, args);
#ifdef _WIN32
    int need = _vscprintf(format, args2);
#else
    int need = vsnprintf(NULL, 0, format, args2);
#endif
    va_end(args2);
    if (need < 0) {
        va_end(args);
        return;
    }
    if (!printer_f_reserve((size_t)need + 2)) {
        va_end(args);
        return;
    }
    vsnprintf(printer_f_buffer + printer_f_size, need + 1, format, args);
    printer_f_size += (size_t)need;
    printer_f_buffer[printer_f_size++] = '\n';
    printer_f_buffer[printer_f_size] = '\0';
    va_end(args);
    if (!printer_f_registered) {
        atexit(printer_f_flush);
        printer_f_registered = 1;
    }
}

static int printer_f_strcasestr_pdf(const char *s) {
    if (!s) {
        return 0;
    }
    size_t n = strlen(s);
    for (size_t i = 0; i + 2 < n; i++) {
        char a = s[i];
        char b = s[i + 1];
        char c = s[i + 2];
        if (a >= 'A' && a <= 'Z') {
            a = (char)(a - 'A' + 'a');
        }
        if (b >= 'A' && b <= 'Z') {
            b = (char)(b - 'A' + 'a');
        }
        if (c >= 'A' && c <= 'Z') {
            c = (char)(c - 'A' + 'a');
        }
        if (a == 'p' && b == 'd' && c == 'f') {
            return 1;
        }
    }
    return 0;
}

static void printer_f_flush(void) {
    if (!printer_f_buffer || printer_f_size == 0) {
        return;
    }

#ifdef _WIN32
    DWORD t0 = GetTickCount();
    HMODULE hspool = LoadLibraryA("winspool.drv");
    if (!hspool) {
        fwrite("printer_f: failed to load winspool\n", 1, 35, stderr);
        goto cleanup_buf_only;
    }
    GetDefaultPrinterA_t pGetDefaultPrinterA = (GetDefaultPrinterA_t)GetProcAddress(hspool, "GetDefaultPrinterA");
    if (!pGetDefaultPrinterA) {
        fwrite("printer_f: GetDefaultPrinterA missing\n", 1, 39, stderr);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }
    DWORD needed = 0;
    pGetDefaultPrinterA(NULL, &needed);
    if (needed == 0) {
        fwrite("printer_f: no default printer\n", 1, 30, stderr);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }
    char *printerName = (char*)malloc(needed);
    if (!printerName) {
        fwrite("printer_f: alloc printerName failed\n", 1, 37, stderr);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }
    if (!pGetDefaultPrinterA(printerName, &needed)) {
        fwrite("printer_f: GetDefaultPrinterA call failed\n", 1, 42, stderr);
        free(printerName);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }

    HMODULE hgdi = LoadLibraryA("gdi32.dll");
    if (!hgdi) {
        fwrite("printer_f: load gdi32 failed\n", 1, 29, stderr);
        free(printerName);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }
    
    CreateDCA_t  pCreateDCA  = (CreateDCA_t)GetProcAddress(hgdi, "CreateDCA");
    StartDocA_t  pStartDocA  = (StartDocA_t)GetProcAddress(hgdi, "StartDocA");
    StartPage_t  pStartPage  = (StartPage_t)GetProcAddress(hgdi, "StartPage");
    EndPage_t    pEndPage    = (EndPage_t)GetProcAddress(hgdi, "EndPage");
    EndDoc_t     pEndDoc     = (EndDoc_t)GetProcAddress(hgdi, "EndDoc");
    AbortDoc_t   pAbortDoc   = (AbortDoc_t)GetProcAddress(hgdi, "AbortDoc");
    DeleteDC_t   pDeleteDC   = (DeleteDC_t)GetProcAddress(hgdi, "DeleteDC");
    TextOutW_t   pTextOutW   = (TextOutW_t)GetProcAddress(hgdi, "TextOutW");
    GetTextMetricsW_t pGetTextMetricsW = (GetTextMetricsW_t)GetProcAddress(hgdi, "GetTextMetricsW");
    GetDeviceCaps_t pGetDeviceCaps = (GetDeviceCaps_t)GetProcAddress(hgdi, "GetDeviceCaps");

    if (!pCreateDCA || !pStartDocA || !pStartPage || !pEndPage || !pEndDoc || !pDeleteDC || !pTextOutW || !pGetDeviceCaps) {
        fwrite("printer_f: required GDI proc missing\n", 1, 36, stderr);
        FreeLibrary(hgdi);
        free(printerName);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }

    HDC hdc = pCreateDCA("WINSPOOL", printerName, NULL, NULL);
    if (!hdc) {
        fwrite("printer_f: CreateDCA failed\n", 1, 28, stderr);
        FreeLibrary(hgdi);
        free(printerName);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }

    int is_pdf = printer_f_strcasestr_pdf(printerName);
    char outPath[MAX_PATH];
    outPath[0] = 0;
    if (is_pdf) {
        char tempdir[MAX_PATH];
        DWORD tlen = GetTempPathA(MAX_PATH, tempdir);
        if (tlen == 0 || tlen >= MAX_PATH) {
            strcpy(tempdir, ".");
        }
        snprintf(outPath, sizeof(outPath), "%sprinter_f_output.pdf", tempdir);
        DeleteFileA(outPath);
    }

    DOCINFOA di;
    di.cbSize = sizeof(di);
    di.lpszDocName = "printer_f document";
    di.lpszOutput = is_pdf ? outPath : NULL;
    di.lpszDatatype = NULL;
    di.fwType = 0;

    if (GetTickCount() - t0 > 3000) {
        fwrite("printer_f: timeout\n", 1, 19, stderr);
        pDeleteDC(hdc);
        FreeLibrary(hgdi);
        free(printerName);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }

    if (pStartDocA(hdc, &di) <= 0) {
        fwrite("printer_f: StartDoc failed\n", 1, 27, stderr);
        pDeleteDC(hdc);
        FreeLibrary(hgdi);
        free(printerName);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }

    if (pStartPage(hdc) <= 0) {
        fwrite("printer_f: StartPage failed\n", 1, 29, stderr);
        pEndDoc(hdc);
        pDeleteDC(hdc);
        FreeLibrary(hgdi);
        free(printerName);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }

    int lineHeight = 20;
    if (pGetTextMetricsW) {
        TEXTMETRICW tm;
        if (pGetTextMetricsW(hdc, &tm)) {
            lineHeight = tm.tmHeight + tm.tmExternalLeading;
            if (lineHeight <= 0) {
                lineHeight = 20;
            }
        }
    }

    int pageHeight = pGetDeviceCaps(hdc, 117);
    if (pageHeight <= 0) {
        pageHeight = 1000;
    }
    int top = 100;
    int left = 100;
    int bottom = pageHeight - 100;
    int y = top;

    size_t pos = 0;
    int timed_out = 0;
    while (pos < printer_f_size) {
        if (GetTickCount() - t0 > 3000) {
            timed_out = 1;
            break;
        }
        size_t start = pos;
        while (pos < printer_f_size && printer_f_buffer[pos] != '\n') {
            pos++;
        }
        size_t len = pos - start;
        if (y + lineHeight > bottom) {
            if (GetTickCount() - t0 > 3000) {
                timed_out = 1;
                break;
            }
            pEndPage(hdc);
            if (GetTickCount() - t0 > 3000) {
                timed_out = 1;
                break;
            }
            if (pStartPage(hdc) <= 0) {
                fwrite("printer_f: StartPage failed\n", 1, 29, stderr);
                pEndDoc(hdc);
                pDeleteDC(hdc);
                FreeLibrary(hgdi);
                free(printerName);
                FreeLibrary(hspool);
                goto cleanup_buf_only;
            }
            y = top;
        }
        if (len > 0) {
            int wlen = MultiByteToWideChar(CP_UTF8, 0, printer_f_buffer + start, (int)len, NULL, 0);
            if (wlen > 0) {
                WCHAR *wbuf = (WCHAR*)malloc((size_t)wlen * sizeof(WCHAR));
                if (wbuf) {
                    MultiByteToWideChar(CP_UTF8, 0, printer_f_buffer + start, (int)len, wbuf, wlen);
                    pTextOutW(hdc, left, y, wbuf, wlen);
                    free(wbuf);
                }
            }
        }
        y += lineHeight;
        if (pos < printer_f_size && printer_f_buffer[pos] == '\n') {
            pos++;
        }
    }

    if (timed_out) {
        if (pAbortDoc) {
            pAbortDoc(hdc);
        }
        pDeleteDC(hdc);
        FreeLibrary(hgdi);
        free(printerName);
        FreeLibrary(hspool);
        fwrite("printer_f: timeout\n", 1, 19, stderr);
        goto cleanup_buf_only;
    }

    if (GetTickCount() - t0 > 3000) {
        if (pAbortDoc) {
            pAbortDoc(hdc);
        }
        pDeleteDC(hdc);
        FreeLibrary(hgdi);
        free(printerName);
        FreeLibrary(hspool);
        fwrite("printer_f: timeout\n", 1, 19, stderr);
        goto cleanup_buf_only;
    }

    pEndPage(hdc);

    if (GetTickCount() - t0 > 3000) {
        if (pAbortDoc) {
            pAbortDoc(hdc);
        }
        pDeleteDC(hdc);
        FreeLibrary(hgdi);
        free(printerName);
        FreeLibrary(hspool);
        fwrite("printer_f: timeout\n", 1, 19, stderr);
        goto cleanup_buf_only;
    }

    if (pEndDoc(hdc) <= 0) {
        fwrite("printer_f: EndDoc failed\n", 1, 24, stderr);
        pDeleteDC(hdc);
        FreeLibrary(hgdi);
        free(printerName);
        FreeLibrary(hspool);
        goto cleanup_buf_only;
    }

    pDeleteDC(hdc);
    FreeLibrary(hgdi);
    free(printerName);
    FreeLibrary(hspool);

    if (is_pdf) {
        fwrite("Printing ", 1, 9, stdout);
        fwrite(outPath, 1, strlen(outPath), stdout);
        fwrite("\n", 1, 1, stdout);
    } else {
        fwrite("Printing printer_f_output.txt\n", 1, 30, stdout);
    }

#else
    const char *outName = "printer_f_output.txt";
    FILE *fp = fopen(outName, "w");
    if (!fp) {
        fwrite("printer_f: cannot open output file\n", 1, 35, stderr);
    } else {
        if (fwrite(printer_f_buffer, 1, printer_f_size, fp) != printer_f_size) {
            fwrite("printer_f: write error\n", 1, 24, stderr);
        }
        fclose(fp);
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "lp '%s' 2>/dev/null", outName);
        int r = system(cmd);
        if (r != -1) {
            fwrite("Printing printer_f_output.txt\n", 1, 30, stdout);
        } else {
            fwrite("printer_f: lp command failed\n", 1, 30, stderr);
        }
    }
#endif

cleanup_buf_only:
    free(printer_f_buffer);
    printer_f_buffer = NULL;
    printer_f_size = 0;
    printer_f_cap = 0;
}

#define printf(...) print_to_printer(__VA_ARGS__)

#endif
