#include "darwin_portable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

static const char *services[] = {
    "com.apple.kernel",
    "com.apple.launchd",
    "com.apple.bsd",
    "com.apple.io",
    "com.apple.apfs"
};
static bool service_running[] = { true, true, true, false, false };

const char *darwin_portable_state_path(void) {
#if defined(_WIN32)
    const char *appdata = getenv("APPDATA");
    return appdata ? appdata : ".";
#else
    const char *home = getenv("HOME");
    return home ? home : "/tmp";
#endif
}

const char *darwin_portable_hostname(void) {
    static char name[256];
#if defined(_WIN32)
    DWORD size = (DWORD)sizeof(name);
    if (!GetComputerNameA(name, &size)) snprintf(name, sizeof(name), "darwin-prototype");
#else
    if (gethostname(name, sizeof(name)) != 0) snprintf(name, sizeof(name), "darwin-prototype");
    name[sizeof(name) - 1] = '\0';
#endif
    return name;
}

void darwin_portable_boot(void) {
    printf("Darwin prototype booting\n");
    printf("host platform abstraction: portable C runtime\n");
    printf("Mach subsystem initialized\n");
    printf("BSD process layer ready\n");
    printf("IOKit-inspired registry online\n");
    printf("launchd service manager activated\n");
    printf("Darwin %s ready\n", DARWIN_PORTABLE_VERSION);
}

void darwin_portable_shutdown(void) {
    for (size_t i = 0; i < sizeof(service_running) / sizeof(service_running[0]); ++i) service_running[i] = false;
    printf("System shutdown initiated\nKernel state: shutdown\n");
}

void darwin_portable_print_services(void) {
    printf("PID  Status  Label\n");
    for (size_t i = 0; i < sizeof(services) / sizeof(services[0]); ++i)
        printf("%zu    %s  %s\n", i, service_running[i] ? "running" : "stopped", services[i]);
}

void darwin_portable_print_help(void) {
    printf("Darwin 27.0.0 portable observer\n");
    printf("commands: boot uname sw_vers sysctl launchctl hostname pwd whoami ls help shutdown\n");
    printf("targets: Linux macOS BSD Android Windows (user-space simulation)\n");
}

void darwin_portable_list(const char *path) {
#if defined(_WIN32)
    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\*", path);
    WIN32_FIND_DATAA data;
    HANDLE handle = FindFirstFileA(pattern, &data);
    if (handle == INVALID_HANDLE_VALUE) { fprintf(stderr, "ls: cannot open %s\n", path); return; }
    do { if (strcmp(data.cFileName, ".") && strcmp(data.cFileName, "..")) puts(data.cFileName); } while (FindNextFileA(handle, &data));
    FindClose(handle);
#else
    DIR *dir = opendir(path);
    if (!dir) { fprintf(stderr, "ls: cannot open %s\n", path); return; }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) puts(entry->d_name);
    closedir(dir);
#endif
}
