TARGET  = gu
CC      = g++
DEBUG   = -g
OPT     = -O0
WARN    = -Wall -fexceptions

CXXFLAGS = $(DEBUG) $(OPT) $(WARN)
LLVM_CXXFLAGS := $(shell llvm-config --cxxflags)
LLVM_LDFLAGS  := $(shell llvm-config --ldflags)
LLVM_LIBS     := $(shell llvm-config --libs --system-libs)

FILES = src/main.cpp \
        src/lexer/lexer.cpp \
		src/parser/ast/ast.cpp \
        src/parser/parser.cpp \
		src/semantic/validator.cpp \
		src/codegen/translators/gu2c.cpp \
		src/codegen/llvm/IRGenerator.cpp

all: build/debug | $(FILES)
	rm -rf build/debug/obj/*

	for file in $(FILES); do \
		filename=$${file%.*}; \
		filename=$${filename//\//.}; \
		$(CC) -c $(CXXFLAGS) $(LLVM_CXXFLAGS) $$file -o build/debug/obj/"$$filename".o; \
	done

	$(CC) $(CCFLAGS) $(LLVM_LDFLAGS) build/debug/obj/*.o -o build/debug/bin/$(TARGET) $(LLVM_LIBS)

build/debug:
	mkdir -p "build/debug/obj"
	mkdir -p "build/debug/bin"

clean:
	rm -rf build/debug
