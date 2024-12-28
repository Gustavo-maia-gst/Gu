TARGET  = gu
CC      = g++
DEBUG   = -g
OPT     = -O0
WARN    = -Wall
CCFLAGS = $(DEBUG) $(OPT) $(WARN)

FILES = src/main.cpp \
        src/lexicalScanner/LexicalScanner.cpp \
        src/astParser/AstParser.cpp \
		src/astParser/types.cpp

BUILD_DIR = build
OBJS = $(FILES:.cpp=.o)

all: $(BUILD_DIR) | $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) -o build/$(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

%.o: %.cpp
	$(CC) $(CCFLAGS) -c $< -o build/$(@F)

clean:
	echo $(OBJS)
	rm build/*.o
