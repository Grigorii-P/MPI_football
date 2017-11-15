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
#define NUM_AB_and_F (NUM_Players + NUM_Fields)

void fill_ranks(int *newRanks, int flag);
void assign_position(int *myPosition);
void assign_skills(int &speed, int &dribbling, int &kickPower, int rank);
void assign_borders(int *myBoarders, int rank);
bool is_ball_in_my_borders(int *myBorders, int *ballPosition);
int get_field_index_with_ball(bool *ballPossessionIndex);
void my_action(int *ballPosition, int *myPosition, int *allPlayersPositions);

int main(int argc, char *argv[])  {
    int rank, rankA = -1, rankB = -1, rankF = -1, rankAF = -1, rankBF = -1, numtasks;
    int speed, dribbling, kickPower;
    bool IGotBall = false;
    int fieldWithBallIndex = -2;
    bool ballPossessionIndex[NUM_Fields];
    int myPosition[2];
    int myBorders[4];
    int teamA[NUM_Players];
    int teamB[NUM_Players];
    int fields[NUM_Fields];
    int teamAandFields[NUM_AB_and_F];
    int teamBandFields[NUM_AB_and_F];
    int ballPosition[2] = {ROW_BALL_INIT_POS, COL_BALL_INIT_POS};
    int allPlayersPositions[NUM_Players * 2];
    //TODO: как сделать так, чтоб воспринимался NUM_Players вместо NUM_AB_and_F
    int allPlayersPositions_A[NUM_AB_and_F * 2];
    int allPlayersPositions_B[NUM_AB_and_F * 2];
    
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    MPI_Group world_group, teamA_group, teamB_group, teamAandFields_group, teamBandFields_group;
    MPI_Comm teamA_COMM, teamB_COMM, teamAandFields_COMM, teamBandFields_COMM;

    fill_ranks(teamA, 0);
    fill_ranks(teamB, 1);
    fill_ranks(fields, 2);
    fill_ranks(teamAandFields, 3);
    fill_ranks(teamBandFields, 4);

    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Group_incl(world_group, NUM_Players, teamA, &teamA_group);
    MPI_Group_incl(world_group, NUM_Players, teamB, &teamB_group);
//    MPI_Group_incl(world_group, NUM_Fields, fields, &fields_group);
    MPI_Group_incl(world_group, NUM_Players + NUM_Fields, teamAandFields, &teamAandFields_group);
    MPI_Group_incl(world_group, NUM_Players + NUM_Fields, teamBandFields, &teamBandFields_group);
    MPI_Comm_create(MPI_COMM_WORLD, teamA_group, &teamA_COMM);
    MPI_Comm_create(MPI_COMM_WORLD, teamB_group, &teamB_COMM);
//    MPI_Comm_create(MPI_COMM_WORLD, fields_group, &fields_COMM);
    MPI_Comm_create(MPI_COMM_WORLD, teamAandFields_group, &teamAandFields_COMM);
    MPI_Comm_create(MPI_COMM_WORLD, teamBandFields_group, &teamBandFields_COMM);
    
    if(teamA_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamA_COMM, &rankA);
    if(teamB_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamB_COMM, &rankB);
//    if(fields_COMM != MPI_COMM_NULL)
//        MPI_Comm_rank(fields_COMM, &rankF);
    if(teamAandFields_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamAandFields_COMM, &rankAF);
    if(teamBandFields_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamBandFields_COMM, &rankBF);
    
    if (rank < NUM_Players * 2) {
        assign_skills(speed, dribbling, kickPower, rank);
        assign_position(myPosition);
    }
    
    if (rank >= NUM_Players * 2)
        assign_borders(myBorders, rank);

    // FOR LOOP ROUNDS --------------------------------------------------------

    // detect whether the ball is in my (I'm a field) borders
    if (rank >= NUM_Players * 2)
        IGotBall = is_ball_in_my_borders(myBorders, ballPosition);

    //можно и teamBandFields_COMM использовать, в данном случае разницы нет
    //TODO: IGotBall надо обнулять в конце раунда !!!
    //TODO: почему без if не работает?? их так много в коде!
    // fields and players collect info on who possesses the ball and who does not
    if(teamAandFields_COMM != MPI_COMM_NULL)
        MPI_Allgather(&IGotBall, 1, MPI_CXX_BOOL, ballPossessionIndex, 1, MPI_CXX_BOOL, teamAandFields_COMM);
    if(teamBandFields_COMM != MPI_COMM_NULL)
        MPI_Allgather(&IGotBall, 1, MPI_CXX_BOOL, ballPossessionIndex, 1, MPI_CXX_BOOL, teamBandFields_COMM);

    // players receive the rank of a field with the ball
    if (rankAF != -1)
        fieldWithBallIndex = get_field_index_with_ball(ballPossessionIndex);
    if (rankBF != -1)
        fieldWithBallIndex = get_field_index_with_ball(ballPossessionIndex);

    // having 'fieldWithBallIndex' received, players know who they collect info on 'ballPosition' from;
    // players receive 'ballPosition'
    if(teamAandFields_COMM != MPI_COMM_NULL)
        MPI_Bcast(ballPosition, 2, MPI_INT, fieldWithBallIndex, teamAandFields_COMM);
    if(teamBandFields_COMM != MPI_COMM_NULL)
        MPI_Bcast(ballPosition, 2, MPI_INT, fieldWithBallIndex, teamBandFields_COMM);
    
    // each player receives positions of all the other team mates
    if(teamA_COMM != MPI_COMM_NULL)
        MPI_Allgather(myPosition, 2, MPI_INT, allPlayersPositions, 2, MPI_INT, teamA_COMM);
    if(teamB_COMM != MPI_COMM_NULL)
        MPI_Allgather(myPosition, 2, MPI_INT, allPlayersPositions, 2, MPI_INT, teamB_COMM);
    
    // players take decision on what they do next - stay unmoved, move, or give a pass
    if (rank < NUM_Players * 2) {
        my_action(ballPosition, myPosition, allPlayersPositions);
    }
    
    // collect positions of all players in 'allPlayersPositionsA/B' arrays, so a field process can decide on winner;
    // even though players know each others NEW position (it's for more sound implementation of algorithm), they don't use it but rather receive it in next round
    if (teamAandFields_COMM != MPI_COMM_NULL)
        MPI_Allgather(myPosition, 2, MPI_INT, allPlayersPositions_A, 2, MPI_INT, teamAandFields_COMM);
    if (teamBandFields_COMM != MPI_COMM_NULL)
        MPI_Allgather(myPosition, 2, MPI_INT, allPlayersPositions_B, 2, MPI_INT, teamBandFields_COMM);
    
    
    
// ------------------------------------------------------------------------------------
    
    // lets free groups and communicators
    if(teamA_COMM != MPI_COMM_NULL)
        MPI_Comm_free(&teamA_COMM);
    if(teamB_COMM != MPI_COMM_NULL)
        MPI_Comm_free(&teamB_COMM);
    if(teamAandFields_COMM != MPI_COMM_NULL)
        MPI_Comm_free(&teamAandFields_COMM);
    if(teamBandFields_COMM != MPI_COMM_NULL)
        MPI_Comm_free(&teamBandFields_COMM);
    MPI_Group_free(&teamA_group);
    MPI_Group_free(&teamB_group);
    MPI_Group_free(&teamAandFields_group);
    MPI_Group_free(&teamBandFields_group);
    
    MPI_Finalize();
}


void fill_ranks(int *newRanks, int flag) {
    switch (flag) {
        case 0:
            for (int i = 0; i < NUM_Players; i++) {
                newRanks[i] = i;
            }
            break;
        case 1:
            for (int i = 0; i < NUM_Players; i++) {
                newRanks[i] = i + NUM_Players;
            }
            break;
        case 2:
            for (int i = 0; i < NUM_Fields; i++) {
                newRanks[i] = i + NUM_Players * 2;
            }
            break;
        case 3:
            for (int i = 0; i < NUM_Players; i++) {
                newRanks[i] = i;
            }
            for (int i = NUM_Players; i < NUM_AB_and_F; i++) {
                newRanks[i] = i + NUM_Players;
            }
            break;
        case 4:
            for (int i = 0; i < NUM_Players; i++) {
                newRanks[i] = i + NUM_Players;
            }
            for (int i = NUM_Players; i < NUM_AB_and_F; i++) {
                newRanks[i] = i + NUM_Players;
            }
            break;
        default:
            break;
    }
}

void assign_position(int *myPosition) {
    myPosition[0] = 7;
    myPosition[1] = 9;
}

void assign_skills(int &speed, int &dribbling, int &kickPower, int rank) {
    srand(rank);
    speed = (rand() % 10) + 1;
    srand(rank + rand());
    dribbling = (rand() % 10) + 1;
    kickPower = 15 - speed - dribbling;
}

void assign_borders(int *myBoarders, int rank) {
    rank -= NUM_Players * 2;
    int row_shift, col_shift = rank % 4;
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

bool is_ball_in_my_borders(int *myBorders, int *ballPosition) {
    //TODO: что если мяч попадет на крайнюю позицию справа или внизу -> он будет на поле, но ни одно поле его не увидит
    if (ballPosition[0] >= myBorders[0] &&
        ballPosition[0] < myBorders[1] &&
        ballPosition[1] >= myBorders[2] &&
        ballPosition[1] < myBorders[3])
        return true;
    else return false;
}

int get_field_index_with_ball(bool *ballPossessionIndex) {
    int ind = -1;
    //TODO: можно создать отдельный коммуникатор только для fields, в нем искать индекс, а потом использовать в teamAandFields для передачи индекса
    for (int i = NUM_Players; i < NUM_AB_and_F; i++) {
        if (ballPossessionIndex[i] == true) {
            ind = i;
            break;
        }
    }
    return ind;
}

void my_action(int *ballPosition, int *myPosition, int *allPlayersPositions) {
    myPosition[0]=1;
    myPosition[1]=2;
}


















