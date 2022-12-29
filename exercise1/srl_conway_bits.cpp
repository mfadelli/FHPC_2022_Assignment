#include <iostream>
#include <vector>
#include <numeric>
#include <string>
#include <unistd.h> // contains getopt()
#include <fstream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <omp.h>

#define INIT 1
#define RUN  2
#define K_DFLT 100
#define ORDERED 0
#define STATIC  1

int k = K_DFLT;
int e = ORDERED;
int n = 100;
int s = 1;
std::string fname = "snapshot_00000.pbm";

// Initializes the grid with a random pattern // SERIAL
void init_grid(std::vector<int>& grid, const int size) {
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      int index = x * size + y;  // compute the index in the 1D array
      grid[index] = rand() % 2;
    }
  }
}

// SERIAL
int check_neighbours(const std::vector<int>& grid, const int x, const int y, const int size) {
  int count = 0;
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      if (dx == 0 && dy == 0) continue;  // skip the current cell

      // handle wrapping
      int nx = x + dx;
      if (nx < 0) { nx += size; } else if (nx >= size) { nx -= size; }

      int ny = y + dy;
      if (ny < 0) { ny += size; } else if (ny >= size) { ny -= size; }

      if (grid[nx*size + ny]) count++;
    }
  }
  return count;
}

void ordered_update_cell(const int i, const int j, const int size, std::vector<int>& grid) {
  int alive = check_neighbours(grid, i, j, size);
  if (alive < 2 || alive > 3) {
    grid[i*size + j] = 0;
  } else {
    grid[i*size + j] = 1;
  }
}

// SERIAL
void static_update_cell(std::vector<int>& grid, const std::vector<int>& old_grid, const int i, const int j, const int size) {
  int alive = check_neighbours(old_grid, i, j, size);
  int index = i*size + j;
  switch(alive) {
    case 2:
      grid[index] = old_grid[index]; break;
    case 3:
      grid[index] = 1; break;
    default:
      grid[index] = 0;
  }
}

// PARALLEL
void update_grid(const int mode, const int size, std::vector<int>& grid) {
  switch(mode) {
    case 0:
      for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
          ordered_update_cell(i, j, size, grid);
        }
      }
    case 1:
      std::vector<int> old_grid = grid; // can be parallelized (?)
      #pragma omp parallel
      {
      #pragma omp for collapse(2)
      for (int i = 0; i < size; i++) { // for collapse
        for (int j = 0; j < size; j++) {
          static_update_cell(grid, old_grid, i, j, size);
        }
      }
    }
  }
}

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

std::string num_conv(const int number) {
  std::stringstream ss;
  ss << std::setw(5) << std::setfill('0') << number;
  std::string str = ss.str();
  return str;
}

void createSnapshot(const std::vector<int>& arr, const int size, const int step) {
  // Open a file called "snapshot_s" in write mode
  std::string num = num_conv(step);
  std::ofstream snapshot("snapshot_" + num + ".pbm", std::ios::binary);

  // Write the P4 binary PBM file header
  snapshot << "P4\n" << size << ' ' << size << "\n";

  // Iterate through the array and write each element to the file as a single bit
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      snapshot << (char)(arr[i*size + j] & 1);
    }
  }
  snapshot.close();
}


char createCharacterFromBits(const std::array<int, 8>& bits) {
    char result = 0;
    for (int i = 0; i < 8; i++) {
        result = (result << 1) | bits[i];
    } // 00000001 -> 00000010 | 00000000 00000000 00000000 00000001 = 00000011
    return result;
}
/*
void createSnapshot(std::vector<int>& arr, int size,int step) {
  int resto = size%8;
  int quot = size/8;

  // Open a file called "snapshot_s" in write mode
  std::string num = num_conv(step);
  std::ofstream snapshot("snapshot_" + num +".pbm", std::ios::binary);
  // Iterate through the array and write each element to the file, separated by a space
  snapshot << "P4\n" << size << ' ' << size <<"\n";
  int i=0;
  while (i<size*size) {
    int i_mod = i%size;
    if (i_mod < quot*8){ // 1)
    std::array<int, 8> beet={arr[i],arr[i+1],arr[i+2],arr[i+3],arr[i+4],arr[i+5],arr[i+6],arr[i+7]};
    char c = createCharacterFromBits(beet);
    snapshot << c;
    i=i+8;}
    // 2)
    else { std::array<int, 8> boot={0,0,0,0,0,0,0,0};
      for(int j=0; j<resto;j++) {
        if(j<resto) boot[j]=arr[i+j];
      }
      char c = createCharacterFromBits(boot);
      snapshot << c;
      i = i+size-(i%size);
    }
  }
}
*/
void readSnapshot(std::vector<int>& grid, const int size, const std::string namefile) {
  // Open a file in read mode
  std::ifstream snapshot(namefile, std::ios::binary);

  // Read the P4 binary PBM file header
  std::string line;
  getline(snapshot, line);  // skip the "P4" magic number
  getline(snapshot, line);  // skip the dimensions line

  // Read each byte from the file and store its bits in the array
  for (int i = 0; i < size*size; i++) {
    char byte;
    snapshot.read(&byte, 1);
    grid[i] = byte & 1;
  }

  // Close the file
  snapshot.close();
}

int main(int argc, char* argv[]) {
  int action = 0;
  std::string optstring = "irk:e:f:n:s:";
  int c;
  while ((c = getopt(argc, argv, optstring.c_str())) != -1) {
      switch (c) {
        case 'i': action = INIT; break; // INIT = 1
        case 'r': action = RUN; break; // RUN = 2
        case 'k': k = atoi(optarg); break;
        case 'e': e = atoi(optarg); break;
        case 'f': fname = optarg; break;
        case 'n': n = atoi(optarg); break;
        case 's': s = atoi(optarg); break;
        default: std::cout << "argument -" << c << " not known" << std::endl; break;
      }
  }

  // Create the vector with the specified size
  std::vector<int> grid(k*k);

  if (action == 1) {
    std::vector<int> initial_condition = loadMatrix(fname, k);
    for (int i = 0; i < k; i++) {
      for (int j = 0; j < k; j++) {
        std::cout << initial_condition[i*k + j] << ' ';
      }
      std::cout << '\n';
    }
    createSnapshot(initial_condition, k, 0);
  }

  std::chrono::duration<double> update_elapsed, write_elapsed;

  if (action == 2) {
    #pragma omp parallel
    #pragma omp master
    {
      //int nthreads = omp_get_num_threads();
      //std::cout << "nthreads: " << nthreads << std::endl;
    }
    // readSnapshot(grid, k, fname);
    init_grid(grid, k);
    for (int i = 1; i <= n; i++) {
      auto start = std::chrono::high_resolution_clock::now();
      update_grid(e, k, grid);
      auto end = std::chrono::high_resolution_clock::now();
      update_elapsed += end - start;
      if (i%s == 0) {
        auto start = std::chrono::high_resolution_clock::now();
        createSnapshot(grid, k, i);
        auto end = std::chrono::high_resolution_clock::now();
        write_elapsed += end - start;
      }
    }
    std::cout << "Time to update the grid once: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(update_elapsed/double(n)).count()
              << " ms\n";

    std::cout << "Time to write the grid once: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(write_elapsed*double(s)/double(n)).count()
              << " ms\n";

    std::cout << "(average over " << n << " iterations)" << std::endl;
  }

  return 0;
}
