## Makefile
PROJECT_NAME = $(shell basename "$(realpath ./)")

CXX = gcc
## CXX = clangcc
CXXFLAGS = -Wall -g -O3 -Wno-unused-variable -march=native -mtune=native

OS = $(shell uname -s)
ifeq ($(OS), Darwin)
	LDFLAGS = -Xpreprocessor -fopenmp -lomp
else
	LDFLAGS = -fopenmp
endif

ifeq ($(CXX), clang++)
	CXXFLAGS += -stdlib=libc++
endif

SRCS = *.c
INCLUDES = -I./
TARGET = bin/release/$(PROJECT_NAME)

## Speed up compile times
CCFLAGS += -MMD -MP

all:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CCFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)

run: all
	./$(TARGET)

sanitize:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CCFLAGS) $(SRCS) $(LDFLAGS) -fsanitize=address -o $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
