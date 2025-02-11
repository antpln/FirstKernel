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
KERNEL_DEST = ./kernel
LIBC_DIR = libc

# Create kernel directory if it doesn't exist
$(shell mkdir -p $(KERNEL_DEST))

# Compiler flags
CFLAGS = -O2 -g -std=gnu99 -ffreestanding -Wall -Wextra -I$(INCLUDE_DIR) -I$(LIBC_DIR)/include
CXXFLAGS = -O2 -g -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -I$(INCLUDE_DIR) -I$(LIBC_DIR)/include
LDFLAGS = -ffreestanding -O2 -nostdlib

# Source files
CSOURCES = $(shell find $(SRC_DIR) -name '*.c')
CPPSOURCES = $(shell find $(SRC_DIR) -name '*.cpp')
ASMSOURCES = $(shell find $(BOOT_DIR) -name '*.s')
KERNEL_ASMSOURCES = $(shell find $(KERNEL_DIR) -name '*.s')
LIBC_CSOURCES = $(shell find $(LIBC_DIR) -type f -name '*.c')
LIBC_CPPSOURCES = $(shell find $(LIBC_DIR) -type f -name '*.cpp')

# Object files
KERNEL_OBJS = $(patsubst $(KERNEL_DIR)/%.s,$(BUILD_DIR)/kernel/%.o,$(KERNEL_ASMSOURCES))
BOOT_OBJS = $(patsubst $(BOOT_DIR)/%.s,$(BUILD_DIR)/boot/%.o,$(ASMSOURCES))
C_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CSOURCES))
CPP_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPPSOURCES))
LIBC_C_OBJS = $(patsubst $(LIBC_DIR)/%,$(BUILD_DIR)/libc/%,$(LIBC_CSOURCES:.c=.o))
LIBC_CPP_OBJS = $(patsubst $(LIBC_DIR)/%,$(BUILD_DIR)/libc/%,$(LIBC_CPPSOURCES:.cpp=.o))

OBJECTS = $(sort $(BOOT_OBJS) $(C_OBJS) $(CPP_OBJS) $(KERNEL_OBJS) $(LIBC_C_OBJS) $(LIBC_CPP_OBJS))

# Output files
KERNEL_ELF = kernel/kernel.bin

# QEMU configuration
QEMU = qemu-system-i386
QEMU_FLAGS = -kernel $(KERNEL_ELF)

.PHONY: all clean run directories iso debug

all: directories $(KERNEL_ELF)

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/kernel
	@mkdir -p $(BUILD_DIR)/boot
	@mkdir -p $(BUILD_DIR)/libc

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

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $< -o $@

# Add .s files to the valid source extensions if not already present
SRCEXTS += .s

# Add assembly compilation rule
%.o: %.s
	$(AS) $< -o $@

# Update libc compilation rules
$(BUILD_DIR)/libc/%.o: $(LIBC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling C++: $<"
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD_DIR)/libc/%.o: $(LIBC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling C: $<"
	$(CC) -c $< -o $@ $(CFLAGS)

iso: $(KERNEL_ELF)
	mkdir -p isodir/boot/grub
	cp $(KERNEL_DEST)/kernel.bin isodir/boot/kernel.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

run: $(KERNEL_ELF)
	$(QEMU) $(QEMU_FLAGS)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf isodir
	rm -f myos.iso
	rm -f $(KERNEL_DEST)/kernel.bin

# Add debug target
debug:
	@echo "LIBC_CSOURCES = $(LIBC_CSOURCES)"
	@echo "LIBC_CPPSOURCES = $(LIBC_CPPSOURCES)"
	@echo "LIBC_C_OBJS = $(LIBC_C_OBJS)"
	@echo "LIBC_CPP_OBJS = $(LIBC_CPP_OBJS)"
	@echo "Build commands that would be used:"
	@echo "C++ files:"
	@for file in $(LIBC_CPPSOURCES); do \
		echo "$(CXX) -c $$file -o build/libc/$$(basename $$file .cpp).o $(CXXFLAGS)"; \
	done
	@echo "LIBC_CPPSOURCES:"
	@for f in $(LIBC_CPPSOURCES); do echo "  $$f"; done
	@echo "LIBC_CPP_OBJS:"
	@for f in $(LIBC_CPP_OBJS); do echo "  $$f"; done
