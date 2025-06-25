# Makefile for EZOM Phase 1
CC = ez80-clang
CFLAGS = -Wall -Oz -mllvm -profile-guided-section-prefix=false
INCLUDES = -Isrc/include
TARGET = ezom_phase1.bin
TEST_TARGET = test_phase1.bin

# Source files
VM_SOURCES = src/vm/main.c src/vm/memory.c src/vm/object.c src/vm/objects.c \
             src/vm/primitives.c src/vm/dispatch.c

TEST_SOURCES = test_phase1.c src/vm/memory.c src/vm/object.c src/vm/objects.c \
               src/vm/primitives.c src/vm/dispatch.c

ALL_OBJECTS = $(VM_SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Targets
all: $(TARGET)

$(TARGET): $(ALL_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Built EZOM Phase 1: $(TARGET)"
	@echo "Size: `ls -l $(TARGET) | awk '{print $$5}'` bytes"

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Built test suite: $(TEST_TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(ALL_OBJECTS) $(TEST_OBJECTS) $(TARGET) $(TEST_TARGET)
	@echo "Cleaned build files"

run: $(TARGET)
	@echo "Running EZOM Phase 1..."
	fab-agon-emulator $(TARGET)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# File dependencies
src/vm/main.o: src/include/ezom_memory.h src/include/ezom_object.h src/include/ezom_primitives.h src/include/ezom_dispatch.h
src/vm/memory.o: src/include/ezom_memory.h src/include/ezom_object.h
src/vm/object.o: src/include/ezom_object.h src/include/ezom_memory.h
src/vm/objects.o: src/include/ezom_object.h src/include/ezom_memory.h
src/vm/primitives.o: src/include/ezom_primitives.h src/include/ezom_object.h
src/vm/dispatch.o: src/include/ezom_dispatch.h src/include/ezom_object.h src/include/ezom_primitives.h
test_phase1.o: src/include/ezom_memory.h src/include/ezom_object.h src/include/ezom_primitives.h src/include/ezom_dispatch.h

.PHONY: all clean run debug test