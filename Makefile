TARGET  = gu
CC      = g++
DEBUG   = -g
OPT     = -O0
WARN    = -Wall
CCFLAGS = $(DEBUG) $(OPT) $(WARN)

FILES = src/main.cpp \
        src/lexer/lexer.cpp \
		src/parser/ast/ast.cpp \
        src/parser/parser.cpp \
		src/semantic/validator.cpp

all: build | $(FILES)
	rm -rf build/obj/*

	for file in $(FILES); do \
		filename=$${file%.*}; \
		filename=$${filename//\//.}; \
		$(CC) -c $(CCFLAGS) $$file -o build/obj/"$$filename".o; \
	done

	$(CC) $(CCFLAGS) build/obj/*.o -o build/bin/$(TARGET)

build:
	mkdir -p "build/obj"
	mkdir -p "build/bin"

clean:
	rm -rf build
