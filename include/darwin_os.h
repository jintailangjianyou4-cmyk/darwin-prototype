#ifndef DARWIN_OS_H
#define DARWIN_OS_H

#include <stdbool.h>

#define DARWIN_OS_NAME "Darwin"
#define DARWIN_OS_RELEASE "27.0.0"
#define DARWIN_BUILD_VERSION "27A"
#define DARWIN_DEFAULT_HOSTNAME "darwin-prototype"
#define DARWIN_DEFAULT_USER "root"
#define DARWIN_MAX_SERVICES 16
#define DARWIN_STATE_ENV "DARWIN_STATE_FILE"
#define DARWIN_STATE_DEFAULT "/tmp/darwin_prototype.state"

typedef enum {
    DARWIN_SERVICE_STOPPED,
    DARWIN_SERVICE_LOADING,
    DARWIN_SERVICE_RUNNING,
    DARWIN_SERVICE_STOPPING,
    DARWIN_SERVICE_WAITING
} darwin_service_state_t;

typedef struct {
    char label[64];
    char program[128];
    char description[160];
    darwin_service_state_t state;
    int pid;
    unsigned long runs;
    bool keep_alive;
} darwin_service_t;

typedef struct {
    char hostname[128];
    char username[64];
    char cwd[512];
    char kernel_state[32];
    bool booted;
    bool shutdown_requested;
    unsigned long event_sequence;
    darwin_service_t services[DARWIN_MAX_SERVICES];
    int service_count;
} darwin_state_t;

void darwin_state_init(darwin_state_t *state);
const char *darwin_state_path(void);
int darwin_state_load(darwin_state_t *state);
int darwin_state_save(const darwin_state_t *state);

void darwin_boot(darwin_state_t *state);
void darwin_shutdown(darwin_state_t *state);
void darwin_print_boot_log(const darwin_state_t *state);

const char *darwin_service_state_name(darwin_service_state_t state);
darwin_service_t *darwin_find_service(darwin_state_t *state, const char *label);
const darwin_service_t *darwin_find_service_const(const darwin_state_t *state, const char *label);
int darwin_service_start(darwin_state_t *state, const char *label);
int darwin_service_stop(darwin_state_t *state, const char *label);
void darwin_print_services(const darwin_state_t *state);
void darwin_print_service(const darwin_service_t *service);

void darwin_print_sysctl_all(void);
int darwin_print_sysctl(const char *key);
void darwin_print_help(void);

#endif
