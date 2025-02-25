## Makefile
PROJECT_NAME = $(shell basename "$(realpath ./)")

SHELL := /bin/bash

CXX = gcc
## CXX = clang
CXXFLAGS = -Wall -g -O3 -march=native -Wno-unused-variable -Wno-unused-function -Wno-format
LDFLAGS = -lz

OS = $(shell uname -s)
ifeq ($(OS), Darwin)
	LDFLAGS += -Xpreprocessor -fopenmp -lomp
else
	LDFLAGS += -fopenmp
endif

ifeq ($(CXX), clang++)
	CXXFLAGS += -stdlib=libc++
endif

SRCS = ./*.c
INCLUDES = -I./
TARGET = bin/release/$(PROJECT_NAME)

MEMFLAGS = -fsanitize=address -fno-omit-frame-pointer

all:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) $(LDFLAGS) -o $(TARGET)

sanitize:
	$(CXX) $(CXXFLAGS) $(MEMFLAGS) $(INCLUDES) $(CCFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)
	./$(TARGET)

run: all
	time ./$(TARGET)


clean:
	rm -f $(TARGET)
