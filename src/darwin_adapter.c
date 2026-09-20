#include "darwin_adapter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(DARWIN_PORTABLE_NO_HOST_APIS) && defined(_WIN32)
#include <windows.h>
#elif !defined(DARWIN_PORTABLE_NO_HOST_APIS)
#include <dirent.h>
#include <unistd.h>
#endif

const char *darwin_adapter_name(void) {
#if defined(DARWIN_PORTABLE_NO_HOST_APIS)
    return "generic-c11";
#elif defined(_WIN32)
    return "win32";
#elif defined(__ANDROID__)
    return "android-posix";
#elif defined(__APPLE__)
    return "apple-posix";
#elif defined(__HAIKU__)
    return "haiku-posix";
#elif defined(__sun)
    return "solaris-posix";
#elif defined(__QNXNTO__)
    return "qnx-posix";
#elif defined(__unix__) || defined(__unix)
    return "posix";
#else
    return "generic-c11";
#endif
}

const char *darwin_adapter_hostname(void) {
    static char name[256];
#if defined(DARWIN_PORTABLE_NO_HOST_APIS)
    snprintf(name, sizeof(name), "darwin-prototype");
#elif defined(_WIN32)
    DWORD size = (DWORD)sizeof(name);
    if (!GetComputerNameA(name, &size)) snprintf(name, sizeof(name), "darwin-prototype");
#else
    if (gethostname(name, sizeof(name)) != 0) snprintf(name, sizeof(name), "darwin-prototype");
    name[sizeof(name) - 1] = '\0';
#endif
    return name;
}

const char *darwin_adapter_cwd(void) {
    static char path[512];
#if defined(DARWIN_PORTABLE_NO_HOST_APIS)
    snprintf(path, sizeof(path), "/");
#elif defined(_WIN32)
    if (!_getcwd(path, sizeof(path))) snprintf(path, sizeof(path), ".");
#else
    if (!getcwd(path, sizeof(path))) snprintf(path, sizeof(path), "/");
#endif
    return path;
}

void darwin_adapter_list(const char *path) {
#if defined(DARWIN_PORTABLE_NO_HOST_APIS)
    (void)path; puts(".\n..\n[host filesystem unavailable]");
#elif defined(_WIN32)
    char pattern[512]; WIN32_FIND_DATAA data; HANDLE h;
    snprintf(pattern, sizeof(pattern), "%s\\*", path); h = FindFirstFileA(pattern, &data);
    if (h == INVALID_HANDLE_VALUE) { fprintf(stderr, "ls: cannot open %s\n", path); return; }
    do { if (strcmp(data.cFileName, ".") && strcmp(data.cFileName, "..")) puts(data.cFileName); } while (FindNextFileA(h, &data)); FindClose(h);
#else
    DIR *d = opendir(path); struct dirent *e;
    if (!d) { fprintf(stderr, "ls: cannot open %s\n", path); return; }
    while ((e = readdir(d))) if (strcmp(e->d_name, ".") && strcmp(e->d_name, "..")) puts(e->d_name); closedir(d);
#endif
}
