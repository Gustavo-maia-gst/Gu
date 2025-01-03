# Defina a versão mínima do CMake necessária
cmake_minimum_required(VERSION 3.10)

# Defina o nome do projeto
project(gu)

# Defina as opções de compilação
set(CMAKE_CXX_STANDARD 17)  # Defina a versão do C++ (se necessário)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")  # Opções de warnings
set(CMAKE_CXX_FLAGS_DEBUG "-g")  # Flags de debug
set(CMAKE_CXX_FLAGS_RELEASE "-O0")  # Flags de otimização (ajuste conforme necessário)

# Defina a variável do alvo
set(TARGET gu)

# Adicione o diretório de arquivos fonte
set(SOURCES
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/ast/ast.cpp
    src/parser/parser.cpp
    src/semantic/validator.cpp
    src/codegen/translators/gu2c.cpp
)

# Criação do diretório de saída
set(BUILD_DIR ${CMAKE_BINARY_DIR}/build)
set(OBJ_DIR ${BUILD_DIR}/obj)
set(BIN_DIR ${BUILD_DIR}/bin)

# Defina os diretórios de objetos e binários
file(MAKE_DIRECTORY ${OBJ_DIR})
file(MAKE_DIRECTORY ${BIN_DIR})

# Crie o executável
add_executable(${TARGET} ${SOURCES})

# Defina os diretórios de saída
set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OBJ_DIR}
)

# Incluir o diretório de objetos para os arquivos de objetos (como se fosse a compilação separada)
target_include_directories(${TARGET} PRIVATE ${OBJ_DIR})

# Caso precise adicionar algum comportamento extra no build, como copiar arquivos ou algo assim, você pode usar CMake e commands adicionais.
