#include <iostream>
#include <chrono>
#include <vector>
#include <random>

// Function using an if-else construct
void func1(std::vector<int>& v, int pos, int cs) {
  if (cs < 2 || cs > 3) {
    v[pos] = 0;
  } else if (cs == 3) {
    v[pos] = 1;
  } else {
    v[pos] = v[pos];
  }
}

// Function using a switch statement
void func2(std::vector<int>& v, int pos, int cs) {
  switch (cs) {
    case 2:
      v[pos] = v[pos];
      break;
    case 3:
      v[pos] = 1;
      break;
    default:
      v[pos] = 0;
      break;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <n> <size>" << std::endl;
    return 1;
  }

  // Parse the value of n and size from the command line argument
  int n = std::atoi(argv[1]);
  int size = std::atoi(argv[2]);

  // Create a random number generator
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 8); // as in the number of alive neighbours
  std::uniform_int_distribution<> dis2(0, size-1);

  // First we initialize a vector to be passed to the functions as in our problem (the grid)
  std::vector<int> v(size, 0);

  // Then we initialize a vector of cases (from 0 to 8) chosen randomly from a uniform distribution
  std::vector<int> Case(n);

  for (size_t i = 0; i < n; i++) {
    Case[i] = dis(gen);
  }

  // Then we initialize a vector of indices
  std::vector<int> Position(size);

  for (size_t i = 0; i < size; i++) {
    Position[i] = dis2(gen);
  }

  // Measure the time it takes to call func1 n times
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < size; j++) {
      func1(v, Position[j], Case[i]);
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Time taken to call if-else function " << n << " times: " << elapsed.count()
            << " milliseconds" << std::endl;

  // Measure the time it takes to call func2 n times
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < size; j++) {
      func2(v, Position[j], Case[i]);
    }
  }
  end = std::chrono::high_resolution_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Time taken to call switch function " << n << " times: " << elapsed.count()
            << " milliseconds" << std::endl;

  return 0;
}
