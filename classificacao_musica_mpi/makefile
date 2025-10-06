all:
	mpicc -O2 -o sentiment_mpi sentiment_mpi.c

run:
	mpirun -np 4 ./sentiment_mpi

clean:
	rm -f sentiment_mpi
