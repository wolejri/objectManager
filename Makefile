
CC = clang++
CFLAGS = -g -Wall

PROG = unitTest
HDRS = ObjectManager.h
SRCS = main0.c objectManager.c
MAIN = main0.c
OBJS = main0.o objectManager.o

# compiling rules

# WARNING: *must* have a tab before each definition

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG)

object/objectManager.o: objectManager.c $(HDRS)
	$(CC) $(CFLAGS) -c objectManager.c -o objectManager.o

object/main0.o: $(MAIN) $(HDRS)
	$(CC) $(CFLAGS) -c $(MAIN) -o main0.o

clean:
	rm -f $(PROG) $(OBJS)


