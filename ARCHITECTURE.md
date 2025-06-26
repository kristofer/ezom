# EZOM Architecture Documentation
## Easy Simple Object Machine for ez80 Agon Light 2

**Version:** 1.0  
**Last Updated:** 2025-06-26  
**Target Platform:** ez80 Agon Light 2 (512KB RAM)  

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [System Architecture](#system-architecture)
3. [Core Components](#core-components)
4. [Memory Architecture](#memory-architecture)
5. [Object System](#object-system)
6. [Virtual Machine Design](#virtual-machine-design)
7. [Compiler Architecture](#compiler-architecture)
8. [Standard Library](#standard-library)
9. [Implementation Phases](#implementation-phases)
10. [Future Extensions](#future-extensions)
11. [Development Guidelines](#development-guidelines)

---

## Project Overview

EZOM (Easy Simple Object Machine) is a minimal object-oriented programming language and virtual machine designed specifically for the ez80 Agon Light 2 retro computer. The project follows a phased implementation approach, building from a basic interpreter to a full virtual machine with garbage collection and optimizations.

### Design Principles

- **Simplicity**: Minimal language constructs for easy implementation and learning
- **Performance**: Efficient execution on ez80 hardware with 512KB RAM constraints
- **Educational**: Suitable for teaching object-oriented virtual machine concepts
- **Modularity**: Clean separation between interpreter, virtual machine, and compiler components
- **Extensibility**: Architecture supports future enhancements and optimizations

### Current Status

- **Phase 1**: ✅ Basic object system with primitive operations
- **Phase 1.5**: ✅ Extended parser and AST evaluation
- **Phase 2**: ✅ Complete language features (classes, inheritance, blocks)
- **Phase 3**: 🔄 Memory management and garbage collection (planned)
- **Phases 4-6**: 📋 Standard library, optimization, and system integration (planned)

---

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    EZOM System Architecture                 │
├─────────────────────────────────────────────────────────────┤
│  Application Layer                                          │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │ REPL Shell  │ │  IDE Tools  │ │  Test Suite │           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
├─────────────────────────────────────────────────────────────┤
│  Language Layer                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              EZOM Language Runtime                  │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │Standard Lib │ │System Calls │ │Collections  │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  └─────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│  Execution Layer                                           │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                Virtual Machine                      │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │Interpreter  │ │   Compiler  │ │   Bytecode  │   │   │
│  │  │   Engine    │ │   Pipeline  │ │  Generator  │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  └─────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│  Object System Layer                                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              Object Management                      │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │   Object    │ │   Method    │ │    Class    │   │   │
│  │  │  Creation   │ │  Dispatch   │ │ Hierarchy   │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  └─────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│  Memory Management Layer                                   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              Memory Management                      │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │   Garbage   │ │   Memory    │ │   Object    │   │   │
│  │  │ Collection  │ │ Allocation  │ │   Pools     │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  └─────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│  Platform Layer                                            │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              ez80 Agon Platform                     │   │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │   │
│  │  │     MOS     │ │    VDP      │ │   Hardware  │   │   │
│  │  │ Integration │ │ Graphics    │ │   Access    │   │   │
│  │  └─────────────┘ └─────────────┘ └─────────────┘   │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

---

## Core Components

### 1. Object System (`src/vm/object.c`, `src/vm/objects.c`)

**Purpose**: Foundation of the object-oriented runtime
**Location**: `src/vm/object.c`, `src/vm/objects.c`, `src/include/ezom_object.h`

```c
// Core object structure (6-byte header)
typedef struct ezom_object {
    uint24_t class_ptr;     // Pointer to class object (3 bytes)
    uint16_t hash;          // Object hash code (2 bytes) 
    uint8_t  flags;         // GC and type flags (1 byte)
} ezom_object_t;
```

**Key Features**:
- Compact 6-byte object headers optimized for ez80
- Built-in types: Object, Integer, String, Symbol, Class
- Unified object creation interface
- Hash-based object identity

**Responsibilities**:
- Object lifecycle management
- Type system enforcement
- Memory layout optimization
- Object header maintenance

### 2. Memory Management (`src/vm/memory.c`)

**Purpose**: Heap allocation and memory lifecycle management
**Location**: `src/vm/memory.c`, `src/include/ezom_memory.h`

**Current Implementation** (Phase 1-2):
- Simple bump allocator
- 2-byte aligned allocation
- Basic statistics tracking
- 56KB heap at `0x042000`

**Future Implementation** (Phase 3):
- Mark-and-sweep garbage collector
- Free list allocator with size classes
- Large object heap
- Memory compaction
- Weak references and finalizers

### 3. Method Dispatch System (`src/vm/dispatch.c`)

**Purpose**: Dynamic method lookup and message sending
**Location**: `src/vm/dispatch.c`, `src/include/ezom_dispatch.h`

```c
// Method dispatch interface
uint24_t ezom_send_message(ezom_message_t* msg);
uint24_t ezom_send_unary_message(uint24_t receiver, uint24_t selector);
uint24_t ezom_send_binary_message(uint24_t receiver, uint24_t selector, uint24_t arg);
```

**Key Features**:
- Linear search in class hierarchy (Phase 1-2)
- Polymorphic inline cache (PIC) planned for Phase 6
- Primitive function dispatch
- Super call handling

### 4. Primitive Operations (`src/vm/primitives.c`)

**Purpose**: Built-in operations for core types
**Location**: `src/vm/primitives.c`, `src/include/ezom_primitives.h`

**Primitive Categories**:
- Object operations: `class`, `=`, `hash`, `println`
- Integer arithmetic: `+`, `-`, `*`, `/`, `<`, `>`
- String operations: `length`, `+`, `at:`
- Control flow: `ifTrue:`, `ifFalse:`, `to:do:`

### 5. Parser and AST System (`src/vm/lexer.c`, `src/vm/parser.c`, `src/vm/ast.c`)

**Purpose**: Source code analysis and abstract syntax tree generation
**Location**: Multiple files in `src/vm/` with `src/include/ezom_*` headers

**Components**:
- **Lexer** (`ezom_lexer.h`): Tokenizes EZOM source code
- **Parser** (`ezom_parser.h`): Builds AST from tokens
- **AST** (`ezom_ast.h`): Abstract syntax tree representation
- **Evaluator** (`ezom_evaluator.h`): Interprets AST nodes

**Supported Constructs**:
- Class definitions with inheritance
- Method definitions (instance and class methods)
- Block expressions with parameters
- Message sending (unary, binary, keyword)
- Control flow constructs

### 6. Bootstrap System (`src/vm/bootstrap.c`)

**Purpose**: Initialize core classes and runtime environment
**Location**: `src/vm/bootstrap.c`

**Responsibilities**:
- Create fundamental classes (Object, Class, Integer, String)
- Install primitive methods
- Set up class hierarchy
- Initialize global objects

### 7. Execution Context (`src/vm/context.c`)

**Purpose**: Manage execution environments and variable scoping
**Location**: `src/vm/context.c`, `src/include/ezom_context.h`

**Features**:
- Lexical scoping for blocks
- Local variable management
- Method invocation contexts
- Stack frame management

---

## Memory Architecture

### ez80 Memory Layout (512KB Total)

```
Memory Map:
0x000000 - 0x03FFFF: MOS/System (256KB)
0x040000 - 0x07FFFF: EZOM VM Space (256KB)

EZOM VM Space Detail:
├── 0x040000 - 0x041FFF: VM Code (8KB)
├── 0x042000 - 0x04FFFF: Object Heap (56KB)  [Phase 1-2]
├── 0x050000 - 0x05FFFF: Method Cache (64KB) [Phase 2+]
├── 0x060000 - 0x06FFFF: Stack Space (64KB)  [Phase 2+]
└── 0x070000 - 0x07FFFF: String Pool (64KB)  [Phase 2+]

Phase 3 Extended Layout:
├── 0x042000 - 0x04FFFF: Small Object Heap (56KB)
├── 0x050000 - 0x057FFF: Large Object Heap (32KB)
├── 0x058000 - 0x059FFF: GC Mark Stack (8KB)
├── 0x05A000 - 0x05AFFF: Free List Metadata (4KB)
└── 0x05B000 - 0x05FFFF: String Interning Pool (20KB)
```

### Object Layout Optimization

**Small Objects** (≤ 256 bytes):
- Allocated in main heap using size classes
- Managed by free list allocator
- Fast allocation/deallocation

**Large Objects** (> 256 bytes):
- Separate heap region
- Direct allocation without size classes
- Tracked in linked list for GC

**String Interning**:
- Dedicated string pool for symbols
- Hash-based deduplication
- Reduces memory usage for repeated strings

---

## Object System

### Class Hierarchy

```
Object
  ├── Class
  ├── Metaclass
  ├── Integer
  ├── String
  ├── Symbol
  ├── Array
  ├── Block
  ├── Context
  ├── Boolean
  │   ├── True
  │   └── False
  ├── Nil
  └── [User-defined classes]
```

### Object Creation Pattern

All objects are created through factory functions:

```c
uint24_t ezom_create_integer(int16_t value);
uint24_t ezom_create_string(const char* data, uint16_t length);
uint24_t ezom_create_symbol(const char* data, uint16_t length);
uint24_t ezom_create_array(uint16_t size);
uint24_t ezom_create_class(char* name, uint24_t superclass, ...);
```

### Method Installation

Methods are added to classes via helper functions:

```c
void ezom_install_method(uint24_t class_ptr, char* selector, 
                        uint24_t code, uint8_t arg_count, bool is_primitive);
void ezom_install_primitive(uint24_t class_ptr, char* selector, uint8_t prim_num);
```

---

## Virtual Machine Design

### Current Architecture (Phase 1-2)

**Interpretation Model**: Direct AST evaluation
- Source → Lexer → Parser → AST → Evaluator → Result
- No bytecode generation yet
- Suitable for prototyping and small programs

### Future Architecture (Phase 3+)

**Hybrid Model**: AST + Bytecode compilation
- Source → Lexer → Parser → AST → Compiler → Bytecode → VM → Result
- Bytecode generation for performance-critical code
- AST interpretation for development/debugging

### Bytecode Design (Planned)

**Stack-Based VM** with 32 core instructions:

```assembly
// Stack Operations
PUSH_LOCAL     <index>     // Push local variable
POP_LOCAL      <index>     // Pop to local variable
PUSH_LITERAL   <index>     // Push literal from pool

// Object Operations  
SEND           <selector>  // Send message
SEND_SUPER     <selector>  // Send to super
RETURN                    // Return from method

// Control Flow
JUMP           <offset>    // Unconditional jump
JUMP_TRUE      <offset>    // Jump if true
JUMP_FALSE     <offset>    // Jump if false

// Array/Block Operations
NEW_ARRAY      <size>      // Create new array
NEW_BLOCK      <offset>    // Create new block
BLOCK_RETURN              // Return from block
```

---

## Compiler Architecture

### Multi-Phase Compilation

```
Source Code
     ↓
┌─────────────┐
│   Lexer     │  → Tokens
│  (lexer.c)  │
└─────────────┘
     ↓
┌─────────────┐
│   Parser    │  → AST
│ (parser.c)  │
└─────────────┘
     ↓
┌─────────────┐
│  Semantic   │  → Annotated AST
│  Analysis   │
└─────────────┘
     ↓
┌─────────────┐
│  Optimizer  │  → Optimized AST/Bytecode
│   (Phase 6) │
└─────────────┘
     ↓
┌─────────────┐
│   Code      │  → Bytecode/Machine Code
│ Generation  │
└─────────────┘
```

### Current Implementation

**Phase 1-2**: AST-based interpretation
- Direct execution of parsed AST
- No optimization passes
- Suitable for language development

**Phase 3+**: Bytecode compilation
- AST → Bytecode translation
- Constant folding and dead code elimination
- Register allocation for local variables

### Planned Optimizations (Phase 6)

- **Method Inlining**: Inline small methods
- **Polymorphic Inline Caching**: Cache method lookups
- **Loop Optimization**: Reduce overhead in loops
- **Dead Code Elimination**: Remove unused code
- **Constant Propagation**: Optimize constant expressions

---

## Standard Library

### Core Classes (Built-in)

**Object** - Root class:
```smalltalk
Object = (
    class = primitive
    = other = primitive
    hash = primitive
    println = primitive
)
```

**Integer** - 16-bit signed integers:
```smalltalk
Integer = Object (
    + other = primitive
    - other = primitive
    * other = primitive
    / other = primitive
    < other = primitive
    > other = primitive
    to: end do: block = primitive
)
```

**String** - Immutable character sequences:
```smalltalk
String = Object (
    length = primitive
    at: index = primitive
    + other = primitive
    = other = primitive
)
```

### Extended Library (Phases 4-5)

**Array** - Dynamic arrays:
```smalltalk
Array = Object (
    | elements size |
    
    at: index = ( ^elements at: index )
    at: index put: value = ( elements at: index put: value )
    size = ( ^size )
    do: block = ( 1 to: size do: [:i | block value: (self at: i)] )
)
```

**Collection** - Abstract collection protocol:
```smalltalk
Collection = Object (
    do: block = subclassResponsibility
    select: block = ( | result |
        result := self species new.
        self do: [:each | 
            (block value: each) ifTrue: [result add: each]].
        ^result
    )
)
```

### System Integration Classes

**System** - VM interaction:
```smalltalk
System = Object (
    ----
    exit: code = primitive
    gc = primitive
    version = ( ^'EZOM 1.0' )
)
```

**File** - File I/O operations:
```smalltalk
File = Object (
    | handle name |
    
    open: filename mode: mode = primitive
    read: bytes = primitive
    write: data = primitive
    close = primitive
)
```

---

## Implementation Phases

### Phase 1: Minimal VM ✅ (Completed)
- Basic object representation
- Simple method dispatch
- Memory allocation
- Core primitives (integers, basic I/O)

**Files Implemented**:
- `object.c/h` - Object system core
- `memory.c/h` - Basic allocation
- `primitives.c/h` - Core primitives
- `dispatch.c/h` - Method dispatch
- `objects.c/h` - Object creation

### Phase 1.5: Extended Parser ✅ (Completed)
- Complete lexer implementation
- AST-based parser
- Expression evaluation
- Control flow constructs

**Files Implemented**:
- `lexer.c/h` - Tokenization
- `parser.c/h` - AST generation
- `ast.c/h` - AST representation
- `evaluator.c/h` - AST interpretation

### Phase 2: Core Language ✅ (Completed)
- Class definitions and inheritance
- Block objects and evaluation
- Method compilation and execution
- Bootstrap system

**Files Implemented**:
- `bootstrap.c` - Runtime initialization
- `context.c/h` - Execution contexts

### Phase 3: Memory Management 🔄 (In Progress)
- Mark-and-sweep garbage collector
- Free list allocator
- Memory compaction
- Weak references and finalizers

**Planned Files**:
- `gc.c/h` - Garbage collection
- `freelist.c/h` - Memory allocation
- `finalize.c/h` - Object finalization
- `weak.c/h` - Weak references

### Phase 4: Standard Library 📋 (Planned)
- Complete Object class
- Integer/String classes with full operations
- Array class with iteration
- Collection hierarchy

### Phase 5: System Integration 📋 (Planned)
- File loading from SD card
- VGA display integration
- Keyboard input handling
- Error reporting and debugging

### Phase 6: Optimization 📋 (Planned)
- Bytecode generation
- Method caching improvements
- Performance profiling
- Memory layout optimization

---

## Future Extensions

### Virtual Machine Separation

**Goal**: Separate interpreter from virtual machine for better modularity

```
Current Architecture:
[Source] → [Parser] → [AST] → [Evaluator] → [Result]

Future Architecture:
[Source] → [Compiler] → [Bytecode] → [VM] → [Result]
                   ↘            ↗
                    [Interpreter] (for development)
```

**Benefits**:
- Better performance through bytecode execution
- Easier optimization and profiling
- Clear separation of concerns
- Support for multiple frontends

### Compiler Pipeline

**Planned Architecture**:
1. **Frontend**: Lexer, Parser, Semantic Analysis
2. **Middle-end**: AST transformations, optimizations
3. **Backend**: Bytecode generation, machine code emission

**Standard Library in SOM**:
- Core classes written in EZOM itself
- Bootstrap minimal C implementation
- Self-hosting standard library
- Better integration with language features

### Platform Extensions

**Agon Light 2 Specific**:
- VGA graphics library
- Sound synthesis support
- Serial communication
- SD card file system
- Real-time clock integration

**Cross-Platform Support**:
- Desktop development environment
- Web-based emulator
- Cross-compilation tools
- Debugging and profiling tools

---

## Development Guidelines

### Code Organization

**File Naming Convention**:
- `ezom_*.h` - Public header files
- `ezom_*.c` - Implementation files
- `test_*.c` - Test suites
- `benchmark_*.c` - Performance tests

**Module Boundaries**:
- Object system (object, memory, primitives)
- Language runtime (dispatch, context, bootstrap)
- Compiler frontend (lexer, parser, ast)
- Future: Bytecode backend (compiler, optimizer, vm)

### Memory Management

**Current Guidelines**:
- Use factory functions for object creation
- No manual memory management (GC in Phase 3)
- Prefer stack allocation for temporary data
- Minimize heap allocations in hot paths

**Future Guidelines** (Phase 3+):
- Use GC-aware allocation functions
- Properly manage object references
- Use weak references for circular structures
- Implement finalizers for resource cleanup

### Performance Considerations

**ez80 Optimization**:
- Minimize memory allocations
- Use 16-bit integers where possible
- Avoid deep recursion (stack limited)
- Cache frequently accessed data

**Code Quality**:
- Follow C89 style for AgDev compatibility
- Use descriptive names for clarity
- Comment complex algorithms
- Maintain test coverage

### Testing Strategy

**Phase-based Testing**:
- Unit tests for each component
- Integration tests for phase completion
- Performance benchmarks
- Regression test suite

**Test Categories**:
- Basic functionality tests
- Memory management tests
- Performance benchmarks
- Platform-specific tests
- Language compliance tests

---

## Conclusion

EZOM represents a carefully designed object-oriented language and virtual machine architecture optimized for the ez80 Agon Light 2 platform. The phased implementation approach ensures that each component can be thoroughly tested and validated before proceeding to the next level of functionality.

The current implementation provides a solid foundation with core object system, method dispatch, and language parsing capabilities. Future phases will add essential features like garbage collection, a complete standard library, and performance optimizations while maintaining the simplicity and educational value that makes EZOM an excellent platform for learning virtual machine implementation.

The architecture balances the constraints of the ez80 platform with the need for a complete, functional programming environment, making it suitable for both educational purposes and practical retro computing applications.