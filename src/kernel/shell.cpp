
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <kernel/keyboard.h>
#include <kernel/isr.h>
#include <kernel/timer.h>
#include <kernel/ramfs.h>
#include <kernel/heap.h>
#include <kernel/shell.h>

#define SHELL_BUFFER_SIZE 256
#define NUM_COMMANDS 256

// A global command buffer and index to keep track of the current command.
static char shell_buffer[SHELL_BUFFER_SIZE];
static uint32_t shell_index = 0;
static FSNode* current_dir = NULL;

static bool input_enabled = true;

void cmd_ls(const char* args) {
    if (!current_dir) current_dir = fs_get_root();
    for (size_t i = 0; i < current_dir->child_count; i++) {
        if(current_dir->children[i]->type == FS_FILE) {
            printf("%s  ", current_dir->children[i]->name);
        } else {
            printf("%s/  ", current_dir->children[i]->name);
        }
    }
    printf("\n");
}

void cmd_cd(const char* args) {
    if (!current_dir) current_dir = fs_get_root();
    if (!args || strlen(args) == 0) {
        printf("Usage: cd <directory>\n");
        return;
    }

    if (strcmp(args, "..") == 0) {
        if (current_dir && current_dir->parent) {
            current_dir = current_dir->parent;
        } else {
            printf("Already at root.\n");
        }
        return;
    }

    FSNode* target = fs_find_child(current_dir, args);
    if (target && target->type == FS_DIRECTORY) {
        current_dir = target;
    } else {
        printf("cd: No such directory '%s'\n", args);
    }
}

void cmd_cat(const char* args) {
    if (!current_dir) current_dir = fs_get_root();
    if (!args || strlen(args) == 0) {
        printf("Usage: cat <file>\n");
        return;
    }

    FSNode* file = fs_find_child(current_dir, args);
    if (file && file->type == FS_FILE) {
        printf("%s\n", file->data);
    } else {
        printf("cat: No such file '%s'\n", args);
    }
}

void cmd_touch(const char* args) {
    if (!current_dir) current_dir = fs_get_root();
    if (!args || strlen(args) == 0) {
        printf("Usage: touch <filename>\n");
        return;
    }

    FSNode* file = fs_create_node(args, FS_FILE);
    if (file) {
        file->size = 256;
        file->data = (uint8_t*)kmalloc(file->size);
        memset(file->data, 0, file->size);
        fs_add_child(current_dir, file);
        printf("File '%s' created.\n", args);
    }
}

void cmd_mkdir(const char* args) {
    if (!current_dir) current_dir = fs_get_root();
    if (!args || strlen(args) == 0) {
        printf("Usage: mkdir <dirname>\n");
        return;
    }

    FSNode* dir = fs_create_node(args, FS_DIRECTORY);
    if (dir) {
        fs_add_child(current_dir, dir);
        printf("Directory '%s' created.\n", args);
    }
}

void cmd_uptime(const char* args) {
    uint32_t ticks = get_ticks();
    printf("Uptime: %u ms\n", ticks);
}

void cmd_rm(const char* args) {
    if (!current_dir) current_dir = fs_get_root();
    if (!args || strlen(args) == 0) {
        printf("Usage: rm <filename>\n");
        return;
    }

    FSNode* file = fs_find_child(current_dir, args);
    if (file && file->type == FS_FILE) {
        fs_remove_child(current_dir, file);
        printf("File '%s' removed.\n", args);
    } else {
        printf("rm: No such file '%s'\n", args);
    }
}

void cmd_rmdir(const char* args) {
    if (!current_dir) current_dir = fs_get_root();
    if (!args || strlen(args) == 0) {
        printf("Usage: rmdir <dirname>\n");
        return;
    }

    FSNode* dir = fs_find_child(current_dir, args);
    if (dir && dir->type == FS_DIRECTORY) {
        fs_remove_child(current_dir, dir);
        printf("Directory '%s' removed.\n", args);
    } else {
        printf("rmdir: No such directory '%s'\n", args);
    }
}

shell_command_t commands[NUM_COMMANDS] = {
    {"help", cmd_help, "Show available commands"},
    {"ls", cmd_ls, "List directory contents"},
    {"cd", cmd_cd, "Change directory"},
    {"cat", cmd_cat, "Display file contents"},
    {"touch", cmd_touch, "Create a new file"},
    {"mkdir", cmd_mkdir, "Create a new directory"},
    {"rm", cmd_rm, "Remove a file"},
    {"rmdir", cmd_rmdir, "Remove a directory"},
    {"uptime", cmd_uptime, "Show system uptime"},
};

void cmd_help(const char* args) {
    printf("Available commands:\n");
    for (size_t i = 0; i < sizeof(commands) / sizeof(shell_command_t); i++) {
        if(commands[i].function != NULL) {
            printf("  %s: %s\n", commands[i].name, commands[i].description);
        }
    }
}

void shell_process_command(const char* cmd)
{
    // Split the command into command name and arguments.
    char* command = strtok((char*)cmd, " ");
    char* args = strtok(NULL, " ");

    // Find the command in the list and execute it.
    for (size_t i = 0; i < sizeof(commands) / sizeof(shell_command_t); i++)
    {
        if (strcmp(command, commands[i].name) == 0)
        {
            commands[i].function(args);
            return;
        }
    }

    printf("Command not found: %s\n", command);
}

// This function is called by the keyboard interrupt handler whenever a valid character arrives.
void shell_handle_key(keyboard_event ke)
{
    if (!input_enabled) return;
    // Handle backspace. We assume backspace is sent as ASCII 0x08 or DEL (127).
    if (ke.scancode == '\b' || ke.scancode == 127)
    {
        if (shell_index > 0)
        {
            shell_index--;
            // Erase the last character on screen (move back, print space, move back again).
            printf("\b \b");
        }
        return;
    }

    // If the character is newline or carriage return, we process the command.
    if (ke.scancode == '\n' || ke.scancode == '\r' || ke.enter)
    {
        // Terminate the command string.
        shell_buffer[shell_index] = '\0';
        printf("\n");
        shell_process_command(shell_buffer);
        // Reset for the next command.
        shell_index = 0;
        printf("nutshell> ");
        return;
    }

    if (ke.backspace)
    {
        if (shell_index > 0)
        {
            shell_index--;
            // Erase the last character on screen (move back, print space, move back again).
            printf("\b \b");
        }
        return;
    }

    // If the character is a printable character, we add it to the buffer.
    char c = kb_to_ascii(ke);
    if (c != 0)
    {
        // Check if we have space in the buffer.
        if (shell_index < SHELL_BUFFER_SIZE - 1)
        {
            shell_buffer[shell_index] = c;
            shell_index++;
            printf("%c", c);
        }
    }
}

// Initialize the shell (print a welcome message and the prompt).
void shell_init()
{
    printf("Welcome to nutshell!\n");
    printf("nutshell> ");
}
