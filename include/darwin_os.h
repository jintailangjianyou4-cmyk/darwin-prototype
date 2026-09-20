#ifndef DARWIN_OS_H
#define DARWIN_OS_H

#include <stdbool.h>

#define DARWIN_OS_NAME "Darwin"
#define DARWIN_OS_RELEASE "27.0.0"
#define DARWIN_KERNEL_RELEASE "27.0.0"
#define DARWIN_BUILD_VERSION "27A"
#define DARWIN_HOSTNAME "darwin-prototype"
#define DARWIN_USERNAME "root"
#define DARWIN_DEFAULT_CWD "/"
#define DARWIN_STATE_ENV "DARWIN_STATE_FILE"
#define DARWIN_STATE_PATH "/tmp/darwin_runtime_state"
#define DARWIN_MAX_SERVICES 16
#define DARWIN_MAX_NAME 64

typedef struct {
    char name[64];
    char description[128];
    bool running;
} darwin_service_t;

typedef struct {
    char hostname[64];
    char username[32];
    char cwd[256];
    char kernel_state[32];
    bool booted;
    bool shutdown_requested;
    darwin_service_t services[DARWIN_MAX_SERVICES];
    int service_count;
} darwin_state_t;

void darwin_set_default_state(darwin_state_t *state);
const char *darwin_state_path(void);
int darwin_load_state(darwin_state_t *state);
int darwin_save_state(const darwin_state_t *state);
int darwin_boot_system(void);
int darwin_shutdown_system(void);
void darwin_register_default_services(darwin_state_t *state);
int darwin_update_service_status(const char *name, bool running);
const char *darwin_get_service_status_string(bool running);
void darwin_print_banner(void);

#endif
