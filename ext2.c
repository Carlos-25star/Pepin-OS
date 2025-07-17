#include "ext2.h"
#include "screen.h"
#include "mm.h"
#include "lib.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

/* Variable global del sistema de archivos */
struct ext2_fs ext2_fs;

/*
 * Inicializa el sistema de archivos Ext2
 */
int ext2_init(void)
{
    print("ext2   : initializing Ext2 filesystem...\n");
    
    /* Leer el superbloque */
    if (ext2_read_superblock() != 0) {
        print("ext2   : ERROR - Failed to read superblock\n");
        return -1;
    }
    
    /* Verificar la signatura mágica */
    if (ext2_fs.superblock.s_magic != EXT2_SIGNATURE) {
        print("ext2   : ERROR - Invalid Ext2 signature: 0x");
        print_hex(ext2_fs.superblock.s_magic);
        print("\n");
        return -1;
    }
    
    /* Calcular el tamaño del bloque */
    ext2_fs.block_size = 1024 << ext2_fs.superblock.s_log_block_size;
    ext2_fs.inode_size = EXT2_INODE_SIZE;
    ext2_fs.first_data_block = ext2_fs.superblock.s_first_data_block;
    
    print("ext2   : block size: ");
    print_dec(ext2_fs.block_size);
    print(" bytes\n");
    
    print("ext2   : total blocks: ");
    print_dec(ext2_fs.superblock.s_blocks_count);
    print("\n");
    
    print("ext2   : total inodes: ");
    print_dec(ext2_fs.superblock.s_inodes_count);
    print("\n");
    
    /* Leer los descriptores de grupo */
    if (ext2_read_group_desc() != 0) {
        print("ext2   : ERROR - Failed to read group descriptors\n");
        return -1;
    }
    
    print("ext2   : filesystem initialized successfully\n");
    return 0;
}

/*
 * Lee el superbloque del disco
 */
int ext2_read_superblock(void)
{
    char *buffer = (char *)kmalloc(1024);
    if (buffer == NULL) {
        print("ext2   : ERROR - Cannot allocate buffer for superblock\n");
        return -1;
    }
    
    /* Leer el sector que contiene el superbloque (sector 2, offset 1024) */
    if (ide_read_sectors(IDE_MASTER, 2, 2, buffer) != 0) {
        print("ext2   : ERROR - Cannot read superblock from disk\n");
        kfree(buffer);
        return -1;
    }
    
    /* Copiar el superbloque desde el buffer */
    memcpy(&ext2_fs.superblock, buffer, sizeof(struct ext2_superblock));
    
    kfree(buffer);
    return 0;
}

/*
 * Lee los descriptores de grupo
 */
int ext2_read_group_desc(void)
{
    u32 groups_count;
    u32 desc_per_block;
    u32 blocks_needed;
    u32 first_desc_block;
    char *buffer;
    
    /* Calcular el número de grupos */
    groups_count = (ext2_fs.superblock.s_blocks_count + ext2_fs.superblock.s_blocks_per_group - 1) / 
                   ext2_fs.superblock.s_blocks_per_group;
    
    ext2_fs.groups_count = groups_count;
    
    print("ext2   : number of groups: ");
    print_dec(groups_count);
    print("\n");
    
    /* Calcular cuántos bloques necesitamos para los descriptores */
    desc_per_block = ext2_fs.block_size / sizeof(struct ext2_group_desc);
    blocks_needed = (groups_count + desc_per_block - 1) / desc_per_block;
    
    /* El primer bloque de descriptores está después del superbloque */
    if (ext2_fs.block_size == 1024) {
        first_desc_block = 2;  /* Superbloque está en el bloque 1 */
    } else {
        first_desc_block = 1;  /* Superbloque está en el bloque 0 */
    }
    
    /* Asignar memoria para los descriptores */
    ext2_fs.group_desc = (struct ext2_group_desc *)kmalloc(groups_count * sizeof(struct ext2_group_desc));
    if (ext2_fs.group_desc == NULL) {
        print("ext2   : ERROR - Cannot allocate memory for group descriptors\n");
        return -1;
    }
    
    /* Leer los descriptores de grupo */
    buffer = (char *)kmalloc(ext2_fs.block_size);
    if (buffer == NULL) {
        print("ext2   : ERROR - Cannot allocate buffer for group descriptors\n");
        kfree(ext2_fs.group_desc);
        return -1;
    }
    
    if (ext2_read_block(first_desc_block, buffer) != 0) {
        print("ext2   : ERROR - Cannot read group descriptors from disk\n");
        kfree(buffer);
        kfree(ext2_fs.group_desc);
        return -1;
    }
    
    /* Copiar los descriptores */
    memcpy(ext2_fs.group_desc, buffer, groups_count * sizeof(struct ext2_group_desc));
    
    kfree(buffer);
    print("ext2   : group descriptors loaded successfully\n");
    return 0;
}

/*
 * Lee un bloque del disco
 */
int ext2_read_block(u32 block_num, void *buffer)
{
    u32 sector_start;
    u32 sectors_per_block;
    
    /* Calcular el sector inicial y cuántos sectores por bloque */
    sectors_per_block = ext2_fs.block_size / 512;
    sector_start = block_num * sectors_per_block;
    
    /* Leer el bloque */
    if (ide_read_sectors(IDE_MASTER, sector_start, sectors_per_block, buffer) != 0) {
        print("ext2   : ERROR - Cannot read block ");
        print_dec(block_num);
        print(" from disk\n");
        return -1;
    }
    
    return 0;
}

/*
 * Lee un inodo del disco
 */
int ext2_read_inode(u32 inode_num, struct ext2_inode *inode)
{
    u32 group_num;
    u32 inode_index;
    u32 inode_block;
    u32 inode_offset;
    struct ext2_group_desc *group;
    char *buffer;
    
    /* Verificar que el número de inodo sea válido */
    if (inode_num == 0 || inode_num > ext2_fs.superblock.s_inodes_count) {
        print("ext2   : ERROR - Invalid inode number ");
        print_dec(inode_num);
        print("\n");
        return -1;
    }
    
    /* Calcular el grupo y el índice del inodo */
    group_num = (inode_num - 1) / ext2_fs.superblock.s_inodes_per_group;
    inode_index = (inode_num - 1) % ext2_fs.superblock.s_inodes_per_group;
    
    /* Obtener el descriptor del grupo */
    group = &ext2_fs.group_desc[group_num];
    
    /* Calcular la posición del inodo */
    inode_block = group->bg_inode_table + (inode_index * ext2_fs.inode_size) / ext2_fs.block_size;
    inode_offset = (inode_index * ext2_fs.inode_size) % ext2_fs.block_size;
    
    /* Leer el bloque que contiene el inodo */
    buffer = (char *)kmalloc(ext2_fs.block_size);
    if (buffer == NULL) {
        print("ext2   : ERROR - Cannot allocate buffer for inode\n");
        return -1;
    }
    
    if (ext2_read_block(inode_block, buffer) != 0) {
        print("ext2   : ERROR - Cannot read inode block\n");
        kfree(buffer);
        return -1;
    }
    
    /* Copiar el inodo */
    memcpy(inode, buffer + inode_offset, sizeof(struct ext2_inode));
    
    kfree(buffer);
    return 0;
}

/*
 * Lee el contenido de un archivo
 */
int ext2_read_file(struct ext2_inode *inode, void *buffer, u32 size)
{
    u32 bytes_read = 0;
    u32 block_num = 0;
    u32 bytes_to_read;
    char *file_buffer;
    char *dest = (char *)buffer;
    
    /* Verificar que sea un archivo regular */
    if (!EXT2_S_ISREG(inode->i_mode)) {
        print("ext2   : ERROR - Not a regular file\n");
        return -1;
    }
    
    /* Limitar el tamaño a leer */
    if (size > inode->i_size) {
        size = inode->i_size;
    }
    
    /* Asignar buffer temporal */
    file_buffer = (char *)kmalloc(ext2_fs.block_size);
    if (file_buffer == NULL) {
        print("ext2   : ERROR - Cannot allocate file buffer\n");
        return -1;
    }
    
    /* Leer bloques directos */
    while (bytes_read < size && block_num < 12) {
        if (inode->i_block[block_num] == 0) {
            break;
        }
        
        if (ext2_read_block(inode->i_block[block_num], file_buffer) != 0) {
            print("ext2   : ERROR - Cannot read file block\n");
            kfree(file_buffer);
            return -1;
        }
        
        bytes_to_read = ext2_fs.block_size;
        if (bytes_read + bytes_to_read > size) {
            bytes_to_read = size - bytes_read;
        }
        
        memcpy(dest + bytes_read, file_buffer, bytes_to_read);
        bytes_read += bytes_to_read;
        block_num++;
    }
    
    kfree(file_buffer);
    return bytes_read;
}

/*
 * Busca un archivo en el directorio raíz
 */
int ext2_find_file(const char *name, struct ext2_inode *inode)
{
    struct ext2_inode root_inode;
    char *buffer;
    struct ext2_dir_entry *entry;
    u32 offset = 0;
    u32 name_len = strlen(name);
    
    /* Leer el inodo raíz */
    if (ext2_read_inode(EXT2_ROOT_INODE, &root_inode) != 0) {
        print("ext2   : ERROR - Cannot read root inode\n");
        return -1;
    }
    
    /* Verificar que sea un directorio */
    if (!EXT2_S_ISDIR(root_inode.i_mode)) {
        print("ext2   : ERROR - Root is not a directory\n");
        return -1;
    }
    
    /* Asignar buffer para el directorio */
    buffer = (char *)kmalloc(ext2_fs.block_size);
    if (buffer == NULL) {
        print("ext2   : ERROR - Cannot allocate directory buffer\n");
        return -1;
    }
    
    /* Leer el primer bloque del directorio */
    if (ext2_read_block(root_inode.i_block[0], buffer) != 0) {
        print("ext2   : ERROR - Cannot read directory block\n");
        kfree(buffer);
        return -1;
    }
    
    /* Buscar el archivo */
    while (offset < root_inode.i_size) {
        entry = (struct ext2_dir_entry *)(buffer + offset);
        
        /* Verificar que la entrada sea válida */
        if (entry->rec_len == 0) {
            break;
        }
        
        /* Comparar nombres */
        if (entry->name_len == name_len && entry->inode != 0) {
            if (memcmp(entry->name, name, name_len) == 0) {
                /* Archivo encontrado, leer su inodo */
                if (ext2_read_inode(entry->inode, inode) != 0) {
                    print("ext2   : ERROR - Cannot read file inode\n");
                    kfree(buffer);
                    return -1;
                }
                kfree(buffer);
                return 0;
            }
        }
        
        offset += entry->rec_len;
    }
    
    kfree(buffer);
    return -1;  /* Archivo no encontrado */
}

/*
 * Lista el contenido de un directorio
 */
int ext2_list_dir(struct ext2_inode *dir_inode, void (*callback)(struct ext2_dir_entry *))
{
    char *buffer;
    struct ext2_dir_entry *entry;
    u32 offset = 0;
    
    /* Verificar que sea un directorio */
    if (!EXT2_S_ISDIR(dir_inode->i_mode)) {
        print("ext2   : ERROR - Not a directory\n");
        return -1;
    }
    
    /* Asignar buffer para el directorio */
    buffer = (char *)kmalloc(ext2_fs.block_size);
    if (buffer == NULL) {
        print("ext2   : ERROR - Cannot allocate directory buffer\n");
        return -1;
    }
    
    /* Leer el primer bloque del directorio */
    if (ext2_read_block(dir_inode->i_block[0], buffer) != 0) {
        print("ext2   : ERROR - Cannot read directory block\n");
        kfree(buffer);
        return -1;
    }
    
    /* Recorrer las entradas */
    while (offset < dir_inode->i_size) {
        entry = (struct ext2_dir_entry *)(buffer + offset);
        
        /* Verificar que la entrada sea válida */
        if (entry->rec_len == 0) {
            break;
        }
        
        /* Llamar al callback si la entrada es válida */
        if (entry->inode != 0) {
            callback(entry);
        }
        
        offset += entry->rec_len;
    }
    
    kfree(buffer);
    return 0;
}