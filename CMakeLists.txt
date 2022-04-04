cmake_minimum_required(VERSION 3.20)
project(alcohol VERSION 0.1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(components)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(COMPONENT_DEFAULT_COMPILE_FEATURES cxx_std_20)
set(COMPONENT_SOURCE_ROOT src)

##
## Defining libraries
##

add_component(util_for_each_argument
    HEADER src/util/for_each_argument.hpp)
target_link_libraries(util_for_each_argument INTERFACE CONAN_PKG::boost)

add_component(util_unique_from_ref
    HEADER src/util/make_unique_from_ref.hpp)

add_component(util_indent_text
    HEADER src/util/indent_text.hpp
    IMPL src/util/indent_text.cpp)

add_component(ast_manager
    HEADER src/ast/manager.hpp)

add_component(ast_expressions
    HEADER src/ast/expressions.hpp
    IMPL src/ast/expressions.cpp
    TEST src/ast/expressions.test.cpp)
target_link_libraries(ast_expressions PUBLIC ast_manager)

add_component(ast_statements
    HEADER src/ast/statements.hpp
    IMPL src/ast/statements.cpp
    TEST src/ast/statements.test.cpp)
target_link_libraries(ast_statements PUBLIC CONAN_PKG::gsl-lite util_for_each_argument util_indent_text ast_manager ast_expressions)

add_component(address_expr_collector
    HEADER src/linter/address_expr_collector.hpp
    IMPL src/linter/address_expr_collector.cpp)
target_link_libraries(address_expr_collector PUBLIC CONAN_PKG::gsl-lite ast_manager ast_statements ast_expressions)

add_component(linter
    HEADER src/linter/linter.hpp
    IMPL src/linter/linter.cpp)
target_link_libraries(linter PUBLIC ast_manager ast_statements ast_expressions address_expr_collector)

## 
## LLVM setup
##

# LLVM_DIR must be set to the prefix of /share/llvm/cmake via commandline
if(LLVM_HINT)
    find_package(LLVM REQUIRED CONFIG HINTS "${LLVM_HINT}")
else()
    message("\n-DLLVM_HINT not set. Will use system-wide installation. Set LLVM_HINT where /share/llvm/cmake can be accessed, if you want a specific version.\n")
    find_package(LLVM REQUIRED CONFIG)
endif()
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# We incorporate the CMake features provided by LLVM:
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# LLVM_DIR must be set to the prefix of /share/llvm/cmake via commandline
find_package(Clang CONFIG REQUIRED)

message("LLVM STATUS:
  Definitions ${LLVM_DEFINITIONS}
  Includes    ${LLVM_INCLUDE_DIRS}
              ${CLANG_INCLUDE_DIRS}
  Libraries   ${LLVM_LIBRARY_DIRS}"
)

# Now set the LLVM header and library paths:
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

############### alcohol setup ###############

# Use the same C++ standard as LLVM does
set(CMAKE_CXX_STANDARD 14 CACHE STRING "")

# LLVM is normally built without RTTI. Be consistent with that.
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

add_library(alcohol SHARED
    src/plugin/AlcoholPluginAction.hpp
    src/plugin/AlcoholPluginAction.cpp
    src/plugin/Main.cpp
)

# Allow undefined symbols in shared objects on Darwin (this is the default
# behaviour on Linux)
target_link_libraries(alcohol
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
