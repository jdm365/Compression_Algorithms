## Makefile
PROJECT_NAME = $(shell basename "$(realpath ./)")

## CXX = g++
CXX = clang++
CXXFLAGS = -std=c++11 -Wall -g -O3 -Wno-unused-result -march=native
## CXXFLAGS = -Wall -g -O3
MEMFLAGS = -fsanitize=address -fno-omit-frame-pointer
OS = $(shell uname -s)
ifeq ($(OS), Darwin)
	LDFLAGS = -Xpreprocessor -fopenmp -lomp
else
	LDFLAGS = -fopenmp
endif

ifeq ($(CXX), clang++)
	CXXFLAGS += -stdlib=libc++
endif

SRCS = src/*
INCLUDES = -Iinclude
TARGET = bin/release/$(PROJECT_NAME)

## Speed up compile times
CCFLAGS += -MMD -MP

all:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CCFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)

sanitize:
	$(CXX) $(CXXFLAGS) $(MEMFLAGS) $(INCLUDES) $(CCFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)
	./$(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
