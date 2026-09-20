#include "darwin_os.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_string(char *dst, size_t size, const char *src) {
    snprintf(dst, size, "%s", src ? src : "");
}

static void add_service(darwin_state_t *state, const char *label, const char *program,
                        const char *description, int pid, bool keep_alive) {
    if (state->service_count >= DARWIN_MAX_SERVICES) return;
    darwin_service_t *service = &state->services[state->service_count++];
    set_string(service->label, sizeof(service->label), label);
    set_string(service->program, sizeof(service->program), program);
    set_string(service->description, sizeof(service->description), description);
    service->state = pid >= 0 ? DARWIN_SERVICE_RUNNING : DARWIN_SERVICE_STOPPED;
    service->pid = pid;
    service->runs = pid >= 0 ? 1UL : 0UL;
    service->keep_alive = keep_alive;
}

void darwin_state_init(darwin_state_t *state) {
    memset(state, 0, sizeof(*state));
    set_string(state->hostname, sizeof(state->hostname), DARWIN_DEFAULT_HOSTNAME);
    set_string(state->username, sizeof(state->username), DARWIN_DEFAULT_USER);
    set_string(state->cwd, sizeof(state->cwd), "/");
    set_string(state->kernel_state, sizeof(state->kernel_state), "cold");
    add_service(state, "com.apple.kernel", "/mach_kernel", "Darwin kernel core", 0, true);
    add_service(state, "com.apple.launchd", "/sbin/launchd", "System service manager", 1, true);
    add_service(state, "com.apple.bsd", "/sbin/bsd_init", "BSD process and VFS layer", 2, true);
    add_service(state, "com.apple.iokit", "/sbin/iokitd", "IOKit registry service", 3, true);
    add_service(state, "com.example.darwin-observer", "darwin-shell", "Example user service", -1, true);
}

const char *darwin_state_path(void) {
    const char *path = getenv(DARWIN_STATE_ENV);
    return path && *path ? path : DARWIN_STATE_DEFAULT;
}

int darwin_state_load(darwin_state_t *state) {
    /* The prototype deliberately starts from a deterministic model. */
    darwin_state_init(state);
    FILE *file = fopen(darwin_state_path(), "r");
    if (!file) return 0;

    char key[64], value[256];
    while (fscanf(file, "%63[^=]=%255[^\n]\n", key, value) == 2) {
        if (!strcmp(key, "hostname")) set_string(state->hostname, sizeof(state->hostname), value);
        else if (!strcmp(key, "username")) set_string(state->username, sizeof(state->username), value);
        else if (!strcmp(key, "cwd")) set_string(state->cwd, sizeof(state->cwd), value);
        else if (!strcmp(key, "kernel_state")) set_string(state->kernel_state, sizeof(state->kernel_state), value);
        else if (!strcmp(key, "booted")) state->booted = atoi(value) != 0;
        else if (!strcmp(key, "shutdown_requested")) state->shutdown_requested = atoi(value) != 0;
    }
    fclose(file);
    return 0;
}

int darwin_state_save(const darwin_state_t *state) {
    FILE *file = fopen(darwin_state_path(), "w");
    if (!file) {
        fprintf(stderr, "darwin: cannot save state: %s\n", strerror(errno));
        return -1;
    }
    fprintf(file, "hostname=%s\nusername=%s\ncwd=%s\nkernel_state=%s\nbooted=%d\nshutdown_requested=%d\n",
            state->hostname, state->username, state->cwd, state->kernel_state,
            state->booted ? 1 : 0, state->shutdown_requested ? 1 : 0);
    fclose(file);
    return 0;
}

const char *darwin_service_state_name(darwin_service_state_t state) {
    switch (state) {
        case DARWIN_SERVICE_LOADING: return "loading";
        case DARWIN_SERVICE_RUNNING: return "running";
        case DARWIN_SERVICE_STOPPING: return "stopping";
        case DARWIN_SERVICE_WAITING: return "waiting";
        default: return "stopped";
    }
}

darwin_service_t *darwin_find_service(darwin_state_t *state, const char *label) {
    for (int i = 0; i < state->service_count; ++i)
        if (!strcmp(state->services[i].label, label)) return &state->services[i];
    return NULL;
}

const darwin_service_t *darwin_find_service_const(const darwin_state_t *state, const char *label) {
    for (int i = 0; i < state->service_count; ++i)
        if (!strcmp(state->services[i].label, label)) return &state->services[i];
    return NULL;
}

int darwin_service_start(darwin_state_t *state, const char *label) {
    darwin_service_t *service = darwin_find_service(state, label);
    if (!service) return -1;
    if (service->state == DARWIN_SERVICE_RUNNING || service->state == DARWIN_SERVICE_LOADING) return 0;
    service->state = DARWIN_SERVICE_LOADING;
    service->state = DARWIN_SERVICE_RUNNING;
    service->pid = 100 + (int)service->runs + 1;
    service->runs++;
    return 0;
}

int darwin_service_stop(darwin_state_t *state, const char *label) {
    darwin_service_t *service = darwin_find_service(state, label);
    if (!service) return -1;
    if (service->state == DARWIN_SERVICE_STOPPED || service->state == DARWIN_SERVICE_STOPPING) return 0;
    service->state = DARWIN_SERVICE_STOPPING;
    service->pid = -1;
    service->state = DARWIN_SERVICE_STOPPED;
    return 0;
}

void darwin_boot(darwin_state_t *state) {
    state->booted = true;
    state->shutdown_requested = false;
    set_string(state->kernel_state, sizeof(state->kernel_state), "running");
    for (int i = 0; i < state->service_count; ++i) {
        if (state->services[i].keep_alive) {
            state->services[i].state = DARWIN_SERVICE_RUNNING;
            if (state->services[i].pid < 0) state->services[i].pid = i;
        }
    }
    darwin_state_save(state);
}

void darwin_shutdown(darwin_state_t *state) {
    state->shutdown_requested = true;
    for (int i = state->service_count - 1; i >= 0; --i)
        darwin_service_stop(state, state->services[i].label);
    state->booted = false;
    set_string(state->kernel_state, sizeof(state->kernel_state), "shutdown");
    darwin_state_save(state);
}

void darwin_print_services(const darwin_state_t *state) {
    puts("PID  Status   Label");
    for (int i = 0; i < state->service_count; ++i) {
        const darwin_service_t *s = &state->services[i];
        printf("%-4d %-8s %s\n", s->pid, darwin_service_state_name(s->state), s->label);
    }
}

void darwin_print_service(const darwin_service_t *s) {
    printf("system/%s\n  state = %s\n  pid = %d\n  program = %s\n  runs = %lu\n  keepalive = %s\n  description = %s\n",
           s->label, darwin_service_state_name(s->state), s->pid, s->program,
           s->runs, s->keep_alive ? "true" : "false", s->description);
}

void darwin_print_sysctl_all(void) {
    puts("kern.ostype: Darwin\nkern.osrelease: 27.0.0\nkern.osrevision: 1");
    puts("kern.version: Darwin Kernel Version 27.0.0: xnu-prototype");
    puts("kern.hostname: darwin-prototype\nkern.maxproc: 32\nkern.maxfiles: 10240");
    puts("kern.ipc.mach_ports: 64\nkern.vm.pagesize: 4096");
    puts("hw.machine: portable\nhw.model: DarwinPrototype1,1\nhw.ncpu: 4\nhw.memsize: 4294967296");
    puts("hw.byteorder: 1234\nhw.optional_64bit_ops: 1");
    puts("vfs.rootfs: virtual-apfs\nvfs.generic.maxtimes: 1");
}

int darwin_print_sysctl(const char *key) {
    if (!strcmp(key, "kern.ostype")) puts("Darwin");
    else if (!strcmp(key, "kern.osrelease")) puts("27.0.0");
    else if (!strcmp(key, "hw.ncpu")) puts("4");
    else if (!strcmp(key, "hw.memsize")) puts("4294967296");
    else if (!strcmp(key, "kern.vm.pagesize")) puts("4096");
    else { fprintf(stderr, "sysctl: unknown oid '%s'\n", key); return -1; }
    return 0;
}

void darwin_print_boot_log(const darwin_state_t *state) {
    (void)state;
    puts("00:00:01 kernel: handoff complete");
    puts("00:00:02 mach: IPC subsystem initialized");
    puts("00:00:03 bsd: VM, credentials, and VFS initialized");
    puts("00:00:04 iokit: platform expert matched root device");
    puts("00:00:05 iokit: registry published");
    puts("00:00:06 apfs: virtual root filesystem mounted");
    puts("00:00:07 launchd: bootstrap namespace created");
    puts("00:00:08 launchd: service set loaded");
    puts("00:00:09 loginwindow: multi-user state reached");
}

void darwin_print_help(void) {
    puts("Darwin prototype commands:");
    puts("  darwin_kernel, uname, sw_vers, sysctl, launchctl");
    puts("  hostname, pwd, whoami, ls, help, shutdown");
}
