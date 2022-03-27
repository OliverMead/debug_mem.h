# debug\_mem.h
![Status Indicator](https://github.com/OliverMead/debug_mem.h/actions/workflows/cmake.yml/badge.svg?branch=main)
## IMPORTANT

*This is not ready for use since it is not thread-safe, and the testing in
place is not rigorous enough.*

## Summary

A simple memory debugging helper that (when enabled) keeps track of
allocations, freeing, reallocation and - using the `pass_pointer` and
`return_pointer` macros - where pointers are passed to and returned from
functions.

Allocation information is written to the file named by the initialiser. Each
line of the log will include the file, line number and name of the function in
which the call occurred. By processing this output, you can find the cause of a
variety of dynamic memory related issues.

In addition, the library (optionally) tracks all allocations in a hash table.
When this is enabled: buffers are over-allocated, and a checksum is written to
the extra allocated space. Through this, any buffer can be 'checked' to see if
any part of the program has attempted to write to memory outside of what the
buffer should have (and will have once the library is disabled).

This library is a CMake project (including a test suite) providing a header
file and a shared or static object file.

## Usage

A full example program is below. Define `DEBUG_MEM_ENABLE` before including the
header (this can and should be done with a compiler option) to enable tracking
of allocations. This causes macros to be defined which replace `malloc`,
`free`, etc. with functions which log their use.

At the very beginning of the program, call `debug_mem_init` to start the
logging system. This function requires a filename for output, and an initial
capacity for the allocation hash table. If this capacity is zero, this hash
table will not be created and any calls to the 'check' functions will return
zero (success).

At the end of the program, call `debug_mem_end` to close the log file and free
the allocation hash table (if that is enabled). With the allocation hash table
enabled, this function will also free any remaining (tracked) allocations, log
the number of allocations which weren't previously freed and also return this
quantity.

Calls to `debug_mem_...` functions should be wrapped in enable checks as below
so that enabling/disabling this system is as simple as adding or removing a 
compiler option.

```c
#define DEBUG_MEM_ENABLE
#include <debug_mem.h>
#include <inttypes.h>

int main()
{
#ifdef DEBUG_MEM_ENABLE
    int err = debug_mem_init( "memory.log", 10 );
    if ( err ) {
        fprintf( stderr, "Failed to initialise memory debugger\n" );
        return 1;
    }
#endif
    int *buffers[100];
    for ( size_t i = 0; i < 100; i++ ) {
        buffers[i] = calloc( 40, sizeof( int ) );
        // suppress declared but not used warning
        // just writes the index into the first sizeof(size_t) bytes 
        // of the buffer
        ( ( size_t * ) buffers[i] )[0] = i;
    }
    for ( size_t i = 0; i < 100; i++) {
        free( buffers[i] );
    }
#ifdef DEBUG_MEM_ENABLE
    size_t n = debug_mem_end();
#endif
    return 0;
}
```
## TODO
- Make thread safe (mutex)
- Write more tests
