A Small Extract from a C Compiler
---
This is a small extract from a C compiler I am working on.  It handles the validation and interpretation of the "basic type" part of a declaration.  It doesn't handle the full part -- structs, unions, enums, and typedefs are not included.


What is a declaration in C?
---
Here are a few examples of declarations in C:

```C
int                             a;
extern unsigned long long       b, c;
static volatile short           d;
static struct abc               e;
static struct {
        int     x;
        char    *p;
}                               f;
```

This declares the variables `a`, `b`, `c`, `d`, `e`, and `f`.  The "basic type" is the part on the left (`int`, `extern unsigned long long`, etc).

Structs are complicated so the extract doesn't handle them.

The basic type consists of a list -- in any order! -- of what I call "type atoms".  They are things like the storage classes `auto`, `extern`, `register`, `static` and the type specifiers `signed`, `unsigned`, `short`, `long`, `char`, `int`, `float`, `double`, `void` and the type qualifiers `const`, `restrict`, `volatile`.  There are others, but those are the obvious ones.  Things like declarations and definitions of structs, unions, and enums are also "type atoms".

Typedefs are also "type atoms" (that syntactically are treated like a storage class) but they are also quite complicated so the extract doesn't handle them.  In fact, the compiler I am working on doesn't handle them at all.

Here's the hard part: these type atoms can occur in any order.  Some of them are only allowed to occur once (storage classes), others can occur many times without affecting the meaning.  For example `const int int const` is equivalent to just `const int`.

And the other hard part: the "real" types consist of one or more of the type specifiers in whatever order.  For example, `unsigned short` or `short unsigned` or `unsigned short int` all specify the same type, usually a 16-bit unsigned integer type on modern hardware.

Some combinations are abbreviations: `unsigned` = `unsigned int`, `short` = `short int`, etc.

Some combinations are invalid: `unsigned float`, `short long`, `signed unsigned`, for example.


What are the abstractions used in the code extract?
---
The main abstraction is the "list".  It is just a memory allocated array (a "vector") of pointers to list elements.  Each list type has two functions: `xxx_list_new()` and `xxx_list_append()`.  The append function uses the standard trick of doubling the size of the allocated array every time it runs out of space -- this gives us an amortized O(1) cost.

The parser builds a complete AST (Abstract Syntax Tree) before any kind of analysis happens.  Lists are used heavily as part of the "skeleton" of that tree.  For example, there is a list of type atoms for each declaration.

The extract doesn't have a parser and it only has the definitions for a tiny, tiny part of the AST, namely the type atom list.

There is a function for building a type atom list, one for printing out the content of a type atom list, and one for *interpreting and validating a type atom list*.  The last one is the interesting one!


What is the trick?
---
The trick is to use bits!  Each type atom already has a unique number (which happens to be less than 31) so I also give it a specific bit: `1 << type_atom_number`.

The core function in the extract -- `elab_decls()` -- loops over a type atom list while building up a bit vector of the type atoms it has seen.

It detects conflicts by building up a "conflict vector" of bits representing future type atoms that would conflict with the ones it has already seen.

This conflict vector doesn't start as `0`.  It is passed in from the outside because some type atoms are not allowed in all the contexts where there can be declarations.

Finally, the actual type is looked up in a table of known types.


Why is this not obvious?
---
Because the C99 standard is not written to make it obvious.  The rules about which type atoms can be repeated and which ones can't are stated on different pages of the standard.  The rules about which type atoms together constitute C types are written on yet other pages.

I expect most C compilers to use a variation of the method outlined here.

Some C compilers get it slightly wrong.  For example, `tcc` and `clang` have bugs in this area.