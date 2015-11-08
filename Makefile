PROG=	feedrandom
SRCS=	feedrandom.c sha512.c sha512-api.c
OBJS=	feedrandom.o sha512.o sha512-api.o

CC=		cc
CFLAGS+= -O2 -pipe -std=c99 -pedantic -Wall

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PROG) $(OBJS)
