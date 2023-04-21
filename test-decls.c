/* test [decls] 

   convert a type atom list to a uint32_t that describes the type
   
   (only lower 10 bits are used)

 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "comp-ast.h"
#include "comp-print.h"


// type info -- this is the uint32_t bits [decls] returns
// (we only use the lower 10 bits)
enum type_info {
//   low 5 bits = basic type
        TI_MASK         = 0x1F,

        TI_VOID         = 0,    // void

        // 3 character types
        TI_CHAR         = 1,    // char, implementation-defined signedness
        TI_SCHAR        = 2,    // signed char
        TI_UCHAR        = 3,    // unsigned char

        // 4 signed integer types, 4 unsigned integer types
        // (signed/unsigned char are also included in the standard signed/unsigned integer types)
        TI_SSHORT       = 4,    // signed short
        TI_USHORT       = 5,    // unsigned short
        TI_SINT         = 6,    // signed int
        TI_UINT         = 7,    // unsigned int
        TI_SLONG        = 8,    // signed long
        TI_ULONG        = 9,    // unsigned long
        TI_SLONGLONG    = 10,   // signed long long
        TI_ULONGLONG    = 11,   // unsigned long long

        // 3 real fp types
        TI_FLOAT        = 12,   // float
        TI_DOUBLE       = 13,   // double
        TI_LDOUBLE      = 14,   // long double

        // the boolean type
        TI_BOOL         = 15,   // _Bool

        // 3 complex fp types
        TI_FCOMPLEX     = 16,   // float _Complex
        TI_DCOMPLEX     = 17,   // double _Complex
        TI_LDCOMPLEX    = 18,   // long double _Complex

        TI_STRUCTUNION  = 29,   // struct/union ...
        TI_ENUM         = 30,   // enum ...
        TI_TYPEDEFNAME  = 31,   // things like bool, uint32_t, size_t

//   next 2 bits = type qualifiers
        TIQ_MASK        = 0x60,
        TIQ_NONE        = 0 << 5,
        TIQ_CONST       = 1 << 5,
        TIQ_RESTRICT    = 2 << 5,
        TIQ_VOLATILE    = 3 << 5,

//   next 3 bits = storage class
        TISC_MASK       = 0x380,
        TISC_AUTO       = 0 << 7,
        TISC_EXTERN     = 1 << 7,
        TISC_REGISTER   = 2 << 7,
        TISC_STATIC     = 3 << 7,
        TISC_TYPEDEF    = 4 << 7, //?
};


const char *type_info_str[] = {
        [TI_VOID       ] = "void",

        [TI_CHAR       ] = "char",
        [TI_SCHAR      ] = "signed char",
        [TI_UCHAR      ] = "unsigned char",

        [TI_SSHORT     ] = "short",
        [TI_USHORT     ] = "unsigned short",
        [TI_SINT       ] = "int",
        [TI_UINT       ] = "unsigned",
        [TI_SLONG      ] = "long",
        [TI_ULONG      ] = "unsigned long",
        [TI_SLONGLONG  ] = "long long",
        [TI_ULONGLONG  ] = "unsigned long long",

        [TI_FLOAT      ] = "float",
        [TI_DOUBLE     ] = "double",
        [TI_LDOUBLE    ] = "long double",

        [TI_BOOL       ] = "_Bool",

        [TI_FCOMPLEX   ] = "float _Complex",
        [TI_DCOMPLEX   ] = "double _Complex",
        [TI_LDCOMPLEX  ] = "long double _Complex",

        [TI_STRUCTUNION] = "structunion",
        [TI_ENUM       ] = "enum",
        [TI_TYPEDEFNAME] = "typedefname",
};


#if 0
// these are the "atoms", a list of which is the input to [decls]
// we use '1 << atom#' to encode the atoms we have seen
enum type_atom_kind {
        // storage class specifier
        0-4:    SC_TYPEDEF, SC_EXTERN, SC_STATIC, SC_AUTO, SC_REGISTER,
        // type qualifier -- also used with '*'
        5-8:    TQ_NONE, TQ_CONST, TQ_RESTRICT, TQ_VOLATILE,
        // function specifier
        9:      FS_INLINE,
        // type specifier
        10-24:  TS_VOID, TS_CHAR, TS_SHORT, TS_INT, TS_LONG, TS_LONG2, TS_FLOAT, TS_DOUBLE, TS_SIGNED,
                TS_UNSIGNED, TS_BOOL, TS_COMPLEX, TS_UINT8, TS_UINT16, TS_UINT32,

        25:     TS_STRUCTUNION,                 // ptr to struct/union info
        26:     TS_ENUM,                        // ptr to enum info
        27:     TS_TYPEDEFNAME,                 // ptr to type info
};
#endif


#define B(x)            (1U<<(x))

#define TS_MASK         (B(TS_VOID)    | B(TS_CHAR)  | B(TS_SHORT)   | B(TS_INT)    | B(TS_LONG) | \
                        B(TS_LONG2)    | B(TS_FLOAT) | B(TS_DOUBLE)  | B(TS_SIGNED) | \
                        B(TS_UNSIGNED) | B(TS_BOOL)  | B(TS_COMPLEX) | B(TS_UINT8)  | \
                        B(TS_UINT16)   | B(TS_UINT32))

// enum type_atom_kind -> conflict mask
uint32_t  t_conflict[/* enum type_atom_kind */] = {
        // storage classes
        // ---
        // auto/extern/register/static => conflict w/ auto/extern/register/static
        // these can occur only once -- right?
        [TISC_AUTO     ] =          0 | B(SC_EXTERN) | B(SC_REGISTER) | B(SC_STATIC),
        [TISC_EXTERN   ] = B(SC_AUTO) |            0 | B(SC_REGISTER) | B(SC_STATIC),
        [TISC_REGISTER ] = B(SC_AUTO) | B(SC_EXTERN) |              0 | B(SC_STATIC),
        [TISC_STATIC   ] = B(SC_AUTO) | B(SC_EXTERN) | B(SC_REGISTER) |            0,

        // type specifiers
        // ----
        // these can be repeated any number of times

        // void         => conflicts w/ all other type specifiers
        [TS_VOID       ] = TS_MASK &~ B(TS_VOID),

        // char         => conflicts w/ short/long, float/double, void, struct/union/enum, type-name
        [TS_CHAR       ] = B(TS_SHORT) | B(TS_LONG) | B(TS_FLOAT) | B(TS_DOUBLE) | B(TS_VOID) | B(TS_STRUCTUNION) | B(TS_ENUM) | B(TS_TYPEDEFNAME),

        // signed       => conflicts w/ unsigned
        [TS_SIGNED     ] = B(TS_UNSIGNED),

        // unsigned     => conflicts w/ signed
        [TS_UNSIGNED   ] = B(TS_SIGNED),

        // short        => conflicts w/ long, long2
        [TS_SHORT      ] = B(TS_LONG) | B(TS_LONG2),

        // long         => conflicts w/ short
        [TS_LONG       ] = B(TS_SHORT),

        // long2        => conflicts w/ short
        [TS_LONG2      ] = B(TS_SHORT),

        // int          => conflicts w/ void, char, float, double, struct/union/enum, type-name
        [TS_INT        ] = B(TS_VOID) | B(TS_CHAR) | B(TS_FLOAT) | B(TS_DOUBLE) | B(TS_STRUCTUNION) | B(TS_ENUM) | B(TS_TYPEDEFNAME),

        // float        => conflicts w/ void, char, int, short/long/long2, signed/unsigned, struct/union/enum, type-name
        [TS_FLOAT      ] = (uint32_t)-1, // not allowed at all

        // double       => conflicts w/ ...
        [TS_DOUBLE     ] = (uint32_t)-1, // not allowed at all

        // struct/union/enum => conflicts w/ void
        [TS_STRUCTUNION] = B(TS_VOID) | B(TS_ENUM),
        [TS_ENUM       ] = B(TS_VOID) | B(TS_STRUCTUNION),
        [TS_TYPEDEFNAME] = (uint32_t)-1,        // not allowed at all
};


// convert type specifier mask to actual types (low 5 bits of enum type_info)
struct {
        uint32_t                bits;
        enum type_info          ti;
} valid_ti[] = {
        // void
        {.ti = TI_VOID, .bits = B(TS_VOID)},

        // char, signed char, unsigned char
        {.ti = TI_CHAR , .bits =                  B(TS_CHAR)},
        {.ti = TI_SCHAR, .bits = B(TS_SIGNED)   | B(TS_CHAR)},
        {.ti = TI_UCHAR, .bits = B(TS_UNSIGNED) | B(TS_CHAR)},

        // short
        {.ti = TI_SSHORT, .bits =                B(TS_SHORT)            },
        {.ti = TI_SSHORT, .bits = B(TS_SIGNED) | B(TS_SHORT)            },
        {.ti = TI_SSHORT, .bits = B(TS_SIGNED) | B(TS_SHORT) | B(TS_INT)},

        // unsigned short
        {.ti = TI_USHORT, .bits = B(TS_UNSIGNED) | B(TS_SHORT)            },
        {.ti = TI_USHORT, .bits = B(TS_UNSIGNED) | B(TS_SHORT) | B(TS_INT)},

        // int
        {.ti = TI_SINT, .bits =                B(TS_INT)},
        {.ti = TI_SINT, .bits = B(TS_SIGNED) | B(TS_INT)},

        // unsigned
        {.ti = TI_UINT, .bits = B(TS_UNSIGNED)            },
        {.ti = TI_UINT, .bits = B(TS_UNSIGNED) | B(TS_INT)},

        // long
        {.ti = TI_SLONG, .bits =                B(TS_LONG)               },
        {.ti = TI_SLONG, .bits =                B(TS_LONG) | B(TS_INT)   },
        {.ti = TI_SLONG, .bits = B(TS_SIGNED) | B(TS_LONG)               },
        {.ti = TI_SLONG, .bits = B(TS_SIGNED) | B(TS_LONG) | B(TS_INT)   },

        // unsigned long
        {.ti = TI_ULONG, .bits = B(TS_UNSIGNED) | B(TS_LONG) },
        {.ti = TI_ULONG, .bits = B(TS_UNSIGNED) | B(TS_LONG) | B(TS_INT)},

        // long long
        {.ti = TI_SLONGLONG, .bits =                B(TS_LONG) | B(TS_LONG2)            },
        {.ti = TI_SLONGLONG, .bits =                B(TS_LONG) | B(TS_LONG2) | B(TS_INT)},
        {.ti = TI_SLONGLONG, .bits = B(TS_SIGNED) | B(TS_LONG) | B(TS_LONG2)            },
        {.ti = TI_SLONGLONG, .bits = B(TS_SIGNED) | B(TS_LONG) | B(TS_LONG2) | B(TS_INT)},

        // unsigned long long
        {.ti = TI_ULONGLONG, .bits = B(TS_UNSIGNED) | B(TS_LONG) | B(TS_LONG2)            },
        {.ti = TI_ULONGLONG, .bits = B(TS_UNSIGNED) | B(TS_LONG) | B(TS_LONG2) | B(TS_INT)},

        // float
        {.ti = TI_FLOAT, .bits = B(TS_FLOAT)},

        // double
        {.ti = TI_DOUBLE, .bits = B(TS_DOUBLE)},

        // long double
        {.ti = TI_LDOUBLE, .bits = B(TS_LONG) | B(TS_DOUBLE)},
        // allow 'long long double'?

        // _Bool
        {.ti = TI_BOOL, .bits = B(TS_BOOL)},

        // float _Complex
        {.ti = TI_FCOMPLEX, .bits = B(TS_COMPLEX)},
        {.ti = TI_FCOMPLEX, .bits = B(TS_FLOAT) | B(TS_COMPLEX)},

        // double _Complex
        {.ti = TI_DCOMPLEX, .bits = B(TS_DOUBLE) | B(TS_COMPLEX)},

        // long double _Complex
        {.ti = TI_LDCOMPLEX, .bits = B(TS_LONG) | B(TS_DOUBLE) | B(TS_COMPLEX)},
        // allow 'long long double _Complex'?

};


static uint32_t elab_decls(struct type_atom_list *list, uint32_t init_conflict)
{
        // this is only for things that aren't a proper struct/union/enum/typedef
        // make sure that type atom lists with struct/union/enum/typedef are caught via the conflict masks
        uint32_t        info = 0;
        uint32_t        conflict = init_conflict;         // depends on context
        for (unsigned i=0; i < list->cnt; i++) {
                enum type_atom_kind     atom = list->list[i]->kind;

                // handle 'long long'
                if ((atom == TS_LONG) && (info & B(TS_LONG)))
                        atom = TS_LONG2;

                // conflict with previous type atoms (or context)?
                if (B(atom) & conflict) {
                        printf("'storage class not ok' or 'type not ok' or 'restrict not ok'\n");
                        exit(1);
                }

                // remember we have seen this atom
                info |= B(atom);

                // remember what future atoms it conflicts with
                conflict |= t_conflict[atom];
        }

        printf("info    : %08X\n", info);
        printf("conflict: %08X\n", conflict);

        printf("info:\n");
        for (unsigned i=0; i <= 31; i++) {
                if (info & (1UL<<i)) {
                        printf("'%s' ", type_atom_str[i]);
                }
        }
        printf("\n");

        printf("conflict:\n");
        for (unsigned i=0; i <= 31; i++) {
                if (conflict & (1UL<<i)) {
                        if (type_atom_str[i])
                                printf("'%s' ", type_atom_str[i]);
                        else
                                printf("B(%u) ", i);
                }
        }
        printf("\n");

        // special case struct/union and enum (and typename in the future)
        // they are allowed to have storage class and const/register/volatile type quals
        // (yes, 'register' can be used with structs!  and arrays!)

        // also specialize typedef as "storage class" in the future

//        ... special case struct/union/enum ... specialize typedef

        // ok, we have a mask w/ storage class, type qualifiers, and type specifiers.
        // the type specifiers are not struct/union/enum/typename and the storage class is not typedef.
        // 
        // let's convert that to a type_info mask -- specifically, let's look up the
        // type specifiers and see what type they correspond to.
        uint32_t        tsbits = info & TS_MASK;
        uint32_t        ti=0;
        printf("tsbits: %08X\n", tsbits);
        for (unsigned i=0; i < ARRAYSIZE(valid_ti); i++) {
//                printf("%u: %08X == %08X?\n", i, valid_ti[i].bits, tsbits);
                if (valid_ti[i].bits == tsbits) {
                        ti = valid_ti[i].ti;
                        goto FOUND;
                }
        }

        printf("type not ok\n");
        exit(1);

FOUND:
        /* FIXME the bits for the storage class and type qualifiers should be
           tagged on here.
         */

        printf("%u\n", ti);
        return ti;
}


static void dump_atoms(struct type_atom_list *list) __attribute__((unused));
static void dump_atoms(struct type_atom_list *list)
{
        for (unsigned i=0; i < list->cnt; i++) {
                if (i)
                        printf(" ");
                printf("%d/%s", list->list[i]->kind, type_atom_str[list->list[i]->kind]);
        }
        printf("\n");
}

static struct type_atom_list *mk_list(unsigned cnt, enum type_atom_kind atoms[cnt])
{
        struct type_atom_list   *list = type_atom_list_new();
        for (unsigned i=0; i < cnt; i++) {
                struct type_atom        *p = calloc(1, sizeof(struct type_atom));
                p->kind = atoms[i];
                type_atom_list_append(list, p);
        }
        return list;
}


int main(int argc, char *argv[])
{
        (void) argc, (void) argv;

        // initialize allocator
        ar_init();

        struct type_atom_list   *list;
        uint32_t        typ;

        printf("int:\n");
        list = mk_list(1, (enum type_atom_kind[]) {TS_INT});
//        dump_atoms(list);
        typ = elab_decls(list, 0x00);
        printf("type: %03X  [%s]\n\n", typ, type_info_str[typ & TI_MASK]);

        printf("extern int:\n");
        list = mk_list(2, (enum type_atom_kind[]) {SC_EXTERN, TS_INT});
//        dump_atoms(list);
        typ = elab_decls(list, 0x00);
        printf("type: %03X  [%s]\n\n", typ, type_info_str[typ & TI_MASK]);

        printf("static volatile long long int:\n");
        list = mk_list(5, (enum type_atom_kind[]) {SC_STATIC, TQ_VOLATILE, TS_LONG, TS_LONG, TS_INT});
//        dump_atoms(list);
        typ = elab_decls(list, 0x00);
        printf("type: %03X  [%s]\n", typ, type_info_str[typ & TI_MASK]);

        return EXIT_SUCCESS;
}