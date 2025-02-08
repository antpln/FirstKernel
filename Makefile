# Cross-compiler setup
CC = i686-elf-gcc
CXX = i686-elf-g++
AS = i686-elf-as

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BOOT_DIR = boot
KERNEL_DIR = $(SRC_DIR)/kernel
KERNEL_DEST = /kernel

# Create kernel directory if it doesn't exist
$(shell mkdir -p $(KERNEL_DEST))

# Compiler flags
CFLAGS = -O2 -g -std=gnu99 -ffreestanding -Wall -Wextra -I$(INCLUDE_DIR)
CXXFLAGS = -O2 -g -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -I$(INCLUDE_DIR)
LDFLAGS = -ffreestanding -O2 -nostdlib

# Source files
CSOURCES = $(shell find $(SRC_DIR) -name '*.c')
CPPSOURCES = $(shell find $(SRC_DIR) -name '*.cpp')
ASMSOURCES = $(shell find $(BOOT_DIR) -name '*.s')

# Object files
OBJECTS = $(patsubst $(BOOT_DIR)/%.s,$(BUILD_DIR)/boot/%.o,$(ASMSOURCES))
OBJECTS += $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CSOURCES))
OBJECTS += $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPPSOURCES))

# Output files
KERNEL_ELF = $(BUILD_DIR)/kernel.elf

# QEMU configuration
QEMU = qemu-system-i386
QEMU_FLAGS = -kernel $(KERNEL_ELF)

.PHONY: all clean run directories iso

all: directories $(KERNEL_ELF)

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/kernel
	@mkdir -p $(BUILD_DIR)/boot

$(KERNEL_ELF): $(OBJECTS)
	$(CXX) -T linker.ld -o $(KERNEL_DEST)/kernel.bin $(LDFLAGS) $(OBJECTS) -lgcc
	grub-file --is-x86-multiboot $(KERNEL_DEST)/kernel.bin

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/boot/%.o: $(BOOT_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $< -o $@

iso: $(KERNEL_ELF)
	mkdir -p isodir/boot/grub
	cp $(KERNEL_DEST)/kernel.bin isodir/boot/kernel.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

run: $(KERNEL_ELF)
	$(QEMU) $(QEMU_FLAGS)

clean:
	rm -rf $(BUILD_DIR)
