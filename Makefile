CC ?= cc
AR ?= ar
RANLIB ?= ranlib
CPPFLAGS ?= -Iinclude -Isource/tai-saver
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
SANITIZER_FLAGS ?= -fsanitize=address,undefined -fno-omit-frame-pointer

CORE_SOURCES := source/tai-saver/tai_saver.c source/tai-saver/sha256.c
CORE_OBJECTS := build/tai_saver.o build/sha256.o
LIBRARY := build/libtai_saver.a
TEST_BINARY := build/test_tai_saver
SANITIZER_BINARY := build/test_tai_saver_san

.PHONY: all test sanitize clean
all: $(LIBRARY)

build:
	mkdir -p $@

build/tai_saver.o: source/tai-saver/tai_saver.c include/tai_saver.h source/tai-saver/sha256.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/sha256.o: source/tai-saver/sha256.c source/tai-saver/sha256.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(CORE_OBJECTS)
	$(AR) rcs $@ $(CORE_OBJECTS)
	$(RANLIB) $@

$(TEST_BINARY): tests/test_tai_saver.c $(LIBRARY) include/tai_saver.h source/tai-saver/sha256.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIBRARY) -o $@ $(LDFLAGS)

test: $(TEST_BINARY)
	./$(TEST_BINARY)

$(SANITIZER_BINARY): $(CORE_SOURCES) tests/test_tai_saver.c include/tai_saver.h source/tai-saver/sha256.h | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Wpedantic $(SANITIZER_FLAGS) $(CORE_SOURCES) tests/test_tai_saver.c -o $@ $(LDFLAGS)

sanitize: $(SANITIZER_BINARY)
	ASAN_OPTIONS=detect_leaks=1 ./$(SANITIZER_BINARY)

clean:
	rm -rf build
