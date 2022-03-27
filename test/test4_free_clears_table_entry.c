#define DEBUG_MEM_ENABLE
#include <debug_mem.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>

#define TESTNAME "test4_free_clears_table_entry"
int main()
{
#ifdef DEBUG_MEM_ENABLE
    int err = debug_mem_init( "memory_" TESTNAME ".log", 10 );
    if ( err ) {
        fprintf( stderr, "Failed to initialise memory debugger\n" );
        return 1;
    }
#endif
    int *buffer;
    int buffer_size = 4000;
    buffer = malloc( sizeof &buffer * buffer_size );
    int *buf2;
    buf2 = malloc( sizeof &buffer * buffer_size );
    // freeing should reduce the table length
#ifdef DEBUG_MEM_ENABLE
    assert( debug_mem_table_length() == 2 );
#endif
    free( buffer );
#ifdef DEBUG_MEM_ENABLE
    assert( debug_mem_table_length() == 1 );
#endif
    free( buf2 );
#ifdef DEBUG_MEM_ENABLE
    size_t n = debug_mem_end();
    assert ( n == 0 );
#endif
    return 0;
}
