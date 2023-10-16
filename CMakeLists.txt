cmake_minimum_required(VERSION 3.28)

project(
    cxx-init
    VERSION 1.0.0
    DESCRIPTION "cxx-init"
    HOMEPAGE_URL "https://github.com/vsytch/cxx-init"
    LANGUAGES CXX
)

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

add_executable(main)
target_sources(main
    PRIVATE
        src/main.cpp
    PRIVATE
        FILE_SET CXX_MODULES
            BASE_DIRS
                "${CMAKE_CURRENT_SOURCE_DIR}"
            FILES
                src/Ast.cxxm
                src/Environment.cxxm
                src/Interpreter.cxxm
                src/Lox.cxxm
                src/Object.cxxm
                src/Parser.cxxm
                src/RuntimeError.cxxm
                src/Scanner.cxxm
                src/TokenType.cxxm)
target_include_directories(main PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(main PRIVATE ${Boost_LIBRARIES} fmt::fmt magic_enum::magic_enum range-v3)
