#include "darwin_observer.h"
#include <stdio.h>
#include <string.h>

static void add_process(darwin_observer_t *o, int pid, int ppid, const char *name, const char *state, int cpu, int memory) {
    if (o->process_count >= DARWIN_MAX_PROCESSES) return;
    darwin_process_t *p = &o->processes[o->process_count++];
    p->pid = pid; p->ppid = ppid; p->cpu = cpu; p->memory = memory;
    snprintf(p->name, sizeof(p->name), "%s", name);
    snprintf(p->state, sizeof(p->state), "%s", state);
}

static void add_service(darwin_observer_t *o, const char *label, const char *program, int pid, const char *state, int keep_alive) {
    darwin_service_t *s = &o->services[o->service_count++];
    snprintf(s->label, sizeof(s->label), "%s", label);
    snprintf(s->program, sizeof(s->program), "%s", program);
    snprintf(s->state, sizeof(s->state), "%s", state);
    s->pid = pid; s->runs = 1; s->keep_alive = keep_alive;
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
    snprintf(o->devices[0].class_name, 48, "IOPlatformExpertDevice"); snprintf(o->devices[0].name, 64, "platform"); snprintf(o->devices[0].state, 16, "published"); o->devices[0].parent = -1;
    snprintf(o->devices[1].class_name, 48, "IONetworkInterface"); snprintf(o->devices[1].name, 64, "en0"); snprintf(o->devices[1].state, 16, "online"); o->devices[1].parent = 0;
    snprintf(o->devices[2].class_name, 48, "IOUSBController"); snprintf(o->devices[2].name, 64, "usb0"); snprintf(o->devices[2].state, 16, "online"); o->devices[2].parent = 0;
    snprintf(o->devices[3].class_name, 48, "IONVMeController"); snprintf(o->devices[3].name, 64, "nvme0"); snprintf(o->devices[3].state, 16, "online"); o->devices[3].parent = 0;
    snprintf(o->devices[4].class_name, 48, "IOAudioEngine"); snprintf(o->devices[4].name, 64, "audio0"); snprintf(o->devices[4].state, 16, "ready"); o->devices[4].parent = 0;
}

void darwin_observer_log(darwin_observer_t *o, const char *level, const char *message) {
    if (o->log_count >= DARWIN_MAX_LOGS) return;
    darwin_log_t *l = &o->logs[o->log_count++];
    snprintf(l->timestamp, sizeof(l->timestamp), "00:%02d:%02d", o->log_count / 60, o->log_count % 60);
    snprintf(l->level, sizeof(l->level), "%s", level);
    snprintf(l->message, sizeof(l->message), "%s", message);
}

void darwin_observer_boot(darwin_observer_t *o) {
    o->booted = 1; o->shutdown_requested = 0;
    darwin_observer_log(o, "INFO", "Darwin boot: kernel phase entered");
    darwin_observer_log(o, "INFO", "Mach IPC subsystem initialized");
    darwin_observer_log(o, "INFO", "BSD process and VFS layers initialized");
    darwin_observer_log(o, "INFO", "IOKit registry published");
    darwin_observer_log(o, "INFO", "launchd service manager ready");
}

void darwin_observer_shutdown(darwin_observer_t *o) {
    o->shutdown_requested = 1; o->booted = 0;
    for (int i = 0; i < o->service_count; ++i) {
        snprintf(o->services[i].state, sizeof(o->services[i].state), "stopped");
        o->services[i].pid = -1;
    }
    darwin_observer_log(o, "NOTICE", "launchd drained services");
    darwin_observer_log(o, "NOTICE", "kernel shutdown requested");
}

static int process_value(const darwin_process_t *p, const char *key) { return strcmp(key, "mem") == 0 ? p->memory : p->cpu; }
void darwin_print_processes(const darwin_observer_t *o, int top_mode, const char *sort_key) {
    puts(top_mode ? "Processes: 4 total, sorted by CPU (refresh 1s)" : "  PID  PPID  STAT  %CPU  RSS(KB)  COMMAND");
    if (top_mode) puts("  PID  USER     %CPU  %MEM  STATE     COMMAND");
    int shown = top_mode ? 5 : o->process_count;
    for (int pass = 0; pass < o->process_count && pass < shown; ++pass) {
        int best = -1;
        for (int i = 0; i < o->process_count; ++i) {
            int used = 0; for (int j = 0; j < pass; ++j) if (o->processes[j].pid == o->processes[i].pid) used = 1;
            if (!used && (best < 0 || process_value(&o->processes[i], sort_key) > process_value(&o->processes[best], sort_key))) best = i;
        }
        if (best < 0) break;
        const darwin_process_t *p = &o->processes[best];
        if (top_mode) printf("%5d  root     %3d%%  %3d%%  %-8s %s\n", p->pid, p->cpu, p->memory / 100, p->state, p->name);
        else printf("%5d  %4d  %-5s %4d  %7d  %s\n", p->pid, p->ppid, p->state, p->cpu, p->memory, p->name);
    }
}
void darwin_print_ports(const darwin_observer_t *o) { puts("NAME  OWNER  RIGHTS"); for (int i=0;i<o->port_count;i++) printf("%4d  %5d  %s\n",o->ports[i].name,o->ports[i].owner_pid,o->ports[i].rights); }
void darwin_print_devices(const darwin_observer_t *o) { puts("IORegistry:/"); for (int i=0;i<o->device_count;i++) printf("%s %s (%s) [%s]\n",o->devices[i].parent < 0 ? "+-" : "  +-",o->devices[i].name,o->devices[i].class_name,o->devices[i].state); }
void darwin_print_logs(const darwin_observer_t *o) { for (int i=0;i<o->log_count;i++) printf("%s kernel[%s]: %s\n",o->logs[i].timestamp,o->logs[i].level,o->logs[i].message); }
int darwin_spawn(darwin_observer_t *o, const char *name) { if (o->process_count >= DARWIN_MAX_PROCESSES) return -1; int pid=100+o->process_count; add_process(o,pid,1,name,"running",0,512); char msg[128]; snprintf(msg,sizeof(msg),"BSD process %d (%s) created",pid,name); darwin_observer_log(o,"INFO",msg); return pid; }
int darwin_kill(darwin_observer_t *o, int pid) { for (int i=0;i<o->process_count;i++) if(o->processes[i].pid==pid && pid>3){ char msg[96]; snprintf(msg,sizeof(msg),"process %d terminated",pid); snprintf(o->processes[i].state,sizeof(o->processes[i].state),"zombie"); darwin_observer_log(o,"INFO",msg); return 0; } return -1; }
int darwin_send_message(darwin_observer_t *o,int port,const char *message) { if(o->port_count>=DARWIN_MAX_PORTS)return -1; darwin_port_t*p=&o->ports[o->port_count++];p->name=port;p->owner_pid=1;snprintf(p->rights,sizeof(p->rights),"send+receive");char log[200];snprintf(log,sizeof(log),"mach_msg port=%d: %s",port,message);darwin_observer_log(o,"IPC",log);return 0; }
void darwin_print_services(const darwin_observer_t *o) { puts("PID  Status   Label"); for(int i=0;i<o->service_count;i++) printf("%-4d %-8s %s\n",o->services[i].pid,o->services[i].state,o->services[i].label); }
int darwin_service_print(const darwin_observer_t *o,const char *label) { for(int i=0;i<o->service_count;i++) if(!strcmp(o->services[i].label,label)){const darwin_service_t*s=&o->services[i];printf("gui/%s\n\tstate = %s\n\tpid = %d\n\tprogram = %s\n\truns = %d\n\tkeepalive = %s\n",s->label,s->state,s->pid,s->program,s->runs,s->keep_alive?"true":"false");return 0;}return -1; }
int darwin_service_set_state(darwin_observer_t *o,const char *label,int running) { for(int i=0;i<o->service_count;i++) if(!strcmp(o->services[i].label,label)){darwin_service_t*s=&o->services[i];snprintf(s->state,sizeof(s->state),"%s",running?"running":"stopped");s->pid=running?200+i:-1;s->runs+=running?1:0;char msg[160];snprintf(msg,sizeof(msg),"launchd %s service %s",running?"started":"stopped",label);darwin_observer_log(o,"INFO",msg);return 0;}return -1; }
