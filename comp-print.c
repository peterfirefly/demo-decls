/* print AST in two ways -- raw and nice
   the nice version requires a little help from GNU indent.
 */

#include "comp-print.h"
#include "comp-ast.h"

const char *type_atom_str[] = {
        [SC_TYPEDEF ] = "typedef",
        [SC_EXTERN  ] = "extern",
        [SC_STATIC  ] = "static",
        [SC_AUTO    ] = "auto",
        [SC_REGISTER] = "register",

        // type qualifier
        [TQ_NONE    ] = "",
        [TQ_CONST   ] = "const",
        [TQ_RESTRICT] = "restrict",
        [TQ_VOLATILE] = "volatile",

        // type specifier
        [TS_VOID       ] = "void",
        [TS_CHAR       ] = "char",
        [TS_SHORT      ] = "short",
        [TS_INT        ] = "int",
        [TS_LONG       ] = "long",
        [TS_LONG2      ] = "long2",
        [TS_FLOAT      ] = "float",
        [TS_DOUBLE     ] = "double",
        [TS_SIGNED     ] = "signed",
        [TS_UNSIGNED   ] = "unsigned",
        [TS_BOOL       ] = "_Bool",
        [TS_COMPLEX    ] = "_Complex",
        [TS_STRUCTUNION] = "structunion",
        [TS_ENUM       ] = "enum",
        [TS_TYPEDEFNAME] = "typedefname",

        [TS_UINT8 ] = "uint8_t",
        [TS_UINT16] = "uint16_t",
        [TS_UINT32] = "uint32_t",

        // function specifier
        [FS_INLINE] = "inline",
};
