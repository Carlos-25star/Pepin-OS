#include "lib.h"

/*
 * memcpy: copia 'count' bytes de 'src' a 'dest'
 */
void *memcpy(void *dest, const void *src, u32 count)
{
    char *d = (char*)dest;
    const char *s = (const char*)src;
    
    while (count--)
        *d++ = *s++;
    
    return dest;
}

/*
 * memset: llena 'count' bytes de 'dest' con 'val'
 */
void *memset(void *dest, u8 val, u32 count)
{
    char *d = (char*)dest;
    
    while (count--)
        *d++ = val;
    
    return dest;
}

/*
 * strlen: calcula la longitud de una cadena
 */
u32 strlen(const char *s)
{
    u32 len = 0;
    
    while (*s++)
        len++;
    
    return len;
}

void insl(int port, void *addr, int cnt) {
    asm volatile(
        "cld\n\t"
        "repne\n\t"
        "insl"
        : "=D" (addr), "=c" (cnt)
        : "d" (port), "0" (addr), "1" (cnt)
        : "memory", "cc"
    );
}

// Escribir múltiples palabras de 32 bits a un puerto
void outsl(int port, const void *addr, int cnt) {
    asm volatile(
        "cld\n\t"
        "repne\n\t"
        "outsl"
        : "=S" (addr), "=c" (cnt)
        : "d" (port), "0" (addr), "1" (cnt)
        : "cc"
    );
}

/*
 * memcmp: compara dos bloques de memoria
 */
int memcmp(const void *s1, const void *s2, u32 n)
{
    const unsigned char *p1 = s1, *p2 = s2;
    while (n-- > 0) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}
