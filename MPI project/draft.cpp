#include "mpi.h"
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>
#include <string.h>

#define NUM_Players 11

void make_step(int *myPosition, int *ballPosition, int speed);

int main(int argc, char *argv[])
{
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Finalize();
    
    
}



void make_step(int *myPosition, int *ballPosition, int speed) {
    int row_offset = ballPosition[0]-myPosition[0];
    int col_offset = ballPosition[1]-myPosition[1];
    printf("row_offset %d\n",row_offset);
    printf("col_offset %d\n",col_offset);
    if (abs(row_offset) + abs(col_offset) <= speed) {
        myPosition[0] = ballPosition[0];
        myPosition[1] = ballPosition[1];
    }
    else {
        if (abs(row_offset) <= speed) {
            myPosition[0] += row_offset;
        }
        else {
            if (row_offset < 0)
                myPosition[0] += -1 * speed;
            else
                myPosition[0] += speed;
        }
        
        if (speed - abs(row_offset) > 0) {
            if (abs(col_offset) <= abs(speed - abs(row_offset))) {
                myPosition[1] += col_offset;
            }
            else {
                if (col_offset < 0)
                    myPosition[1] += -1 * abs(speed - abs(row_offset));
                else
                    myPosition[1] += speed - abs(row_offset);
            }
        }
    }
}





