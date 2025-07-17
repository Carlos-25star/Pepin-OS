#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "types.h"

/* Números de llamadas al sistema */
#define SYS_PRINT 1

/* Funciones */
void init_syscalls(void);
void do_syscalls(int sys_num);

#endif
