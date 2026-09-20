#include "darwin_os.h"

#include <stdio.h>

int main(void) {
    if (darwin_shutdown_system() != 0) {
        fprintf(stderr, "shutdown: failed to initiate Darwin shutdown\n");
        return 1;
    }

    printf("System shutdown initiated\n");
    printf("Kernel state: shutdown\n");
    return 0;
}
