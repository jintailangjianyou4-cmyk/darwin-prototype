#include "darwin_observer.h"
#include <stdio.h>
#include <string.h>

static void add_process(darwin_observer_t *o, int pid, int ppid, const char *name, const char *state, int cpu, int memory) {
    darwin_process_t *p = &o->processes[o->process_count++]; p->pid=pid; p->ppid=ppid; p->cpu=cpu; p->memory=memory;
    snprintf(p->name,sizeof(p->name),"%s",name); snprintf(p->state,sizeof(p->state),"%s",state);
}
void darwin_observer_init(darwin_observer_t *o) { memset(o,0,sizeof(*o)); o->next_port=100; add_process(o,0,0,"kernel_task","running",2,4096); add_process(o,1,0,"launchd","running",1,2048); add_process(o,2,1,"bsd_init","running",1,1024); add_process(o,3,1,"iokitd","sleeping",0,768); }
void darwin_observer_log(darwin_observer_t *o,const char *level,const char *message) { if(o->log_count>=DARWIN_MAX_LOGS)return; darwin_log_t *l=&o->logs[o->log_count++]; snprintf(l->timestamp,sizeof(l->timestamp),"%04d-01-01T00:%02d:00",27,o->log_count); snprintf(l->level,sizeof(l->level),"%s",level); snprintf(l->message,sizeof(l->message),"%s",message); }
void darwin_observer_boot(darwin_observer_t *o) { o->booted=1;o->shutdown_requested=0; darwin_observer_log(o,"INFO","Mach IPC subsystem initialized");darwin_observer_log(o,"INFO","BSD process layer initialized");darwin_observer_log(o,"INFO","IOKit registry published");darwin_observer_log(o,"INFO","launchd service manager ready"); }
void darwin_observer_shutdown(darwin_observer_t *o) { o->shutdown_requested=1;o->booted=0;darwin_observer_log(o,"NOTICE","services drained; kernel shutdown requested"); }
int darwin_spawn(darwin_observer_t *o,const char *name) { if(o->process_count>=DARWIN_MAX_PROCESSES)return -1; int pid=100+o->process_count;add_process(o,pid,1,name,"running",0,512);darwin_observer_log(o,"INFO","process spawned");return pid; }
int darwin_send_message(darwin_observer_t *o,int port,const char *message) { if(o->port_count>=DARWIN_MAX_PORTS)return -1; darwin_port_t *p=&o->ports[o->port_count++];p->name=port;p->owner_pid=1;snprintf(p->rights,sizeof(p->rights),"send+receive");char log[200];snprintf(log,sizeof(log),"mach_port %d message: %s",port,message);darwin_observer_log(o,"IPC",log);return 0; }
void darwin_print_processes(const darwin_observer_t *o,int top_mode) { puts("PID  PPID STATE     CPU% MEM(KB) COMMAND");for(int i=0;i<o->process_count;i++){const darwin_process_t*p=&o->processes[i];printf("%-4d %-4d %-9s %-4d %-7d %s\n",p->pid,p->ppid,p->state,p->cpu,p->memory,p->name);if(top_mode&&i>=7)break;} }
void darwin_print_ports(const darwin_observer_t *o) { puts("NAME OWNER RIGHTS");for(int i=0;i<o->port_count;i++)printf("%d %d %s\n",o->ports[i].name,o->ports[i].owner_pid,o->ports[i].rights); }
void darwin_print_devices(const darwin_observer_t *o) { puts("IOKit device tree");for(int i=0;i<o->device_count;i++)printf("+- %s (%s) [%s]\n",o->devices[i].name,o->devices[i].class_name,o->devices[i].state); }
void darwin_print_logs(const darwin_observer_t *o) { for(int i=0;i<o->log_count;i++)printf("%s [%s] %s\n",o->logs[i].timestamp,o->logs[i].level,o->logs[i].message); }
void darwin_print_services(void) { puts("PID STATUS LABEL\n0 running com.apple.kernel\n1 running com.apple.launchd\n2 running com.apple.bsd\n3 running com.apple.iokit"); }
int darwin_service_status(const char *service) { if(!strcmp(service,"com.apple.kernel")||!strcmp(service,"com.apple.launchd")||!strcmp(service,"com.apple.bsd")||!strcmp(service,"com.apple.iokit"))return 0;return -1; }
