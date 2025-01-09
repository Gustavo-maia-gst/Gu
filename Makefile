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
		src/semantic/validator.cpp \
		src/codegen/translators/gu2c.cpp

all: build/debug | $(FILES)
	rm -rf build/debug/obj/*

	for file in $(FILES); do \
		filename=$${file%.*}; \
		filename=$${filename//\//.}; \
		$(CC) -c $(CCFLAGS) $$file -o build/debug/obj/"$$filename".o; \
	done

	$(CC) $(CCFLAGS) build/debug/obj/*.o -o build/debug/bin/$(TARGET)

build/debug:
	mkdir -p "build/debug/obj"
	mkdir -p "build/debug/bin"

clean:
	rm -rf build/debug
