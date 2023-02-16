#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <vector>
#include <array>

// Convert an iteration number into a string that is used for naming the snapshots
std::string num_conv(const int number) {
  std::stringstream ss;
  ss << std::setw(5) << std::setfill('0') << number; //change the amount of 0s based on the number of digits of number
  std::string str = ss.str();
  return str;
}

// Convert an array of 8 zeros and ones into the corresponding character
char createCharacterFromBits(const std::array<int, 8>& bits) {
    char result = 0;
    for (int i = 0; i < 8; i++) {
    // shift the value of result to the left by 1 bit (using the << operator) and then
    // perform a logical OR operation (using the | operator) with the value of the current bit
        result = (result << 1) | bits[i];
    }
    return result;
}

// From a grid (stored in arr) print a pbm P4 image
void createSnapshot(const std::vector<int>& arr, const int size, const int step) {
  int resto = size%8; // number of significant bits for the last character in a row
  int quot = size/8; // number of characters for a row

  // Open a binary file called "snapshot_s" in write mode
  std::string num = num_conv(step); // obtain the number of iteration in the correct format
  std::ofstream snapshot("snapshot_" + num + ".pbm", std::ios::binary);

  // Iterate through the array and write each element to the file, separated by a space
  snapshot << "P4\n" << size << ' ' << size <<"\n"; // header of a pbm file with magic number 4
  int i = 0;
  while (i < size*size) {
    int i_mod = i%size; // index of the column
    if (i_mod < quot*8) { // numbers not at the end of the row
      std::array<int, 8> beet = {arr[i],arr[i+1],arr[i+2],arr[i+3],arr[i+4],arr[i+5],arr[i+6],arr[i+7]};
      char c = createCharacterFromBits(beet); // transform 8 bits into the corresponding character
      snapshot << c; // write the char to file
      i = i + 8; // go to the next group of 8 numbers
    }
    // if we are at the end of a row we should expand artificially the group to have 8 numbers
    else { std::array<int, 8> boot = {0,0,0,0,0,0,0,0}; // initialise to 0 the array
      for (int j = 0; j < resto; j++) { // write only the following "resto" numbers in the array
        if (j < resto) boot[j] = arr[i+j];
      }
      char c = createCharacterFromBits(boot);
      snapshot << c;
      i = i + size - (i%size); //goes to the first element of the next row
    }
  }
}

// Convert a character into is binary representation and write it in an array of size 8
std::array<int, 8> createBitsFromCharacter(char c) {
  std::array<int, 8> bits{};
  for (int i = 0; i < 8; i++) {
    bits[i] = (c & (1 << (7 - i))) != 0;
  }
  return bits;
}

// Copy the characters from a pbm P4 file into a vector of characters
void readSnapshot(std::vector<char>&char_file, const std::string namefile){
  std::ifstream file(namefile);
  file.seekg(0);
  std::string line;
  std::getline(file, line); // skip the first line which contains "P4"
  std::getline(file, line); // skip the second line which contains "height and width"

  // Read the characters from the file into the vector
  for (int i = 0; i < char_file.size(); i++) {
    file.read(&char_file[i], 1);
  }

  file.close();
}

// Convert a vector of characters into a vector of integers (in fact a grid), taking into account non significant bits
void convertchars(std::vector<char>& char_file, const int size, std::vector<int>& grid){
  int len = char_file.size(); // number of characters in the vector
  int k = size % 8; // number of significant bits of the last character of a row
  int n = (size/8) + 1; // number of characters that codify a single row of the grid
  #pragma omp parallel for
  for(int i = 0 ; i < len; i++) {
    char c = char_file[i];
    int rw = i/n; // number of rows already written
    int cl = i%n; // number of characters already converted in the current row
    if (k == 0) { // special case in which k is a multiple of 8 and so all bits are significant
      std::array<int, 8> bits = createBitsFromCharacter(c); // obtain the corresponding array of 0s and 1s
      std::copy(bits.begin(), bits.end(), grid.begin() + i*8); // write them in the grid
    } else { // k is not a multiple of 8
      if ((i+1) % n == 0 ) { // the character considered is at the end of a row
      std::array<int, 8> bits = createBitsFromCharacter(c);
      std::copy(bits.begin(), bits.begin() + k, grid.begin() + rw*size + 8*cl); // write only the first k bits
      } else { // the character is not at the end of a row
        std::array<int, 8> bits = createBitsFromCharacter(c);
        std::copy(bits.begin(), bits.end(), grid.begin() + rw*size + 8*cl); // write all the array in the grid
      }
    }
  }
}

// A function for printing the grid on the terminal to check the correctness of the code
void printGrid(const std::vector<int>& grid, const int rows, const int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      std::cout << grid[i*cols + j] << " ";
    }
    std::cout << "\n";
  }
  std::cout << std::endl;
}
