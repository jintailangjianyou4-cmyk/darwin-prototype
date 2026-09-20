#include "darwin_os.h"
#include <stdio.h>
#include <string.h>
int main(int argc, char **argv) { if (argc == 1 || !strcmp(argv[1], "-a")) darwin_print_sysctl_all(); else return darwin_print_sysctl(argv[1]); return 0; }
