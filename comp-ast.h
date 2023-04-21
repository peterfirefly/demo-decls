/* comp-ast.h -- Abstract Syntax Tree (expression trees + statement lists) */

#ifndef COMP_AST__H
#define COMP_AST__H


#include "comp-util.h"


/***/

/* arena allocator -- the other functions are only used inside comp-ast.c */

void ar_init(void);

/***/

enum type_atom_kind {
        // storage class specifier
        SC_TYPEDEF, SC_EXTERN, SC_STATIC, SC_AUTO, SC_REGISTER,
        // type qualifier -- also used with '*'
        TQ_NONE, TQ_CONST, TQ_RESTRICT, TQ_VOLATILE,
        // function specifier
        FS_INLINE,
        // type specifier
        TS_VOID, TS_CHAR, TS_SHORT, TS_INT, TS_LONG, TS_LONG2, TS_FLOAT, TS_DOUBLE, TS_SIGNED,
        TS_UNSIGNED, TS_BOOL, TS_COMPLEX, TS_UINT8, TS_UINT16, TS_UINT32,

        TS_STRUCTUNION,                 // ptr to struct/union info
        TS_ENUM,                        // ptr to enum info
        TS_TYPEDEFNAME,                 // ptr to type info
};

struct type_atom {
#if 0
        location_t              loc;
#endif
        enum type_atom_kind     kind;
#if 0
        union {
                struct struct_def       struct_def;     // TS_STRUCTUNION
                struct enum_def         enum_def;       // TS_ENUM
        };
#endif
};

DECLARE_LIST_TYPE(type_atom)


struct type_atom_list *type_atom_list_new(void);
struct type_atom_list *type_atom_list_append(struct type_atom_list *p, struct type_atom *e);

#endif