#include "darwin_portable.h"

#include <stdio.h>
#include <string.h>

static void usage(void) { darwin_portable_print_help(); }

int main(int argc, char **argv) {
    const char *command = argc > 1 ? argv[1] : "help";

    if (!strcmp(command, "boot")) { darwin_portable_boot(); return 0; }
    if (!strcmp(command, "uname")) { printf("Darwin %s\n", DARWIN_PORTABLE_VERSION); return 0; }
    if (!strcmp(command, "sw_vers")) {
        puts("ProductName: Darwin"); puts("ProductVersion: 27.0.0"); puts("BuildVersion: 27A"); return 0;
    }
    if (!strcmp(command, "sysctl")) {
        puts("kern.ostype: Darwin"); puts("kern.osrelease: 27.0.0"); puts("kern.osrevision: 1"); puts("hw.machine: portable"); return 0;
    }
    if (!strcmp(command, "launchctl")) { darwin_portable_print_services(); return 0; }
    if (!strcmp(command, "hostname")) { puts(darwin_portable_hostname()); return 0; }
    if (!strcmp(command, "pwd")) { puts("/"); return 0; }
    if (!strcmp(command, "whoami")) { puts("root"); return 0; }
    if (!strcmp(command, "ls")) { darwin_portable_list(argc > 2 ? argv[2] : "."); return 0; }
    if (!strcmp(command, "shutdown")) { darwin_portable_shutdown(); return 0; }
    if (!strcmp(command, "help") || !strcmp(command, "--help")) { usage(); return 0; }

    fprintf(stderr, "darwinctl: unknown command: %s\n", command);
    usage();
    return 1;
}
