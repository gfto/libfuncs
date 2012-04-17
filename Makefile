CC = $(CROSS)$(TARGET)gcc
LINK = $(CROSS)$(TARGET)ld -o
MKDEP = $(CC) -M -o $*.d $<

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
	@$(MKDEP)
	$(Q)echo "  CC	libfuncs	$<"
	$(Q)$(CC) $(CFLAGS) -c $<

-include $(OBJS:.o=.d)

clean:
	$(Q)echo "  RM	$(PROG) $(OBJS:.o=.o) $(OBJS:.o=.d)"
	$(Q)$(RM) $(PROG) $(OBJS:.o=.o) $(OBJS:.o=.d) *~

distclean: clean
