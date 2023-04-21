/* comp-ast.c -- Abstract Syntax Tree (expression trees + statement lists) */

// yes, I would like strdup(), please (not necessary for C2X)
#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdlib.h>

#include "comp-ast.h"

/***/

/***/

/* arena allocator */

#ifndef AST_FREE
struct {
        char            *ptr;
        unsigned        size, used;
} arena;
#endif

void ar_init()
{
#ifndef AST_FREE
        arena.size = 10*1024*1024;
        arena.used = 0;
        arena.ptr = malloc(arena.size);
#endif
}

/* keep it static => it's likely to be inlined */
static void *ar_alloc(size_t size)
{
#ifdef AST_FREE
        return calloc(1, size);
#else
        void    *p = arena.ptr + arena.used;
        assert(arena.used + size < arena.size);
        arena.used += size;
        return p;
#endif
}

#ifdef AST_FREE
# define ar_free(p)     free(p)
#else
# define ar_free(p)     do {} while(0)
#endif


/***/

DEFINE_LIST_NEW(type_atom)
DEFINE_LIST_APPEND(type_atom)

/***/

