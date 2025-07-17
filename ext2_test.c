/*
 * Test functions for Ext2 filesystem
 * To be called from kernel after initialization
 */

#include "ext2.h"
#include "screen.h"
#include "mm.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

/*
 * Test callback function for directory listing
 */
void ext2_test_list_callback(struct ext2_dir_entry *entry)
{
    char name[256];
    int i;
    
    /* Copy name and null terminate */
    for (i = 0; i < entry->name_len; i++) {
        name[i] = entry->name[i];
    }
    name[i] = '\0';
    
    print("  ");
    print(name);
    print(" (inode: ");
    print_dec(entry->inode);
    print(", type: ");
    print_dec(entry->file_type);
    print(")\n");
}

/*
 * Test Ext2 filesystem functionality
 */
void ext2_test(void)
{
    struct ext2_inode test_inode;
    char *buffer;
    int bytes_read;
    
    print("\n=== Testing Ext2 Filesystem ===\n");
    
    /* Test 1: List root directory */
    print("Test 1: Listing root directory contents\n");
    struct ext2_inode root_inode;
    
    if (ext2_read_inode(EXT2_ROOT_INODE, &root_inode) == 0) {
        print("Root directory contents:\n");
        ext2_list_dir(&root_inode, ext2_test_list_callback);
    } else {
        print("ERROR: Cannot read root inode\n");
    }
    
    /* Test 2: Find and read hello.txt */
    print("\nTest 2: Finding and reading hello.txt\n");
    if (ext2_find_file("hello.txt", &test_inode) == 0) {
        print("Found hello.txt (size: ");
        print_dec(test_inode.i_size);
        print(" bytes)\n");
        
        /* Read file content */
        buffer = (char *)kmalloc(test_inode.i_size + 1);
        if (buffer != NULL) {
            bytes_read = ext2_read_file(&test_inode, buffer, test_inode.i_size);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                print("Content: ");
                print(buffer);
                print("\n");
            } else {
                print("ERROR: Cannot read file content\n");
            }
            kfree(buffer);
        } else {
            print("ERROR: Cannot allocate buffer\n");
        }
    } else {
        print("File hello.txt not found\n");
    }
    
    /* Test 3: Find and read test.txt */
    print("\nTest 3: Finding and reading test.txt\n");
    if (ext2_find_file("test.txt", &test_inode) == 0) {
        print("Found test.txt (size: ");
        print_dec(test_inode.i_size);
        print(" bytes)\n");
        
        /* Read file content */
        buffer = (char *)kmalloc(test_inode.i_size + 1);
        if (buffer != NULL) {
            bytes_read = ext2_read_file(&test_inode, buffer, test_inode.i_size);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                print("Content: ");
                print(buffer);
                print("\n");
            } else {
                print("ERROR: Cannot read file content\n");
            }
            kfree(buffer);
        } else {
            print("ERROR: Cannot allocate buffer\n");
        }
    } else {
        print("File test.txt not found\n");
    }
    
    /* Test 4: Try to find non-existent file */
    print("\nTest 4: Searching for non-existent file\n");
    if (ext2_find_file("nonexistent.txt", &test_inode) == 0) {
        print("ERROR: Found non-existent file!\n");
    } else {
        print("Correctly reported file not found\n");
    }
    
    print("\n=== Ext2 Tests Complete ===\n");
}