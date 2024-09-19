CFLAGS=-Wall -std=c11

output: file_io.o formatter.o common.o uniplex_exec.o multiplex_exec.o main.o
	$(CC) $(CFLAGS) $^ -o ppage

debug: file_io_dbg.o formatter_dbg.o common_dbg.o uniplex_exec_dbg.o multiplex_exec_dbg.o main_dbg.o
	$(CC) $(CFLAGS) -g $^ -o ppage_dbg

main.o: main.c file_io.h
	$(CC) $(CFLAGS) -c $<

main_dbg.o: main.c file_io.h
	$(CC) $(CFLAGS) -c -g $< -o $@

common.o: common.c common.h file_io.h
	$(CC) $(CFLAGS) -c $<

common_dbg.o: common.c common.h file_io.h
	$(CC) $(CFLAGS) -c -g $<

uniplex_exec.o: uniplex_exec.c uniplex_exec.h common.h formatter.h file_io.h
	$(CC) $(CFLAGS) -c $<

uniplex_exec_dbg.o: uniplex_exec.c uniplex_exec.h common.h formatter.h file_io.h
	$(CC) $(CFLAGS) -c $<

multiplex_exec.o: multiplex_exec.c multiplex_exec.h common.h formatter.h file_io.h
	$(CC) $(CFLAGS) -c $<

multiplex_exec_dbg.o: multiplex_exec.c multiplex_exec.h common.h formatter.h file_io.h
	$(CC) $(CFLAGS) -c -g $<

file_io.o: file_io.c file_io.h
	$(CC) $(CFLAGS) -c $<

file_io_dbg.o: file_io.c file_io.h
	$(CC) $(CFLAGS) -c -g $< -o $@

formatter.o: formatter.c formatter.h file_io.h
	$(CC) $(CFLAGS) -c $<

formatter_dbg.o: formatter.c formatter.h file_io.h
	$(CC) $(CFLAGS) -c -g $< -o $@

install: output
	cp -v ppage /usr/local/bin/ppage

clean:
	rm -v *.o ppage ppage_dbg 2> /dev/null || true

