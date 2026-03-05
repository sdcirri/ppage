CC       = clang
CFLAGS   = -Wall -Wextra -std=c11
RELFLAGS = -O2 -D_FORTIFY_SOURCE=3 -fstack-protector-strong -fPIE
LDFLAGS  = -pie -Wl,-z,relro -Wl,-z,now
DBGFLAGS = -g -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address,undefined -fsanitize=alignment,bounds,null -fno-sanitize=leak

SRC      = main.c file_io.c formatter.c common.c uniplex_exec.c multiplex_exec.c
OBJ      = $(SRC:.c=.o)
DBGOBJ   = $(SRC:.c=_dbg.o)

.PHONY: output debug clean install

output: $(OBJ)
	$(CC) $(CFLAGS) $(RELFLAGS) $^ -o ppage $(LDFLAGS)

debug: $(DBGOBJ)
	$(CC) $(CFLAGS) $(DBGFLAGS) $^ -o ppage_dbg

%.o: %.c
	$(CC) $(CFLAGS) -O2 -c $< -o $@

%_dbg.o: %.c
	$(CC) $(CFLAGS) $(DBGFLAGS) -c $< -o $@

install: output
	cp -v ppage /usr/local/bin/ppage

clean:
	rm -v *.o ppage ppage_dbg 2> /dev/null || true

