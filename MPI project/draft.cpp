#include "mpi.h"
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>
#include <string.h>

using namespace std;
int main(int argc, char *argv[])
{
    
//    int myid, numprocs;
//    int color, broad_val, new_id, new_nodes;
//    MPI_Comm New_Comm;
//    MPI_Init(&argc, &argv);
//    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
//    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
//    color = myid%2;
//    MPI_Comm_split(MPI_COMM_WORLD, color, myid, &New_Comm);
//    MPI_Comm_rank(New_Comm, &new_id);
//    MPI_Comm_size(New_Comm, &new_nodes);
//
//    if(new_id == 0) broad_val = color;
//    MPI_Bcast(&broad_val, 1, MPI_INT, 0, New_Comm);
//    printf("Old_proc[%d] has new rank %d, received value %d\n",myid,new_id,broad_val);
//    MPI_Finalize();
    
    
    
    //    if (rank < 11) {
    //        color = 0;
    //        MPI_Comm_split(MPI_COMM_WORLD, color, rank, &MyTeam_COMM);
    //        MPI_Comm_split(MPI_COMM_WORLD, color, rank, &teamAandFields_COMM);
    //    }
    //    else if (rank >= 11 && rank < 22) {
    //        color = 1;
    //        MPI_Comm_split(MPI_COMM_WORLD, color, rank, &MyTeam_COMM);
    //        MPI_Comm_split(MPI_COMM_WORLD, color, rank, &teamBandFields_COMM);
    //    }
    //    else {
    //        color = 0;
    //        MPI_Comm_split(MPI_COMM_WORLD, color, rank, &teamAandFields_COMM);
    //        color = 1;
    //        MPI_Comm_split(MPI_COMM_WORLD, color, rank, &teamBandFields_COMM);
    //    }
    
    
    
}
