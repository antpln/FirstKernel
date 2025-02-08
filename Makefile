# Cross-compiler setup
CC = i686-elf-gcc
CXX = i686-elf-g++
AS = i686-elf-as

# Compiler flags
CFLAGS = -O2 -g -std=gnu99 -ffreestanding -Wall -Wextra
CXXFLAGS = -O2 -g -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti
LDFLAGS = -ffreestanding -O2 -nostdlib

# Source files
CSOURCES = $(wildcard *.c)
CPPSOURCES = $(wildcard *.cpp)
ASMSOURCES = $(wildcard *.s)

# Object files
OBJECTS = $(ASMSOURCES:.s=.o) $(CSOURCES:.c=.o) $(CPPSOURCES:.cpp=.o)

# Output files
KERNEL = mykernel.bin

# QEMU configuration
QEMU = qemu-system-i386
QEMU_FLAGS = -kernel

.PHONY: all clean run

all: $(KERNEL)

$(KERNEL): $(OBJECTS)
	$(CXX) -T linker.ld -o $@ $(LDFLAGS) $(OBJECTS) -lgcc

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.s
	$(AS) $< -o $@

run: $(KERNEL)
	$(QEMU) $(QEMU_FLAGS) $(KERNEL)

clean:
	rm -f $(KERNEL) $(OBJECTS) *.o *~
