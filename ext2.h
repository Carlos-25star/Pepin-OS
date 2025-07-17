#ifndef EXT2_H_
#define EXT2_H_

#include "types.h"
#include "ide.h"

/* Constantes Ext2 */
#define EXT2_SIGNATURE          0xEF53
#define EXT2_SUPERBLOCK_OFFSET  1024
#define EXT2_BLOCK_SIZE         1024
#define EXT2_INODE_SIZE         128
#define EXT2_ROOT_INODE         2

/* Estados del sistema de archivos */
#define EXT2_VALID_FS           1
#define EXT2_ERROR_FS           2

/* Métodos de manejo de errores */
#define EXT2_ERRORS_CONTINUE    1
#define EXT2_ERRORS_RO          2
#define EXT2_ERRORS_PANIC       3

/* Tipos de archivo */
#define EXT2_S_IFSOCK   0xC000
#define EXT2_S_IFLNK    0xA000
#define EXT2_S_IFREG    0x8000  /* Archivo regular */
#define EXT2_S_IFBLK    0x6000
#define EXT2_S_IFDIR    0x4000  /* Directorio */
#define EXT2_S_IFCHR    0x2000
#define EXT2_S_IFIFO    0x1000

/* Máscaras de permisos */
#define EXT2_S_ISUID    0x0800
#define EXT2_S_ISGID    0x0400
#define EXT2_S_ISVTX    0x0200
#define EXT2_S_IRUSR    0x0100
#define EXT2_S_IWUSR    0x0080
#define EXT2_S_IXUSR    0x0040
#define EXT2_S_IRGRP    0x0020
#define EXT2_S_IWGRP    0x0010
#define EXT2_S_IXGRP    0x0008
#define EXT2_S_IROTH    0x0004
#define EXT2_S_IWOTH    0x0002
#define EXT2_S_IXOTH    0x0001

/* Macros para verificar tipos de archivo */
#define EXT2_S_ISDIR(mode)  (((mode) & EXT2_S_IFDIR) == EXT2_S_IFDIR)
#define EXT2_S_ISREG(mode)  (((mode) & EXT2_S_IFREG) == EXT2_S_IFREG)

/* Estructura del superbloque */
struct ext2_superblock {
    u32 s_inodes_count;         /* Número total de inodos */
    u32 s_blocks_count;         /* Número total de bloques */
    u32 s_r_blocks_count;       /* Número de bloques reservados */
    u32 s_free_blocks_count;    /* Número de bloques libres */
    u32 s_free_inodes_count;    /* Número de inodos libres */
    u32 s_first_data_block;     /* Primer bloque de datos */
    u32 s_log_block_size;       /* Tamaño del bloque */
    u32 s_log_frag_size;        /* Tamaño del fragmento */
    u32 s_blocks_per_group;     /* Bloques por grupo */
    u32 s_frags_per_group;      /* Fragmentos por grupo */
    u32 s_inodes_per_group;     /* Inodos por grupo */
    u32 s_mtime;                /* Tiempo de montaje */
    u32 s_wtime;                /* Tiempo de escritura */
    u16 s_mnt_count;            /* Número de montajes */
    u16 s_max_mnt_count;        /* Máximo número de montajes */
    u16 s_magic;                /* Signature mágica */
    u16 s_state;                /* Estado del sistema de archivos */
    u16 s_errors;               /* Comportamiento cuando hay errores */
    u16 s_minor_rev_level;      /* Nivel de revisión menor */
    u32 s_lastcheck;            /* Tiempo de la última verificación */
    u32 s_checkinterval;        /* Intervalo máximo entre verificaciones */
    u32 s_creator_os;           /* OS que creó el sistema */
    u32 s_rev_level;            /* Nivel de revisión */
    u16 s_def_resuid;           /* UID por defecto para bloques reservados */
    u16 s_def_resgid;           /* GID por defecto para bloques reservados */
    u8 reserved[940];           /* Relleno hasta 1024 bytes */
} __attribute__ ((packed));

/* Estructura del descriptor de grupo */
struct ext2_group_desc {
    u32 bg_block_bitmap;        /* Bloque que contiene el bitmap de bloques */
    u32 bg_inode_bitmap;        /* Bloque que contiene el bitmap de inodos */
    u32 bg_inode_table;         /* Primer bloque de la tabla de inodos */
    u16 bg_free_blocks_count;   /* Número de bloques libres en el grupo */
    u16 bg_free_inodes_count;   /* Número de inodos libres en el grupo */
    u16 bg_used_dirs_count;     /* Número de directorios en el grupo */
    u16 bg_pad;                 /* Relleno */
    u8 bg_reserved[12];         /* Reservado */
} __attribute__ ((packed));

/* Estructura del inodo */
struct ext2_inode {
    u16 i_mode;                 /* Tipo de archivo y permisos */
    u16 i_uid;                  /* UID del propietario */
    u32 i_size;                 /* Tamaño en bytes */
    u32 i_atime;                /* Tiempo de acceso */
    u32 i_ctime;                /* Tiempo de creación */
    u32 i_mtime;                /* Tiempo de modificación */
    u32 i_dtime;                /* Tiempo de eliminación */
    u16 i_gid;                  /* GID del propietario */
    u16 i_links_count;          /* Número de enlaces */
    u32 i_blocks;               /* Número de bloques */
    u32 i_flags;                /* Flags del archivo */
    u32 i_osd1;                 /* Específico del OS */
    u32 i_block[15];            /* Punteros a bloques */
    u32 i_generation;           /* Generación del archivo */
    u32 i_file_acl;             /* ACL del archivo */
    u32 i_dir_acl;              /* ACL del directorio */
    u32 i_faddr;                /* Dirección del fragmento */
    u8 i_osd2[12];              /* Específico del OS */
} __attribute__ ((packed));

/* Estructura de entrada de directorio */
struct ext2_dir_entry {
    u32 inode;                  /* Número de inodo */
    u16 rec_len;                /* Longitud de la entrada */
    u8 name_len;                /* Longitud del nombre */
    u8 file_type;               /* Tipo de archivo */
    char name[256];             /* Nombre del archivo */
} __attribute__ ((packed));

/* Estructura del sistema de archivos */
struct ext2_fs {
    struct ext2_superblock superblock;
    struct ext2_group_desc *group_desc;
    u32 groups_count;
    u32 block_size;
    u32 inode_size;
    u32 first_data_block;
} __attribute__ ((packed));

/* Variables globales */
extern struct ext2_fs ext2_fs;

/* Funciones públicas */
int ext2_init(void);
int ext2_read_superblock(void);
int ext2_read_group_desc(void);
int ext2_read_inode(u32 inode_num, struct ext2_inode *inode);
int ext2_read_block(u32 block_num, void *buffer);
int ext2_read_file(struct ext2_inode *inode, void *buffer, u32 size);
int ext2_find_file(const char *name, struct ext2_inode *inode);
int ext2_list_dir(struct ext2_inode *dir_inode, void (*callback)(struct ext2_dir_entry *));

#endif