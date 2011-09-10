CC = $(CROSS)$(TARGET)gcc
LINK = $(CROSS)$(TARGET)ld -o
LIBRARY_LINK_OPTS =  -L. -r
CFLAGS = -ggdb -Wall -Wextra -Wshadow -Wformat-security -O2
RM = /bin/rm -f
Q=@

OBJS = queue.o list.o cbuf.o io.o log.o http_response.o asyncdns.o server.o misc.o
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
