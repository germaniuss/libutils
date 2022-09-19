#ifndef _UTILS_PATH_H
#define _UTILS_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <direct.h> 
#else
#include <unistd.h>
#endif

char* getexecname();
char* getexecdir();
char* getcurrentdir();
void path_add(char** path, const char* dir);

#ifdef __cplusplus
}
#endif

#if defined(_UTILS_IMPL) || defined(_UTILS_PATH_IMPL)

#include "str.h"
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)

char* getexecname() {
    char buf[FILENAME_MAX];
    GetModuleFileName(NULL, buf, FILENAME_MAX);
    return str_create(buf);
};

char* getexecdir() {
    char* val = getexecname();
    char * tld = strrchr(val, '\\');
    *tld = '\0';
    char* ret = str_create(val);
    str_destroy(&val);
    return ret;
}

char* getcurrentdir() {
    char buf[FILENAME_MAX];
    _getcwd(buf, FILENAME_MAX);
    return str_create(buf);
}

void path_add(char** path, const char* dir) {
    if (dir == NULL) return;
    if (*dir == 'C' || *path == NULL) {
        str_destroy(path);
        *path = str_create(dir);
    } else if (*dir) str_append_fmt(path, "\\%s", dir);
}

#else

char* getexecname() {
    // On linux
    char buf[FILENAME_MAX];
    int bytes = readlink("/proc/self/exe", buf, FILENAME_MAX);
    buf[bytes] = 0;
    return str_create(buf);
};

char* getexecdir() {
    // On linux
    char* val = getexecname();
    char * tld = strrchr(val, '/');
    *tld = '\0';
    char* ret = str_create(val);
    str_destroy(&val);
    return ret;
}

char* getcurrentdir() {
    // On linux
    char buf[FILENAME_MAX];
    getcwd(buf, FILENAME_MAX);
    return str_create(buf);
}

void path_add(char** path, const char* dir) {
    if (dir == NULL) return;
    if (*dir == '/' || *path == NULL) {
        str_destroy(path);
        *path = str_create(dir);
    } else if (*dir) str_append_fmt(path, "/%s", dir);
}

#endif
#endif
#endif