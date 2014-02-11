# 
#SE 3BB4, assignment 1
#student name: Matthew Schweitzer
#student number: 1139402
#file Makefile
#
CC=gcc
CFLAGS=-c #-Wall
SOURCES=assgn1.c job.c log.c
EXECUTABLE=assgn1 job

all: $(EXECUTABLE)

assgn1: assgn1.o log.o
	$(CC) assgn1.o log.o -o assgn1

assgn1.o: assgn1.c
	$(CC) $(CFLAGS) assgn1.c

job: job.o log.o
	$(CC) job.o log.o -o job

job.o: job.c
	$(CC) $(CFLAGS) job.c

log.o: log.c
	$(CC) $(CFLAGS) log.c

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf  *.o assgn1 job log