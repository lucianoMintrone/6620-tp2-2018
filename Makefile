# Do not edit -- this file documents how this program was built for your machine.
SYSTYPE	= LINUX4
SYSLIBS	= -lm
CC	= gcc $(WARN)
OPT	= -O
DEBUG	= -g
EXPORT	= AUXLIBS='' CCARGS='' OPT='-O' DEBUG='-g'
WARN	= -Wall
SHELL	= /bin/sh
DEFS    = -I. -D$(SYSTYPE)
CFLAGS  = $(DEBUG) $(OPT) $(DEFS)
OPTS	= 'CC=$(CC)'
SRCS	= main.c
PROG	= tp2

$(PROG): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(SYSLIBS)

update: $(PROG)
	:

makefiles Makefiles:
	(echo "# Do not edit -- this file documents how this program was built for your machine."; $(SHELL) makedefs) >makedefs.tmp
	set +e;                                \
	if cmp makedefs.tmp makedefs.out; then \
	    rm makedefs.tmp;                   \
	else                                   \
	    mv makedefs.tmp makedefs.out;      \
	fi >/dev/null 2>/dev/null
	rm -f Makefile; (cat makedefs.out Makefile.in) >Makefile

clean:
	rm -f Makefile
	cp Makefile.init Makefile
	rm -f bin/[!CRS]* lib/[!CRS]* include/[!CRS]* libexec/[!CRS]*     \
	      junk *core .nfs* .pure *.out *.out.* *.tmp *.a *~ *- *.orig \
*.t *.o *.bak make.err *.gmon $(PROG)