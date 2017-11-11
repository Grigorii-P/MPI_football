#include "mpi.h"
#include <stdio.h>
#include <cstdlib>

#define X_field 128
#define Y_filed 64
#define NUM_ROUNDS 3
#define NUM_STEPS 10

void make_step(int *my_position, int *ball_position);
int *kick_ball();
int find_winner();
bool are_positions_equal(int *ball_position, int *new_ball_position);

int foo(int a, int b) {
    return a-b;
}

// TODO: прокомментировать всю стратегию
int main(int argc, char *argv[])  {
    int numOfPlayers = atoi(argv[3])-1, rank;
    int my_position[2];
    int ball_position[2] = {32,64};
    // TODO: можно удалить и сделать ball_position
    // TODO: NULL вместо nullptr
    int *new_ball_position = ball_position; // has new coordinates of the ball [row,column]
    int players_positions[numOfPlayers][2]; // coordinates of each player
    bool winner;
    int winner_index;

    MPI_Init(&argc,&argv);
//    MPI_Comm_size(MPI_COMM_WORLD, &);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (int i = 0; i < NUM_ROUNDS; i++) {
        // TODO: если не сработает, используй MPI_Barrier() следующей строчкой
        MPI_Bcast(&ball_position, 2, MPI_INT, 0, MPI_COMM_WORLD);

        for (int j = 0; j < NUM_STEPS; j++) {
            // ---------------- players' part
            if (rank >= 1) {
                // TODO: возможно, стоит вернуть '&'
                make_step(my_position, ball_position);
                MPI_Send(&my_position, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
                MPI_Recv(&winner, 1, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (winner) {
//                    new_ball_position = kick_ball();
                    // TODO: может быть проблема с адресацией new_ball_position
                    MPI_Send(&new_ball_position, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
            }

            // ---------------- field's part
            if (rank == 0) {
                for (int p = 1; p < numOfPlayers; p++) {
                    MPI_Recv(&players_positions[p], 2, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                winner_index = find_winner(); // if '-1' is returned, no one become a winnner

                for (int p = 0; p < numOfPlayers; p++) {
                    (p == winner_index) ? winner = true : winner = false;
                    MPI_Send(&winner, 1, MPI_CXX_BOOL, p, 0, MPI_COMM_WORLD);
                }

                if (winner_index != -1)
                    MPI_Recv(&new_ball_position, 2, MPI_INT, winner_index, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (!are_positions_equal(ball_position, new_ball_position)) {
                    ball_position = new_ball_position;
                    
                }




            }
            MPI_Barrier(MPI_COMM_WORLD);
        }

    }







    MPI_Finalize();
}

