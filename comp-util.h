/* comp-util.h -- string interning, symbol tables, miscellaneous */

#ifndef COMP_UTIL__H
#define COMP_UTIL__H

/***/

#define ARRAYSIZE(x)    (sizeof(x)/sizeof((x)[0]))

/***/

/* lists */

// type
#define DECLARE_LIST_TYPE(x)                    \
        struct x##_list {                       \
                unsigned        cnt;            \
                unsigned        alloccnt;       \
                struct x        **list;         \
        };


// implementation
#define DEFINE_LIST_NEW(x)                                              \
        struct x##_list *x##_list_new(void)                             \
        {                                                               \
                struct x##_list *p = ar_alloc(sizeof(struct x##_list));   \
                p->cnt = p->alloccnt = 0;                               \
                p->list = NULL;                                         \
                return p;                                               \
        }

#define DEFINE_LIST_APPEND(x)                                           \
        struct x##_list *x##_list_append(struct x##_list *p, struct x *e)     \
        {                                                               \
                /* NULL creates a new list */                           \
                if (!p)                                                 \
                        p = x##_list_new();                             \
                if (p->cnt == p->alloccnt) {                            \
                        /* allocate stuff */                            \
                        if (p->alloccnt == 0)                           \
                                p->alloccnt = 50;                       \
                        else                                            \
                                p->alloccnt = p->alloccnt*2;            \
                        p->list = realloc(p->list, sizeof(p->list[0]) * p->alloccnt);\
                }                                                       \
                p->list[p->cnt++] = e;                                  \
                return p;                                               \
        }

#endif