PROG=	feedrandom
SRCS=	feedrandom.c
OBJS=	feedrandom.o

CC=		cc
CFLAGS+= -O2 -pipe -std=c99 -pedantic -Wall

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PROG) $(OBJS)
