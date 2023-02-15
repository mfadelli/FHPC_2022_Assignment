#include <vector>
#include <omp.h>
#include <mpi.h>

// Function that, given a grid, its size and the indexes of a cell calculates the number of its alive neighbour
int check_neighbours(const std::vector<int>& grid, const int x, const int y, const int rows, const int cols) {
  int count = 0;
  #pragma omp simd collapse(2)
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      if (dx == 0 && dy == 0) continue;  // skip the current cell

      // handle wrapping
      int nx = x + dx;
      // boundary conditions on the rows
      if (nx < 0) { nx += rows; } else if (nx >= rows) { nx -= rows; }

      int ny = y + dy;
      // boundary conditions on the columns
      if (ny < 0) { ny += cols; } else if (ny >= cols) { ny -= cols; }

      if (grid[nx*cols + ny]) count++; // nx*cols + ny is the index of the cell in a one dimensional array
    }
  }
  return count; // number of alive neighbours
}

// Function that upgrades a grid in the ordered evolution mode (inherehently serial)
void ordered_update_grid(const int rows, const int cols, std::vector<int>& grid) {
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          int alive = check_neighbours(grid, i, j, rows, cols); // alive contains the number of alive neighbours
          if (alive < 2 || alive > 3) {
            grid[i*cols + j] = 0;
          } else {
            grid[i*cols + j] = 1;
          }
        }
      }
}

// Function that upgrades a grid in the static evolution mode
// old_grid is used for evaluation, grid will contain the updated values
void static_update_grid(const int rows, const int cols, std::vector<int>& grid, std::vector<int>& old_grid) {
    #pragma omp parallel shared(grid, old_grid)
    {
      #pragma omp for schedule(static) collapse(2)
        for (int i = 1; i < rows - 1; i++) { // to avoid updating halo rows
          for (int j = 0; j < cols; j++) {
            int alive = check_neighbours(old_grid, i, j, rows, cols); // evaluated on the old_grid
            int index = i*cols + j; // index of the cell: simplest way to index a 1-D vector to store a 2-D matrix
            switch(alive) {
              case 2:
                grid[index] = old_grid[index]; break; // in the case of two living neighbours, the value is the old value
              case 3:
                grid[index] = 1; break;
              default:
                grid[index] = 0; break;
            }
          }
        }
    }
}

// Function used to send and receive between MPI processes the halo rows, necessary for counting the alive neighbours
void mpiCalls(std::vector<int>& subgrid, const int k, const int world_rank, const int world_size) {
  MPI_Request request1, request2;

  MPI_Isend(&subgrid[k], k, MPI_INT, (world_rank - 1 + world_size) % world_size, 0, MPI_COMM_WORLD, &request1);
  MPI_Irecv(&subgrid[subgrid.size() - k], k, MPI_INT, (world_rank + 1) % world_size, 0, MPI_COMM_WORLD, &request1);

  MPI_Isend(&subgrid[subgrid.size() - 2*k], k, MPI_INT, (world_rank + 1) % world_size, 1, MPI_COMM_WORLD, &request2);
  MPI_Irecv(&subgrid[0], k, MPI_INT, (world_rank - 1 + world_size) % world_size, 1, MPI_COMM_WORLD, &request2);

  MPI_Wait(&request1, MPI_STATUS_IGNORE);
  MPI_Wait(&request2, MPI_STATUS_IGNORE);
}
