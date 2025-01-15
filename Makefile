TARGET  = gu
CC      = g++
DEBUG   = -g
OPT     = -O0
WARN    = -Wall

CXXFLAGS = $(DEBUG) $(OPT) $(WARN)
LLVM_CXXFLAGS := $(shell llvm-config --cxxflags)
LLVM_LDFLAGS  := $(shell llvm-config --ldflags)
LLVM_LIBS     := $(shell llvm-config --libs --system-libs)

FILES = src/main/main.cpp \
		src/main/argHandler.cpp \
        src/lexer/lexer.cpp \
		src/ast/ast.cpp \
        src/parser/parser.cpp \
		src/semantic/validator.cpp \
		src/semantic/libcDefiner.cpp \
		src/semantic/importManager.cpp \
		src/codegen/translators/gu2c.cpp \
		src/codegen/llvm/assembler.cpp

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
