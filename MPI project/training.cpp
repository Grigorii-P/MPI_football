#include "mpi.h"
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>

#define ROWS 64
#define COLS 128
#define ROW_BALL_INIT_POS ROWS/2
#define COL_BALL_INIT_POS COLS/2
#define NUM_ROUNDS 15
#define NUM_STEPS 10

// position of a player shifts for 1 cell
void make_step(int *my_position, int *ball_position);
// player kicks the ball at random position
void kick_ball(int *ball_position, int *new_ball_position);
// returns the rank of a winner
int who_is_winner(int *players_positions, int *ball_position, int numOfPlayers);
// in case of two or more winners returns the rank of a randomly chosen winner
int random_winner(bool *winners, int numPlayers);

// TODO: прокомментировать всю стратегию
int main(int argc, char *argv[])  {
    int rank, numtasks;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // TODO: добавить недостающие данные, напр., количество побед
    int numOfPlayers = numtasks-1;
    srand(rank);
    int my_position[2] = {rand() % ROWS, rand() % COLS};
    int ball_position[2] = {ROW_BALL_INIT_POS, COL_BALL_INIT_POS};
    // position of the ball after being kicked randomly
    // TODO: можно удалить и сделать ball_position
    int new_ball_position[2] = {ROW_BALL_INIT_POS, COL_BALL_INIT_POS}; // has new coordinates of the ball [row,column]
    int players_positions[numOfPlayers * 2]; // coordinates of each player
    bool winner;
    bool is_there_any_winner;
    int winner_index;

    for (int i = 0; i < NUM_ROUNDS; i++) {
        is_there_any_winner = false;
        if (rank == 0) printf("ROUND %d, ball position - (%d,%d)\n", i+1, ball_position[0], ball_position[1]);
        MPI_Bcast(&ball_position, 2, MPI_INT, 0, MPI_COMM_WORLD);

        for (int j = 0; j < NUM_STEPS; j++) {

            // ---------------- field's part
            if (rank == 0) {
                for (int p = 1; p <= numOfPlayers; p++) {
                    MPI_Recv(&players_positions[(p-1)*2], 2, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                winner_index = who_is_winner(players_positions, ball_position, numOfPlayers); // if '-1', there is no winner

                for (int p = 1; p <= numOfPlayers; p++) {
                    (p == winner_index) ? winner = true : winner = false;
                    MPI_Send(&winner, 1, MPI_CXX_BOOL, p, 0, MPI_COMM_WORLD);
                }

                if (winner_index != -1) {
                    is_there_any_winner = true;
                    MPI_Recv(new_ball_position, 2, MPI_INT, winner_index, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    ball_position[0] = new_ball_position[0];
                    ball_position[1] = new_ball_position[1];
                }
            }

            // ---------------- players' part
            if (rank != 0) {
                printf("rank - %d: ", rank);
                printf("previous position - (%d,%d), ",my_position[0],my_position[1]);
                make_step(my_position, ball_position);
                printf("current position - (%d,%d)\n",my_position[0],my_position[1]);
                MPI_Send(my_position, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
                MPI_Recv(&winner, 1, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (winner) {
                    kick_ball(ball_position, new_ball_position);
                    MPI_Send(new_ball_position, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Bcast(&is_there_any_winner, 1, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);
            if (is_there_any_winner) break;
        }
    }
    MPI_Finalize();
}


void make_step(int *my_position, int *ball_position) {
    int row_step = 0, col_step = 0;
    int row_offset = ball_position[0]-my_position[0];
    int col_offset = ball_position[1]-my_position[1];
    
    // (0,0) is the upper left; (64,128) is the bottom right
    if (row_offset > 0) row_step = 1;
    else if (row_offset < 0) row_step = -1;
    
    if (col_offset > 0) col_step = 1;
    else if (col_offset < 0) col_step = -1;
    
    // first, a player moves along row-axis
    if (row_offset != 0) my_position[0] += row_step;
    // then a player moves along column-axis
    else if (col_offset != 0) my_position[1] += col_step;
}

void kick_ball(int *ball_position, int *new_ball_position) {
    int new_row = rand() % ROWS;
    int new_col = rand() % COLS;
    new_ball_position[0] = new_row;
    new_ball_position[1] = new_col;
}

int who_is_winner(int *players_positions, int *ball_position, int numOfPlayers) {
    int count=0;
    bool winners[numOfPlayers];
    for (int i = 0; i < numOfPlayers; i++) {
        if (players_positions[i*2] == ball_position[0] && players_positions[i*2+1] == ball_position[1]) {
            count++;
            winners[i] = true;
        }
    }
    switch (count) {
        case 0:
            return -1;
        case 1:
        {
            for (int i = 0; i < numOfPlayers; i++) {
                if (winners[i] == true) return i+1;
            }
        }
            break;
        default:
            return random_winner(winners, numOfPlayers);
            break;
    }
    return -999;
}

int random_winner(bool *winners, int numOfPlayers) {
    int i;
    while (true) {
        i = rand() % numOfPlayers;
        if (winners[i] == true) return i+1;
    }
}
