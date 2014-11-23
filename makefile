CC = gcc
CFLAGS = -Wall -g
SRCS = simhttp.c
OBJS = $(SRCS:.c=.o)

all: simhttp

simhttp: simhttp.o
	$(CC) $(CFLAGS) -o simhttp simhttp.c

simhttp.o: simhttp.c simhttp.h
	$(CC) $(CFLAGS) -c simhttp.c


.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: depend clean

clean:
	$(RM) simhttp *.o 

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE

