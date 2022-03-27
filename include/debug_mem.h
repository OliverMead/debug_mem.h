/*
 * Oliver Mead 2022
 * Memory debugging 'system' inspired by Eskil Steenberg's Forge Debugging:
 *      http://gamepipeline.org/forge_Debugging_.html
 *
 * If you happen to be reading this, then I must have put it on GitHub and
 * made it public domain. Copy the file byte-for-byte, slap your name at the
 * top and charge £50 for a single seat license, see if I care.
 * You're very welcome, although I'd recommend using Forge (above) over this
 * since the developer has much more experience, that system has many more
 * features, and it will probably be better maintained.
 *
 * Pros of this implementation:
 * + Very simple, just outputs allocation information to a file (when the system
 *   is enabled), and you can write a simple script to analyse it (or I may
 *   [have] include[d] one)
 * + Provides a macro to be used whenever passing around a pointer to some
 *   (potentially heap allocated) memory, which will create a log entry
 *   indicating that this has happened, so you can keep track of where your
 *   malloc'ed memory is used.
 * + Standard C18 - this will have been tested on linux (gcc) with
 *   `--std=c18 -Wall -Wextra -pedantic` and on Windows through CMake specifying
 *   C18 and similar warnings enabled.
 *
 * Problems:
 * - Probably not thread-safe, this notice will be removed if that changes.
 * - I wrote it, this isn't in itself a problem, but you should probably read
 *   it over for yourself before putting it to use and especially before relying
 *   on it.
 * - This is not a replacement for a proper debugger, and may introduce
 *   unexpected bugs of its own, or hide bugs that would exist if the system
 *   were disabled. Always test your code with (memory) debugging disabled
 *   before release.
 */
#ifndef DEBUG_MEM_H
#define DEBUG_MEM_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef  DEBUG_MEM_ENABLE
#define malloc(n)         debug_mem_malloc(n, __FILE__, __LINE__, __func__)
#define calloc(n, s)      debug_mem_calloc(n, s, __FILE__, __LINE__, __func__)
#define free(n)           debug_mem_free(n, __FILE__, __LINE__, __func__)
#define pass_pointer(type, n)   (type) debug_mem_pass_pointer((void *) n, __FILE__, __LINE__, __func__)
#define return_pointer(type, n)  return (type) debug_mem_return_pointer((void *) n, __FILE__, __LINE__, __func__)
#else
#define pass_pointer(type, n)    n
#define return_pointer(type, n)  n
#endif

extern int debug_mem_init( const char*, size_t );
extern size_t debug_mem_end();
extern void *debug_mem_malloc(
    size_t,  const char*,
    unsigned int,    const char* );
extern void *debug_mem_calloc(
    size_t, size_t,
    const char*, unsigned int, const char* );
extern void debug_mem_free(
    void *,    const char*,
    unsigned int,    const char* );
extern void *debug_mem_pass_pointer(
    void *,    const char*,
    unsigned int,    const char* );
extern int debug_mem_check( const void* );
extern size_t debug_mem_check_all( );
extern size_t debug_mem_table_length();
extern size_t debug_mem_table_capacity();

extern void *debug_mem_pass_pointer(
    void *, const char*,
    unsigned int, const char* );

extern void *debug_mem_return_pointer(
    void *, const char*,
    unsigned int, const char* );
#endif
