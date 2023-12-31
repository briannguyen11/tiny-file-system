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

dump1:
	hexdump -C -v tinyFSDisk

dump2:
	hexdump -C -v tinyFSDiskRand

clean:
	rm tinyFSDisk tinyFSDiskRand file1 file2 file3 file4 file5 file6

demo1:
	$(CC) $(CFLAGS) libDisk.c libTinyFS.c tfsTest.c -o  demo1 -lm

new:
	make test
	make run


