CC = gcc
CFLAGS = -g -O2 -Wall -W
LDFLAGS =
LIBS = -lm

all: wmm_grid

wmm_grid: wmm_grid.o GeomagnetismLibrary.o
	${CC} ${LDFLAGS} -o wmm_grid wmm_grid.o GeomagnetismLibrary.o ${LIBS}

GeomagnetismLibrary.o:
	${CC} ${CFLAGS} -c GeomagnetismLibrary.c

clean:
	rm -f *.o
	rm -f wmm_grid
