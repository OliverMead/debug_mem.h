cmake_minimum_required(VERSION 3.22)

project(debug_mem VERSION 0.1.3 DESCRIPTION "simple memory debugging library")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "$\{ORIGIN\}")

if (MSVC)
    add_compile_options(/W4 /D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -pedantic-errors -Wextra)
endif()

if (CMAKE_BUILD_TYPE=DEBUG)
    if (MSVC)
        add_compile_options()
    else()
        add_compile_options(-g -Og)
    endif()
endif()

add_library(debug_mem 
    src/debug_mem.c
    src/mem_table.c
)

set_target_properties(debug_mem PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(debug_mem PROPERTIES PUBLIC_HEADER include/debug_mem.h)

include_directories(PRIVATE include)

install(
    TARGETS debug_mem
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

enable_testing()

add_executable(       test7 test/test7_checksum_pointer_correct.c)
target_link_libraries(test7 PUBLIC debug_mem)
add_executable(       test6 test/test6_table_auto_shrink.c)
target_link_libraries(test6 PUBLIC debug_mem)
add_executable(       test5 test/test5_table_auto_expand.c)
target_link_libraries(test5 PUBLIC debug_mem)
add_executable(       test4 test/test4_free_clears_table_entry.c)
target_link_libraries(test4 PUBLIC debug_mem)
add_executable(       test3 test/test3_check_all.c)
target_link_libraries(test3 PUBLIC debug_mem)
add_executable(       test2 test/test2_out_of_bounds.c)
target_link_libraries(test2 PUBLIC debug_mem)
add_executable(       test1 test/test1_unfreed_counter.c)
target_link_libraries(test1 PUBLIC debug_mem)
add_test("Unfreed Item Counter" 
    test1)
add_test("Out of bounds writing shows in debug_mem_check" 
    test2)
add_test("debug_check_all correctly reports number of troubled allocations" 
    test3)
add_test("free() clears table entry for given pointer, reducing table length" 
    test4)
add_test("Table automatically expands as allocation increases"
    test5)
add_test("Table automatically shrinks as allocations are freed, minimum size as original size" 
    test6)
add_test("Checksum is read correctly when the buffer is an odd size"
    test7)

# add_custom_command(TARGET test1 
#     POST_BUILD
#     COMMAND ctest --output-on-failure -C $<CONFIGURATION>)
# add_custom_command(TARGET test1
#     POST_BUILD
#     COMMAND rm memory_test*.log)
