INC=-Iinclude
CFLAGS=$(INC)

PARSERDIR=input_parse

SRCS=utils.c mshell.c builtins.c
OBJS:=$(SRCS:.c=.o)

all: mshell 

mshell: $(OBJS) siparse.a
	cc $(CFLAGS) $(OBJS) siparse.a -o $@ -w

%.o: %.c
	cc $(CFLAGS) -c $< -w

siparse.a:
	$(MAKE) -C $(PARSERDIR)

clean:
	rm -f mshell *.o
