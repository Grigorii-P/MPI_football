//MARK: удалить ненужные библиотеки
#include "mpi.h"
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>
#include <string.h>
#include <cmath>

#define ROWS 96
#define COLS 128
#define ROW_BALL_INIT_POS ROWS/2
#define COL_BALL_INIT_POS COLS/2
#define NUM_ROUNDS 5
#define NUM_STEPS 10
#define NUM_Fields 12
#define NUM_Players 10
#define NUM_AB_and_F (NUM_Players + NUM_Fields)
#define ROUNDS 1
#define HALF 1
#define goalCoordY1 43
#define goalCoordY2 51
#define goalCoordX1 0
#define goalCoordX2 125

void fill_ranks(int *newRanks, int flag);
void assign_position(int *myPosition, int *myFieldSector, int rank, bool checkTeam, int half);
void assign_skills(int &speed, int &dribbling, int &kickPower, int rank);
void assign_borders(int *myBoarders, int rank);
bool is_ball_in_my_borders(int *myBorders, int *ballPosition);
int get_field_index_with_ball(bool *ballPossessionIndex);
void make_step(int *myPosition, int *myFieldSector, int speed, int *ballPosition, int &metersTotal);
void identify_winner(int *allPlayersPositions_A, int *allPlayersPositions_B, int *ballPosition, int *winner);
void fill_winner_arrays(bool *winnerA, bool *winnerB, int *winner);
bool did_win_ball(bool *winners, int rank);
void kick_ball(int *ballPosition, int *myPosition, int *myFieldSector, int *allPlayersPositions, int goalXcoord, int *newBallPosition, int kickPower);
void get_ball_position(int *newBallPositionsA, int *newBallPositionsB, int *ballPosition);

int main(int argc, char *argv[])  {
    int rank, rankA = -1, rankB = -1, rankF = -1, rankAF = -1, rankBF = -1, numtasks;
    int speed = 0, dribbling = 0, kickPower = 0;
    int fieldWithBallIndex = -2;
    int goalXcoord;
    int metersTotal = 0;
    bool IGotBall = false;
    bool didWinBall;
    bool ballPossessionIndex[NUM_Fields];
    int myPosition[2];
    int myFieldSector[4];
    int myBorders[4];
    int teamA[NUM_Players], teamB[NUM_Players], fields[NUM_Fields];
    int teamAandFields[NUM_AB_and_F], teamBandFields[NUM_AB_and_F];
    int ballPosition[2] = {ROW_BALL_INIT_POS, COL_BALL_INIT_POS};
    int newBallPosition[2], newBallPositionsA[NUM_AB_and_F * 2], newBallPositionsB[NUM_AB_and_F * 2];
    int allPlayersPositions_A[NUM_AB_and_F * 2], allPlayersPositions_B[NUM_AB_and_F * 2];
    int winner[2];
    bool winnersA[NUM_Players], winnersB[NUM_Players];
    
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
    
    if (teamA_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamA_COMM, &rankA);
    if (teamB_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamB_COMM, &rankB);
//    if(fields_COMM != MPI_COMM_NULL)
//        MPI_Comm_rank(fields_COMM, &rankF);
    if (teamAandFields_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamAandFields_COMM, &rankAF);
    if (teamBandFields_COMM != MPI_COMM_NULL)
        MPI_Comm_rank(teamBandFields_COMM, &rankBF);
    
    if (rank < NUM_Players * 2) {
        assign_skills(speed, dribbling, kickPower, rank);
    }
    
    if (rank >= NUM_Players * 2)
        assign_borders(myBorders, rank);

    
    for (int j = 0; j < HALF; j++) {
        
        if (rank < NUM_Players * 2) {
            bool checkTeam = (rankAF > -1);
            if (rankAF != -1)
                assign_position(myPosition, myFieldSector, rankAF, checkTeam, j);
            if (rankBF != -1)
                assign_position(myPosition, myFieldSector, rankBF, checkTeam, j);
        }
        
        // TODO: для каждого игрока заполняем координату ворот
//        goalXcoord =
        
        for (int j = 0; j < ROUNDS; j++) {
            
            // TODO: IGotBall надо обнулять в конце раунда !!!
            // detect whether the ball is in my (I'm a field) borders
            if (rank >= NUM_Players * 2)
                IGotBall = is_ball_in_my_borders(myBorders, ballPosition);
            
            // fields and players collect info on who possesses the ball
            if (teamAandFields_COMM != MPI_COMM_NULL)
                MPI_Allgather(&IGotBall, 1, MPI_CXX_BOOL, ballPossessionIndex, 1, MPI_CXX_BOOL, teamAandFields_COMM);
            if (teamBandFields_COMM != MPI_COMM_NULL)
                MPI_Allgather(&IGotBall, 1, MPI_CXX_BOOL, ballPossessionIndex, 1, MPI_CXX_BOOL, teamBandFields_COMM);
            
            // players receive the rank of a field with the ball
            if (rankAF != -1)
                fieldWithBallIndex = get_field_index_with_ball(ballPossessionIndex);
            if (rankBF != -1)
                fieldWithBallIndex = get_field_index_with_ball(ballPossessionIndex);
            
            // having 'fieldWithBallIndex' received, players know who they collect info on 'ballPosition' from;
            // players receive 'ballPosition'
            if (teamAandFields_COMM != MPI_COMM_NULL)
                MPI_Bcast(ballPosition, 2, MPI_INT, fieldWithBallIndex, teamAandFields_COMM);
            if (teamBandFields_COMM != MPI_COMM_NULL)
                MPI_Bcast(ballPosition, 2, MPI_INT, fieldWithBallIndex, teamBandFields_COMM);
            
            // FP0 (NUM_Players) collect players' positions, so each player can collect it afterwards to take further decision
            if (teamAandFields_COMM != MPI_COMM_NULL)
                MPI_Gather(myPosition, 2, MPI_INT, allPlayersPositions_A, 2, MPI_INT, NUM_Players, teamAandFields_COMM);
            if (teamBandFields_COMM != MPI_COMM_NULL)
                MPI_Gather(myPosition, 2, MPI_INT, allPlayersPositions_B, 2, MPI_INT, NUM_Players, teamBandFields_COMM);
            
            // TODO: вместо allPlayersPositions_A длиною NUM_AB_and_F x 2 можно сделать так, чтоб field копировал значения из этого массива в allPlayersPositions и затем уже именно этот массив и передавать
            // players collect positions of teammates from FP0
            if (teamAandFields_COMM != MPI_COMM_NULL)
                MPI_Bcast(allPlayersPositions_A, 2, MPI_INT, NUM_Players, teamAandFields_COMM);
            if (teamBandFields_COMM != MPI_COMM_NULL)
                MPI_Bcast(allPlayersPositions_B, 2, MPI_INT, NUM_Players, teamBandFields_COMM);
            
            // players take decision on what they do next - move or stay unmoved
            // если я добежал быстрее, мне все равно придется ждать остальных
            if (rank < NUM_Players * 2)
                make_step(myPosition, myFieldSector, speed, ballPosition, metersTotal);
            
            // FP0 (rankAF/BF = NUM_Players) collects positions of all players in 'allPlayersPositionsA/B' arrays, so it can choose a winner
            if (teamAandFields_COMM != MPI_COMM_NULL)
                MPI_Gather(myPosition, 2, MPI_INT, allPlayersPositions_A, 2, MPI_INT, NUM_Players, teamAandFields_COMM);
            if (teamBandFields_COMM != MPI_COMM_NULL)
                MPI_Gather(myPosition, 2, MPI_INT, allPlayersPositions_B, 2, MPI_INT, NUM_Players, teamBandFields_COMM);
            
            // FP0 identifies the index and the team of a winner
            if (rank == NUM_Players * 2) {
                identify_winner(allPlayersPositions_A, allPlayersPositions_B, ballPosition, winner);
                fill_winner_arrays(winnersA, winnersB, winner);
            }
            
            // send 'winners' arrays so that each player can detect if he is a winner;
            // FP0 is 'NUM_Players'
            // TODO: хорошее решение! - winnersA/B обнуляются внутри fill_winner_arrays() перед обработкой
            if (teamAandFields_COMM != MPI_COMM_NULL)
                MPI_Bcast(winnersA, NUM_Players, MPI_CXX_BOOL, NUM_Players, teamAandFields_COMM);
            if (teamBandFields_COMM != MPI_COMM_NULL)
                MPI_Bcast(winnersB, NUM_Players, MPI_CXX_BOOL, NUM_Players, teamBandFields_COMM);
            
            // отсюда не проверял -----------------------------------------
            
            // if a player wins the ball, he kicks it to the a certain position or gives a pass
            if (rankA != -1) {
                if ((didWinBall = did_win_ball(winnersA, rankA)))
                    kick_ball(ballPosition, myPosition, myFieldSector, allPlayersPositions_A, goalXcoord, newBallPosition, kickPower);
                else {
                    newBallPosition[0] = -1;
                    newBallPosition[1] = -1;
                }
            }
            if (rankB != -1) {
                if ((didWinBall = did_win_ball(winnersB, rankB)))
                    kick_ball(ballPosition, myPosition, myFieldSector, allPlayersPositions_B, goalXcoord, newBallPosition, kickPower);
                else {
                    newBallPosition[0] = -1;
                    newBallPosition[1] = -1;
                }
            }
            
            // FP0 (rankAF/BF = NUM_Players) receives new ball position (or nothing if there were no winners)
            if (teamAandFields_COMM != MPI_COMM_NULL)
                MPI_Gather(newBallPosition, 2, MPI_INT, newBallPositionsA, 2, MPI_INT, NUM_Players, teamAandFields_COMM);
            if (teamBandFields_COMM != MPI_COMM_NULL)
                MPI_Gather(newBallPosition, 2, MPI_INT, newBallPositionsB, 2, MPI_INT, NUM_Players, teamBandFields_COMM);
            
            // get the ball position for the next round
            get_ball_position(newBallPositionsA, newBallPositionsB, ballPosition);
        }
    }
    
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
    
    // MARK: нестыковка с задачей #1 - The players send their new location to the process of the field in charge of their location - но это бред! что если в одном поле находятся сразу несколько игроков?
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

void assign_position(int *myPosition, int *myFieldSector, int rank, bool checkTeam, int half) {
    // TODO: менять позиции в зависимости от тайма
    int shift, col_shift, row_shift = -1;
    checkTeam ? shift = 12 : shift = 0;
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
    // MARK: это версия для 10 игроков
    myPosition[0] = row_shift * 19;
    myFieldSector[0] = myPosition[0];
    myFieldSector[1] = myPosition[0] + (19-1);
    if (row_shift == 4) {
        myFieldSector[1] = myPosition[0] + 19;
    }
    
    if (!checkTeam && col_shift == 0) {
        myPosition[1] = col_shift * 65;
        myFieldSector[2] = myPosition[1];
        myFieldSector[3] = myFieldSector[2] + 65;
    }
    if (!checkTeam && col_shift == 1) {
        myPosition[1] = col_shift * 65;
        myFieldSector[2] = myPosition[1];
        myFieldSector[3] = myFieldSector[2] + 60;
    }
    
    if (checkTeam && col_shift == 0) {
        myPosition[1] = col_shift * 60;
        myFieldSector[2] = myPosition[1];
        myFieldSector[3] = myFieldSector[2] + 60;
    }
    if (checkTeam && col_shift == 1) {
        myPosition[1] = col_shift * 60;
        myFieldSector[2] = myPosition[1];
        myFieldSector[3] = myFieldSector[2] + 65;
    }
    
//    if (rank != 10) {
//        myPosition[0] = row_shift * 19;
//        myPosition[1] = col_shift * 57 + shift;
//        myFieldSector[0] = myPosition[0];
//        myFieldSector[1] = myPosition[0] + (19-1);
//        myFieldSector[2] = myPosition[1];
//        myFieldSector[3] = myPosition[1] + (57-1);
//
//        if (row_shift == 4) {
//            myFieldSector[1] = myPosition[0] + 19;
//        }
//    }
//    else if (rank == 10 && checkTeam) {
//        myPosition[0] = 0;
//        myPosition[1] = 114;
//        myFieldSector[0] = myPosition[0];
//        myFieldSector[1] = 95;
//        myFieldSector[2] = myPosition[1];
//        myFieldSector[3] = 125;
//    }
//    else if (rank == 10 && !checkTeam) {
//        myPosition[0] = 0;
//        myPosition[1] = 0;
//        myFieldSector[0] = myPosition[0];
//        myFieldSector[1] = 95;
//        myFieldSector[2] = myPosition[1];
//        myFieldSector[3] = shift-1;
//    }
    
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
    else
        row_shift = 2;
    myBoarders[0] = row_shift * 32;
    myBoarders[1] = (row_shift + 1) * 32 - 1;
    myBoarders[2] = col_shift * 32;
    myBoarders[3] = (col_shift + 1) * 32 - 1;
    
    if (col_shift == 3) {
        myBoarders[3] = (col_shift + 1) * 32 - 3;
    }
}

bool is_ball_in_my_borders(int *myBorders, int *ballPosition) {
    if (ballPosition[0] >= myBorders[0] &&
        ballPosition[0] <= myBorders[1] &&
        ballPosition[1] >= myBorders[2] &&
        ballPosition[1] <= myBorders[3])
        return true;
    return false;
}

int get_field_index_with_ball(bool *ballPossessionIndex) {
    int ind = -1;
    for (int i = NUM_Players; i < NUM_AB_and_F; i++) {
        if (ballPossessionIndex[i] == true) {
            ind = i;
            break;
        }
    }
    return ind;
}

void make_step(int *myPosition, int *myFieldSector, int speed, int *ballPosition, int &metersTotal) {
    if (is_ball_in_my_borders(myFieldSector, ballPosition)) {
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
                metersTotal += abs(row_offset);
            }
            else {
                if (row_offset < 0)
                    myPosition[0] += -1 * speed;
                else
                    myPosition[0] += speed;
                metersTotal += speed;
            }
            
            if (speed - abs(row_offset) > 0) {
                if (abs(col_offset) <= abs(speed - abs(row_offset))) {
                    myPosition[1] += col_offset;
                    metersTotal += abs(col_offset);
                }
                else {
                    if (col_offset < 0)
                        myPosition[1] += -1 * abs(speed - abs(row_offset));
                    else
                        myPosition[1] += speed - abs(row_offset);
                    metersTotal += speed - abs(row_offset);
                }
            }
        }
    }
}

bool are_positions_equal(int *a, int *b) {
    return (a[0] == b[0] && a[1] == b[1]);
}

void identify_winner(int *allPlayersPositions_A, int *allPlayersPositions_B, int *ballPosition, int *winner) {
    int count = 0, indA = -1, indB = -1, luck;
    // choose the very first player while iterating because there might be only one player from a team holding the ball at the moment since each of them holds its own sector
    for (int i = 0; i < NUM_Players; i++) {
        if (are_positions_equal(&allPlayersPositions_A[i*2], ballPosition)) {
            count++;
            indA = i;
            break;
        }
    }
    for (int i = 0; i < NUM_Players; i++) {
        if (are_positions_equal(&allPlayersPositions_B[i*2], ballPosition)) {
            count++;
            indB = i;
            break;
        }
    }
    if (count == 0) {
        winner[0] = -1;
        winner[1] = -1;
    }
    else if (count == 1 && indA != -1) {
        winner[0] = 0;
        winner[1] = indA;
    }
    else if (count == 1 && indB != -1) {
        winner[0] = 1;
        winner[1] = indB;
    }
    else if (count > 1) {
        srand(time(0));
        luck = rand() % 2;
        winner[0] = luck;
        if (luck == 0)
            winner[1] = indA;
        else
            winner[1] = indB;
    }
}

void fill_winner_arrays(bool *winnerA, bool *winnerB, int *winner) {
    for (int i = 0; i < NUM_Players; i++) {
        winnerA[i] = false;
    }
    for (int i = 0; i < NUM_Players; i++) {
        winnerB[i] = false;
    }
    
    if (winner[0] == 0)
        winnerA[winner[1]] = true;
    if (winner[0] == 1)
        winnerB[winner[1]] = true;
}

bool did_win_ball(bool *winners, int rank) {
    return (winners[rank] == true);
}

int manhattanDist(int *A, int *B) {
    return abs(A[0] - B[0]) + abs(A[1] - B[1]);
}

void kick_towards(int *kickBallTo, int *ballPosition, int *newBallPosition, int kickPower) {
    int kick = kickPower * 2;
    int row_offset = kickBallTo[0]-ballPosition[0];
    int col_offset = kickBallTo[1]-ballPosition[1];
    if (abs(row_offset) + abs(col_offset) <= kick) {
        newBallPosition[0] = kickBallTo[0];
        newBallPosition[1] = kickBallTo[1];
    }
    else {
        if (abs(row_offset) <= kick)
            newBallPosition[0] += row_offset;
        else {
            if (row_offset < 0)
                newBallPosition[0] += -1 * kick;
            else
                newBallPosition[0] += kick;
        }
        
        if (kick - abs(row_offset) > 0) {
            if (abs(col_offset) <= abs(kick - abs(row_offset)))
                newBallPosition[1] += col_offset;
            else {
                if (col_offset < 0)
                    newBallPosition[1] += -1 * abs(kick - abs(row_offset));
                else
                    newBallPosition[1] += kick - abs(row_offset);
            }
        }
    }
}

void kick_ball(int *ballPosition, int *myPosition, int *myFieldSector, int *allPlayersPositions, int goalXcoord, int *newBallPosition, int kickPower) {
    // direction for a goal, right = true, left = false
    int dist, distToTarget, target, goalPoint[2], kickBallTo[2];
    bool dir;
    myPosition[1] > goalXcoord ? dir = false : dir = true;
    // find the closest teammate between me and a goal
    int closest[] = {-1,INT8_MAX};
    // point in a goal to kick the ball to
    srand(time(0));
    target = (rand() % (goalCoordY2 - goalCoordY1)) + goalCoordY1;
    goalPoint[0] = target;
    goalPoint[1] = goalCoordX2;
    distToTarget = manhattanDist(goalPoint, myPosition);
    // if a goal is at the right side
    if (dir) {
        for (int i = 0; i < NUM_Players; i++) {
            if (allPlayersPositions[i*2+1] > myPosition[1]) {
                dist = manhattanDist(&allPlayersPositions[i*2], myPosition);
                if (dist < closest[1]) {
                    closest[0] = i;
                    closest[1] = dist;
                }
            }
        }
        if (closest[0] != -1) {
            if (closest[1] < distToTarget) {
                kickBallTo[0] = allPlayersPositions[closest[0] * 2];
                kickBallTo[1] = allPlayersPositions[closest[0] * 2 + 1];
            }
            else {
                kickBallTo[0] = goalPoint[0];
                kickBallTo[1] = goalPoint[1];
            }
        }
        else {
            kickBallTo[0] = goalPoint[0];
            kickBallTo[1] = goalPoint[1];
        }
    }
    
    // if a goal is at the left side
    if (!dir) {
        for (int i = 0; i < NUM_Players; i++) {
            if (allPlayersPositions[i*2+1] < myPosition[1]) {
                dist = manhattanDist(&allPlayersPositions[i*2], myPosition);
                if (dist < closest[1]) {
                    closest[0] = i;
                    closest[1] = dist;
                }
            }
        }
        if (closest[0] != -1) {
            if (closest[1] < distToTarget) {
                kickBallTo[0] = allPlayersPositions[closest[0] * 2];
                kickBallTo[1] = allPlayersPositions[closest[0] * 2 + 1];
            }
            else {
                kickBallTo[0] = goalPoint[0];
                kickBallTo[1] = goalPoint[1];
            }
        }
        else {
            kickBallTo[0] = goalPoint[0];
            kickBallTo[1] = goalPoint[1];
        }
    }
    kick_towards(kickBallTo, ballPosition, newBallPosition, kickPower);
}

void get_ball_position(int *newBallPositionsA, int *newBallPositionsB, int *ballPosition) {
    // TODO: заполнить функцию
}












