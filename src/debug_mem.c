#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include "debug_mem.h"
#include "mem_table.h"
/* #include <stdint.h> */

static FILE *logfile;
static bool initialised;
static MemHT* table;

// set initial_capacity to 0 to disable memory checking
extern int debug_mem_init( const char* log_location, size_t initial_capacity )
{
    if ( !initialised ) {
        logfile = fopen( log_location, "w" );
        if ( logfile == NULL )
            return 1;
        if ( initial_capacity ) {
            MemHT *ht = table_init( initial_capacity );
            if ( ht == NULL )
                return 2;
            table = ht;
        }
        initialised = true;
    }
    return 0;
}

static int debug_mem_checker( const void* buf, size_t buffer_size,
                              uint8_t checksum )
{
    uint8_t cmp_checksum = ( ( uint8_t * ) buf )[buffer_size];
    if ( checksum == cmp_checksum ) {
        if ( initialised )
            fprintf(
                logfile,
                "Checking buffer @%" PRIXPTR " successful: checksum %"
                PRIX8 " matches last byte %" PRIX8 "\n",
                ( uintptr_t ) buf, checksum, cmp_checksum );
        return 0;
    } else {
        if ( initialised )
            fprintf(
                logfile,
                "Checking buffer @%" PRIXPTR
                " unsuccessful: checksum %" PRIX8
                " does not match last byte %" PRIX8 "\n",
                ( uintptr_t ) buf, checksum, cmp_checksum );
        return 1;
    }
}

// return 0: entry found and checksum cleared
// return 1: entry found but checksum not correct
// return -1: entry not found
// Also returns 0 if memory checking is not enabled
extern int debug_mem_check( const void* buf )
{
    if ( table != NULL ) {
        size_t buffer_size;
        uint8_t checksum;
        if ( table_get( table, buf, &buffer_size, &checksum ) ) {
            return debug_mem_checker( buf, buffer_size, checksum );
        } else {
            if ( initialised )
                fprintf(
                    logfile,
                    "Attempted to check buffer @%" PRIXPTR
                    " when memory checking is not enabled, enable\
                        memory checking by passing `true` to debug_mem_init\n",
                    ( uintptr_t ) buf );
            return -1;
        }
    }
    return 0;
}

// returns the number of allocations which either were not in the table,
// or had overwritten their checksum bytes
extern size_t debug_mem_check_all()
{
    if ( table == NULL )
        return 0;
    if ( table_length( table ) == 0 ) {
        fprintf( logfile, "debug_mem_check_all cannot be used when memory\
                  checking is not enabled\n" );
        return 0;
    }
    HTIter iter = table_iterator( table );
    size_t errors = 0;
    while ( table_iter_next( &iter ) ) {
        if ( debug_mem_checker( iter.location, iter.size, iter.checksum ) ) {
            errors ++;
        }
    }
    fprintf( logfile, "CheckAll Summary: %" PRIuPTR " of %"
             PRIuPTR " allocations failed boundary verification\n",
             errors, table_length( table ) );
    return errors;
}

extern size_t debug_mem_end()
{
    size_t n_unfreed = 0;
    if ( table != NULL ) {
        n_unfreed = table_destroy( table );
        fprintf( logfile,
                 "Destroyed allocation table with %lu un-freed items\n",
                 n_unfreed );
        table = NULL;
    }
    fclose( logfile );
    return n_unfreed;
}

extern size_t debug_mem_table_length()
{
    if ( table != NULL )
        return table_length( table );
    else
        return 0;
}

extern size_t debug_mem_table_capacity()
{
    if ( table != NULL )
        return table_capacity( table );
    else
        return 0;
}

extern void *debug_mem_pass_pointer( void *p, const char* filename,
                                     unsigned int line, const char* func )
{
    if ( initialised )
        fprintf( logfile, "%s, %s (line %u): pass_pointer(@%" PRIXPTR ")\n",
                 filename, func, line, ( uintptr_t ) p );
    return p;
}

extern void *debug_mem_return_pointer( void *p, const char* filename,
                                       unsigned int line, const char* func )
{
    if ( initialised )
        fprintf( logfile, "%s, %s (line %u): return_pointer(@%" PRIXPTR ")\n",
                 filename, func, line, ( uintptr_t ) p );
    return p;
}

extern void *debug_mem_malloc(
    size_t size,  const char* filename,
    unsigned int line,    const char* func )
{
    void* p;
    if ( table != NULL )
        p = malloc( size + 1 );
    else
        p = malloc( size );
    if ( p == NULL ) {
        return NULL;
    }
    if ( table != NULL ) {
        table_set( table, ( uintptr_t ) p, size );
    }
    if ( initialised )
        fprintf( logfile, "%s, %s (line %u): malloc(%lu) -> @%" PRIXPTR "\n",
                 filename, func, line, size, ( uintptr_t ) p );
    return p;
}

extern void *debug_mem_calloc(
    size_t nmemb, size_t size,
    const char* filename, unsigned int line, const char* func )
{
    void *p;
    if ( table != NULL )
        p = malloc( ( nmemb * size ) + 1 );
    else
        p = calloc( nmemb, size );
    if ( p == NULL )
        return NULL;
    if ( table != NULL )
        table_set( table, ( uintptr_t ) p, nmemb * size );
    if ( initialised ) {
        fprintf( logfile, "%s, %s (line %u): calloc(%lu, %lu) -> @%"
                 PRIXPTR "\n", filename, func, line, nmemb, size,
                 ( uintptr_t ) p );
    }
    return p;
}

extern void debug_mem_free(
    void *buf,    const char* filename,
    unsigned int line,    const char* func )
{
    if ( table != NULL )
        table_remove( table, buf );
    if ( initialised )
        fprintf( logfile, "%s, %s (line %u): free(@%" PRIXPTR ")\n",
                 filename, func, line, ( uintptr_t ) buf );
    free( buf );
}
