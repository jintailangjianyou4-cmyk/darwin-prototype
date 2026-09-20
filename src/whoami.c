#include "darwin_os.h"
#include <stdio.h>
int main(void) { darwin_state_t s; darwin_state_load(&s); puts(s.username); return 0; }
