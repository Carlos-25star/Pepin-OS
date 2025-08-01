CC = i686-linux-gnu-gcc
CFLAGS = -m32 -c -ffreestanding -fno-pic -fno-stack-protector -nostdlib -Wall
LD = i686-linux-gnu-ld
# Cambio crítico: usar dirección 0x100000 para GRUB
LDFLAGS = -m elf_i386 -Ttext=0x100000 --entry=_start
NASM = nasm
NASMFLAGS = -f elf32

# Objetos actualizados - boot.o debe ir PRIMERO, agregado heap.o, ide.o, ext2.o y ext2_test.o
OBJECTS = boot.o kernel.o screen.o gdt.o lib.o idt.o isr.o pic.o kbd.o interrupt.o task.o syscall.o mm.o process.o schedule.o sched.o heap.o ide.o ext2.o ext2_test.o

all: kernel

# Compilar el multiboot header
boot.o: boot.asm
	$(NASM) $(NASMFLAGS) -o boot.o boot.asm

# Kernel para GRUB (ELF, no binario)
kernel: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o kernel

kernel.o: kernel.c
	$(CC) $(CFLAGS) kernel.c

screen.o: screen.c
	$(CC) $(CFLAGS) screen.c
	
sched.o: sched.asm
	$(NASM) $(NASMFLAGS) -o $@ $<

gdt.o: gdt.c
	$(CC) $(CFLAGS) gdt.c

lib.o: lib.c
	$(CC) $(CFLAGS) lib.c

idt.o: idt.c
	$(CC) $(CFLAGS) idt.c

isr.o: isr.c
	$(CC) $(CFLAGS) isr.c

pic.o: pic.c
	$(CC) $(CFLAGS) pic.c

kbd.o: kbd.c
	$(CC) $(CFLAGS) kbd.c

task.o: task.c
	$(CC) $(CFLAGS) task.c

syscall.o: syscall.c
	$(CC) $(CFLAGS) syscall.c

mm.o: mm.c
	$(CC) $(CFLAGS) mm.c

process.o: process.c
	$(CC) $(CFLAGS) process.c

schedule.o: schedule.c
	$(CC) $(CFLAGS) schedule.c

# Nueva regla para heap.o
heap.o: heap.c
	$(CC) $(CFLAGS) heap.c

# Nueva regla para ide.o
ide.o: ide.c
	$(CC) $(CFLAGS) ide.c

# Nueva regla para ext2.o
ext2.o: ext2.c
	$(CC) $(CFLAGS) ext2.c

# Nueva regla para ext2_test.o
ext2_test.o: ext2_test.c
	$(CC) $(CFLAGS) ext2_test.c

interrupt.o: interrupt.asm
	$(NASM) $(NASMFLAGS) interrupt.asm

# Verificar multiboot
check: kernel
	mbchk kernel

# Mostrar símbolos
symbols: kernel
	nm -n kernel

# Probar con GRUB2 (método moderno)
iso: kernel
	mkdir -p iso/boot/grub
	cp kernel iso/boot/
	echo 'menuentry "Pepin OS" {' > iso/boot/grub/grub.cfg
	echo '    multiboot /boot/kernel' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o pepin.iso iso
	rm -rf iso

# Probar con QEMU usando multiboot directo (con disco Ext2)
run-multiboot: kernel ext2_disk.img
	qemu-system-i386 -kernel kernel -hda ext2_disk.img

# Probar con ISO
run-iso: iso
	qemu-system-i386 -cdrom pepin.iso

clean:
	rm -f *.o kernel *.iso
	rm -rf iso

debug: kernel
	qemu-system-i386 -kernel kernel -s -S

.PHONY: all clean run-multiboot run-iso debug check symbols iso
