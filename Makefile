CC = gcc
CFLAGS = -Wall -g -std=c99
PROG = tinyFSDemo
OBJS = tinyFSDemo.o libTinyFS.o libDisk.o

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

tinyFsDemo.o: tinyFSDemo.c libTinyFS.h tinyFS.h TinyFS_errno.h
	$(CC) $(CFLAGS) -c -o $@ $<

libTinyFS.o: libTinyFS.c libTinyFS.h tinyFS.h libDisk.h libDisk.o TinyFS_errno.h
	$(CC) $(CFLAGS) -c -o $@ $<

libDisk.o: libDisk.c libDisk.h tinyFS.h TinyFS_errno.h
	$(CC) $(CFLAGS) -c -o $@ $<


# Additional Targets

diskTest: 
	$(CC) $(CFLAGS) libDisk.c diskTest.c -o diskTest

runDisk:
	./diskTest

cleanDisk: 
	rm disk0.dsk disk1.dsk disk2.dsk disk3.dsk

test:
	$(CC) $(CFLAGS) libDisk.c libTinyFS.c myTfsTest.c -o  myTfsTest -lm

run:
	./myTfsTest

dump:
	hexdump -C -v tinyFSDisk

clean:
	rm tinyFSDisk tinyFSDiskRand file1 file2 file3


