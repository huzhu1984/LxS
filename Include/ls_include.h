#ifndef LS_INCLUDE_H
#define LS_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <termios.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#endif

#include "ls_types.h"
#include "ls_common.h"

#ifdef __cplusplus
}
#endif

#endif // LS_INCLUDE_H
