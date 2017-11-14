//TODO: удалить ненужные библиотеки
#include "mpi.h"
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>
#include <string.h>
//#include "training.h"


#define ROWS 64
#define COLS 128
#define ROW_BALL_INIT_POS ROWS/2
#define COL_BALL_INIT_POS COLS/2
#define NUM_ROUNDS 5
#define NUM_STEPS 10

// position of a player shifts for 1 cell
void make_step(int *my_position, int *ball_position, int &metersTotal);
// player kicks the ball at random position
void kick_ball(int *ball_position, int *new_ball_position);
// did a player reach the ball at a current step
bool did_reach_ball(int *my_position, int *ball_position);
// returns the rank of a winner
int who_is_winner(bool *playersReachedBall, int numOfPlayers);
// in case of two or more winners returns the rank of a randomly chosen winner
int random_winner(bool *winners, int numPlayers);
void values_to_false(bool *arr, int n);

int main(int argc, char *argv[])  {
    int rank, numtasks;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // TODO: заменить '_' в названиях переменных на большие буквы, но следи, чтоб имена не пересеклись
    int numOfPlayers = numtasks-1;
    int winnerIndex=-1;
    int metersTotal=0;
    int timesReachedBall=0;
    int timesWonBall=0;
    int kickedBall=0;
    bool winner;
    bool isThereAnyWinner;
    int players_meters[numOfPlayers+1];
    bool playersReachedBall[numOfPlayers];
    bool players_won_ball[numOfPlayers];
    int times_reached_ball[numOfPlayers+1];
    int times_won_ball[numOfPlayers+1];
    srand(rank);
    int my_position[2] = {rand() % ROWS, rand() % COLS};
    int ball_position[2] = {ROW_BALL_INIT_POS, COL_BALL_INIT_POS};
    int new_ball_position[2] = {ROW_BALL_INIT_POS, COL_BALL_INIT_POS}; // has new coordinates of the ball [row,column]
    int initial_players_positions[(numOfPlayers+1) * 2];
    int players_positions[(numOfPlayers+1) * 2]; // coordinates of each player

    values_to_false(playersReachedBall, numOfPlayers);
    values_to_false(players_won_ball, numOfPlayers);

    for (int i = 0; i < NUM_ROUNDS; i++) {
        
        MPI_Gather(my_position, 2, MPI_INT, initial_players_positions, 2, MPI_INT, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            printf("round - %d\n", i+1);
            printf("ball - %d,%d\n", ball_position[0], ball_position[1]);
        }
        isThereAnyWinner = false;
        MPI_Bcast(&ball_position, 2, MPI_INT, 0, MPI_COMM_WORLD);

        for (int j = 0; j < NUM_STEPS; j++) {

            // ---------------- field's part
            if (rank == 0) {
                for (int p = 1; p <= numOfPlayers; p++) {
                    MPI_Recv(&players_positions[(p-1)*2], 2, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Recv(&playersReachedBall[p-1], 1, MPI_CXX_BOOL, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                winnerIndex = who_is_winner(playersReachedBall ,numOfPlayers); // if '-1', there is no winner

                for (int p = 0; p < numOfPlayers; p++) {
                    (p == winnerIndex) ? winner = true : winner = false;
                    MPI_Send(&winner, 1, MPI_CXX_BOOL, p+1, 0, MPI_COMM_WORLD);
                }

                if (winnerIndex != -1) {
                    players_won_ball[winnerIndex] = true;
                    isThereAnyWinner = true;
                    MPI_Recv(new_ball_position, 2, MPI_INT, winnerIndex+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    ball_position[0] = new_ball_position[0];
                    ball_position[1] = new_ball_position[1];
                }
            }

            // ---------------- players' part
            if (rank != 0) {
                make_step(my_position, ball_position, metersTotal);
                MPI_Send(my_position, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
                bool gotBall = false;
                if ((gotBall = did_reach_ball(my_position, ball_position))) {
                    timesReachedBall++;
                }
                MPI_Send(&gotBall, 1, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD);
                MPI_Recv(&winner, 1, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (winner) {
                    timesWonBall++;
                    kick_ball(ball_position, new_ball_position);
                    MPI_Send(new_ball_position, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
                    kickedBall++;
                }
            }
            // TODO: проверить, нужен ли здесь барьер
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Bcast(&isThereAnyWinner, 1, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);
            if (isThereAnyWinner) break;
        }

        MPI_Gather(&metersTotal, 1, MPI_INT, players_meters, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(&timesReachedBall, 1, MPI_INT, times_reached_ball, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(&timesWonBall, 1, MPI_INT, times_won_ball, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(my_position, 2, MPI_INT, players_positions, 2, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            for (int l = 0; l < numOfPlayers; l++) {
                // TODO: убрать скобки, выводить координаты along long axis first
                printf("%d (%d,%d) (%d,%d) %d %d %d %d %d\n",l,initial_players_positions[(l+1)*2],initial_players_positions[(l+1)*2+1],players_positions[(l+1)*2],players_positions[(l+1)*2+1],playersReachedBall[l],players_won_ball[l],players_meters[l+1],times_reached_ball[l+1],times_won_ball[l+1]);
            }

            for (int j = 0; j < numOfPlayers; j++) {
                if (players_won_ball[j] == true) {
                    players_won_ball[j] = false;
                    break;
                }
            }
        }
    }
    MPI_Finalize();
}


void make_step(int *my_position, int *ball_position, int &metersTotal) {
    int row_step = 0, col_step = 0;
    int row_offset = ball_position[0]-my_position[0];
    int col_offset = ball_position[1]-my_position[1];
    
    // (0,0) is the upper left; (64,128) is the bottom right
    if (row_offset > 0) row_step = 1;
    else if (row_offset < 0) row_step = -1;
    
    if (col_offset > 0) col_step = 1;
    else if (col_offset < 0) col_step = -1;
    
    // first, a player moves along row-axis
    if (row_offset != 0) {
        my_position[0] += row_step;
        metersTotal++;
    }
    // then a player moves along column-axis
    else if (col_offset != 0) {
        my_position[1] += col_step;
        metersTotal++;
    }
}

void kick_ball(int *ball_position, int *new_ball_position) {
    int new_row = rand() % ROWS;
    int new_col = rand() % COLS;
    new_ball_position[0] = new_row;
    new_ball_position[1] = new_col;
}

bool did_reach_ball(int *my_position, int *ball_position) {
    return (my_position[0] == ball_position[0] && my_position[1] == ball_position[1]);
}

int who_is_winner(bool *playersReachedBall, int numOfPlayers) {
    int count=0;
    for (int i = 0; i < numOfPlayers; i++) {
        if (playersReachedBall[i] == true) {
            count++;
        }
    }
    switch (count) {
        case 0:
            return -1;
        case 1:
        {
            for (int i = 0; i < numOfPlayers; i++) {
                if (playersReachedBall[i] == true) return i;
            }
        }
            break;
        default:
            return random_winner(playersReachedBall, numOfPlayers);
            break;
    }
    return -999; // will never happen
}

int random_winner(bool *winners, int numOfPlayers) {
    int i;
    while (true) {
        i = rand() % numOfPlayers;
        if (winners[i] == true) return i;
    }
}

void values_to_false(bool *arr, int n) {
    for (int i=0; i < n; i++) {
        arr[i] = false;
    }
}
