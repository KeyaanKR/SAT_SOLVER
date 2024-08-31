#include <bits/stdc++.h>

const int SIZE = 9; // Size of the Sudoku grid

void inputSudoku(int sudoku[SIZE][SIZE]) {
  std::clog << "Enter the Sudoku puzzle (0 for empty cells):"
            << std::endl; // Use clog for input prompt
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      std::cin >> sudoku[i][j]; // Input the Sudoku numbers
    }
  }
}

void printSudoku(int sudoku[SIZE][SIZE]) {
  std::clog << "Sudoku Puzzle:" << std::endl; // Use clog for output
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      std::clog << sudoku[i][j] << " "; // Print each number
    }
    std::clog << std::endl; // New line after each row
  }
}

int main() {
  int sudoku[SIZE][SIZE] = {0}; // Initialize the Sudoku grid with zeros
  inputSudoku(sudoku);          // Input the Sudoku puzzle
  printSudoku(sudoku);          // Print the entered Sudoku puzzle
  return 0;
}
