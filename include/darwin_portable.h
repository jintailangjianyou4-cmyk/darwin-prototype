#ifndef DARWIN_PORTABLE_H
#define DARWIN_PORTABLE_H

#include <stdbool.h>

#define DARWIN_PORTABLE_NAME "Darwin"
#define DARWIN_PORTABLE_VERSION "27.0.0"
#define DARWIN_PORTABLE_BUILD "27A"

void darwin_portable_boot(void);
void darwin_portable_shutdown(void);
void darwin_portable_print_services(void);
void darwin_portable_print_help(void);
void darwin_portable_list(const char *path);
const char *darwin_portable_hostname(void);
const char *darwin_portable_state_path(void);

#endif
