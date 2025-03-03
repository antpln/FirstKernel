#ifndef RAMFS_H
#define RAMFS_H

#include <stddef.h>
#include <stdint.h>

// Types of filesystem nodes.
enum FSNodeType {
    FS_FILE,
    FS_DIRECTORY,
};

// Maximum number of children a directory can have.
#define MAX_CHILDREN 32

// A filesystem node: either a file or a directory.
typedef struct FSNode {
    char name[64];
    FSNodeType type;
    size_t size;         // For files: current size; for directories: may be unused.
    uint8_t* data;       // For files: pointer to file contents (allocated via kmalloc).
    struct FSNode* parent;
    struct FSNode** children; // Array of pointers to children (if directory)
    size_t child_count;
} FSNode;

// Filesystem interface.
FSNode* fs_create_node(const char* name, FSNodeType type);
void fs_add_child(FSNode* parent, FSNode* child);
void fs_remove_child(FSNode* parent, FSNode* child);
FSNode* fs_find_child(FSNode* parent, const char* name);

void fs_init();
FSNode* fs_get_root();

// Basic file operations.
int fs_read(FSNode* file, size_t offset, size_t size, uint8_t* buffer);
int fs_write(FSNode* file, size_t offset, size_t size, const uint8_t* buffer);

#endif // RAMFS_H
