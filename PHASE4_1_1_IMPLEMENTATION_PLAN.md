# Phase 4.1.1 Class Definition Parser Implementation Plan

## Overview
Complete the class definition parsing and compilation system to enable full SOM class definitions from .som files.

## Current Status Analysis

### ✅ COMPLETED Infrastructure
- **Lexer**: Full tokenization of SOM syntax including class keywords
- **Parser Framework**: Basic structure for class definition parsing
- **AST System**: Complete AST nodes for class definitions, methods, variables
- **Evaluator**: Basic class evaluation with inheritance support
- **File Loading**: Integration with parsing pipeline

### 🔄 PARTIALLY IMPLEMENTED
- **Class Definition Parsing**: Basic structure exists, needs completion
- **Method Compilation**: Partial implementation, needs method body compilation
- **Instance Variable System**: Basic support, needs integration
- **Assignment Parsing**: Basic framework, needs full implementation

### ❌ MISSING CRITICAL COMPONENTS
1. **Method Body Compilation**: Converting method AST to executable code
2. **Instance Variable Access**: Runtime support for instance variable access
3. **Assignment Statement Processing**: Full `:=` operator support
4. **Method Installation**: Proper method storage in class dictionaries
5. **Class Hierarchy Creation**: Runtime class object creation from AST

## Implementation Tasks

### Task 1: Complete Method Body Compilation
**Priority: CRITICAL**
**Files: `src/evaluator.c`, `src/parser.c`**

#### Subtasks:
1. **Method AST to Executable Code**
   - Convert method body AST to executable primitives
   - Handle local variable declarations
   - Support method parameters
   - Implement return statements

2. **Local Variable Scoping**
   - Create proper variable binding contexts
   - Handle method parameter access
   - Support local variable assignments

3. **Method Execution Context**
   - Create method execution contexts
   - Handle `self` and `super` references
   - Support method return values

### Task 2: Instance Variable System Integration
**Priority: CRITICAL**
**Files: `src/evaluator.c`, `src/objects.c`**

#### Subtasks:
1. **Instance Variable Access**
   - Implement runtime instance variable lookup
   - Support instance variable assignment
   - Handle inheritance of instance variables

2. **Object Creation with Instance Variables**
   - Modify object creation to allocate instance variable space
   - Initialize instance variables properly
   - Support inherited instance variables

3. **Instance Variable Indexing**
   - Create mapping from variable names to indices
   - Support fast instance variable access
   - Handle variable inheritance chains

### Task 3: Assignment Statement Processing
**Priority: HIGH**
**Files: `src/parser.c`, `src/evaluator.c`**

#### Subtasks:
1. **Assignment Parser Enhancement**
   - Parse `:=` operator correctly
   - Handle variable reference resolution
   - Support nested assignments

2. **Assignment Evaluation**
   - Evaluate assignment expressions
   - Handle instance variable assignments
   - Support local variable assignments

3. **Variable Context Resolution**
   - Resolve variable types (instance, local, parameter)
   - Handle variable scope correctly
   - Support proper variable binding

### Task 4: Method Installation Pipeline
**Priority: HIGH**
**Files: `src/evaluator.c`, `src/dispatch.c`**

#### Subtasks:
1. **Method Dictionary Population**
   - Install methods from AST into class dictionaries
   - Handle method selector creation
   - Support method overriding

2. **Method Compilation and Storage**
   - Compile method bodies to executable form
   - Store method metadata (parameters, locals)
   - Handle primitive method tagging

3. **Method Lookup Enhancement**
   - Ensure method lookup works with compiled methods
   - Support inheritance-based method resolution
   - Handle super method calls

### Task 5: Class Hierarchy Creation
**Priority: HIGH**
**Files: `src/evaluator.c`, `src/objects.c`**

#### Subtasks:
1. **Class Object Creation**
   - Create class objects from AST definitions
   - Set up inheritance relationships
   - Initialize class metadata

2. **Global Class Registration**
   - Register classes in global namespace
   - Support class name resolution
   - Handle class loading order

3. **Class Method Installation**
   - Install class methods (after `----` separator)
   - Create metaclass relationships
   - Support class method dispatch

## Implementation Strategy

### Phase 1: Core Method Compilation (Week 1)
1. Complete method body compilation
2. Implement local variable scoping
3. Test with simple method definitions

### Phase 2: Instance Variable System (Week 1-2)
1. Implement instance variable access
2. Integrate with object creation
3. Test with classes containing instance variables

### Phase 3: Assignment Processing (Week 2)
1. Complete assignment statement parsing
2. Implement assignment evaluation
3. Test with assignment-heavy code

### Phase 4: Method Installation (Week 2-3)
1. Complete method installation pipeline
2. Integrate with class definition evaluation
3. Test with complex class hierarchies

### Phase 5: Integration and Testing (Week 3)
1. Integrate all components
2. Test with comprehensive .som files
3. Validate against test suite

## Success Criteria

### Functional Tests
- [ ] Parse and compile Counter.som successfully
- [ ] Create Counter instances with working increment method
- [ ] Handle instance variable access and modification
- [ ] Support class inheritance (Animal -> Dog -> Puppy)
- [ ] Execute method calls with parameters
- [ ] Handle assignment statements correctly

### Integration Tests
- [ ] Load and execute all test .som files
- [ ] Class hierarchy creation works correctly
- [ ] Method dispatch works with compiled methods
- [ ] Instance variable inheritance works
- [ ] Assignment operations work properly

### Performance Tests
- [ ] Class definition parsing completes quickly
- [ ] Method compilation is efficient
- [ ] Instance variable access is fast
- [ ] Memory usage is reasonable

## Current Implementation Gaps

### Missing Functions
1. `ezom_compile_method_body()` - Convert method AST to executable code
2. `ezom_install_methods_from_ast()` - Install methods from AST
3. `ezom_create_class_with_inheritance()` - Create class with proper inheritance
4. `ezom_evaluate_instance_variable_access()` - Instance variable access
5. `ezom_evaluate_assignment_complete()` - Full assignment evaluation

### Missing Data Structures
1. Method compilation context
2. Instance variable mapping tables
3. Class creation metadata
4. Method execution contexts

## Testing Strategy

### Unit Tests
- Method compilation accuracy
- Instance variable access correctness
- Assignment statement processing
- Class hierarchy creation

### Integration Tests
- Complete .som file execution
- Complex class hierarchies
- Method inheritance and overriding
- Instance variable inheritance

### Regression Tests
- Existing functionality preservation
- Performance benchmarks
- Memory usage validation

## Risk Mitigation

### Technical Risks
1. **Method compilation complexity** - Start with simple methods, gradually add complexity
2. **Instance variable performance** - Implement efficient indexing system
3. **Assignment parsing edge cases** - Comprehensive test coverage
4. **Class hierarchy bugs** - Systematic testing of inheritance

### Integration Risks
1. **Bootstrap system conflicts** - Careful integration with existing bootstrap
2. **Memory management issues** - Thorough memory leak testing
3. **Primitive system conflicts** - Maintain compatibility with existing primitives

## Next Steps

1. **Immediate**: Start implementing `ezom_compile_method_body()`
2. **Short-term**: Complete instance variable system integration
3. **Medium-term**: Implement assignment statement processing
4. **Long-term**: Complete integration and comprehensive testing

This plan provides a structured approach to completing Phase 4.1.1 Class Definition Parser implementation.
