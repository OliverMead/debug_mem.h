#define DEBUG_MEM_ENABLE
#include <debug_mem.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>

#define TESTNAME "test6_table_auto_shrink"
int main()
{
#ifdef DEBUG_MEM_ENABLE
    int err = debug_mem_init( "memory_" TESTNAME ".log", 10 );
    if ( err ) {
        fprintf( stderr, "Failed to initialise memory debugger\n" );
        return 1;
    }
#endif
    int *buffers[20];
    int buffer_size = 4000;
    for ( size_t i = 0; i < 20; i++ ) {
        buffers[i] = malloc( sizeof &buffers[i] * buffer_size );
    }
#ifdef DEBUG_MEM_ENABLE
    assert( debug_mem_table_capacity() >= 20 );
#endif
    for ( size_t i = 0; i< 20; i++ ) {
        free( buffers[i] );
    }
#ifdef DEBUG_MEM_ENABLE
    assert( debug_mem_table_capacity() == 10 );
    size_t n = debug_mem_end();
    assert ( n == 0 );
#endif
    return 0;
}
