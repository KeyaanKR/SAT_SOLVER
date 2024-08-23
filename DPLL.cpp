#include <bits/stdc++.h>

class DPLL {
private:
  std::set<char> assign_t, assign_f;
  int n_prop = 0, n_decs = 0;

  void print_cnf(const std::vector<std::string> &cnf) {
    std::string CNF;
    // parse through the cnf
    for (const auto &clause : cnf) {
      // if the clause is not empty, add it to the string
      if (!clause.empty()) {
        CNF += "(" + clause + ")";
        std::replace(CNF.begin(), CNF.end(), ' ', '+');
      }
    }
    // if the string is empty, add an empty clause
    if (CNF.empty())
      CNF = "()";
    std::cout << CNF << std::endl;
  }

  bool is_unit_clause(const std::string &clause) {
    // check for whitespace, if found then not a unit clause
    if (clause.length() > 0 &&
        std::find(clause.begin(), clause.end(), ' ') != clause.end())
      return false;
    return true;
  }

  bool solve(std::vector<std::string> cnf, std::set<char> literals) {
    std::cout << "\nCNF = ";
    print_cnf(cnf);

    std::vector<char> new_t, new_f;
    n_decs++;

    // remove duplicate clauses
    std::sort(cnf.begin(), cnf.end());
    cnf.erase(std::unique(cnf.begin(), cnf.end()), cnf.end());

    // find unit clauses
    std::vector<std::string> units;
    std::copy_if(cnf.begin(), cnf.end(), std::back_inserter(units),
                 [](const std::string &clause) { return clause.length() < 3; });

    // if there are unit clauses
    if (!units.empty()) {
      for (const auto &unit : units) {
        n_prop++;
        // if the unit is a negation
        if (unit[0] == '~') {
          new_f.push_back(unit[1]);
          assign_f.insert(unit[1]);
          cnf.erase(std::remove(cnf.begin(), cnf.end(), unit), cnf.end());
          // remove the negation from clauses
          for (auto &clause : cnf) {
            auto pos = clause.find(unit[1]);
            // if the negation is found, remove it
            if (pos != std::string::npos) {
              clause.erase(pos, 1);
              clause.erase(std::remove(clause.begin(), clause.end(), ' '),
                           clause.end());
            }
          }
        } else {
          new_t.push_back(unit[0]);
          assign_t.insert(unit[0]);
          // remove the unit from the clauses
          for (auto it = cnf.begin(); it != cnf.end();) {
            auto pos = it->find("~" + unit);
            if (pos != std::string::npos) {
              it->erase(pos, 2);
              it->erase(std::remove(it->begin(), it->end(), ' '), it->end());
            } else if (it->find(unit) != std::string::npos) {
              it = cnf.erase(it);
              continue;
            }
            it++;
          }
        }
      }
    }

    std::cout << "units = ";
    for (const auto &unit : units)
      std::cout << unit << " ";
    std::cout << "\nCNF after unit propagation = ";
    print_cnf(cnf);

    // if the cnf is empty, return true
    if (cnf.empty())
      return true;
    // if there is an empty clause, return false and backtrack
    if (std::any_of(cnf.begin(), cnf.end(),
                    [](const std::string &clause) { return clause.empty(); })) {
      // remove the assignments and backtrack
      for (char literal : new_t)
        assign_t.erase(literal);
      for (char literal : new_f)
        assign_f.erase(literal);
      std::cout << "Null clause found, backtracking..." << std::endl;
      return false;
    }

    literals.clear();
    for (const auto &clause : cnf) {
      for (char c : clause) {
        if (std::isalpha(c) || std::isdigit(c))
          literals.insert(c);
      }
    }

    if (!literals.empty()) {
      char literal = *literals.begin();
      std::vector<std::string> new_cnf = cnf;
      new_cnf.push_back(std::string(1, literal));

      if (solve(new_cnf, literals))
        return true;

      new_cnf = cnf;
      new_cnf.push_back("~" + std::string(1, literal));
      if (solve(new_cnf, literals))
        return true;
    }

    for (char literal : new_t)
      assign_t.erase(literal);
    for (char literal : new_f)
      assign_f.erase(literal);
    return false;
  }

public:
  void dpll(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error opening file " << filename << std::endl;
      return;
    }

    std::vector<std::string> cnf;
    std::set<char> literals;
    std::string line;

    while (std::getline(file, line)) {
      cnf.push_back(line);
      for (char c : line) {
        if (std::isalpha(c) || std::isdigit(c))
          literals.insert(c);
      }
    }

    if (solve(cnf, literals)) {
      std::cout << "\nNumber of Splits = " << n_decs << std::endl;
      std::cout << "Unit Propagations = " << n_prop << std::endl;
      std::cout << "\nResult: SATISFIABLE" << std::endl;
      std::cout << "Solution:" << std::endl;
      for (char literal : assign_t)
        std::cout << "\t\t" << literal << " = True" << std::endl;
      for (char literal : assign_f)
        std::cout << "\t\t" << literal << " = False" << std::endl;
    } else {
      std::cout << "\nReached starting node!" << std::endl;
      std::cout << "Number of Splits = " << n_decs << std::endl;
      std::cout << "Unit Propagations = " << n_prop << std::endl;
      std::cout << "\nResult: UNSATISFIABLE" << std::endl;
    }
    std::cout << std::endl;
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
    return 1;
  }

  DPLL solver;
  solver.dpll(argv[1]);

  return 0;
}
