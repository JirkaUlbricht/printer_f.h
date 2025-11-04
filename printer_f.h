#ifndef PINTER_F_H
#define PINTER_F_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static void print_to_printer(const char *format, ...) {
    char tmpfile[256];
    FILE *fp;
    va_list args;

// Windows
#ifdef _WIN32
    tmpfile[0] = '\0';
    tmpnam(tmpfile);
    fp = fopen(tmpfile, "w");
    if (!fp) {
        perror("fopen");
        return;
    }
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);
    fclose(fp);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "print /D:default \"%s\" >nul 2>nul", tmpfile);
    system(cmd);
    remove(tmpfile);

// Linux and macOS
#elif defined(__linux__) || defined(__APPLE__)
    snprintf(tmpfile, sizeof(tmpfile), "/tmp/printXXXXXX");
    int fd = mkstemp(tmpfile);
    if (fd == -1) {
        perror("mkstemp");
        return;
    }
    fp = fdopen(fd, "w");
    if (!fp) {
        perror("fdopen");
        return;
    }
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);
    fclose(fp);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "lp '%s' 2>/dev/null", tmpfile);
    system(cmd);
    remove(tmpfile);

// Unsupported OS
#else
    va_start(args, format);
    vfprintf(stdout, "Unsupported OS\n", args);
    va_end(args);
#endif
}

#define printf(...) print_to_printer(__VA_ARGS__)

#endif
