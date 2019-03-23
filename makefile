CC=gcc
objects = sitemanagerd.o isalreadyrunning.o watchtime.o bandt.o daemonize.o
headers = isalreadyrunning.h constants.h watchtime.h bandt.h daemonize.h

prog : $(objects)
	$(CC) -o sitemanagerd $(objects) -lrt
	cp ./sitemanagerd /usr/sbin/sitemanagerd
	
sitemanagerd.o : sitemanagerd.c $(headers)
	$(CC) -c sitemanagerd.c
	
isalreadyrunning.o : isalreadyrunning.c
	$(CC) -c isalreadyrunning.c
	
watchtime.o : watchtime.c
	$(CC) -c watchtime.c
	
bandt.o : bandt.c
	$(CC) -c bandt.c
	
daemonize.o : daemonize.c
	$(CC) -c daemonize.c
	
clean :
	rm sitemanagerd sitemanagerd.o isalreadyrunning.o watchtime.o bandt.o daemonize.o