cmake_minimum_required(VERSION 3.25.1)

project(
    cxx-init
    VERSION 1.0.0
    DESCRIPTION "cxx-init"
    HOMEPAGE_URL "https://github.com/vsytch/cxx-init"
    LANGUAGES CXX
)

find_package(fmt REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE fmt::fmt magic_enum::magic_enum range-v3)
