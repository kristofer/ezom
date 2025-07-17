# EZOM Test Program Suite

This directory contains a comprehensive set of SOM (Simple Object Machine) test programs designed to validate the file loading functionality and various VM features of the EZOM interpreter.

## Test Files Overview

### Basic Functionality Tests
- **`hello_world.som`** - Simple "Hello World" program
- **`basic_math.som`** - Arithmetic operations and comparisons
- **`string_test.som`** - String manipulation and operations
- **`array_test.som`** - Array creation, access, and modification

### Control Flow Tests
- **`loop_test.som`** - Loop constructs (timesRepeat:, to:do:, whileTrue:, whileFalse:)
- **`block_test.som`** - Block objects, parameters, and local variables

### Object-Oriented Tests
- **`counter.som`** - Simple class with instance variables
- **`calculator.som`** - Array operations and calculations
- **`inheritance_test.som`** - Class inheritance with Animal/Dog/Cat hierarchy

### Advanced Tests
- **`fibonacci.som`** - Recursive Fibonacci calculator
- **`error_test.som`** - Error handling and edge cases
- **`all_tests.som`** - Comprehensive test runner (requires all other test files)

### Test Infrastructure
- **`test_runner.som`** - File loading test runner
- **`README.md`** - This documentation file

## Usage

Each test file can be loaded and executed independently to test specific VM functionality:

```bash
# Example usage (depends on file loading implementation)
./ezom load hello_world.som
./ezom load basic_math.som
./ezom load loop_test.som
```

## Test Coverage

The test suite covers the following EZOM language features:

### Phase 3 Features (Object System)
- ✅ Object creation and initialization
- ✅ Method dispatch and message sending
- ✅ Class hierarchy and inheritance
- ✅ Instance variables and accessors

### Phase 4 Features (Advanced Language Constructs)
- ✅ Conditional statements (ifTrue:, ifFalse:, ifTrue:ifFalse:)
- ✅ Loop constructs (timesRepeat:, to:do:, whileTrue:, whileFalse:)
- ✅ Block objects with parameters and local variables
- ✅ String operations and concatenation
- ✅ Array operations (at:, at:put:, length)

### Primitive Operations
- ✅ Integer arithmetic (+, -, *, /, =, <, >)
- ✅ Boolean operations (ifTrue:, ifFalse:, not)
- ✅ String operations (length, concatenation)
- ✅ Array operations (creation, access, modification)
- ✅ Object operations (class, =, hash, println)

## Expected Behavior

Each test file should:
1. Load successfully without parse errors
2. Execute all methods without runtime errors
3. Produce meaningful output demonstrating the tested features
4. Return control to the VM properly

## File Structure

```
test_programs/
├── hello_world.som          # Simple greeting program
├── basic_math.som           # Arithmetic and comparison tests
├── string_test.som          # String manipulation tests
├── array_test.som           # Array operation tests
├── loop_test.som            # Loop construct tests
├── block_test.som           # Block functionality tests
├── counter.som              # Simple class with state
├── calculator.som           # Array-based calculations
├── inheritance_test.som     # Class inheritance tests
├── fibonacci.som            # Recursive algorithm test
├── error_test.som           # Error handling tests
├── all_tests.som            # Comprehensive test runner
├── test_runner.som          # File loading test runner
└── README.md               # This documentation
```

## Integration with File Loading

These test files are designed to work with the file loading functionality implemented in the EZOM VM. The file loading system should:

1. Parse SOM class definitions from `.som` files
2. Create appropriate class objects in the VM
3. Install methods in the class method dictionaries
4. Handle class inheritance properly
5. Support both instance and class methods (separated by `----`)

## Error Handling Tests

The `error_test.som` file contains intentional error conditions to test the VM's error handling:

- Division by zero
- Array index out of bounds
- Method calls on nil objects

These tests help ensure the VM handles error conditions gracefully rather than crashing.

## Performance Tests

The `fibonacci.som` file tests recursive method calls and can be used to evaluate VM performance for computationally intensive tasks.

## Development Notes

- All files follow SOM syntax conventions
- Comments use double quotes: `" This is a comment "`
- Class methods are separated from instance methods by `----`
- Files test features implemented in Phase 3 and Phase 4 of the EZOM VM
- Error tests are designed to fail gracefully, not crash the VM
