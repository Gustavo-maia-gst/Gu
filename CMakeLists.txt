cmake_minimum_required(VERSION 3.10)

project(gu)
set(TARGET gu)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

set(SOURCES
    src/main.cpp
    src/lexer/lexer.cpp
    src/ast/ast.cpp
    src/parser/parser.cpp
    src/semantic/validator.cpp
    src/semantic/libcDefiner.cpp
    src/codegen/translators/gu2c.cpp
    src/codegen/llvm/assembler.cpp
)

#set(CMAKE_EXE_LINKER_FLAGS "-static")

set(OBJ_DIR ${CMAKE_BINARY_DIR}/obj)
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)

file(MAKE_DIRECTORY ${OBJ_DIR} ${BIN_DIR})

add_executable(${TARGET} ${SOURCES})

llvm_map_components_to_libnames(LLVM_LIBS support core irreader)
target_link_libraries(${TARGET} ${LLVM_LIBS})

set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

