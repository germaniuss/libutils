#ifndef _UTILS_PIPE_H
#define _UTILS_PIPE_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define READONLY GENERIC_READ
#define WRITEONLY GENERIC_WRITE
#define READWRITE GENERIC_WRITE | GENERIC_READ
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#define READONLY O_RDONLY
#define WRITEONLY O_WRONLY
#define READWRITE O_RDWR
#endif

typedef struct file_desc {
    char* name;
    #if defined(_WIN32) || defined(_WIN64)
    HANDLE hPipe;
    int isReadPipe;
    #elif defined(__linux__)
    int fd;
    #endif
} file_desc;

void pipe_create(file_desc* pipe, const char* name);
int pipe_open(file_desc* pipe, int mode);
void pipe_close(file_desc* pipe);

int file_open(file_desc* pipe, const char* name, int mode, int lock);
char* file_read(file_desc* pipe, char buf[], unsigned int size);
void file_write(file_desc* pipe, const char* str);
void file_close(file_desc* pipe);

#ifdef __cplusplus
}
#endif

#if defined(_UTILS_IMPL) || defined(_UTILS_PIPE_IMPL)

#include <string.h>

#if defined(_WIN32) || defined(_WIN64)

void pipe_create(file_desc* pipe, const char* name) {

    // all access secutrity descriptor
    PSECURITY_DESCRIPTOR psd = NULL;
    BYTE  sd[SECURITY_DESCRIPTOR_MIN_LENGTH];
    psd = (PSECURITY_DESCRIPTOR)sd;
    InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(psd, TRUE, (PACL)NULL, FALSE);
    SECURITY_ATTRIBUTES sa = {sizeof(sa), psd, FALSE};

    pipe->name = (char*) malloc(strlen(name) + 10);
    memcpy(pipe->name, "\\\\.\\pipe\\", 10);
    strcat(pipe->name, name);
    pipe->hPipe = CreateNamedPipe(
        TEXT(pipe->name),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        1024 * 16,
        1024 * 16,
        NMPWAIT_USE_DEFAULT_WAIT,
        &sa
    );
}

int pipe_open(file_desc* pipe, int mode) {
    if (mode == READONLY) {
        pipe->isReadPipe = TRUE;
        return ConnectNamedPipe(pipe->hPipe, NULL);
    } return file_open(pipe,  pipe->name, mode, 1);
}

void pipe_close(file_desc* pipe) {
    if (pipe->isReadPipe) {
        DisconnectNamedPipe(pipe->hPipe);
    } else file_close(pipe);
}

int file_open(file_desc* pipe, const char* name, int mode, int lock) {
    if (!lock) pipe->hPipe = CreateFile(TEXT(name), mode,  FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
    else pipe->hPipe = CreateFile(TEXT(name), mode,  0, NULL, OPEN_ALWAYS, 0, NULL);
    return pipe->hPipe != INVALID_HANDLE_VALUE;
}

char* file_read(file_desc* pipe, char buf[], unsigned int size) {
    DWORD dwRead;
    ReadFile(pipe->hPipe, buf, size, &dwRead, NULL);
}

void file_write(file_desc* pipe, const char* str) {
    DWORD dwWritten;
    WriteFile(pipe->hPipe,str, strlen(str) + 1, &dwWritten, NULL);
}

void file_close(file_desc* pipe) {
    CloseHandle(pipe->hPipe);
}

#else

void pipe_create(file_desc* pipe, const char* name) {
    pipe->name = (char*) malloc(strlen(name) + 6);
    memcpy(pipe->name, "/tmp/", 6);
    strcat(pipe->name, name);
    mkfifo(pipe->name, 0666);
}

int pipe_open(file_desc* pipe, int mode) {
    return file_open(pipe, pipe->name, mode, 0);
}

void pipe_close(file_desc* pipe) {
    file_close(pipe);
}

int file_open(file_desc* pipe, const char* name, int mode, int lock) {
    pipe->fd = open(name, mode | O_CREAT, 0666);
    if (lock) return (flock(pipe->fd, LOCK_EX | LOCK_NB) != -1 && EWOULDBLOCK != errno);
    return pipe->fd != -1;
}

char* file_read(file_desc* pipe, char buf[], unsigned int size) {
    read(pipe->fd, buf, size);
}

void file_write(file_desc* pipe, const char* str) {
    write(pipe->fd, str, strlen(str) + 1);
}

void file_close(file_desc* pipe) {
    close(pipe->fd);
}

#endif
#endif
#endif