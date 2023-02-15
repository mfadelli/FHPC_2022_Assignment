#include <chrono>
#include "read_write.h"
#include "game.h"
#include "utilities.h"

int main(int argc, char* argv[]) {

  int action = 0;
  handleOptions(argc, argv, action); // get the parameters from the terminal

  auto t_start = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> total_elapsed;

  // General MPI initialization
  MPI_Init(&argc, &argv);
  int world_size, world_rank, how_many_rows, rest;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Check if program is run with option -h
  for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-h") == 0) {
          if (world_rank == 0) printManual();
          return 0; // Return without running the main program
      }
  }

  // Initialise arrays for storing indexes to divide the grid
  int* sendcounts = new int[world_size];
  int* sendcounts_elem = new int[world_size];
  int* displs = new int[world_size];

  how_many_rows = k / world_size;
  rest = k - (world_size * how_many_rows);

  //if (world_rank == 0) { std::cout << "\nk = " << k << "\nchunk_size = " << how_many_rows << "\nrest = " << rest << std::endl; }
  //if (world_rank == 0) std::cout << "sendcounts = ";
  // determine how many rows should each process receive
  for (int i = 0; i < world_size; i++) {
    if (i < rest) {
        sendcounts[i] = how_many_rows + 1;
        //if (world_rank == 0) std::cout << sendcounts[i] << " ";
    } else {
        sendcounts[i] = how_many_rows;
        //if (world_rank == 0) std::cout << sendcounts[i] << " ";
      }
  }

  // determine the row indeces
  //if (world_rank == 0) std::cout << "\ndispls = 0 ";
  displs[0] = 0;
  for (int i = 0; i < world_size-1; i++) {
      displs[i+1] = displs[i] + sendcounts[i];
      //if (world_rank == 0) std::cout << displs[i+1] << " ";
  }
  //if (world_rank == 0) std::cout << std::endl;

  // determine how many elements should each process receive and their starting point in the grid
  // multiply by the number of elements in a row (cols)
  //if (world_rank == 0) std::cout << "sendcounts (elements) = ";
  for (int i = 0; i < world_size; i++) {
      sendcounts_elem[i] = sendcounts[i]*k;
      //if (world_rank == 0) std::cout << sendcounts_elem[i] << " ";
      displs[i] = displs[i]*k;
  }
  //if (world_rank == 0) std::cout << "\n" << std::endl;

  // Declare the initial full grid;
  std::vector<int> grid(k*k);
  double update_elapsed, write_elapsed, init_elapsed;

  if (world_rank == 0) {
    if (action == 1) {
      if (init_mode == "random") {
        auto start = MPI_Wtime();
        init_grid(grid, k);
        auto end = MPI_Wtime();
        init_elapsed += end - start;
        if (world_rank == 0) std::cout << "\nTime to randomly initialize the grid: " << init_elapsed << " s\n";
        createSnapshot(grid, k, 0);
      }

      if (init_mode == "file") {
        std::vector<int> initial_condition = loadMatrix(fname, k);
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < k; j++) {
                std::cout << initial_condition[i*k + j] << ' ';
            }
            std::cout << '\n';
        }
        createSnapshot(initial_condition, k, 0);
      }
    }
  }

  if (action == 2) {

    // Print the number of threads
    int nthr, tid;
    #pragma omp parallel private(nthr, tid)
    {
      tid = omp_get_thread_num();
      if (world_rank == 0 && tid == 0) {
         nthr = omp_get_num_threads();
        std::cout << "\nNumber of omp threads: " << nthr << std::endl;
      }
    }

    // Print the number of MPI processes
    if (world_rank == 0) std::cout << "Number of MPI processes: " << world_size << std::endl;
    // Initialize the full grid;
    // Calculate the numbers of characters needed to store the grid in the pbm file
    if(world_rank == 0) {
      int chr_in_a_row;
      if (k%8 == 0) {
       chr_in_a_row=(k/8);
      } else {
        chr_in_a_row = (k/8) + 1;
      }
      int n_char = chr_in_a_row*k;
      auto start1 = MPI_Wtime();
      std::vector<char> char_file(n_char);
      readSnapshot(char_file, fname); //write the characters in a vector
      convertchars(char_file,k,grid); //convert the vector of characters in a vector of integers
      auto end1 = MPI_Wtime();
      std::cout << "\nTime to read snapshot: " << end1 - start1 << "\n";
    }

    // Divide the matrix into chunks
    auto start2 = MPI_Wtime();
    std::vector<int> subgrid1(sendcounts_elem[world_rank] + 2*k), subgrid2(sendcounts_elem[world_rank] + 2*k);
    auto end2 = MPI_Wtime();
    if (world_rank == 0) std::cout << "Time to initialize the subgrids: " << end2 - start2 << "\n";

    // Send the chunks to different MPI processes
    auto start3 = MPI_Wtime();
    MPI_Scatterv(grid.data(), sendcounts_elem, displs, MPI_INT, &subgrid1[k], sendcounts_elem[world_rank], MPI_INT, 0, MPI_COMM_WORLD);
    auto end3 = MPI_Wtime();
    if (world_rank == 0) std::cout << "Time to scatter the grid: " << end3 - start3 << "\n";

    // Send also the ghost rows
    auto start4 = MPI_Wtime();
    mpiCalls(subgrid1, k, world_rank, world_size);
    auto end4 = MPI_Wtime();
    if (world_rank == 0) std::cout << "Time to send ghost rows the 1st time: " << end4 - start4 << "\n";

    for (int i = 1; i <= n; i++) {
      // i is the number of the iteration
      auto start = MPI_Wtime();
      if (i%2 == 0) { // odd iterations
        static_update_grid(sendcounts[world_rank] + 2, k, subgrid1, subgrid2); //new_grid=subgrid1, old_grid=subgrid2
        mpiCalls(subgrid1, k, world_rank, world_size); //exchange halo rows
      }
      if (i%2 == 1) { // even iterations
        static_update_grid(sendcounts[world_rank] + 2, k, subgrid2, subgrid1); //new_grid=subgrid2, old_grid=subgrid1
        mpiCalls(subgrid2, k, world_rank, world_size); //exchange halo rows
      }
      auto end = MPI_Wtime();
      update_elapsed += end - start;

      // Reconstruct the matrix and print it
      if (s == 0) s = n; // Print the grid only at the end if s is 0
      if (i%s == 0) {
        auto start = MPI_Wtime();

        if (i%2 == 0) { // odd iterations
          MPI_Gatherv(&subgrid1[k], sendcounts_elem[world_rank], MPI_INT, grid.data(), sendcounts_elem, displs, MPI_INT, 0, MPI_COMM_WORLD); //reconstruct the matrix from subgrid1, excluding halo rows
          if (world_rank == 0) createSnapshot(grid, k, i); //dump the grid inside snapshot_0000i
        }

        if (i%2 == 1) { // even iterations
          MPI_Gatherv(&subgrid2[k], sendcounts_elem[world_rank], MPI_INT, grid.data(), sendcounts_elem, displs, MPI_INT, 0, MPI_COMM_WORLD); //reconstruct the matrix from subgrid2, excluding halo rows
          if (world_rank == 0) createSnapshot(grid, k, i); //dump the grid inside snapshot_0000i
        }
        auto end = MPI_Wtime();
        write_elapsed += end - start;
      }
    }

    if (world_rank == 0) { // print the partial times, profiling
      std::cout << "Time to update the grid once: " << update_elapsed/double(n) << " s\n";
      std::cout << "Time to write the grid once: " << write_elapsed*double(s)/double(n) << " s\n";
      std::cout << "(average over " << n << " iterations)" << std::endl;
    }
  }

  auto t_end = std::chrono::high_resolution_clock::now();
  total_elapsed = t_end - t_start;
  if (world_rank == 0) { // print the total time
    std::cout << "\nTotal runtime: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(total_elapsed).count() / 1000.0
              << " s\n" << std::endl;
  }

  MPI_Finalize();

  return 0;
}
