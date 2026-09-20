#include "darwin_observer.h"
#include <stdio.h>
#include <string.h>

static void add_process(darwin_observer_t *o, int pid, int ppid, const char *name, const char *state, int cpu, int memory) {
    if (o->process_count >= DARWIN_MAX_PROCESSES) return;
    darwin_process_t *p = &o->processes[o->process_count++];
    p->pid = pid;
    p->ppid = ppid;
    p->cpu = cpu;
    p->memory = memory;
    snprintf(p->name, sizeof(p->name), "%s", name);
    snprintf(p->state, sizeof(p->state), "%s", state);
}

static void add_service(darwin_observer_t *o, const char *label, const char *program, int pid, const char *state, int keep_alive) {
    if (o->service_count >= DARWIN_MAX_SERVICES) return;
    darwin_service_t *s = &o->services[o->service_count++];
    snprintf(s->label, sizeof(s->label), "%s", label);
    snprintf(s->program, sizeof(s->program), "%s", program);
    snprintf(s->state, sizeof(s->state), "%s", state);
    s->pid = pid;
    s->runs = 1;
    s->keep_alive = keep_alive;
}

static void add_device(darwin_observer_t *o, int index, int parent, const char *class_name, const char *name, const char *state) {
    snprintf(o->devices[index].class_name, sizeof(o->devices[index].class_name), "%s", class_name);
    snprintf(o->devices[index].name, sizeof(o->devices[index].name), "%s", name);
    snprintf(o->devices[index].state, sizeof(o->devices[index].state), "%s", state);
    o->devices[index].parent = parent;
}

void darwin_observer_init(darwin_observer_t *o) {
    memset(o, 0, sizeof(*o));
    o->next_port = 100;

    add_process(o, 0, 0, "kernel_task", "running", 2, 4096);
    add_process(o, 1, 0, "launchd", "running", 1, 2048);
    add_process(o, 2, 1, "bsd_init", "running", 1, 1024);
    add_process(o, 3, 1, "iokitd", "sleeping", 0, 768);

    add_service(o, "com.apple.kernel", "/mach_kernel", 0, "running", 1);
    add_service(o, "com.apple.launchd", "/sbin/launchd", 1, "running", 1);
    add_service(o, "com.apple.bsd", "/sbin/bsd_init", 2, "running", 1);
    add_service(o, "com.apple.iokit", "/sbin/iokitd", 3, "running", 1);
    add_service(o, "com.example.darwin-observer", "darwin-shell", -1, "stopped", 1);

    o->device_count = 5;
    add_device(o, 0, -1, "IOPlatformExpertDevice", "platform", "published");
    add_device(o, 1, 0, "IONetworkInterface", "en0", "online");
    add_device(o, 2, 0, "IOUSBController", "usb0", "online");
    add_device(o, 3, 0, "IONVMeController", "nvme0", "online");
    add_device(o, 4, 0, "IOAudioEngine", "audio0", "ready");
}

void darwin_observer_log(darwin_observer_t *o, const char *level, const char *message) {
    if (o->log_count >= DARWIN_MAX_LOGS) return;
    darwin_log_t *log = &o->logs[o->log_count++];
    snprintf(log->timestamp, sizeof(log->timestamp), "00:%02d:%02d", o->log_count / 60, o->log_count % 60);
    snprintf(log->level, sizeof(log->level), "%s", level);
    snprintf(log->message, sizeof(log->message), "%s", message);
}

void darwin_observer_boot(darwin_observer_t *o) {
    o->booted = 1;
    o->shutdown_requested = 0;
    darwin_observer_log(o, "INFO", "Darwin boot: kernel phase entered");
    darwin_observer_log(o, "INFO", "Mach zones and IPC subsystem initialized");
    darwin_observer_log(o, "INFO", "BSD process, credential, and VFS layers initialized");
    darwin_observer_log(o, "INFO", "IOKit registry published platform expert");
    darwin_observer_log(o, "INFO", "launchd bootstrap namespace created");
    darwin_observer_log(o, "INFO", "Darwin userspace reached multi-user state");
}

void darwin_observer_shutdown(darwin_observer_t *o) {
    o->shutdown_requested = 1;
    o->booted = 0;
    darwin_observer_log(o, "NOTICE", "shutdown: stopping launchd-managed services");
    for (int i = 0; i < o->service_count; ++i) {
        if (!strcmp(o->services[i].state, "running")) {
            snprintf(o->services[i].state, sizeof(o->services[i].state), "stopped");
            o->services[i].pid = -1;
            char message[160];
            snprintf(message, sizeof(message), "launchd stopped %s", o->services[i].label);
            darwin_observer_log(o, "NOTICE", message);
        }
    }
    darwin_observer_log(o, "NOTICE", "shutdown: kernel halt requested");
}

static int process_value(const darwin_process_t *p, const char *key) {
    return !strcmp(key, "mem") || !strcmp(key, "memory") ? p->memory : p->cpu;
}

void darwin_print_processes(const darwin_observer_t *o, int top_mode, const char *sort_key) {
    puts(top_mode ? "Processes: virtual table; refresh interval 1s" : "  PID  PPID  STAT   %CPU  RSS(KB)  COMMAND");
    if (top_mode) puts("  PID  USER     %CPU  %MEM  STATE     COMMAND");

    int shown = top_mode ? 5 : o->process_count;
    int selected[DARWIN_MAX_PROCESSES] = {0};
    for (int pass = 0; pass < o->process_count && pass < shown; ++pass) {
        int best = -1;
        for (int i = 0; i < o->process_count; ++i) {
            if (!selected[i] && (best < 0 || process_value(&o->processes[i], sort_key) > process_value(&o->processes[best], sort_key))) best = i;
        }
        if (best < 0) break;
        selected[best] = 1;
        const darwin_process_t *p = &o->processes[best];
        if (top_mode) printf("%5d  root     %3d%%  %3d%%  %-8s %s\n", p->pid, p->cpu, p->memory / 100, p->state, p->name);
        else printf("%5d  %4d  %-6s %4d  %7d  %s\n", p->pid, p->ppid, p->state, p->cpu, p->memory, p->name);
    }
}

void darwin_print_ports(const darwin_observer_t *o) {
    puts("NAME  OWNER  RIGHTS");
    for (int i = 0; i < o->port_count; ++i) printf("%4d  %5d  %s\n", o->ports[i].name, o->ports[i].owner_pid, o->ports[i].rights);
}

void darwin_print_devices(const darwin_observer_t *o) {
    puts("IORegistry:/");
    for (int i = 0; i < o->device_count; ++i) printf("%s %s (%s) [%s]\n", o->devices[i].parent < 0 ? "+-" : "  +‑", o->devices[i].name, o->devices[i].class_name, o->devices[i].state);
}

void darwin_print_logs(const darwin_observer_t *o) {
    for (int i = 0; i < o->log_count; ++i) printf("%s kernel[%s]: %s\n", o->logs[i].timestamp, o->logs[i].level, o->logs[i].message);
}

int darwin_spawn(darwin_observer_t *o, const char *name) {
    if (o->process_count >= DARWIN_MAX_PROCESSES) return -1;
    int pid = 100 + o->process_count;
    add_process(o, pid, 1, name, "running", 0, 512);
    char message[128];
    snprintf(message, sizeof(message), "bsd: process %d (%s) created", pid, name);
    darwin_observer_log(o, "INFO", message);
    return pid;
}

int darwin_kill(darwin_observer_t *o, int pid) {
    for (int i = 0; i < o->process_count; ++i) {
        if (o->processes[i].pid == pid && pid > 3) {
            char message[96];
            snprintf(message, sizeof(message), "bsd: process %d terminated", pid);
            snprintf(o->processes[i].state, sizeof(o->processes[i].state), "zombie");
            darwin_observer_log(o, "INFO", message);
            return 0;
        }
    }
    return -1;
}

int darwin_send_message(darwin_observer_t *o, int port, const char *message) {
    if (o->port_count >= DARWIN_MAX_PORTS) return -1;
    darwin_port_t *p = &o->ports[o->port_count++];
    p->name = port;
    p->owner_pid = 1;
    snprintf(p->rights, sizeof(p->rights), "send+receive");
    char log_message[200];
    snprintf(log_message, sizeof(log_message), "mach_msg: port=%d payload=%s", port, message);
    darwin_observer_log(o, "IPC", log_message);
    return 0;
}

void darwin_print_services(const darwin_observer_t *o) {
    puts("PID  Status   Label");
    for (int i = 0; i < o->service_count; ++i) printf("%-4d %-8s %s\n", o->services[i].pid, o->services[i].state, o->services[i].label);
}

int darwin_service_print(const darwin_observer_t *o, const char *label) {
    for (int i = 0; i < o->service_count; ++i) {
        if (!strcmp(o->services[i].label, label)) {
            const darwin_service_t *s = &o->services[i];
            printf("system/%s\n\tstate = %s\n\tpid = %d\n\tprogram = %s\n\truns = %d\n\tkeepalive = %s\n", s->label, s->state, s->pid, s->program, s->runs, s->keep_alive ? "true" : "false");
            return 0;
        }
    }
    return -1;
}

int darwin_service_set_state(darwin_observer_t *o, const char *label, int running) {
    for (int i = 0; i < o->service_count; ++i) {
        if (!strcmp(o->services[i].label, label)) {
            darwin_service_t *s = &o->services[i];
            snprintf(s->state, sizeof(s->state), "%s", running ? "running" : "stopped");
            s->pid = running ? 200 + i : -1;
            if (running) s->runs++;
            char message[160];
            snprintf(message, sizeof(message), "launchd %s service %s", running ? "started" : "stopped", label);
            darwin_observer_log(o, "INFO", message);
            return 0;
        }
    }
    return -1;
}
