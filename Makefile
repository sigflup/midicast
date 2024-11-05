CC = cc
AR = ar
RANLIB = ranlib
INSTALL = install
PREFIX = /usr/local

all: midicast libmidicli.a
	@echo "all is good"

install: all
	${INSTALL} midicast ${PREFIX}/bin
	${INSTALL} libmidicli.a ${PREFIX}/lib
	${INSTALL} midicli.h ${PREFIX}/include
	${INSTALL} midicast.1 ${PREFIX}/man/man1
	makewhatis ${PREFIX}/man

midicast: link.o serv.o
	${CC} link.o serv.o -o midicast

libmidicli.a: midicli.o
	rm -rf libmidicli.a
	${AR} -q -v libmidicli.a midicli.o
	${RANLIB} libmidicli.a

midicli.o: midicli.c midicli.h
	${CC} -c midicli.c 

link.o: link.c link.h
	${CC} -c link.c

serv.o: serv.c msg.h
	${CC} -c serv.c

clean:
	rm -f libmidicli.a midicli.o serv.o link.o midicast
