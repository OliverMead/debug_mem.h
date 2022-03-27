#ifndef MEM_TABLE_H
#define MEM_TABLE_H

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define FNV_OFFSET 1469598103934656037UL
#define FNV_PRIME 1099511628211UL

typedef struct MemHT MemHT;

typedef struct {
    const void* location;
    size_t size;
    uint8_t checksum;

    MemHT* _table;
    size_t _index;
} HTIter;

extern MemHT* table_init( size_t initial_capacity );
extern size_t table_length( MemHT* table );
extern size_t table_capacity( MemHT* table );
extern size_t table_destroy( MemHT* table );
extern uintptr_t table_set( MemHT* table, uintptr_t location, size_t size );
extern bool table_remove( MemHT* table, const void *location );
extern bool table_get( MemHT* table, const void *location,
                       size_t *size_pointer, uint8_t *checksum_pointer );
extern HTIter table_iterator( MemHT* table );
extern bool table_iter_next( HTIter* iterator );
#endif
