#include "darwin_adapter.h"
#include "darwin_observer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static darwin_observer_t o;
static void help(void) { puts("darwin-shell commands:\n  boot | reboot | shutdown\n  ps [-axo] | top [-o cpu|mem]\n  sysctl -a | sysctl <key> | kextstat\n  launchctl list | launchctl print|start|stop <label>\n  service status <label>\n  mach ports | mach send <port> <message>\n  iokit tree | dmesg | log show\n  spawn <name> | kill <pid>\n  hostname | pwd | whoami | ls [path] | help | exit"); }
static char *next_token(char **s) { char *p=*s; while(*p==' '||*p=='\t')p++; if(!*p){*s=p;return NULL;} char *start=p; while(*p&&*p!=' '&&*p!='\t'&&*p!='\n')p++; if(*p)*p++='\0';*s=p;return start; }
int main(void) { char line[512]; darwin_observer_init(&o); puts("Darwin Shell 27.0.0 (simulated user space)"); help(); for(;;){ fputs("darwin% ",stdout);fflush(stdout);if(!fgets(line,sizeof(line),stdin))break; char *cursor=line,*cmd=next_token(&cursor); if(!cmd)continue; char *arg=next_token(&cursor),*arg2=next_token(&cursor);
 if(!strcmp(cmd,"exit")||!strcmp(cmd,"quit"))break;
 if(!strcmp(cmd,"help")){help();continue;}
 if(!strcmp(cmd,"boot")){darwin_observer_boot(&o);puts("boot complete");continue;}
 if(!strcmp(cmd,"reboot")){darwin_observer_shutdown(&o);darwin_observer_boot(&o);puts("reboot complete");continue;}
 if(!strcmp(cmd,"shutdown")){darwin_observer_shutdown(&o);puts("shutdown requested");continue;}
 if(!strcmp(cmd,"ps")){darwin_print_processes(&o,0,"cpu");continue;}
 if(!strcmp(cmd,"top")){darwin_print_processes(&o,1,(arg&&(!strcmp(arg,"-o")||!strcmp(arg,"-sort"))&&arg2)?arg2:"cpu");continue;}
 if(!strcmp(cmd,"sysctl")){if(!arg||!strcmp(arg,"-a")){puts("kern.ostype: Darwin\nkern.osrelease: 27.0.0\nkern.version: Darwin Kernel Version 27.0.0\nkern.ipc.mach_ports: 64\nkern.proc.max: 32\nhw.machine: portable");}else if(!strcmp(arg,"kern.ostype"))puts("Darwin");else if(!strcmp(arg,"kern.osrelease"))puts("27.0.0");else puts("sysctl: unknown oid");continue;}
 if(!strcmp(cmd,"kextstat")){puts("Index Refs Address        Size       Name\n   0    1 0x00000000     0x1000     com.apple.kernel\n   1    1 0x00000000     0x0800     com.apple.iokit");continue;}
 if(!strcmp(cmd,"launchctl")){if(!arg||!strcmp(arg,"list"))darwin_print_services(&o);else if(!strcmp(arg,"print")&&arg2){if(darwin_service_print(&o,arg2))puts("launchctl: service not found");}else if((!strcmp(arg,"start")||!strcmp(arg,"stop"))&&arg2){if(darwin_service_set_state(&o,arg2,!strcmp(arg,"start")))puts("launchctl: service not found");else puts(!strcmp(arg,"start")?"service started":"service stopped");}else puts("usage: launchctl list|print|start|stop <label>");continue;}
 if(!strcmp(cmd,"service")&&arg&&!strcmp(arg,"status")&&arg2){printf("%s: %s\n",arg2,darwin_service_print(&o,arg2)?"not found":"see details above");continue;}
 if(!strcmp(cmd,"mach")){if(arg&&!strcmp(arg,"ports"))darwin_print_ports(&o);else if(arg&&!strcmp(arg,"send")&&arg2){int p=atoi(arg2);char *msg=next_token(&cursor);darwin_send_message(&o,p,msg?msg:"shell message");puts("message sent");}continue;}
 if(!strcmp(cmd,"iokit")){if(arg&&!strcmp(arg,"tree"))darwin_print_devices(&o);continue;}
 if(!strcmp(cmd,"dmesg")||(!strcmp(cmd,"log")&&arg&&!strcmp(arg,"show"))){darwin_print_logs(&o);continue;}
 if(!strcmp(cmd,"spawn")&&arg){int pid=darwin_spawn(&o,arg);printf("spawned pid %d\n",pid);continue;}
 if(!strcmp(cmd,"kill")&&arg){printf(darwin_kill(&o,atoi(arg))?"kill: no such process\n":"process terminated\n");continue;}
 if(!strcmp(cmd,"hostname")){puts(darwin_adapter_hostname());continue;} if(!strcmp(cmd,"pwd")){puts(darwin_adapter_cwd());continue;} if(!strcmp(cmd,"whoami")){puts("root");continue;} if(!strcmp(cmd,"ls")){darwin_adapter_list(arg?arg:".");continue;}
 puts("darwin-shell: command not found"); }
 return 0; }
