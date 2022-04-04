# This will be local for each add_subdirectory and reverted as we exit it.
# We can use this behavior to check if a component inside a project was or
# was not yet created.
set(tests_created_for_this_project FALSE)

set(COMPONENT_SOURCE_ROOT ${CMAKE_SOURCE_DIR})

# add_component creates a new component consisting of a single header file
# and a single test driver with an optional single implementation file as well.
# As a result, the function will create two targets for the component itself
# and the test driver prefixed with the name of the project.
#
# Additionally, as soon as first component is defined, a test aggregating
# target is created, that will execute tests for all of the project's
# components using CTest.
#
# Syntax:
# add_component(component_name HEADER header_file [TEST test_file] [IMPL implementation_file])
function(add_component name)
    set(options)
    set(one_value_keywords HEADER IMPL TEST)
    set(multi_value_keywords)
    cmake_parse_arguments(PARSE_ARGV 1 file "${options}" "${one_value_keywords}" "${multi_value_keywords}")

    if(NOT DEFINED file_HEADER)
        message(FATAL "add_component: HEADER argument missing. add_component should always be called with the HEADER and TEST arguments. Example: add_component(foo HEADER foo.h IMPL foo.cpp TEST foo.test.cpp)")
    endif()

    add_library(${name} STATIC ${file_HEADER} ${file_IMPL})
    target_compile_features(${name} PUBLIC ${COMPONENT_DEFAULT_COMPILE_FEATURES})
    target_include_directories(${name} PUBLIC ${COMPONENT_SOURCE_ROOT})

    if(DEFINED file_TEST)
        add_executable(${name}_test ${file_TEST} src/boost/ut.hpp)
        target_link_libraries(${name}_test PRIVATE ${name})

        if (MSVC)
            target_compile_definitions(${name}_test PRIVATE BOOST_UT_DISABLE_MODULE)
        endif()

        add_test(NAME ${name} COMMAND ${name}-test)

        # Create a run-all target for tests, when creating the first test for the current project.
        # ctest should never compile the tests themselves, because we lose the progress feedback
        # from the IDEs and bulid systems. Instead we add CMake target dependencies to this
        # custom target instead.
        if(NOT ${tests_created_for_this_project})
            set(tests_created_for_this_project TRUE PARENT_SCOPE)
            add_custom_target("${PROJECT_NAME}_tests"
                ctest --output-on-failure -R "${PROJECT_NAME}" --test-dir ${CMAKE_BINARY_DIR} -C "$<CONFIG>"
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
        endif()

        add_dependencies(${PROJECT_NAME}_tests ${name}_test)
    endif()
endfunction()
