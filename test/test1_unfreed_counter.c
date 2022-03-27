#define DEBUG_MEM_ENABLE
#include <debug_mem.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>

#define TESTNAME "test1_unfreed_counter"
int main()
{
#ifdef DEBUG_MEM_ENABLE
    int err = debug_mem_init( "memory_" TESTNAME ".log", 10 );
    if ( err ) {
        fprintf( stderr, "Failed to initialise memory debugger\n" );
        return 1;
    }
#endif
    int *buffers[100];
    for ( size_t i = 0; i < 100; i++ ) {
        buffers[i] = calloc( 40, sizeof( int ) );
        // suppress declared but not used warning
        ( ( size_t * ) buffers[i] )[0] = i;
    }
    for ( size_t i = 0; i < 100; i += 2 ) {
        free( buffers[i] );
    }
#ifdef DEBUG_MEM_ENABLE
    size_t n = debug_mem_end();
    assert ( n == 50 ); // freeing every second buffer
#endif
    return 0;
}
