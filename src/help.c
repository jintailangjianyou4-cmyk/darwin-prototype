#include "darwin_os.h"

#include <stdio.h>

int main(void) {
    printf("Available Darwin-inspired commands:\n");
    printf("  darwin_kernel  Boot the Darwin-style kernel emulation\n");
    printf("  uname          Show OS identity\n");
    printf("  sw_vers        Show product information\n");
    printf("  sysctl         Show kernel-like properties\n");
    printf("  launchctl      List or manage services\n");
    printf("  hostname       Show system hostname\n");
    printf("  pwd            Print working directory\n");
    printf("  whoami         Show effective user\n");
    printf("  ls             List directory entries\n");
    printf("  shutdown       Shut down the simulated system\n");
    return 0;
}
