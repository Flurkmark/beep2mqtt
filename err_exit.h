#ifndef HAS_ERR_EXIT_H
#define HAS_ERR_EXIT_h
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#define ERR_EXIT_BUF_SIZ 512
#define ERR_EXIT_SIZ_OF_INTSTRING 12

/* short hand name, that prepends filename and line number */ 
/* the ## before args, removes preceding non whitespace if args is empty */ 
#define EE(format, args...) \
err_exit(1, LOG_ERR, "%s:%d: " format ,__FILE__, __LINE__, ##args)


#define EW(format, args...) \
err_exit(0, LOG_WARNING, "%s:%d: " format ,__FILE__, __LINE__, ##args)

// EI.. error info, not my most clever name
#define EI(format, args...) \
    err_exit(0, LOG_INFO, "%s:%d: " format, __FILE__, __LINE__, ##args)

void err_exit(int, int, char *, ...);
#endif
