#include <iostream>
#include <cstdlib>
#include <vector>
#include <array>
#include <numeric>
#include <string>
#include <unistd.h>
#include <cstring>
#include <fstream>

#define INIT 1
#define RUN  2

#define K_DFLT 100

#define ORDERED 0
#define STATIC  1

int action = 0;
int k = K_DFLT;
int e = ORDERED;
int n = 10000;
int s = 1;
std::string fname ="";

// il fucking modulooooooooooooo
int mod(int a, int b){
    int r = a % b;
    return r < 0 ? r + b : r;
}

int ij_to_k(int i, int j, int size) { return i*size + j; }

int check_neighbours(int i, int j, int size, std::vector<int>& grid) {
  // who are the neighbours?

  std::array<int, 8> neighbours;
  neighbours[0] = grid[ij_to_k(mod(i-1, size), mod(j-1, size), size)];
  neighbours[1] = grid[ij_to_k(mod(i-1, size), j, size)];
  neighbours[2] = grid[ij_to_k(mod(i-1, size), mod(j+1, size), size)];
  neighbours[3] = grid[ij_to_k(i, mod(j-1, size), size)];
  neighbours[4] = grid[ij_to_k(i, mod(j+1, size), size)];
  neighbours[5] = grid[ij_to_k(mod(i+1, size), mod(j-1, size), size)];
  neighbours[6] = grid[ij_to_k(mod(i+1, size), j, size)];
  neighbours[7] = grid[ij_to_k(mod(i+1, size), mod(j+1, size), size)];

  int alive = std::accumulate(neighbours.begin(), neighbours.end(), 0);
  // unalive yourself lmao
  return alive;

}

void ordered_update_cell(int i, int j, int size, std::vector<int>& grid) { // try to use case switch
  int alive = check_neighbours(i, j, size, grid);
  if (alive < 2 || alive > 3) {
    grid[ij_to_k(i, j, size)] = 0;
  } else {
    grid[ij_to_k(i, j, size)] = 1;
  }
}

void static_update_cell(int i, int j, int size, std::vector<int>& grid, std::vector<int>& old_grid) {
  int alive = check_neighbours(i, j, size, old_grid);
  if (alive < 2 || alive > 3) {
    grid[ij_to_k(i, j, size)] = 0;
  } else {
    grid[ij_to_k(i, j, size)] = 1;
  }
}

void update_grid(int mode, int size, std::vector<int>& grid) {
  if (mode == 1) {
    std::vector<int> old_grid = grid;
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < size; j++) {
        static_update_cell(i, j, size, grid, old_grid);
      }
    }
  }
  if (mode == 0) {
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < size; j++) {
        ordered_update_cell(i, j, size, grid);
      }
    }
  }
}

void createSnapshot(std::vector<int>& arr, int size, int step) {
  // Open a file called "snapshot_s" in write mode
  std::ofstream snapshot("snapshot_" + std::to_string(s)+".pbm");
  // Iterate through the array and write each element to the file, separated by a space
  snapshot<<"P1\n"<<size<<' '<<size<<"\n";
  for (int i = 0; i < size*size; i++) {
    snapshot << arr[i] << ' ';
  }
  // Close the file
  snapshot.close();
}

void readSnapshot(std::vector<int>& arr, int size, std::string namefile) {
  // Open a file called "snapshot_s" in read mode
  std::ifstream snapshot(namefile);
  std::string line;
  getline(snapshot, line);  // skip the first line
  getline(snapshot, line);  //skip the second line
  // Read each number from the file and store it in the array
  for (int i = 0; i < size*size; i++) {
    snapshot >> arr[i];
  }

  // Close the file
  snapshot.close();
  }



int main(int argc, char* argv[]) {
  int action = 0;
  std::string optstring = "irk:e:f:n:s:";

  int c;
  while ((c = getopt(argc, argv, optstring.c_str())) != -1)
  {
      switch (c)
      {
      case 'i':
          action = INIT;
          break;

      case 'r':
          action = RUN;
          break;

      case 'k':
          k = atoi(optarg);
          break;

      case 'e':
          e = atoi(optarg);
          break;

      case 'f':
          //fname = (char *)malloc(sizeof(optarg) + 1);
          //sprintf(fname, "%s", optarg);
          fname=optarg;
          break;

      case 'n':
          n = atoi(optarg);
          break;

      case 's':
          s = atoi(optarg);
          break;

      default:
          std::cout << "argument -" << c << " not known" << std::endl;
          break;
      }
  }


  // Create the vector with the specified size
  std::vector<int> grid(k*k);
  if(action==2){
      readSnapshot(grid,k,fname);
  }
  else{
  for (int i = 0; i < k*k; i++) {
    grid[i] = 0;
  }
  grid[5] = 1;
  grid[6] = 1;
  grid[9] = 1;
  grid[10] = 1;
  }
  for (size_t i = 0; i < k; i++) {
    for (size_t j = 0; j < k; j++) {
      std::cout << grid[i*k + j] << ' ';
    }
    std::cout << '\n';
  }

  std::cout << "\n";

  for (size_t i = 0; i < n; i++) {
    update_grid(e, k, grid);
  }

  std::cout << "\n";

  for (size_t i = 0; i < k; i++) {
    for (size_t j = 0; j < k; j++) {
      std::cout << grid[i*k + j] << ' ';
    }
    std::cout << '\n';
  }
  createSnapshot(grid,k,s);
  //if (fname != NULL)
  //    free(fname);

  return 0;
}
