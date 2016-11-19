cmake_minimum_required(VERSION 3.5)
project(qsh)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    include/qsh/alloc/allocator.hpp
    include/qsh/alloc/arena.hpp
    include/qsh/parser/parse_tree.hpp
    include/qsh/parser/parser.hpp
    include/qsh/types/types.hpp
    include/qsh/util/noncopyable.hpp
    include/qsh/util/range.hpp
    include/qsh/util/shared_ptr.hpp
    include/qsh/util/stack_pimpl.hpp
    include/qsh/util/stack_storage.hpp
    include/qsh/config.h
    include/qsh/version.hpp
    src/libqsh/alloc/allocator.cpp
    src/libqsh/alloc/arena.cpp
    src/libqsh/parser/parser.cpp
    src/libqsh/types/types.cpp
    src/libqsh/version.cpp)

add_library(qsh SHARED ${SOURCE_FILES})
target_include_directories(qsh PRIVATE include PRIVATE src/libqsh)
target_compile_definitions(qsh PRIVATE BUILD_QSH)

function(add_test x)
    add_executable(${x} tests/${x}.cpp)
    target_link_libraries(${x} qsh)
endfunction(add_test)
include_directories(include)
add_test(Arena)
add_test(LibQsh)
add_test(Parser)
add_test(Types)
add_test(Utils)
