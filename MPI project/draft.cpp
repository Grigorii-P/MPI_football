//#include "mpi.h"
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>
#include <string.h>

#define NUM_Players 22

void assign_borders(int *myBoarders, int rank);

int main(int argc, char *argv[])
{
    int a[4];
    assign_borders(a, 24);
//    int rank, ma=-1, mb=-1;
//    MPI_Group MPI_GROUP_WORLD, group_a, group_b;
//    MPI_Comm comm_a, comm_b;
//
//    MPI_Init(&argc, &argv);
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    MPI_Comm_group(MPI_COMM_WORLD, &MPI_GROUP_WORLD);
//
//    static int list_a[] = {0, 1};
//    static int list_b[] = {0, 2 ,3};
//
//    int size_list_a = sizeof(list_a)/sizeof(int);
//    int size_list_b = sizeof(list_b)/sizeof(int);
//
//
//    MPI_Group_incl(MPI_GROUP_WORLD, size_list_a, list_a, &group_a);
//    MPI_Group_incl(MPI_GROUP_WORLD, size_list_b, list_b, &group_b);
//
//
//    MPI_Comm_create(MPI_COMM_WORLD, group_a, &comm_a);
//    MPI_Comm_create(MPI_COMM_WORLD, group_b, &comm_b);
//
//
//    if(comm_a != MPI_COMM_NULL)
//        MPI_Comm_rank(comm_a, &ma);
//    if(comm_b != MPI_COMM_NULL)
//        MPI_Comm_rank(comm_b, &mb);
//
////    if(comm_b != MPI_COMM_NULL) {
//    printf("Old rank [%d], new rank [%d]\n",rank,mb);
////    }
//
//    MPI_Finalize();
    
}

void assign_borders(int *myBoarders, int rank) {
    rank -= NUM_Players * 2;
    int row_shift, col_shift = rank % 3;
    if (rank < 4)
        row_shift = 0;
    else if (rank >= 4 && rank < 8)
        row_shift = 1;
    else row_shift = 2;
    myBoarders[0] = row_shift * 32;
    myBoarders[1] = (row_shift + 1) * 32 - 1;
    myBoarders[2] = col_shift * 32;
    myBoarders[3] = (col_shift + 1) * 32 - 1;
}

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


