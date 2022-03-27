#define DEBUG_MEM_ENABLE
#include <debug_mem.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>

#define TESTNAME "test2_out_of_bounds"
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
    // overwrite the checksum portion of the buffer
    ( ( uint8_t * ) buffer )[buffer_size * sizeof &buffer] = 6;
#ifdef DEBUG_MEM_ENABLE
    assert( debug_mem_check( buffer ) == 1 );
#endif
    free( buffer );
#ifdef DEBUG_MEM_ENABLE
    size_t n = debug_mem_end();
    assert ( n == 0 );
#endif
    return 0;
}
