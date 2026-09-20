#include "darwin_adapter.h"
#include "darwin_observer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static darwin_observer_t observer;

static void print_help(void) {
    puts("darwin-shell commands:");
    puts("  boot | reboot | shutdown");
    puts("  ps [-axo] | top [-o cpu|mem]");
    puts("  sysctl -a | sysctl <oid> | kextstat");
    puts("  launchctl list | print <label> | start <label> | stop <label>");
    puts("  service status <label>");
    puts("  mach ports | mach send <port> <message>");
    puts("  iokit tree | ioreg");
    puts("  dmesg | log show [--last boot]");
    puts("  spawn <name> | kill <pid>");
    puts("  hostname | pwd | whoami | ls [path] | help | exit");
}

static char *next_token(char **cursor) {
    char *p = *cursor;
    while (*p == ' ' || *p == '\t') ++p;
    if (!*p || *p == '\n') { *cursor = p; return NULL; }
    char *start = p;
    while (*p && *p != ' ' && *p != '\t' && *p != '\n') ++p;
    if (*p) *p++ = '\0';
    *cursor = p;
    return start;
}

int main(void) {
    char line[512];
    darwin_observer_init(&observer);
    puts("Darwin Shell 27.0.0 (simulated user space)");
    print_help();

    for (;;) {
        fputs("darwin% ", stdout);
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;

        char *cursor = line;
        char *command = next_token(&cursor);
        char *arg1 = next_token(&cursor);
        char *arg2 = next_token(&cursor);
        if (!command) continue;

        if (!strcmp(command, "exit") || !strcmp(command, "quit")) break;
        if (!strcmp(command, "help")) { print_help(); continue; }
        if (!strcmp(command, "boot")) { darwin_observer_boot(&observer); puts("boot complete"); continue; }
        if (!strcmp(command, "reboot")) { darwin_observer_shutdown(&observer); darwin_observer_boot(&observer); puts("reboot complete"); continue; }
        if (!strcmp(command, "shutdown")) { darwin_observer_shutdown(&observer); puts("shutdown requested"); continue; }
        if (!strcmp(command, "ps")) { darwin_print_processes(&observer, 0, "cpu"); continue; }
        if (!strcmp(command, "top")) { darwin_print_processes(&observer, 1, (arg1 && (!strcmp(arg1, "-o") || !strcmp(arg1, "-sort")) && arg2) ? arg2 : "cpu"); continue; }

        if (!strcmp(command, "sysctl")) {
            if (!arg1 || !strcmp(arg1, "-a")) puts("kern.ostype: Darwin\nkern.osrelease: 27.0.0\nkern.version: Darwin Kernel Version 27.0.0\nkern.ipc.mach_ports: 64\nkern.proc.max: 32\nhw.machine: portable");
            else if (!strcmp(arg1, "kern.ostype")) puts("Darwin");
            else if (!strcmp(arg1, "kern.osrelease")) puts("27.0.0");
            else puts("sysctl: unknown oid");
            continue;
        }
        if (!strcmp(command, "kextstat")) { puts("Index Refs Address        Size       Name\n   0    1 0x00000000     0x1000     com.apple.kernel\n   1    1 0x00000000     0x0800     com.apple.iokit"); continue; }

        if (!strcmp(command, "launchctl")) {
            if (!arg1 || !strcmp(arg1, "list")) darwin_print_services(&observer);
            else if (!strcmp(arg1, "print") && arg2) { if (darwin_service_print(&observer, arg2)) puts("launchctl: service not found"); }
            else if ((!strcmp(arg1, "start") || !strcmp(arg1, "stop")) && arg2) { if (darwin_service_set_state(&observer, arg2, !strcmp(arg1, "start"))) puts("launchctl: service not found"); else puts(!strcmp(arg1, "start") ? "service started" : "service stopped"); }
            else puts("usage: launchctl list|print|start|stop <label>");
            continue;
        }
        if (!strcmp(command, "service") && arg1 && !strcmp(arg1, "status") && arg2) { if (darwin_service_print(&observer, arg2)) puts("service: not found"); continue; }
        if (!strcmp(command, "mach")) {
            if (arg1 && !strcmp(arg1, "ports")) darwin_print_ports(&observer);
            else if (arg1 && !strcmp(arg1, "send") && arg2) { char *message = next_token(&cursor); darwin_send_message(&observer, atoi(arg2), message ? message : "shell message"); puts("message sent"); }
            continue;
        }
        if (!strcmp(command, "iokit") || !strcmp(command, "ioreg")) { darwin_print_devices(&observer); continue; }
        if (!strcmp(command, "dmesg") || (!strcmp(command, "log") && arg1 && !strcmp(arg1, "show"))) { darwin_print_logs(&observer); continue; }
        if (!strcmp(command, "spawn") && arg1) { printf("spawned pid %d\n", darwin_spawn(&observer, arg1)); continue; }
        if (!strcmp(command, "kill") && arg1) { puts(darwin_kill(&observer, atoi(arg1)) ? "kill: no such process" : "process terminated"); continue; }
        if (!strcmp(command, "hostname")) { puts(darwin_adapter_hostname()); continue; }
        if (!strcmp(command, "pwd")) { puts(darwin_adapter_cwd()); continue; }
        if (!strcmp(command, "whoami")) { puts("root"); continue; }
        if (!strcmp(command, "ls")) { darwin_adapter_list(arg1 ? arg1 : "."); continue; }
        puts("darwin-shell: command not found");
    }
    return 0;
}
