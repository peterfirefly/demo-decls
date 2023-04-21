# run from cc/ directory

###

#CC=ccache clang
#CC=ccache clang-13 -fPIE
#CC=ccache clang-12 -fPIE
#CC=ccache clang-11 -fPIE
#CC=ccache clang-10 -fPIE
#CC=ccache gcc-12
#CC=ccache gcc-11
#CC=ccache gcc-10
#CC=ccache gcc-9
CC=clang
#LTO=-flto
#SANITIZE=-fsanitize=undefined
#CFLAGS=-W -Wall -Wmissing-prototypes -Wstrict-prototypes -std=c99 -O2 ${SANITIZE} -g -ggdb -gsplit-dwarf
CFLAGS=-W -Wall -Wmissing-prototypes -Wstrict-prototypes -std=c99 -O2 ${LTO} ${SANITIZE} -g -ggdb
#CFLAGS=-W -Wall -Wmissing-prototypes -Wstrict-prototypes -std=c99 -O1 ${LTO} ${SANITIZE} -Isrc

#LD=gcc -fuse-ld=mold
#LD=gcc
#LD=clang-14
LD=clang
LDFLAGS=${LTO} ${SANITIZE} -g -Wl,--as-needed
#LDFLAGS=${LTO} ${SANITIZE} -Wl,--as-needed

###

.PHONY:	help
help:
	@echo "make test"

.PHONY:	all
all:	test

.PHONY:	test
test:	test-decls

# compiler
test-decls.o:	test-decls.c 		comp-ast.h comp-util.h comp-print.h
	@echo "[CC] test-decls"
	@${CC} -c ${CFLAGS} $< -o $@

comp-ast.o:	comp-ast.c		comp-ast.h comp-util.h
	@echo "[CC] comp-ast"
	@${CC} -c ${CFLAGS} $< -o $@

comp-print.o:	comp-print.c		comp-ast.h comp-util.h comp-print.h
	@echo "[CC] comp-print"
	@${CC} -c ${CFLAGS} $< -o $@

test-decls:	test-decls.o comp-ast.o comp-print.o
	@echo "[LINK]"
	@${LD} ${LDFLAGS} $^ -o $@

.PHONY:	clean
clean:
	@rm -f *.o *.dwo
	@rm -f a.out test-decls
