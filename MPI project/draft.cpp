#include "mpi.h"
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>
#include <string.h>

#define NUM_Players 11

void make_step(int *myPosition, int *ballPosition, int speed);
void assign_position(int *myPosition, int *myFieldSector, int rank, bool checkTeam, int half);

int main(int argc, char *argv[])
{
    int rank;

    int mp[2];
    int mf[4];
    bool c = 1;
    int h = 1;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    assign_position(mp, mf, rank, c, h);
    printf("rank [%d], field (%d,%d,%d,%d)\n", rank, mf[0],mf[1],mf[2],mf[3]);
    
    MPI_Finalize();
    
    
}

void assign_position(int *myPosition, int *myFieldSector, int rank, bool checkTeam, int half) {
    int shift, col_shift, row_shift = -1;
    bool whichTeam = 0;
    
    if (checkTeam == 0 && half == 0)
        whichTeam = 0;
    if (checkTeam == 0 && half == 1)
        whichTeam = 1;
    if (checkTeam == 1 && half == 0)
        whichTeam = 1;
    if (checkTeam == 1 && half == 1)
        whichTeam = 0;
    
    whichTeam ? shift = 0 : shift = 12;
    col_shift = rank % 2;
    if (rank < 2)
        row_shift = 0;
    else if (rank < 4)
        row_shift = 1;
    else if (rank < 6)
        row_shift = 2;
    else if (rank < 8)
        row_shift = 3;
    else if (rank < 10)
        row_shift = 4;
    
    // MARK: версия для 10 игроков
        myPosition[0] = row_shift * 19;
        myFieldSector[0] = myPosition[0];
        myFieldSector[1] = myPosition[0] + (19-1);
        if (row_shift == 4) {
            myFieldSector[1] = myPosition[0] + 19;
        }
    
        if (!whichTeam && col_shift == 0) {
            myPosition[1] = col_shift * 65;
            myFieldSector[2] = myPosition[1];
            myFieldSector[3] = myFieldSector[2] + 65;
        }
        if (!whichTeam && col_shift == 1) {
            myPosition[1] = col_shift * 65;
            myFieldSector[2] = myPosition[1];
            myFieldSector[3] = myFieldSector[2] + 60;
        }
    
        if (whichTeam && col_shift == 0) {
            myPosition[1] = col_shift * 60;
            myFieldSector[2] = myPosition[1];
            myFieldSector[3] = myFieldSector[2] + 60;
        }
        if (whichTeam && col_shift == 1) {
            myPosition[1] = col_shift * 60;
            myFieldSector[2] = myPosition[1];
            myFieldSector[3] = myFieldSector[2] + 65;
        }
   
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





