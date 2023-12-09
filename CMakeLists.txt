cmake_minimum_required(VERSION 3.10)

project(Fallacy)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_options(-O3 -DUNICODE)

set(CMAKE_EXECUTABLE_SUFFIX .exe)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

include(CPack OPTIONAL)
set(CPACK_GENERATOR "STGZ;TGZ;TZST;ZIP")
set(CPACK_PACKAGE_VENDOR "xtex")
set(CPACK_RESOURCE_FILE_LICENSE "LICENSE.txt")

add_executable(Fallacy WIN32 Fallacy.cpp)
install(TARGETS Fallacy)

configure_file(FallacyConfig.h.in FallacyConfig.h)
target_include_directories(Fallacy PUBLIC "${PROJECT_BINARY_DIR}")
