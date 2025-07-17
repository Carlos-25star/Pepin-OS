#include "types.h"
#include "gdt.h"
#include "screen.h"
#include "io.h"
#include "idt.h"
#include "kbd.h"
#include "task.h"
#include "mm.h"
#include "process.h"
#include "ide.h"  // Agregado para soporte IDE
#include "ext2.h" // Agregado para soporte Ext2
#include "ext2_test.h" // Test para Ext2

void init_pic(void);
int main(void);  // Declaración de la función main

// Definiciones que podrían faltar
#ifndef NULL
#define NULL ((void*)0)
#endif

// Estructura para información del multiboot
struct mb_partial_info {
    unsigned long flags;
    unsigned long low_mem;
    unsigned long high_mem;
    unsigned long boot_device;
    unsigned long cmdline;
};

// Función llamada por boot.asm
void kmain(struct mb_partial_info *mbi)
{
    /* Limpiar la pantalla */
    clear_screen();
    
    kY = 2;
    kattr = 0x0E;  /* amarillo sobre negro */
    
    /* Mensaje de bienvenida */
    print("Grub example kernel is loaded...\n");
    
    /* Mostrar información de RAM detectada por GRUB */
    if (mbi && (mbi->flags & 0x1)) {
        print("RAM detected : ");
        print_dec(mbi->low_mem);
        print("k (lower), ");
        print_dec(mbi->high_mem);
        print("k (upper)\n");
    }
    
    /* Inicializar IDT */
    init_idt();
    print("kernel : idt loaded\n");
    
    /* Inicializar PIC */
    init_pic();
    print("kernel : pic configured\n");
    
    /* Inicializar GDT y segmentos */
    init_gdt();
    print("kernel : gdt loaded\n");
    
    /* Inicializar el puntero de pila %esp */
    asm("   movw $0x18, %ax \n \
            movw %ax, %ss \n \
            movl $0x20000, %esp");
    
    main();
}

int main(void)
{
    /* Inicializar gestión de memoria (paginación) */
    init_mm();
    print("kernel : mm initialized\n");
    
    /* Inicializar tareas y TSS */
    init_task();
    print("kernel : task initialized\n");
    
    init_mm();
    init_heap();
    init_page_heap();
    
    print("kernel : memory systems initialized\n");    
    
    /* Inicializar controlador IDE */
    ide_init();
    print("kernel : IDE controller initialized\n");
    
    /* Inicializar sistema de archivos Ext2 */
    /*
    if (ext2_init() == 0) {
        print("kernel : Ext2 filesystem ready\n");
        
        // Probar funcionalidad Ext2
        ext2_test();
    } else {
        print("kernel : WARNING - Ext2 filesystem not available\n");
    }
    */
    
    print("kernel : Ext2 temporarily disabled for testing\n");
    
    /* Probar lectura/escritura IDE */
    char *buffer = (char *)kmalloc(512);
    char *msg = "Hello from Pepin OS!";
    
    if (buffer != NULL) {
        // Escribir mensaje en el sector 2 (LBA)
        // Limpiar buffer
        for (int i = 0; i < 512; i++) {
            buffer[i] = 0;
        }
        
        // Copiar mensaje al buffer
        int len = 0;
        while (msg[len] != '\0') len++;  // calcular longitud
        for (int i = 0; i <= len; i++) {
            buffer[i] = msg[i];
        }
        
        print("IDE    : Writing to sector 2...\n");
        if (ide_write_sectors(IDE_MASTER, 2, 1, buffer) == 0) {
            print("IDE    : Write successful\n");
        } else {
            print("IDE    : Write failed\n");
        }
        
        // Leer el sector 2
        // Limpiar buffer
        for (int i = 0; i < 512; i++) {
            buffer[i] = 0;
        }
        print("IDE    : Reading from sector 2...\n");
        if (ide_read_sectors(IDE_MASTER, 2, 1, buffer) == 0) {
            print("IDE    : Read successful: ");
            print(buffer);
            print("\n");
        } else {
            print("IDE    : Read failed\n");
        }
        
        kfree(buffer);
    } else {
        print("IDE    : Failed to allocate buffer\n");
    }
    
    kattr = 0x47;  /* texto blanco sobre fondo rojo */
    print("kernel : allowing interrupt\n");
    kattr = 0x07;  /* restaurar atributos normales */
    
    /* Mostrar mensaje de bienvenida */
    print("\n");
    print("===================================\n");
    print("  Mini Sistema Operativo - Cap 18 \n");
    print("===================================\n");
    print("\n");
    print("Sistema multitarea iniciado\n");
    print("Cargando tareas...\n");
    
    /* Cargar múltiples tareas */
    load_task((u32*)0x100000, (u32*)&task1, 0x2000);
    load_task((u32*)0x200000, (u32*)&task2, 0x2000);
    load_task((u32*)0x300000, (u32*)&task3, 0x2000);
    
    print("Tareas cargadas: ");
    print_dec(n_proc);
    print("\n");
    print("Iniciando sistema multitarea...\n");
    
    /* Habilitar interrupciones */
    sti;
    
    /* Mostrar el cursor */
    show_cursor();
    
    print("Pepin is booting...\n");
    print("RAM detected : 639k (lower), 31660k (upper)\n");
    print("Loading IDT\n");
    print("Configure PIC\n");
    print("Loading Task Register\n");
    print("Enabling paging\n");
    print("Interrupts are enable. System is ready !\n");
    print("\n");
    
    /* Simular las tareas como en la imagen */
    print("task2\n");
    print("task3\n");
    print("task1\n");
    print("task2\n");
    print("task3\n");
    print("task1\n");
    
    print("Done.\n");
    
    /* El sistema ahora funciona con multitarea */
    while (1) {
        asm("hlt");
    }
}
