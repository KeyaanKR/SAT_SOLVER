#include <bits/stdc++.h>

class ENCODER {
public:
  std::vector<std::vector<int>> sudoku;
  std::vector<std::string> clauses;

  void inputSudoku() {
    std::clog << "Enter the Sudoku puzzle (0 for empty cells):" << std::endl;
    for (int i = 0; i < 9; i++) {
      std::vector<int> row;
      for (int j = 0; j < 9; j++) {
        int num;
        std::cin >> num;
        row.push_back(num);
      }
      sudoku.push_back(row);
    }
  }

  int varNum(int row, int col, int num) { return row * 100 + col * 10 + num; }

  void addClause(const std::string &clause) { clauses.push_back(clause); }

  void cellConstraint() {
    for (int row = 1; row <= 9; row++) {
      for (int col = 1; col <= 9; col++) {
        std::string clause;
        // At least one number in cell (row, col)
        for (int num = 1; num <= 9; num++) {
          if (num < 9)
            clause += std::to_string(varNum(row, col, num)) + " ";
          else
            clause += std::to_string(varNum(row, col, num));
        }
        addClause(clause);

        // At most one number in cell (row, col)
        for (int num1 = 1; num1 <= 9; num1++) {
          for (int num2 = num1 + 1; num2 <= 9; num2++) {
            addClause("~" + std::to_string(varNum(row, col, num1)) + " ~" +
                      std::to_string(varNum(row, col, num2)));
          }
        }
      }
    }
  }

  void rowConstraint() {
    for (int row = 1; row <= 9; row++) {
      for (int num = 1; num <= 9; num++) {
        std::string clause;
        // At least one number in row
        for (int col = 1; col <= 9; col++) {
          if (col < 9)
            clause += std::to_string(varNum(row, col, num)) + " ";
          else
            clause += std::to_string(varNum(row, col, num));
        }
        addClause(clause);

        // At most one number in row
        for (int col1 = 1; col1 <= 9; col1++) {
          for (int col2 = col1 + 1; col2 <= 9; col2++) {
            addClause("~" + std::to_string(varNum(row, col1, num)) + " ~" +
                      std::to_string(varNum(row, col2, num)));
          }
        }
      }
    }
  }

  void colConstraint() {
    for (int col = 1; col <= 9; col++) {
      for (int num = 1; num <= 9; num++) {
        std::string clause;
        // At least one number in column
        for (int row = 1; row <= 9; row++) {
          if (row < 9)
            clause += std::to_string(varNum(row, col, num)) + " ";
          else
            clause += std::to_string(varNum(row, col, num));
        }
        addClause(clause);

        // At most one number in column
        for (int row1 = 1; row1 <= 9; row1++) {
          for (int row2 = row1 + 1; row2 <= 9; row2++) {
            addClause("~" + std::to_string(varNum(row1, col, num)) + " ~" +
                      std::to_string(varNum(row2, col, num)));
          }
        }
      }
    }
  }

  void gridConstraint() {
    for (int block = 0; block < 9; block++) {
      for (int num = 1; num <= 9; num++) {
        std::string clause;
        // At least one number in block
        for (int i = 1; i <= 3; i++) {
          for (int j = 1; j <= 3; j++) {
            int row = 3 * (block / 3) + i;
            int col = 3 * (block % 3) + j;
            if (i == 3 && j == 3)
              clause += std::to_string(varNum(row, col, num));
            else
              clause += std::to_string(varNum(row, col, num)) + " ";
          }
        }
        addClause(clause);
        // At most one number in block
        for (int i1 = 1; i1 <= 3; i1++) {
          for (int j1 = 1; j1 <= 3; j1++) {
            for (int i2 = 1; i2 <= 3; i2++) {
              for (int j2 = 1; j2 <= 3; j2++) {
                if (i1 * 3 + j1 < i2 * 3 + j2) {
                  addClause("~" +
                            std::to_string(varNum(3 * (block / 3) + i1,
                                                  3 * (block % 3) + j1, num)) +
                            " ~" +
                            std::to_string(varNum(3 * (block / 3) + i2,
                                                  3 * (block % 3) + j2, num)));
                }
              }
            }
          }
        }
      }
    }
  }

  void knownConstraint() {
    for (int row = 1; row <= 9; row++) {
      for (int col = 1; col <= 9; col++) {
        if (sudoku[row - 1][col - 1] != 0) {
          addClause(std::to_string(varNum(row, col, sudoku[row - 1][col - 1])));
        }
      }
    }
  }

  void encode() {
    cellConstraint();
    rowConstraint();
    colConstraint();
    gridConstraint();
    knownConstraint();
  }

  void saveCNF(const std::string &filename) {
    std::ofstream file(filename);
    for (const auto &clause : clauses) {
      file << clause << std::endl;
    }
    file.close();
  }

  void loadSOL(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error opening file " << filename << std::endl;
      return;
    }

    std::string line;
    while (std::getline(file, line)) {
      if (line.find("Debug:") != std::string::npos) {
        continue;
      }

      if (line[0] != '~') {
        int row = (line[0] - '0');
        int col = (line[1] - '0');
        int num = (line[2] - '0');
        sudoku[row - 1][col - 1] = num;
      }
    }
    file.close();
  }

  void printSudoku() {
    std::cout << "Sudoku solution:" << std::endl;
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        std::cout << sudoku[i][j] << " ";
      }
      std::cout << std::endl;
    }
  }
};

int main() {
  ENCODER encoder;
  encoder.inputSudoku();
  encoder.encode();
  encoder.saveCNF("sudoku.cnf");

  std::cout << "\nSolving Sudoku using DPLL solver..." << std::endl;

  int result = system("./dpll sudoku.cnf");
  if (result != 0) {
    std::cerr << "Error executing DPLL solver." << std::endl;
    return 1; // Exit with error code
  }

  encoder.loadSOL("output_dpll.txt");
  encoder.printSudoku();

  if (remove("sudoku.cnf") != 0) {
    std::cerr << "Error deleting sudoku.cnf" << std::endl;
  } else {
    std::cout << "\nsudoku.cnf is deleted successfully";
  }
  if (remove("output_dpll.txt") != 0) {
    std::cerr << "Error deleting output_dpll.txt" << std::endl;
  } else {
    std::cout << "\noutput_dpll.txt is deleted successfully";
  }

  return 0;
}
