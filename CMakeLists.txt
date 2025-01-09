cmake_minimum_required(VERSION 3.10)

project(gu)
set(TARGET gu)

set(CMAKE_CXX_STANDARD 17)

# Flags de compilação (Debug e Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Encontrar o LLVM (configuração automática)
find_package(LLVM REQUIRED CONFIG)

# Incluir diretórios do LLVM
include_directories(${LLVM_INCLUDE_DIRS})

# Definir as definições do LLVM
add_definitions(${LLVM_DEFINITIONS})

# Fontes do projeto
set(SOURCES
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/ast/ast.cpp
    src/parser/parser.cpp
    src/semantic/validator.cpp
    src/codegen/translators/gu2c.cpp
)

set(CMAKE_STATIC_LIBRARIES TRUE)
set(CMAKE_EXE_LINKER_FLAGS "-static")

# Diretórios de build
set(OBJ_DIR ${CMAKE_BINARY_DIR}/obj)
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)

# Criar diretórios
file(MAKE_DIRECTORY ${OBJ_DIR} ${BIN_DIR})

# Adicionar o executável
add_executable(${TARGET} ${SOURCES})

# Definir propriedades do alvo
set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OBJ_DIR}
)

# Linkar com as bibliotecas do LLVM e componentes necessários
target_link_libraries(${TARGET} ${LLVM_LIBRARIES})

