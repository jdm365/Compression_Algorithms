## Makefile
PROJECT_NAME = $(shell basename "$(realpath ./)")

## CC = gcc
CC = clang
CFLAGS = -Wall -g -O3 -Wno-unused-result -march=native

MEMFLAGS = -fsanitize=address -fno-omit-frame-pointer
OS = $(shell uname -s)
ifeq ($(OS), Darwin)
	LDFLAGS = -Xpreprocessor -fopenmp -lomp
else
	LDFLAGS = -fopenmp
endif

SRCS = ./*.c
INCLUDES = -I./
TARGET = bin/release/$(PROJECT_NAME)

## Speed up compile times
CCFLAGS += -MMD -MP

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(CCFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)

sanitize:
	$(CC) $(CFLAGS) $(MEMFLAGS) $(INCLUDES) $(CCFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)
	./$(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
