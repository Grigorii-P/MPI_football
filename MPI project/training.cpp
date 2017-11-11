#include "mpi.h"
#include <stdio.h>

#define X_field 128
#define Y_filed 64

int main(int argc, char *argv[])  {
    int numtasks, rank;
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    MPI_Finalize();
}

