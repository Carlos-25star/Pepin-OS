#include "types.h"
#include "lib.h"
#include "screen.h"
#include "io.h"

void do_syscalls(int sys_num)
{
    char *u_str;
    int i;

    if (sys_num == 1) {
        /* Obtener el puntero a la cadena desde el registro EBX */
        asm("mov %%ebx, %0": "=m"(u_str) :);
        
        /* Temporización para demostrar la preempción */
        for (i = 0; i < 100000; i++);
        
        /* Deshabilitar interrupciones durante la escritura para evitar corrupción */
        cli;
        
        /* Cambiar el color para mostrar que es output de usuario */
        u8 old_attr = kattr;
        kattr = 0x0A;  /* Verde brillante */
        
        print("user   : ");
        print(u_str);
        
        /* Restaurar el color original */
        kattr = old_attr;
        
        /* Rehabilitar interrupciones */
        sti;
    } else {
        print("syscall: unknown system call ");
        print_dec(sys_num);
        print("\n");
    }

    return;
}
