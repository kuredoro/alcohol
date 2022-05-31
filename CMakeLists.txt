cmake_minimum_required(VERSION 3.20)
project(alcohol VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(CTest)
include(components)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(COMPONENT_DEFAULT_COMPILE_FEATURES cxx_std_20)
set(COMPONENT_SOURCE_ROOT src)

set(BUILD_PLUGIN ON CACHE BOOL "Try to find LLVM and build clang plugin")

##
## Defining libraries
##

add_component(util_for_each_argument src/alc/util/for_each_argument)
target_link_libraries(util_for_each_argument INTERFACE CONAN_PKG::boost)

add_component(util_unique_from_ref src/alc/util/make_unique_from_ref)

add_component(util_indent_text src/alc/util/indent_text)

add_component(ast_manager src/alc/ast/manager)

add_component(ast_expressions src/alc/ast/expressions)
target_link_libraries(ast_expressions PUBLIC
    CONAN_PKG::boost
    ast_manager
)

add_component(ast_replace_var src/alc/ast/replace_var)
target_link_libraries(ast_replace_var PUBLIC
    CONAN_PKG::boost
    ast_manager
    ast_expressions
)

add_component(ast_has_var src/alc/ast/has_var)
target_link_libraries(ast_has_var PUBLIC
    CONAN_PKG::boost
    ast_manager
    ast_expressions
)

add_component(ast_collect_vars src/alc/ast/collect_vars)
target_link_libraries(ast_collect_vars PUBLIC
    CONAN_PKG::gsl-lite
    ast_expressions
)

add_component(ast_simplify_arithmetic src/alc/ast/simplify_arithmetic)
target_link_libraries(ast_simplify_arithmetic PUBLIC
    CONAN_PKG::boost
    ast_manager
    ast_expressions
)

add_component(ast_statements src/alc/ast/statements)
target_link_libraries(ast_statements PUBLIC
    CONAN_PKG::gsl-lite
    util_for_each_argument
    util_indent_text
    ast_manager
    ast_expressions
)

add_component(constraint_set src/alc/constraint/set)
target_link_libraries(constraint_set PUBLIC
    CONAN_PKG::gsl-lite
    CONAN_PKG::z3
    ast_expressions
)

add_component(address_expr_collector src/alc/linter/address_expr_collector)
target_link_libraries(address_expr_collector PUBLIC
    CONAN_PKG::gsl-lite
    ast_manager
    ast_statements
    ast_expressions
    ast_collect_vars
)

add_component(linter_configuration src/alc/linter/configuration)
target_link_libraries(linter_configuration PUBLIC
    CONAN_PKG::spdlog
    ast_manager
    ast_statements
    ast_has_var
    ast_replace_var
    constraint_set
)

add_component(linter src/alc/linter/linter)
target_link_libraries(linter PUBLIC
    CONAN_PKG::spdlog
    ast_manager
    ast_statements
    ast_expressions
    ast_replace_var
    ast_simplify_arithmetic
    address_expr_collector
    linter_configuration
)

## 
## LLVM setup
##

if (BUILD_PLUGIN)
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
        src/alc/plugin/AlcoholPluginAction.hpp
        src/alc/plugin/AlcoholPluginAction.cpp
        src/alc/plugin/AftFuncDeclConsumer.hpp
        src/alc/plugin/AftFuncDeclConsumer.cpp
        src/alc/plugin/Main.cpp
    )
    
    target_link_libraries(alcohol PUBLIC
        ast_manager
        ast_statements
        linter

        # Allow undefined symbols in shared objects on Darwin (this is the default
        # behaviour on Linux)
        "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>"
    )
endif()
