# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

EZOM (Easy Simple Object Machine) is a minimal object-oriented programming language designed for the ez80 Agon Light 2 retro computer. It's based on the SOM (Simple Object Machine) language family but simplified and optimized for the ez80 platform with 512KB RAM.

## Build Commands

The project uses a Makefile with the ez80 AgDev toolchain:

**To build the project:**
```bash
cd vm/
make -f agmakefile
```

This creates `bin/ezom.bin` which can be run on the fab-agon-emulator.

**To test the executable:**
```bash
# Copy the built executable to the emulator's SD card
cp bin/ezom.bin ../../../fab-agon-emulator-v0.9.96-macos/sdcard/bin

# Change to the emulator directory and run in debug mode
cd ../../../fab-agon-emulator-v0.9.96-macos/
./fab-agon-emulator -d

# Check the debug output log file
cat sdcard/ezom.log
```

The program creates debugging output in the `ezom.log` file on the emulator's SD card for analysis.

The target platform is ez80 Agon Light 2 using:
- Compiler: `ez80-clang` (via AgDev toolchain)
- Emulator: `fab-agon-emulator` (optional)
- Flags: `-Wall -Oz -mllvm -profile-guided-section-prefix=false`

## Architecture Overview

EZOM is implemented in phases, with the current code representing Phase 3 complete and Phase 4.3.1 completed:

### Current Implementation Status

**Phase 3 ✅ COMPLETE**: Advanced object system with full SOM-compatible class hierarchy
**Phase 4.3.1 ✅ COMPLETE**: Conditional statements with block-based evaluation
**Phase 4.3.2 ✅ COMPLETE**: Loop constructs with Integer and Block loop methods

### Core Components

**Object System** (`src/vm/object.c`, `src/vm/objects.c`, `src/include/ezom_object.h`):
- 6-byte object headers with class pointers, hash codes, and type flags
- Built-in types: Object, Integer, String, Symbol, Class, Boolean, Block, Context, Array
- Method dictionaries with primitive function dispatch
- Complete SOM-compatible class hierarchy with inheritance
- Class hierarchy with single inheritance

**Memory Management** (`src/vm/memory.c`, `src/include/ezom_memory.h`):
**Memory Management** (`src/vm/memory.c`, `src/include/ezom_memory.h`):
- Complete garbage collection system with mark-and-sweep
- Object allocation with reference tracking
- 256KB heap for native development (56KB for ez80)
- Enhanced memory tracking and debugging

**Method Dispatch** (`src/vm/dispatch.c`, `src/include/ezom_dispatch.h`):
- Linear search in class hierarchy for method lookup
- Message sending with argument handling including keyword messages
- Primitive function table for built-in operations (80+ primitives)
- Support for unary, binary, and keyword messages

**Primitive Operations** (`src/vm/primitives.c`, `src/include/ezom_primitives.h`):
- Object primitives: class, =, hash, println
- Integer arithmetic: +, -, *, /, =, <, >, etc.
- String operations: length, concatenation, comparison
- Boolean operations: ifTrue:, ifFalse:, ifTrue:ifFalse:, not
- Block evaluation: value, value:
- Array operations: at:, at:put:, length
- Type checking and error handling

**Block System** (`src/vm/context.c`, `src/vm/evaluator.c`):
- Block objects with proper EZOM_TYPE_BLOCK flags
- Block evaluation with context creation
- Closure support for accessing outer scope variables
- Working conditional statements with block-based evaluation

See @ARCHITECTURE.md and @Phase1-2.md and @Phase3.md and @PHASE4_IMPLEMENTATION_PLAN.md

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

**Message Sending**: Three main interfaces:
- `ezom_send_unary_message(receiver, selector)` 
- `ezom_send_binary_message(receiver, selector, arg)`
- `ezom_send_message(msg)` for keyword messages with multiple arguments

**Block Objects**: Created via `ezom_create_ast_block()` with proper type flags and context binding.

**Conditional Evaluation**: Uses boolean primitives 50-56 for ifTrue:, ifFalse:, ifTrue:ifFalse:, and not operations.

## Current Development Status

**COMPLETED (Phase 4.3.2)**: Full loop construct support
- ✅ Integer loop methods (to:do:, timesRepeat:)
- ✅ Block loop methods (whileTrue:, whileFalse:)
- ✅ Proper loop variable scoping and block parameter passing
- ✅ All loop primitives working (24, 25, 62, 63)
- ✅ Comprehensive test suite validation

**NEXT PRIORITY**: Based on PHASE4_IMPLEMENTATION_PLAN.md, the next logical step is implementing 4.2.2 Block Objects Enhancement, which includes:
- Block parameters `[ :param | ... ]` and local variables `[ | local | ... ]`
- Enhanced closure capture of outer scope variables
- More sophisticated block evaluation contexts

However, you should validate this priority with the user before proceeding, as they may want to focus on other Phase 4 components like class definition parsing or file loading first.

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