/*
 * Test program to verify Ext2 implementation
 * This simulates the kernel environment for testing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// Simulate kernel types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned char uchar;

// Simulate kernel functions
void print(char *s) { printf("%s", s); }
void print_dec(u32 n) { printf("%u", n); }
void print_hex(u32 n) { printf("0x%08X", n); }
void putcar(char c) { printf("%c", c); }

// Simulate memory functions
void* kmalloc(u32 size) { return malloc(size); }
void kfree(void* ptr) { free(ptr); }
void* memcpy(void* dest, const void* src, u32 count) { return memcpy(dest, src, count); }
void* memset(void* dest, u8 val, u32 count) { return memset(dest, val, count); }
u32 strlen(const char* s) { return strlen(s); }
int memcmp(const void* s1, const void* s2, u32 n) { return memcmp(s1, s2, n); }

// Simulate disk I/O
int disk_fd = -1;
int ide_read_sectors(int drive, u32 lba, u8 num_sectors, void *buffer) {
    if (disk_fd == -1) {
        disk_fd = open("ext2_disk.img", O_RDONLY);
        if (disk_fd == -1) {
            printf("ERROR: Cannot open disk image\n");
            return -1;
        }
    }
    
    lseek(disk_fd, lba * 512, SEEK_SET);
    return read(disk_fd, buffer, num_sectors * 512) == (num_sectors * 512) ? 0 : -1;
}

#define IDE_MASTER 0

// Include our Ext2 implementation
#include "ext2.c"

// Test callback for directory listing
void list_callback(struct ext2_dir_entry *entry) {
    char name[256];
    memcpy(name, entry->name, entry->name_len);
    name[entry->name_len] = '\0';
    
    printf("  %s (inode: %u, type: %u)\n", name, entry->inode, entry->file_type);
}

int main() {
    printf("=== Pepin OS Ext2 Filesystem Test ===\n\n");
    
    // Test filesystem initialization
    if (ext2_init() != 0) {
        printf("ERROR: Failed to initialize Ext2 filesystem\n");
        return 1;
    }
    
    printf("\n=== Testing File Search ===\n");
    struct ext2_inode test_inode;
    
    // Test finding a file
    if (ext2_find_file("hello.txt", &test_inode) == 0) {
        printf("Found hello.txt (size: %u bytes)\n", test_inode.i_size);
        
        // Read and display the file content
        char *buffer = malloc(test_inode.i_size + 1);
        if (buffer) {
            int bytes_read = ext2_read_file(&test_inode, buffer, test_inode.i_size);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("Content: %s\n", buffer);
            }
            free(buffer);
        }
    } else {
        printf("File hello.txt not found\n");
    }
    
    printf("\n=== Testing Directory Listing ===\n");
    struct ext2_inode root_inode;
    if (ext2_read_inode(EXT2_ROOT_INODE, &root_inode) == 0) {
        printf("Root directory contents:\n");
        ext2_list_dir(&root_inode, list_callback);
    }
    
    printf("\n=== Test Complete ===\n");
    
    if (disk_fd != -1) {
        close(disk_fd);
    }
    
    return 0;
}