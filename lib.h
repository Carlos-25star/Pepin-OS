#ifndef _LIB_H_
#define _LIB_H_

#include "types.h"

/* Funciones básicas de librería */
void *memcpy(void *dest, const void *src, u32 count);
void *memset(void *dest, u8 val, u32 count);
void insl(int port, void *addr, int cnt);
void outsl(int port, const void *addr, int cnt);
u32 strlen(const char *s);
int memcmp(const void *s1, const void *s2, u32 n);

#endif
