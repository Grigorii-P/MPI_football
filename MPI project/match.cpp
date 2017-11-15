//TODO: удалить ненужные библиотеки
#include "mpi.h"
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>
#include <string.h>

#define ROWS 96
#define COLS 128
#define ROW_BALL_INIT_POS ROWS/2
#define COL_BALL_INIT_POS COLS/2
#define NUM_ROUNDS 5
#define NUM_STEPS 10
#define NUM_Fields 12
#define NUM_Players 10

void fill_ranks(int *newRanks, int flag);
void assign_skills(int &speed, int &dribbling, int &kickPower, int rank);
void assign_boarders(int *myBoarders ,int rank);
bool is_ball_im_my_borders(int *myBorders, int *ballPosition);
int get_field_index_with_ball(bool *ballPossessionIndex);

int main(int argc, char *argv[])  {
    int count=0;
    int rank, rankA=-1, rankB=-1, rankAF=-1, rankBF=-1, numtasks;
    int speed, dribbling, kickPower;
    bool IGotBall = false;
    int fieldWithBallIndex = -1;
    int myBoarders[4];
    int teamA[NUM_Players];
//    int teamA[] = {0,1,2,3,4,5,6,7,8,9};
    int teamB[NUM_Players];// = {10,11,12,13,14,15,16,17,18,19};
    int teamAandFields[NUM_Players+NUM_Fields];// = {0,1,2,3,4,5,6,7,8,9,20,21,22,23,24,25,26,27,28,29,30,31};
    int teamBandFields[NUM_Players+NUM_Fields];// = {10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    int ballPosition[2] = {ROW_BALL_INIT_POS, COL_BALL_INIT_POS};
    bool ballPossessionIndex[NUM_Players+NUM_Fields];
    
    
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    MPI_Group world_group, teamA_group, teamB_group, teamAandFields_group, teamBandFields_group;
    MPI_Comm teamA_COMM, teamB_COMM, teamAandFields_COMM, teamBandFields_COMM;

    fill_ranks(teamA, 0);
    fill_ranks(teamB, 1);
    fill_ranks(teamAandFields, 2);
    fill_ranks(teamBandFields, 3);

    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    
    MPI_Group_incl(world_group, NUM_Players, teamA, &teamA_group);
    MPI_Group_incl(world_group, NUM_Players, teamB, &teamB_group);
    MPI_Group_incl(world_group, NUM_Players + NUM_Fields, teamAandFields, &teamAandFields_group);
    MPI_Group_incl(world_group, NUM_Players + NUM_Fields, teamBandFields, &teamBandFields_group);
    MPI_Comm_create(MPI_COMM_WORLD, teamA_group, &teamA_COMM);
    MPI_Comm_create(MPI_COMM_WORLD, teamB_group, &teamB_COMM);
    MPI_Comm_create(MPI_COMM_WORLD, teamAandFields_group, &teamAandFields_COMM);
    MPI_Comm_create(MPI_COMM_WORLD, teamBandFields_group, &teamBandFields_COMM);
    
    if(teamA_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamA_COMM, &rankA);
    if(teamB_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamB_COMM, &rankB);
    if(teamAandFields_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamAandFields_COMM, &rankAF);
    if(teamBandFields_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamBandFields_COMM, &rankBF);
    
    

    




    
//    if (rank < 22)
//        assign_skills(speed, dribbling, kickPower, rank);
//    if (rank >= 22) {
//        assign_boarders(myBoarders, rank);
//    }
//
//
//    // FOR LOOP
//
//
//    if (rank >= 22)
//        IGotBall = is_ball_im_my_borders(myBoarders, ballPosition);
//    //можно и teamBandFields_COMM использовать, в данном случае разницы нет
//    //TODO: IGotBall надо обнулять в конце раунда !!!
//    MPI_Allgather(&IGotBall, 1, MPI_CXX_BOOL, ballPossessionIndex, 1, MPI_CXX_BOOL, teamAandFields_COMM);
////    MPI_Allgather(&IGotBall, 1, MPI_CXX_BOOL, ballPossessionIndex, 1, MPI_CXX_BOOL, teamBandFields_COMM);
//
//    if (rank >= 22)
//        fieldWithBallIndex = get_field_index_with_ball(ballPossessionIndex);
//    if (rankAF == fieldWithBallIndex) {
//        printf("ball was at position (%d,%d)\n",ballPosition[0], ballPosition[1]);
//        ballPosition[0]=33;
//        ballPosition[1]=33;
//    }
//
//    MPI_Bcast(ballPosition, 2, MPI_INT, rankAF, teamAandFields_COMM);
//    MPI_Bcast(ballPosition, 2, MPI_INT, rankBF, teamBandFields_COMM);
//
//    if (rank==3)
//        printf("I'm player [%d] and I know that ball is at (%d,%d) position\n",rank,ballPosition[0],ballPosition[1]);


    //TODO: освободить память из-под коммуникаторов
//    if(comm_a != MPI_COMM_NULL)
//        MPI_Comm_free(&comm_a);
//    if(comm_b != MPI_COMM_NULL)
//        MPI_Comm_free(&comm_b);
//    MPI_Group_free(&group_a);
//    MPI_Group_free(&group_b);
//    MPI_Group_free(&MPI_GROUP_WORLD);

    MPI_Finalize();
}


void fill_ranks(int *newRanks, int flag) {
    switch (flag) {
        case 0:
            for (int i = 0; i < NUM_Players; i++) {
                newRanks[i]=i;
            }
            break;
        case 1:
            for (int i = 0; i < NUM_Players; i++) {
                newRanks[i]=i+NUM_Players;
            }
            break;
        case 2:
            for (int i = 0; i < NUM_Players; i++) {
                newRanks[i]=i;
            }
            for (int i = NUM_Players; i < NUM_Players+NUM_Fields; i++) {
                newRanks[i]=i+NUM_Players;
            }
            break;
        case 3:
            for (int i = 0; i < NUM_Players; i++) {
                newRanks[i]=i+NUM_Players;
            }
            for (int i = NUM_Players; i < NUM_Players+NUM_Fields; i++) {
                newRanks[i]=i+NUM_Players;
            }
            break;
        default:
            break;
    }
}

void assign_skills(int &speed, int &dribbling, int &kickPower, int rank) {
    srand(rank);
    speed = (rand() % 10) + 1;
    srand(rank+50);
    dribbling = (rand() % 10) + 1;
    kickPower = 15 - speed - dribbling;
}

void assign_boarders(int *myBoarders ,int rank) {
    rank -= NUM_Players*2;
    int row_shift = rank % 4;
    int col_shift = rank % 3;
    myBoarders[0] = col_shift * 32;
    myBoarders[1] = (col_shift + 1) * 32 - 1;
    myBoarders[2] = row_shift * 32;
    myBoarders[3] = (row_shift + 1) * 32 - 1;
}

bool is_ball_im_my_borders(int *myBorders, int *ballPosition) {
    if (ballPosition[0] > myBorders[0] && ballPosition[0] < myBorders[1] && ballPosition[1] > myBorders[3] && ballPosition[1] < myBorders[4])
        return true;
    else return false;
}

int get_field_index_with_ball(bool *ballPossessionIndex) {
    int ind = -1;
    //TODO: можно создать отдельный коммуникатор только для fields, в нем искать индекс а потом использовать в teamAandFields для передачи индекса
    for (int i = NUM_Players; i < NUM_Players+NUM_Fields; i++) {
        if (ballPossessionIndex[i] == true) {
            ind = i;
            break;
        }
    }
    return ind;
}
