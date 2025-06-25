# EZOM Language Specification
## Easy Simple Object Machine for ez80 Agon Light 2

**Version:** 1.0  
**Target Platform:** ez80 Agon Light 2  
**Implementation Language:** C  

---

## 1. Introduction

EZOM (Easy Simple Object Machine) is a minimal object-oriented programming language designed specifically for the ez80 Agon Light 2 retro computer. It is based on the SOM (Simple Object Machine) language family but simplified and optimized for the constraints and capabilities of the ez80 platform.

### 1.1 Design Goals

- **Simplicity**: Minimal language constructs for easy implementation and learning
- **Performance**: Efficient execution on ez80 hardware with 512KB RAM
- **Educational**: Suitable for teaching object-oriented virtual machine concepts
- **Portability**: Clean C implementation that can be compiled with AgDev toolchain

### 1.2 Key Features

- Dynamic object model with single inheritance
- Message passing with dynamic method dispatch
- Automatic memory management
- Minimal but complete standard library
- Interactive REPL environment
- File-based program execution

---

## 2. Language Syntax

EZOM uses a Smalltalk-inspired syntax with simplified constructs suitable for the ez80 platform.

### 2.1 Basic Syntax Elements

#### 2.1.1 Comments
```
" This is a comment "
```

#### 2.1.2 Literals

**Integers:**
```
42
-17
0
```

**Strings:**
```
'Hello World!'
'This is a string'
```

**Symbols:**
```
#symbol
#plus
#println
```

**Arrays:**
```
#(1 2 3 4)
#('hello' 'world')
#(1 'mixed' #array)
```

### 2.2 Class Definition

```
ClassName = Object (
    | instanceVariable1 instanceVariable2 |
    
    "Instance methods"
    methodName = (
        "method body"
        ^result
    )
    
    methodWith: argument = (
        "method with argument"
        ^self
    )
    
    ----
    
    "Class methods (after ---- separator)"
    new = (
        ^super new initialize
    )
)
```

### 2.3 Method Invocation

**Unary messages:**
```
object methodName
```

**Binary messages:**
```
1 + 2
object = anotherObject
```

**Keyword messages:**
```
object methodWith: argument
object method: arg1 with: arg2
```

### 2.4 Blocks (Closures)

```
[ expressions ]
[ :arg | expressions ]
[ :arg1 :arg2 | expressions ]
```

### 2.5 Control Structures

**Conditionals:**
```
condition ifTrue: [ trueBlock ]
condition ifFalse: [ falseBlock ]
condition ifTrue: [ trueBlock ] ifFalse: [ falseBlock ]
```

**Loops:**
```
1 to: 10 do: [ :i | i println ]
[ condition ] whileTrue: [ body ]
collection do: [ :each | each println ]
```

---

## 3. Object Model

### 3.1 Class Hierarchy

```
Object
  ├── Class
  ├── Metaclass
  ├── Integer
  ├── String
  ├── Symbol
  ├── Array
  ├── Block
  ├── Boolean
  │   ├── True
  │   └── False
  └── Nil
```

### 3.2 Core Classes

#### 3.2.1 Object
The root class providing basic functionality:
- `class` - returns the object's class
- `=` - equality comparison
- `println` - print object followed by newline
- `hash` - hash code for the object

#### 3.2.2 Integer
Fixed-size integers (16-bit on ez80):
- `+`, `-`, `*`, `/`, `%` - arithmetic operations
- `<`, `>`, `<=`, `>=`, `=` - comparison operations
- `to:do:` - iteration construct

#### 3.2.3 String
Immutable character sequences:
- `length` - string length
- `+` - string concatenation
- `at:` - character access
- `=` - string equality

#### 3.2.4 Array
Dynamic arrays:
- `new:` - create array of given size
- `at:` - element access
- `at:put:` - element assignment
- `length` - array size
- `do:` - iteration

#### 3.2.5 Block
Executable code blocks:
- `value` - execute block with no arguments
- `value:` - execute block with one argument
- `value:with:` - execute block with two arguments

---

## 4. Virtual Machine Architecture

### 4.1 Memory Layout (ez80 Specific)

```
Memory Map (512KB total):
0x000000 - 0x03FFFF: MOS/System (256KB)
0x040000 - 0x07FFFF: EZOM VM Space (256KB)
  ├── 0x040000 - 0x041FFF: VM Code (8KB)
  ├── 0x042000 - 0x04FFFF: Object Heap (56KB)  
  ├── 0x050000 - 0x05FFFF: Method Cache (64KB)
  ├── 0x060000 - 0x06FFFF: Stack Space (64KB)
  └── 0x070000 - 0x07FFFF: String Pool (64KB)
```

### 4.2 Object Representation

Each object has a 6-byte header in ADL mode:
```c
typedef struct {
    uint24_t class_ptr;     // Pointer to class object
    uint16_t hash;          // Object hash code
    uint8_t  flags;         // GC and other flags
} object_header_t;
```

### 4.3 Bytecode Instructions

EZOM uses a stack-based bytecode VM with 32 core instructions:

```
// Stack Operations
PUSH_LOCAL     <index>     // Push local variable
POP_LOCAL      <index>     // Pop to local variable
PUSH_GLOBAL    <index>     // Push global variable
POP_GLOBAL     <index>     // Pop to global variable
PUSH_LITERAL   <index>     // Push literal from pool

// Object Operations  
SEND           <selector>  // Send message
SEND_SUPER     <selector>  // Send to super
RETURN                    // Return from method
RETURN_SELF               // Return self

// Control Flow
JUMP           <offset>    // Unconditional jump
JUMP_TRUE      <offset>    // Jump if true
JUMP_FALSE     <offset>    // Jump if false

// Array/Block Operations
NEW_ARRAY      <size>      // Create new array
NEW_BLOCK      <offset>    // Create new block
BLOCK_RETURN              // Return from block

// Primitives
HALT                      // Stop execution
NOP                       // No operation
```

### 4.4 Method Dispatch

EZOM uses a polymorphic inline cache (PIC) for method dispatch:

1. **Lookup Phase**: Search method in class hierarchy
2. **Cache Phase**: Store result in method cache
3. **Invoke Phase**: Execute method or primitive

---

## 5. Memory Management

### 5.1 Garbage Collection

EZOM implements a mark-and-sweep garbage collector optimized for ez80:

**Mark Phase:**
- Uses a mark stack to avoid recursion
- Marks all reachable objects from roots

**Sweep Phase:**
- Scans heap linearly
- Frees unmarked objects
- Compacts free space

### 5.2 Memory Allocation

- **Small objects** (<= 32 bytes): Free list allocation
- **Large objects** (> 32 bytes): First-fit allocation
- **Emergency allocation**: Trigger GC if allocation fails

---

## 6. Standard Library

### 6.1 System Classes

**System** - VM interaction:
```
System = Object (
    ----
    exit: code = primitive
    gc = primitive
    printNewline = primitive
)
```

**Console** - I/O operations:
```
Console = Object (
    ----
    print: string = primitive
    println: string = primitive
    readLine = primitive
)
```

### 6.2 Collection Classes

**Collection** - Abstract collection:
```
Collection = Object (
    do: block = subclassResponsibility
    size = subclassResponsibility
    isEmpty = ( ^self size = 0 )
)
```

---

## 7. Development Workflow

### 7.1 File Structure

```
src/
├── vm/
│   ├── main.c              // VM entry point
│   ├── object.c            // Object system
│   ├── class.c             // Class management
│   ├── interpreter.c       // Bytecode interpreter
│   ├── gc.c               // Garbage collector
│   ├── primitives.c       // Primitive operations
│   └── memory.c           // Memory management
├── compiler/
│   ├── lexer.c            // Lexical analysis
│   ├── parser.c           // Syntax analysis
│   ├── compiler.c         // Code generation
│   └── ast.c              // Abstract syntax tree
├── runtime/
│   ├── Object.som         // Object class
│   ├── Integer.som        // Integer class
│   ├── String.som         // String class
│   ├── Array.som          // Array class
│   └── System.som         // System class
└── tests/
    ├── basic/             // Basic functionality tests
    ├── objects/           // Object system tests
    └── collections/       // Collection tests
```

### 7.2 Build Process

Using AgDev toolchain:

```makefile
# Makefile for EZOM
CC = ez80-clang
CFLAGS = -Wall -Oz -mllvm -profile-guided-section-prefix=false
TARGET = ezom.bin

SOURCES = $(wildcard src/vm/*.c src/compiler/*.c)
OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

test: $(TARGET)
	fab-agon-emulator $(TARGET) tests/basic/hello.som
```

### 7.3 Testing Strategy

**Phase 1: Core VM**
1. Object creation and basic operations
2. Method dispatch (unary, binary, keyword)
3. Memory management and GC
4. Primitive operations

**Phase 2: Language Features**
1. Class definition and instantiation
2. Inheritance and super calls
3. Block creation and evaluation
4. Control flow constructs

**Phase 3: Standard Library**
1. Core classes (Object, Integer, String)
2. Collection classes (Array)
3. System integration
4. I/O operations

**Phase 4: Applications**
1. Hello World program
2. Simple calculator
3. Text-based adventure game
4. Performance benchmarks

---

## 8. Performance Targets

### 8.1 Memory Usage
- **VM Core**: <= 8KB
- **Runtime Heap**: <= 48KB (with room for expansion)
- **Method Cache**: <= 64KB
- **Stack Space**: <= 64KB

### 8.2 Execution Speed
- **Method Dispatch**: <= 100 cycles per call
- **Object Allocation**: <= 50 cycles per object
- **GC Pause**: <= 10ms for full collection
- **Startup Time**: <= 2 seconds

### 8.3 Compatibility
- **MOS Integration**: Full compatibility with Agon MOS
- **File System**: Support for .som file loading from SD card
- **Display**: VGA text output via VDP
- **Input**: PS/2 keyboard support

---

## 9. Implementation Phases

### Phase 1: Minimal VM (Weeks 1-2)
- [ ] Basic object representation
- [ ] Simple method dispatch
- [ ] Memory allocation
- [ ] Core primitives (integers, basic I/O)
- [ ] **Test**: Create and call methods on objects

### Phase 2: Core Language (Weeks 3-4)
- [ ] Complete parser for class definitions
- [ ] Inheritance support
- [ ] Block objects and evaluation
- [ ] Basic control flow
- [ ] **Test**: Define classes, create instances, send messages

### Phase 3: Memory Management (Weeks 5-6)
- [ ] Mark-and-sweep garbage collector
- [ ] Memory compaction
- [ ] Root set management
- [ ] Finalization support
- [ ] **Test**: Long-running programs without memory leaks

### Phase 4: Standard Library (Weeks 7-8)
- [ ] Complete Object class
- [ ] Integer class with arithmetic
- [ ] String class with operations
- [ ] Array class with iteration
- [ ] **Test**: Collection operations and string manipulation

### Phase 5: System Integration (Weeks 9-10)
- [ ] File loading from SD card
- [ ] VGA display integration
- [ ] Keyboard input handling
- [ ] Error reporting and debugging
- [ ] **Test**: Interactive REPL and file execution

### Phase 6: Optimization (Weeks 11-12)
- [ ] Method caching improvements
- [ ] Bytecode optimization
- [ ] Memory layout optimization
- [ ] Performance profiling
- [ ] **Test**: Benchmark suite and performance validation

---

## 10. Example Programs

### 10.1 Hello World
```
Hello = Object (
    run = (
        'Hello, EZOM World!' println.
    )
)
```

### 10.2 Simple Calculator
```
Calculator = Object (
    add: a to: b = ( ^a + b )
    
    run = (
        | result |
        result := self add: 5 to: 3.
        ('5 + 3 = ', result asString) println.
    )
)
```

### 10.3 Counter Class
```
Counter = Object (
    | value |
    
    initialize = (
        value := 0.
        ^self
    )
    
    increment = (
        value := value + 1.
        ^value
    )
    
    value = ( ^value )
    
    ----
    
    new = (
        ^super new initialize
    )
)
```

---

## 11. Conclusion

EZOM provides a complete, yet minimal object-oriented programming environment for the ez80 Agon Light 2. The step-by-step implementation approach ensures that each component can be thoroughly tested before proceeding to the next phase, making it an excellent platform for learning virtual machine implementation while creating a practical programming language for retro computing enthusiasts.

The design balances simplicity with functionality, providing enough features to create interesting programs while remaining comprehensible and maintainable. The performance targets ensure that EZOM will run efficiently within the constraints of the ez80 platform while offering a smooth development experience.

