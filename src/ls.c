#include "darwin_os.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
int main(int argc, char **argv) { const char *path=argc>1?argv[1]:"."; DIR *d=opendir(path); if(!d){perror("ls");return 1;} struct dirent *e; while((e=readdir(d))) if(strcmp(e->d_name,".")&&strcmp(e->d_name,"..")) puts(e->d_name); closedir(d); return 0; }
