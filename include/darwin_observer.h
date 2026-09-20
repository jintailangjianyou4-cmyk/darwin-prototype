#ifndef DARWIN_OBSERVER_H
#define DARWIN_OBSERVER_H

#define DARWIN_VERSION "27.0.0"
#define DARWIN_MAX_PROCESSES 32
#define DARWIN_MAX_PORTS 64
#define DARWIN_MAX_LOGS 128
#define DARWIN_MAX_DEVICES 32
#define DARWIN_MAX_SERVICES 16

typedef struct {
    int pid, ppid;
    char name[48], state[16];
    int cpu, memory;
} darwin_process_t;

typedef struct {
    int name, owner_pid;
    char rights[32];
} darwin_port_t;

typedef struct {
    unsigned long sequence;
    char timestamp[24], level[12], subsystem[24], category[24], message[160];
} darwin_log_t;

typedef struct {
    char class_name[48], name[64], state[16];
    int parent;
} darwin_device_t;

typedef struct {
    char label[64], program[96], state[16];
    int pid, runs, keep_alive;
} darwin_service_t;

typedef struct {
    darwin_process_t processes[DARWIN_MAX_PROCESSES];
    int process_count;
    darwin_port_t ports[DARWIN_MAX_PORTS];
    int port_count, next_port;
    darwin_log_t logs[DARWIN_MAX_LOGS];
    int log_count;
    darwin_device_t devices[DARWIN_MAX_DEVICES];
    int device_count;
    darwin_service_t services[DARWIN_MAX_SERVICES];
    int service_count;
    unsigned long next_log_sequence;
    int booted, shutdown_requested;
} darwin_observer_t;

void darwin_observer_init(darwin_observer_t *o);
void darwin_observer_boot(darwin_observer_t *o);
void darwin_observer_shutdown(darwin_observer_t *o);
void darwin_observer_log(darwin_observer_t *o, const char *level, const char *message);
void darwin_observer_log_event(darwin_observer_t *o, const char *level, const char *subsystem, const char *category, const char *message);
void darwin_print_processes(const darwin_observer_t *o, int top_mode, const char *sort_key);
void darwin_print_ports(const darwin_observer_t *o);
void darwin_print_devices(const darwin_observer_t *o);
void darwin_print_logs(const darwin_observer_t *o, const char *subsystem_filter);
int darwin_spawn(darwin_observer_t *o, const char *name);
int darwin_kill(darwin_observer_t *o, int pid);
int darwin_send_message(darwin_observer_t *o, int port, const char *message);
void darwin_print_services(const darwin_observer_t *o);
int darwin_service_print(const darwin_observer_t *o, const char *label);
int darwin_service_set_state(darwin_observer_t *o, const char *label, int running);

#endif
