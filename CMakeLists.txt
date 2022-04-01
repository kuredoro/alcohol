cmake_minimum_required(VERSION 3.20)
project(alcohol VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

##
## Defining libraries
##

add_library(util STATIC
    src/util/for_each_argument.hpp
    src/util/make_unique_from_ref.hpp
    src/util/indent_text.hpp
    src/util/indent_text.cpp)
target_compile_features(util PUBLIC cxx_std_17)
target_include_directories(util PUBLIC src)
target_link_libraries(util PUBLIC CONAN_PKG::boost)

add_library(ast STATIC
    src/ast/statements.hpp
    src/ast/statements.cpp
    src/ast/expressions.hpp
    src/ast/expressions.cpp)
target_compile_features(ast PUBLIC cxx_std_17)
target_include_directories(ast PUBLIC src)
target_link_libraries(ast PUBLIC util CONAN_PKG::gsl-lite CONAN_PKG::boost)

add_library(address_expr_collector STATIC
    src/linter/address_expr_collector.hpp
    src/linter/address_expr_collector.cpp)
target_compile_features(address_expr_collector PUBLIC cxx_std_20)
target_include_directories(address_expr_collector PUBLIC src)
target_link_libraries(address_expr_collector PUBLIC util ast CONAN_PKG::gsl-lite CONAN_PKG::boost)

add_library(linter STATIC
    src/linter/linter.hpp
    src/linter/linter.cpp)
target_compile_features(linter PUBLIC cxx_std_20)
target_include_directories(linter PUBLIC src)
target_link_libraries(linter PUBLIC util ast address_expr_collector CONAN_PKG::gsl-lite CONAN_PKG::boost)

##
## Tests
##

add_executable(statements_test
    src/boost/ut.hpp
    src/ast/statements.test.cpp)
target_include_directories(statements_test PRIVATE src/)
target_link_libraries(statements_test PRIVATE ast)

add_executable(expressions_test
    src/boost/ut.hpp
    src/ast/expressions.test.cpp)
target_include_directories(expressions_test PRIVATE src/)
target_link_libraries(expressions_test PRIVATE ast)

add_executable(address_expr_collector_test
    src/boost/ut.hpp
    src/linter/address_expr_collector.test.cpp)
target_include_directories(address_expr_collector_test PRIVATE src/)
target_link_libraries(address_expr_collector_test PRIVATE address_expr_collector)

add_executable(linter_test
    src/boost/ut.hpp
    src/linter/linter.test.cpp)
target_include_directories(linter_test PRIVATE src/)
target_link_libraries(linter_test PRIVATE linter)

# MSVC is bogus with modules right now...
if (MSVC)
    target_compile_definitions(statements_test PRIVATE BOOST_UT_DISABLE_MODULE)
    target_compile_definitions(expression_test PRIVATE BOOST_UT_DISABLE_MODULE)
endif()

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
