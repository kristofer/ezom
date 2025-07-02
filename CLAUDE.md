# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

EZOM (Easy Simple Object Machine) is a minimal object-oriented programming language designed for the ez80 Agon Light 2 retro computer. It's based on the SOM (Simple Object Machine) language family but simplified and optimized for the ez80 platform with 512KB RAM.

## Build Commands

The project uses a Makefile with the ez80 AgDev toolchain:

```bash
make                    # Build ezom_phase1.bin
make run               # Build and run in emulator  
make test              # Build and run test_phase1.c
make debug             # Build with debug symbols
make clean             # Clean build files
```

The target platform is ez80 Agon Light 2 using:
- Compiler: `ez80-clang`
- Emulator: `fab-agon-emulator` (optional)
- Flags: `-Wall -Oz -mllvm -profile-guided-section-prefix=false`

## Architecture Overview

EZOM is implemented in phases, with the current code representing Phase 1 - a complete implementation split into modular files:

### Core Components

**Object System** (`src/vm/object.c`, `src/vm/objects.c`, `src/include/ezom_object.h`):
- 6-byte object headers with class pointers, hash codes, and type flags
- Built-in types: Object, Integer, String, Symbol, Class
- Method dictionaries with primitive function dispatch
- Class hierarchy with single inheritance

**Memory Management** (`src/vm/memory.c`, `src/include/ezom_memory.h`):
- Simple bump allocator for ez80 heap (56KB at 0x042000)
- 2-byte aligned allocation 
- Basic statistics tracking
- Will be extended with GC in Phase 3

**Method Dispatch** (`src/vm/dispatch.c`, `src/include/ezom_dispatch.h`):
- Linear search in class hierarchy for method lookup
- Message sending with argument handling
- Primitive function table for built-in operations
- Support for unary and binary messages

**Primitive Operations** (`src/vm/primitives.c`, `src/include/ezom_primitives.h`):
- Object primitives: class, =, hash, println
- Integer arithmetic: +, -, *, =
- String operations: length, concatenation
- Type checking and error handling

### Memory Layout (ez80 Specific)

```
0x040000 - 0x041FFF: VM Code (8KB)
0x042000 - 0x04FFFF: Object Heap (56KB)  
0x050000 - 0x05FFFF: Method Cache (64KB) [Phase 2]
0x060000 - 0x06FFFF: Stack Space (64KB) [Phase 2]
0x070000 - 0x07FFFF: String Pool (64KB) [Phase 2]
```

## Key Patterns

**Object Creation**: All objects created through factory functions like `ezom_create_integer()`, `ezom_create_string()`, etc.

**Method Installation**: Methods added to classes via helper functions that create symbols and populate method dictionaries.

**Message Sending**: Two main interfaces:
- `ezom_send_unary_message(receiver, selector)` 
- `ezom_send_binary_message(receiver, selector, arg)`

**Error Handling**: Simple printf-based error reporting, returns 0/NULL on failure.

## Implementation Phases

**Phase 1** (Current): Basic object system with primitive operations
**Phase 2**: Complete parser, inheritance, blocks, control flow  
**Phase 3**: Mark-and-sweep garbage collection
**Phase 4**: Standard library implementation
**Phase 5**: System integration (file I/O, VGA, keyboard)
**Phase 6**: Optimization and performance tuning

## Testing

The project includes comprehensive test suites:
- `test_phase1.c`: Core VM functionality tests
- Tests cover object creation, method dispatch, memory allocation, primitive operations
- Run with `make test` - expects all tests to pass

## Development Notes

- Code follows C89 style for maximum compatibility with AgDev
- Uses ez80-specific data types (`uint24_t` for pointers)
- Optimized for low memory usage and fast execution
- Designed for educational purposes and retro computing

## File Structure

```
src/
├── include/               # Header files
│   ├── ezom_object.h     # Object system definitions
│   ├── ezom_memory.h     # Memory management
│   ├── ezom_primitives.h # Primitive operations
│   └── ezom_dispatch.h   # Method dispatch
└── vm/                   # VM implementation
    ├── main.c            # VM entry point and bootstrap
    ├── memory.c          # Memory allocation
    ├── object.c          # Object system core
    ├── objects.c         # Object creation functions
    ├── primitives.c      # Primitive implementations
    └── dispatch.c        # Method dispatch engine

test_phase1.c          # Phase 1 test suite
Makefile              # Build system for ez80 AgDev toolchain
README.md             # Project overview
Spec.md               # Complete language specification
Phase1-2.md           # Detailed Phase 1-2 implementation spec  
Phase3.md             # Phase 3 memory management spec
```

The implementation has been split into modular files following the structure outlined in the original consolidated code comments.

## Known Issues and Resolutions

### IY=0xffffff Crash Issue (July 2, 2025)
**Status:** 🔍 ROOT CAUSE IDENTIFIED - Bootstrap Issue

**Problem:** CPU crashes with `memory 0x1000005 out of bounds` during VM initialization.
- **Assembly:** `LD C, (IY+7)` instruction fails when `IY=0xffffff`
- **Actual Location:** Bootstrap phase in `ezom_bootstrap_classes()` function
- **Precise Crash Point:** After Object class method dictionary creation, during Symbol class creation

**Root Cause Analysis (Using File-Based Debug Logging):**
1. **Crash occurs during bootstrap, NOT during integer addition**
2. **Debug log shows:** Program successfully creates Object class and method dictionary
3. **Crash location:** Between lines 17-18 in ezom.log (after method dictionary, before Symbol class)
4. **Issue:** `ezom_create_method_dictionary()` calls `ezom_init_object(ptr, g_object_class, EZOM_TYPE_OBJECT)` 
   when `g_object_class` may not be fully initialized

**Critical Discovery:** The original crash happens much earlier than expected - during VM bootstrap, not during method dispatch.

**Solution Strategy:**
1. **Fix Bootstrap Order:** Ensure `g_object_class` is fully operational before creating method dictionaries
2. **Alternative:** Create method dictionaries with NULL class initially, set class later
3. **Validation:** Add corruption checks in `ezom_init_object()` 
4. **Logging System:** File-based debug logging to `/Users/kristofer/fab-agon-emulator-v0.9.96-macos/sdcard/ezom.log`

**Debug Tools Implemented:**
- File-based logging system in `src/dispatch.c` with `ezom_log()` function
- Detailed bootstrap tracing in `src/bootstrap.c`
- Emergency bypass (now unnecessary once bootstrap is fixed)

**Next Steps:**
1. Fix the bootstrap class creation order issue
2. Remove emergency bypass once root cause is resolved
3. Verify full VM functionality after bootstrap fix

**Files:** 
- `src/bootstrap.c`: Bootstrap function with detailed logging
- `src/dispatch.c`: File logging system implementation
- `PRIMITIVE_CRASH_RESOLUTION_REPORT.md`: Historical analysis (pre-root cause discovery)