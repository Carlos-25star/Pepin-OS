#!/bin/bash

# Script para crear una imagen Ext2 para Pepin OS
# Basado en el tutorial de Michel Izzat

IMAGE_NAME="ext2_disk.img"
MOUNT_POINT="/tmp/pepin_mount"
IMAGE_SIZE="8192"  # 8MB

echo "Creando imagen Ext2 para Pepin OS..."

# Crear imagen vacía
dd if=/dev/zero of=$IMAGE_NAME bs=1024 count=$IMAGE_SIZE

# Formatear como Ext2
mkfs.ext2 -F $IMAGE_NAME

# Crear punto de montaje
sudo mkdir -p $MOUNT_POINT

# Montar la imagen
sudo mount -o loop $IMAGE_NAME $MOUNT_POINT

# Crear estructura de directorios
sudo mkdir -p $MOUNT_POINT/bin
sudo mkdir -p $MOUNT_POINT/usr
sudo mkdir -p $MOUNT_POINT/etc

# Crear archivos de prueba
echo "¡Hola desde Pepin OS!" | sudo tee $MOUNT_POINT/hello.txt
echo "Este es un archivo de prueba para el sistema de archivos Ext2" | sudo tee $MOUNT_POINT/test.txt

# Crear un archivo de configuración
cat << 'EOF' | sudo tee $MOUNT_POINT/etc/motd
===================================
  Pepin OS - Sistema Operativo
  Tutorial de Michel Izzat
===================================

Comandos disponibles:
- help    : Mostrar ayuda
- ls      : Listar archivos
- cat     : Mostrar contenido de archivo
- exec    : Ejecutar binario ELF
- ps      : Lista de procesos
- kill    : Terminar proceso
- meminfo : Información de memoria
===================================
EOF

# Crear un binario ELF simple (hello world)
cat << 'EOF' > /tmp/hello.c
#include <stdio.h>
int main() {
    printf("¡Hola desde un binario ELF!\n");
    return 0;
}
EOF

# Compilar para 32-bit (si es posible)
if command -v gcc-multilib &> /dev/null; then
    gcc -m32 -static -o /tmp/hello /tmp/hello.c
    sudo cp /tmp/hello $MOUNT_POINT/bin/hello
    rm /tmp/hello.c /tmp/hello
else
    # Crear un binario ELF dummy para pruebas
    echo "No se pudo compilar binario ELF real, creando dummy..."
    echo "ELF_DUMMY_BINARY" | sudo tee $MOUNT_POINT/bin/hello
fi

# Crear más archivos de prueba
echo "Contenido del archivo 1" | sudo tee $MOUNT_POINT/file1.txt
echo "Contenido del archivo 2" | sudo tee $MOUNT_POINT/file2.txt
echo "Contenido del archivo 3" | sudo tee $MOUNT_POINT/file3.txt

# Mostrar contenido
echo "Contenido de la imagen Ext2:"
sudo ls -la $MOUNT_POINT/
echo ""
echo "Archivos en /bin:"
sudo ls -la $MOUNT_POINT/bin/
echo ""
echo "Archivos en /etc:"
sudo ls -la $MOUNT_POINT/etc/

# Desmontar
sudo umount $MOUNT_POINT
sudo rmdir $MOUNT_POINT

echo "Imagen Ext2 creada exitosamente: $IMAGE_NAME"
echo "Para probar con QEMU:"
echo "qemu-system-i386 -kernel kernel -hda $IMAGE_NAME"