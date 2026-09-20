#ifndef DARWIN_ADAPTER_H
#define DARWIN_ADAPTER_H

const char *darwin_adapter_hostname(void);
const char *darwin_adapter_cwd(void);
void darwin_adapter_list(const char *path);
const char *darwin_adapter_name(void);

#endif
