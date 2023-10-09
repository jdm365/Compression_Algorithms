## Makefile
PROJECT_NAME = $(shell basename $(CURDIR))

CXX = clang++
CXXFLAGS = -Wall -g -std=c++2a -O3 -march=native -mtune=native -stdlib=libc++
INCLUDES = -I./include
## LDFLAGS = -fopenmp
SRCS = src/*.cpp
BIN = ./bin/release/$(PROJECT_NAME)


all:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(BIN)

run: all
	$(BIN)

