CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude
SRC      = $(wildcard src/*.c)

test: build/test_all
	./build/test_all

# AddressSanitizer + UBSan catch buffer overruns and UB the plain build would hide
sanitize:
	$(CC) $(CFLAGS) -fsanitize=address,undefined -g $(SRC) tests/test_all.c -o build/test_san
	./build/test_san

build/test_all: $(SRC) tests/test_all.c | build
	$(CC) $(CFLAGS) $^ -o $@

build:
	mkdir -p build

clean:
	rm -rf build
