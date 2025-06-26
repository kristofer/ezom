# Agon Light 2 C Development Notes

## Table of Contents
1. [Agon Light 2 Overview](#agon-light-2-overview)
2. [AgonDev Toolchain](#agondev-toolchain)
3. [eZ80 Architecture](#ez80-architecture)
4. [Memory Model](#memory-model)
5. [C Development Considerations](#c-development-considerations)
6. [Build System](#build-system)
7. [Debugging and Testing](#debugging-and-testing)
8. [Known Issues and Workarounds](#known-issues-and-workarounds)

## Agon Light 2 Overview

The Agon Light 2 is a modern 8-bit computer based on the Zilog eZ80 processor, designed as a spiritual successor to classic 8-bit machines. It's part of the broader Agon ecosystem created by Bernardo Kastrup.

### Key Specifications
- **CPU**: Zilog eZ80F92 running at 18.432 MHz
- **Memory**: 512KB RAM (expandable)
- **Video**: ESP32-based VDP (Video Display Processor)
- **Storage**: MicroSD card support
- **I/O**: USB, GPIO, UART
- **Graphics**: VGA output via ESP32 VDP

### Architecture Philosophy
- Modern retro computing platform
- Focus on educational and hobbyist programming
- Native development environment with modern tooling
- Backwards compatibility with Z80 assembly concepts

## AgonDev Toolchain

The AgonDev toolchain is a specialized LLVM-based development environment for the Agon Light 2.

### Core Components
- **Compiler**: Modified LLVM/Clang targeting eZ80
- **Assembler**: `ez80-none-elf-as`
- **Linker**: Custom linker for eZ80 memory model
- **Libraries**: Standard C library adapted for eZ80

### Installation Location
```bash
/Users/kristofer/agondev/release/
├── bin/           # Executables (clang, as, ld, etc.)
├── include/       # System headers
└── lib/           # Libraries and runtime
```

### Compiler Invocation
```bash
# Basic compilation
ez80-none-elf-clang -cc1 -triple ez80-none-unknown-elf [options] file.c

# Key flags observed:
-target-cpu ez80
-mrelocation-model static
-mframe-pointer=none
-Oz                    # Optimize for size
-march=ez80+full      # Full eZ80 instruction set
```

## eZ80 Architecture

### Memory Addressing
The eZ80 uses a **24-bit address space** (16MB addressable), which is crucial for C development:

```c
// 24-bit pointer type definitions
#ifndef uint24_t
#ifdef __ez80__
#define uint24_t unsigned long  // Native 24-bit on eZ80
#else
typedef uint32_t uint24_t;      // 32-bit simulation on other platforms
#endif
#endif
```

### Register Model
- **24-bit registers**: HL, DE, BC, IX, IY, SP
- **16-bit compatibility**: Lower 16 bits accessible as traditional Z80 registers
- **Accumulator**: 8-bit A register

### Instruction Set
- Full Z80 compatibility
- Extended 24-bit addressing modes
- Additional eZ80-specific instructions
- Mix of 8-bit, 16-bit, and 24-bit operations

## Memory Model

### Address Space Layout
```
0x000000 - 0x01FFFF    Flash ROM (128KB)
0x040000 - 0x0BFFFF    RAM (512KB)
0x0C0000 - 0xFFFFFF    I/O and extended addressing
```

### Programming Considerations
1. **Pointer Size**: Always 24-bit (3 bytes)
2. **Memory Efficiency**: Limited RAM requires careful allocation
3. **Stack Management**: Stack grows downward from high memory
4. **Code/Data Separation**: ROM-based code, RAM-based data

### Memory Management Patterns
```c
// Typical memory allocation signature
uint24_t ezom_allocate(size_t size);

// Pointer arithmetic must account for 24-bit addresses
uint24_t ptr = ezom_allocate(100);
// ptr is a 24-bit address value
```

## C Development Considerations

### Data Types
```c
// Standard types work as expected
char         // 8-bit
short        // 16-bit
int          // 16-bit (NOT 32-bit like modern platforms)
long         // 32-bit
uint24_t     // 24-bit addresses (custom type)
```

### Pointer Handling
```c
// All pointers are 24-bit
void* ptr;           // 24-bit pointer
char* str;           // 24-bit pointer to char
uint24_t addr;       // Raw 24-bit address value

// Pointer-to-integer conversions
uint24_t addr = (uint24_t)ptr;
void* ptr = (void*)addr;
```

### Function Signatures
```c
// Functions dealing with object addresses
typedef uint24_t (*ezom_primitive_fn)(uint24_t receiver, 
                                      uint24_t* args, 
                                      uint8_t arg_count);
```

### Format Strings
```c
// Printf format specifiers for 24-bit values
printf("Address: 0x%06lX\n", addr);  // Use %lX for uint24_t
sprintf(buffer, "Object@0x%06lX", obj_ptr);
```

## Build System

### Makefile Structure
The AgonDev build system uses custom makefiles:

```makefile
# Example from agmakefile
CC = ez80-none-elf-clang
AS = ez80-none-elf-as
LD = ez80-none-elf-ld

CFLAGS = -Oz -I. -I$(AGONDEV)/include
ASFLAGS = -march=ez80+full
LDFLAGS = -T linker_script.ld
```

### Build Process
1. **Compilation**: C to eZ80 assembly
2. **Assembly**: Assembly to object files
3. **Linking**: Object files to binary
4. **Address Assignment**: Setting start address (e.g., 0x40000)

### Output Files
```
bin/ezom_phase1.bin     # Final executable binary
bin/ezom_phase1.map     # Memory map
obj/*.o                 # Object files
```

## Debugging and Testing

### Cross-Platform Development
Projects often support both native and eZ80 builds:

```c
#ifdef __ez80__
    // eZ80-specific code
    #define uint24_t unsigned long
#else
    // Native testing code
    typedef uint32_t uint24_t;
#endif
```

### Memory Constraints
- **Code Size**: Optimize for size (-Oz flag)
- **Stack Usage**: Limited stack space
- **Heap Management**: Custom allocators often needed

### Common Patterns
```c
// Error handling with nil returns
if (error_condition) return g_nil;

// Boolean values as addresses
return success ? g_true : g_false;

// Type checking
if (!ezom_is_integer(obj)) return g_nil;
```

## Known Issues and Workarounds

### Type Definition Conflicts
**Problem**: Multiple definitions of `uint24_t` across headers
**Solution**: Use header guards and conditional compilation:

```c
#ifndef uint24_t
#ifdef __ez80__
#define uint24_t unsigned long
#else
typedef uint32_t uint24_t;
#endif
#endif
```

### Format String Warnings
**Problem**: Printf format mismatches for 24-bit values
**Solution**: Use `%lX` for `uint24_t` values:

```c
sprintf(buffer, "Address: 0x%06lX", addr);  // Not %06X
```

### Include Path Issues
**Problem**: System headers not found
**Solution**: Explicit include paths in makefile:

```makefile
CFLAGS += -I$(AGONDEV)/include -I/opt/homebrew/include
```

### Memory Allocation
**Problem**: Standard malloc may not work correctly
**Solution**: Use custom allocators:

```c
uint24_t ezom_allocate(size_t size);
void ezom_deallocate(uint24_t ptr);
```

## Best Practices

### 1. Memory Efficiency
- Use size optimization (-Oz)
- Minimize dynamic allocation
- Prefer stack allocation for small objects
- Use bit fields for flags

### 2. Portable Code
- Use conditional compilation for eZ80 vs native
- Define platform-specific types clearly
- Test on both platforms

### 3. Error Handling
- Use consistent error return values (g_nil)
- Check all allocations
- Validate input parameters

### 4. Documentation
- Document memory ownership
- Specify address vs pointer usage
- Note platform-specific behavior

## Development Workflow

### 1. Setup
```bash
# Ensure AgonDev toolchain is installed
export AGONDEV=/Users/kristofer/agondev/release
export PATH=$AGONDEV/bin:$PATH
```

### 2. Build
```bash
cd vm
make -f agmakefile          # eZ80 build
make -f native_makefile     # Native testing build
```

### 3. Test
```bash
# Native testing first
./bin/native_test

# Deploy to Agon Light 2
# (Copy .bin file to SD card)
```

### 4. Debug
- Use printf debugging (limited by memory)
- Leverage native builds for algorithm testing
- Monitor memory usage carefully

## Conclusion

The Agon Light 2 provides a unique development environment that bridges retro computing with modern tooling. The 24-bit address space and memory constraints require careful consideration, but the AgonDev toolchain provides excellent support for C development. Success requires understanding both the eZ80 architecture and the specific patterns that work well within the platform's constraints.
