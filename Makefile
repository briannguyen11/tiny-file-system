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

disktest: 
	$(CC) $(CFLAGS) libDisk.c diskTest.c -o diskTest

run:
	./diskTest

clean: 
	rm disk0.dsk disk1.dsk disk2.dsk disk3.dsk

tfstest:
	$(CC) $(CFLAGS) libDisk.c libTinyFS.c myTfsTest.c -o boom