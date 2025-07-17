CC = gcc
MPICC = mpicc
CFLAGS = -O2 -Wall

all: odd_even_serial odd_even_openmp odd_even_mpi qsort_serial

odd_even_serial: odd_even_serial.c
	$(CC) $(CFLAGS) -o odd_even_serial odd_even_serial.c

odd_even_openmp: odd_even_openmp.c
	$(CC) $(CFLAGS) -fopenmp -o odd_even_openmp odd_even_openmp.c

odd_even_mpi: odd_even_mpi.c
	$(MPICC) $(CFLAGS) -o odd_even_mpi odd_even_mpi.c

qsort_serial: qsort_serial.c
	$(CC) $(CFLAGS) -o qsort_serial qsort_serial.c

clean:
	rm -f odd_even_serial odd_even_openmp odd_even_mpi qsort_serial

test: all
	./odd_even_serial 1000
	./odd_even_openmp 1000 2
	mpirun -np 2 ./odd_even_mpi 1000
	./qsort_serial 1000
