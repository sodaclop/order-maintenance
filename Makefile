BASE_CFLAGS=-std=c99 -pedantic -W -Wall
CFLAGS=$(BASE_CFLAGS) -O0
CWARN_FLAGS=$(BASE_CFLAGS) -O -Wuninitialized

# TODO: .c files #include and depend upon .h files.
%.o: %.c Makefile
	$(CC) -c $(CWARN_FLAGS) $(CPPFLAGS) $< -o $@
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.PHONY: all

all: baseamort.o