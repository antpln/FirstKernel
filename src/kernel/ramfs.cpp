#include "kernel/ramfs.h"
#include "stdio.h"
#include "string.h"
#include "kernel/heap.h"

// For dynamic allocation, we assume a kernel allocator is available.

static FSNode* root = NULL;

FSNode* fs_create_node(const char* name, FSNodeType type) {
    FSNode* node = (FSNode*)kmalloc(sizeof(FSNode));
    if (!node)
        return NULL;
    memset(node, 0, sizeof(FSNode));
    // Copy the name (ensure null termination)
    for (size_t i = 0; i < sizeof(node->name) - 1 && name[i]; i++) {
        node->name[i] = name[i];
    }
    node->type = type;
    node->size = 0;
    node->data = NULL;
    node->parent = NULL;
    node->child_count = 0;
    if (type == FS_DIRECTORY) {
        // Allocate space for children pointers.
        node->children = (FSNode**)kmalloc(sizeof(FSNode*) * MAX_CHILDREN);
        if (node->children) {
            memset(node->children, 0, sizeof(FSNode*) * MAX_CHILDREN);
        }
    }
    return node;
}

void fs_add_child(FSNode* parent, FSNode* child) {
    if (!parent || parent->type != FS_DIRECTORY)
        return;
    if (parent->child_count >= MAX_CHILDREN) {
        printf("Error: directory '%s' is full.\n", parent->name);
        return;
    }
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

FSNode* fs_find_child(FSNode* parent, const char* name) {
    if (!parent || parent->type != FS_DIRECTORY)
        return NULL;
    for (size_t i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, name) == 0) {
            return parent->children[i];
        }
    }
    return NULL;
}

void fs_init() {
    // Create the root directory.
    root = fs_create_node("/", FS_DIRECTORY);
    if (root == NULL) {
        printf("Error initializing filesystem: could not allocate root directory.\n");
    }
    else {
        printf("[RAMFS] Filesystem initialized.\n");
    }
}

FSNode* fs_get_root() {
    return root;
}

int fs_read(FSNode* file, size_t offset, size_t size, uint8_t* buffer) {
    if (!file || file->type != FS_FILE) {
        return -1;
    }
    if (offset > file->size) {
        return -1;
    }
    size_t read_size = size;
    if (offset + size > file->size) {
        read_size = file->size - offset;
    }
    memcpy(buffer, file->data + offset, read_size);
    return (int)read_size;
}

int fs_write(FSNode* file, size_t offset, size_t size, const uint8_t* buffer) {
    if (!file || file->type != FS_FILE) {
        return -1;
    }
    // For this simple filesystem, we'll assume that the file data has been allocated
    // with a fixed maximum size. Here we do not implement dynamic file growth.
    if (offset + size > file->size) {
        printf("Write error: attempt to write beyond end of file '%s'.\n", file->name);
        return -1;
    }
    memcpy(file->data + offset, buffer, size);
    return (int)size;
}
