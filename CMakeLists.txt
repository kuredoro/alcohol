cmake_minimum_required(VERSION 3.20)
project(alcohol VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

##
## Defining libraries
##

add_library(linter
    src/ast.hpp
    src/ast.cpp
    src/expression.hpp
    src/util.hpp
    src/util.cpp)
target_compile_features(linter PUBLIC cxx_std_20)
target_include_directories(linter PUBLIC src/)
target_link_libraries(linter PUBLIC CONAN_PKG::gsl-lite CONAN_PKG::boost)

##
## Tests
##

add_executable(linter_test
    src/boost/ut.hpp
    src/linter.test.cpp)
target_include_directories(linter_test PRIVATE src/)
target_link_libraries(linter_test PRIVATE linter)

# MSVC is bogus with modules right now...
if (MSVC)
    target_compile_definitions(linter_test PRIVATE BOOST_UT_DISABLE_MODULE)
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
    src/AlcoholPluginAction.hpp
    src/AlcoholPluginAction.cpp
    src/Main.cpp
)

# Allow undefined symbols in shared objects on Darwin (this is the default
# behaviour on Linux)
target_link_libraries(alcohol
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
