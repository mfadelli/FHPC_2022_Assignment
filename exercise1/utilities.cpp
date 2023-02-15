#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <unistd.h> // contains getopt()
#include <vector>
#include <omp.h>
#include <random>

#define INIT 1
#define RUN  2
#define K_DFLT 100
#define ORDERED 0
#define STATIC  1

// Default values for some arguments
int k = K_DFLT;
int e = ORDERED;
int n = 100;
int s = 0;

// Initializes the grid with a random pattern
void init_grid(std::vector<int>& grid, const int size) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distrib(0, 1);
  #pragma omp parallel for
  for (int i = 0; i < size*size; i++) {
    grid[i] = distrib(gen);
  }
}

// Some other defaults values for other arguments
std::string init_mode = "random";
std::string fname = "snapshot_00000.pbm";

// Initialize a matrix of zeros that contains at its center a smaller matrix read from file
std::vector<int> loadMatrix(const std::string& filename, const int size) {
    // Open the file for reading
    std::ifstream file(filename);

    // Read the magic number and the dimensions of the matrix
    std::string magic;
    int rows, cols;
    file >> magic >> rows >> cols;

    // Initialize the bigger square matrix with the desired size
    std::vector<int> biggerMatrix(size * size);

    // Calculate the indices of the top-left corner of the matrix in the bigger matrix
    int top = (size - rows) / 2;
    int left = (size - cols) / 2;

    // Read the matrix from the file and copy it into the bigger matrix
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        int val;
        file >> val;
        biggerMatrix[(top + i) * size + (left + j)] = val;
      }
    }

    // Return the bigger matrix
    return biggerMatrix;
}

void printManual() {
  std::cout << "\nUsage: Conway.x -options\n";
  std::cout << "\nOptions:\n\n";
  std::cout << " -i file/random  Initialize the program\n"
            << "   -i file       Load a pattern from an ASCII .pbm file\n"
            << "   -i random     Initialize the grid randomly\n";
  std::cout << " -r              Run the program\n";
  std::cout << " -k <number>     Set the size of the grid\n";
  std::cout << " -e <number>     Select the type of evolution\n"
            << "    0: ordered evolution\n"
            << "    1: static evolution\n";
  std::cout << " -f <filename>   Set the name of the file to be loaded\n";
  std::cout << " -n <number>     Set the number of iterations\n";
  std::cout << " -s <number>     Choose every how many iterations you want to save grid\n";
  std::cout << " -h              Print this manual\n\n";
  std::cout << "\nExample usage:\n"
            << "\n  Initialization: load a pattern and place it at the center of a larger empty grid\n"
            << "                  this will create a binary .pbm file named snapshot_00000.pbm\n"
            << "    srun Conway.x -i file -f pattern.pbm -k 10\n"
            << "\n  Run: run the program starting from snapshot_00000.pbm, with static evolution,\n"
            << "       for 100 iterations, saving the grid every 5 iterations\n"
            << "    srun Conway.x -r -k 10 -n 100 -e 1 -s 5\n" << std::endl;
}

// To get parameters from the terminal
void handleOptions(int argc, char* argv[], int &action) {
  action = 0;
  std::string optstring = "i:rk:e:f:n:s:h";
  int c;
  while ((c = getopt(argc, argv, optstring.c_str())) != -1) {
      switch (c) {
        case 'i': action = INIT; init_mode = optarg; break; // INIT = 1
        case 'r': action = RUN; break; // RUN = 2
        case 'k': k = atoi(optarg); break; // the grid will be a square matrix k*k
        case 'e': e = atoi(optarg); break; // 0 for ordered evolution, 1 for static evolution
        case 'f': fname = optarg; break; // name of the file to read, argument of loadMatrix if -i, readSnapshot if -r
        case 'n': n = atoi(optarg); break; // number of iterations
        case 's': s = atoi(optarg); break; // frequency of the snapshots
        case 'h': break; // the manual is printed in the main
        default: std::cout << "argument -" << c << " not known\n" << "Run the program with option -h to get help" << std::endl; break;
      }
  }
}
