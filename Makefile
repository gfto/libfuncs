CC = $(CROSS)$(TARGET)gcc
LINK = $(CROSS)$(TARGET)ld -o
LIBRARY_LINK_OPTS =  -L. -r
CFLAGS = -O2 -ggdb -std=c99 -D_GNU_SOURCE
CFLAGS += -Wall -Wextra -Wshadow -Wformat-security -Wstrict-prototypes
RM = /bin/rm -f
Q=@

OBJS = queue.o list.o cbuf.o io.o log.o http_response.o asyncdns.o \
       server.o misc.o

PROG = libfuncs.a

all: $(PROG)

$(PROG): $(OBJS) 
	$(Q)echo "  LINK	$(PROG)"
	$(Q)$(LINK) $@ $(LIBRARY_LINK_OPTS) $(OBJS)

%.o: %.c libfuncs.h
	$(Q)echo "  CC	libfuncs	$<"
	$(Q)$(CC) $(CFLAGS) -c $<

clean:
	$(Q)echo "  RM	$(PROG) $(OBJS)"
	$(Q)$(RM) $(PROG) *.o core *.core *~

distclean: clean
