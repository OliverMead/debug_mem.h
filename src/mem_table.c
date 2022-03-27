#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "mem_table.h"

typedef struct {
    const void *location;
    size_t size;
    uint8_t checksum;
} MemHTFrame;

struct MemHT {
    MemHTFrame *entries;
    size_t capacity;
    size_t min_capacity;
    size_t length;
};

// FNV-1a Hash, not secure, randomised or cryptographic, but
// useful for this purpose
static uintptr_t table_hash( uintptr_t *key )
{
    uintptr_t hash = FNV_OFFSET;
    for ( unsigned int i = 0; i < sizeof( uintptr_t ); i++ ) {
        hash ^= ( uintptr_t )( unsigned char )( *( key + i ) );
        hash *= FNV_PRIME;
    }
    return hash;
}

// entry setting helper function
static uintptr_t table_set_entry( MemHTFrame *entries, size_t capacity,
                                  uintptr_t location, size_t size,
                                  size_t *plength )
{
    uintptr_t hash = table_hash( &location );
    size_t index = ( size_t )( hash % ( uintptr_t )( capacity ) );
    while ( entries[index].location != NULL ) {
        bool cmp = ( uintptr_t )( entries[index].location )
                   == ( uintptr_t )( location );
        if ( cmp ) { // pointer exists, update size
            entries[index].size = size;
            return ( uintptr_t ) entries[index].location;
        }
        index++;
        if ( index >= capacity )
            index = 0;
    }
    // entry does not yet exist, create it and apply checksum to buffer
    entries[index].location = ( const void * ) location;
    entries[index].size = size;
    entries[index].checksum = ( uint8_t )( hash % 256 );
    ( ( uint8_t* )entries[index].location )[size] = entries[index].checksum;
    if ( plength != NULL )
        ( *plength )++;
    return location;
}

static bool table_resize( MemHT* table, size_t new_capacity )
{
    MemHTFrame* new_entries = calloc( new_capacity, sizeof( MemHTFrame ) );
    if ( new_entries == NULL )
        return false;
    for ( size_t i=0; i < table->capacity; i++ ) {
        MemHTFrame entry = table->entries[i];
        if ( entry.location != NULL ) {
            table_set_entry( new_entries, new_capacity,
                             ( uintptr_t ) entry.location, entry.size, NULL );
        }
    }
    free( table->entries );
    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

static inline bool table_expand( MemHT* table )
{
    return table_resize( table, table->capacity * 2 );
}

static inline bool table_shrink( MemHT* table )
{
    if ( ( table->capacity / 2 ) >= table->min_capacity )
        return table_resize( table, table->capacity / 2 );
    else
        return true;
}

extern MemHT* table_init( size_t initial_capacity )
{
    MemHT* ht = malloc( sizeof( MemHT ) );
    if ( ht == NULL )
        return NULL;
    ht->length = 0;
    ht->capacity = initial_capacity;
    ht->min_capacity = initial_capacity;
    ht->entries = calloc( ht->capacity, sizeof( MemHTFrame ) );
    if ( ht->entries == NULL ) {
        free( ht );
        return NULL;
    }
    return ht;
}

extern size_t table_length( MemHT* table )
{
    return table->length;
}

extern size_t table_capacity( MemHT* table )
{
    return table->capacity;
}

// returns the quantity of previously un-freed slots in the table
// freeing all registered pointers along the way
// (if memory is freed by debug_mem, then the entry is also removed
// from the table, so freeing here is a safety net that should be monitored)
extern size_t table_destroy( MemHT* table )
{
    size_t count = 0;
    for ( size_t i = 0; i < table->capacity; i++ ) {
        if ( table->entries[i].location != NULL ) {
            free( ( void * )table->entries[i].location );
            count++;
        }
    }
    free( table->entries );
    free( table );
    return count;
}

// automatically expands the table if it is >=50% full
extern uintptr_t table_set( MemHT* table, uintptr_t location, size_t size )
{
    /* assert( location != NULL ); */
    if ( table->length >= table->capacity / 2 ) {
        if ( !table_expand( table ) )
            return ( uintptr_t ) NULL;
    }
    return table_set_entry( table->entries, table->capacity,
                            location, size, &table->length );
}

extern bool table_remove( MemHT* table, const void *location )
{
    if ( table->length <= table->capacity / 4 ) {
        if ( !table_shrink( table ) )
            return false;
    }

    uintptr_t hash = table_hash( ( uintptr_t * ) &location );
    size_t index = ( size_t )( hash % ( uintptr_t ) ( table->capacity ) );
    size_t index_start = index;
    MemHTFrame *entries = table->entries;
    do {
        bool cmp = ( uintptr_t )( table->entries[index].location )
                   == ( uintptr_t )( location );
        if ( cmp ) {
            entries[index].location = NULL;
            entries[index].size = 0;
            entries[index].checksum = 0;
            table->length--;
            return true;
        }
        index++;
        if ( index >= table->capacity )
            index = 0;
    } while ( index != index_start );
    return false;
}

// populate the given size_pointer and checksum_pointer with the values
// associated with location, returns false if the entry could not be found
extern bool table_get( MemHT* table, const void *location,
                       size_t *size_pointer, uint8_t *checksum_pointer )
{
    uintptr_t hash = table_hash( ( uintptr_t * ) &location );
    size_t index = ( size_t ) ( hash % ( uintptr_t ) ( table->capacity ) );
    size_t index_start = index;
    MemHTFrame *entries = table->entries;
    do {
        bool cmp = ( uintptr_t )( table->entries[index].location )
                   == ( uintptr_t )( location );
        if ( cmp ) {
            MemHTFrame entry = entries[index];
            *size_pointer = entry.size;
            *checksum_pointer = entry.checksum;
            return true;
        }
        index++;
        if ( index >= table->capacity ) {
            index = 0;
        }
    } while ( index != index_start );
    return false;
}


extern HTIter table_iterator( MemHT* table )
{
    HTIter iter;
    iter._table = table;
    iter._index = 0;
    return iter;
}

extern bool table_iter_next( HTIter* iterator )
{
    MemHT* table = iterator->_table;
    while ( iterator->_index < table->capacity ) {
        size_t i = iterator->_index;
        iterator->_index++;
        if ( table->entries[i].location != NULL ) {
            MemHTFrame entry = table->entries[i];
            iterator->location = entry.location;
            iterator->size = entry.size;
            iterator->checksum = entry.checksum;
            return true;
        }
    }
    return false;
}
