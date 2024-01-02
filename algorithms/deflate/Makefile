## Makefile
PROJECT_NAME = $(shell basename "$(realpath ./)")

## CXX = g++
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -g -O3 -Wno-unused-variable -Wno-unused-function
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

SRCS = src/*
INCLUDES = -Iinclude
TARGET = bin/release/$(PROJECT_NAME)

all:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) $(LDFLAGS) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
