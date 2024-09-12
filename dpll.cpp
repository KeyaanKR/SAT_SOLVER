#include <bits/stdc++.h>

class DPLL {
private:
  std::set<std::string> assign_t, assign_f;
  std::atomic<int> depth{0};
  std::atomic<bool> solution_found{false};
  std::mutex mtx;

  // checks if a clause is a unit clause
  bool is_unit_clause(const std::string &clause) {
    std::istringstream iss(clause);
    std::vector<std::string> literals;
    std::string literal;

    while (iss >> literal)
      literals.push_back(literal);

    return literals.size() == 1;
  }

  // removes the negation of a literal in a unit clause from all clauses
  void removeUnitfromClauses(std::vector<std::string> &cnf,
                             const std::string &unit) {
    for (auto &clause : cnf) {
      std::istringstream iss(clause);
      std::vector<std::string> words;
      std::string word;

      while (iss >> word) {
        if (word != unit) {
          words.push_back(word);
        }
      }

      std::ostringstream oss;
      for (size_t i = 0; i < words.size(); ++i) {
        if (i > 0) {
          oss << " ";
        }
        oss << words[i];
      }

      clause = oss.str();
    }
  }

  // deletes the clausea containing the unit clause
  void deleteClause(std::vector<std::string> &cnf, const std::string &unit) {
    cnf.erase(std::remove_if(cnf.begin(), cnf.end(),
                             [&unit](const std::string &clause) {
                               std::istringstream iss(clause);
                               std::string word;
                               while (iss >> word) {
                                 if (word == unit) {
                                   return true;
                                 }
                               }
                               return false;
                             }),
              cnf.end());
  }

  // finds pure literals in the cnf
  void find_pureLiterals(std::vector<std::string> cnf,
                         std::vector<std::string> &pureLiterals) {
    std::unordered_map<std::string, int> posLiteralCount;
    std::unordered_map<std::string, int> negLiteralCount;

    for (const auto &clause : cnf) {
      std::istringstream iss(clause);
      std::string literal;
      while (iss >> literal) {
        if (literal[0] == '~') {
          negLiteralCount[literal.substr(1)]++;
        } else {
          posLiteralCount[literal]++;
        }
      }
    }

    for (const auto &literal : posLiteralCount) {
      if (literal.second > 0 && negLiteralCount[literal.first] == 0) {
        if (std::find(pureLiterals.begin(), pureLiterals.end(),
                      literal.first) == pureLiterals.end())
          pureLiterals.push_back(literal.first);
      }
    }

    for (const auto &literal : negLiteralCount) {
      if (literal.second > 0 && posLiteralCount[literal.first] == 0) {
        if (std::find(pureLiterals.begin(), pureLiterals.end(),
                      "~" + literal.first) == pureLiterals.end())
          pureLiterals.push_back("~" + literal.first);
      }
    }
  }

  bool solve(std::vector<std::string> cnf, std::set<std::string> literals,
             std::set<std::string> local_assign_t,
             std::set<std::string> local_assign_f, int depth = 0) {

    // if a solution has been found by another thread, return false
    if (solution_found.load(std::memory_order_acquire))
      return false;

    // remove duplicates
    std::sort(cnf.begin(), cnf.end());
    cnf.erase(std::unique(cnf.begin(), cnf.end()), cnf.end());

    // checks for unit clauses
    std::vector<std::string> units;
    std::copy_if(
        cnf.begin(), cnf.end(), std::back_inserter(units),
        [this](const std::string &clause) { return is_unit_clause(clause); });

    // checks for pure literals
    find_pureLiterals(cnf, units);

    // assign the unit clauses and pure literals
    if (!units.empty()) {
      for (const auto &unit : units) {
        // removes clauses containing the unit clause,
        // and removes the negation of the unit clause from all clauses
        if (unit[0] == '~') {
          local_assign_f.insert(unit.substr(1));
          deleteClause(cnf, unit);
          removeUnitfromClauses(cnf, unit.substr(1));
        } else {
          local_assign_t.insert(unit);
          deleteClause(cnf, unit);
          removeUnitfromClauses(cnf, '~' + unit);
        }
      }
    }

    if (cnf.empty()) {
      // if other threads have not found a solution yet and this thread has
      // write the assignment to the global variables, and change the flag
      if (!solution_found.exchange(true, std::memory_order_release)) {
        // to prevent simultaneous writing to the global variables
        std::lock_guard<std::mutex> lock(mtx);
        assign_t = local_assign_t;
        assign_f = local_assign_f;
        return true;
      }
      return false;
    }

    // if there are empty clauses, return false
    if (std::any_of(cnf.begin(), cnf.end(),
                    [](const std::string &clause) { return clause.empty(); })) {
      return false;
    }

    literals.clear();
    for (const auto &clause : cnf) {
      std::istringstream iss(clause);
      std::string literal;

      while (iss >> literal) {
        if (literal[0] == '~')
          literals.insert(literal.substr(1));
        else
          literals.insert(literal);
      }
    }

    if (!literals.empty()) {
      std::string literal = *literals.begin();
      std::vector<std::string> new_cnf_t = cnf;
      std::vector<std::string> new_cnf_f = cnf;
      new_cnf_t.push_back(literal);
      new_cnf_f.push_back("~" + literal);

      std::vector<std::future<bool>> futures;

      if (depth < 2) {
        // create two threads to explore the two branches

        // one thread with the literal assigned to true
        auto local_assign_t_true = local_assign_t;
        auto local_assign_f_true = local_assign_f;
        local_assign_t_true.insert(literal);
        // one thread with the literal assigned to false
        auto local_assign_t_false = local_assign_t;
        auto local_assign_f_false = local_assign_f;
        local_assign_f_false.insert(literal);

        // launch the threads
        // emplace_back is used to avoid copying the futures, to save memory
        futures.emplace_back(std::async(
            std::launch::async, [this, new_cnf_t, literals, local_assign_t_true,
                                 local_assign_f_true, depth]() {
              return solve(new_cnf_t, literals, local_assign_t_true,
                           local_assign_f_true, depth + 1);
            }));
        futures.emplace_back(
            std::async(std::launch::async,
                       [this, new_cnf_f, literals, local_assign_t_false,
                        local_assign_f_false, depth]() {
                         return solve(new_cnf_f, literals, local_assign_t_false,
                                      local_assign_f_false, depth + 1);
                       }));

        bool result = false;
        // if one of the threads found a solution, return true
        for (auto &f : futures) {
          if (f.get())
            result = true;
        }

        return result;
      } else {
        auto local_assign_t_true = local_assign_t;
        auto local_assign_f_true = local_assign_f;
        local_assign_t_true.insert(literal);

        auto local_assign_t_false = local_assign_t;
        auto local_assign_f_false = local_assign_f;
        local_assign_f_false.insert(literal);

        return solve(new_cnf_t, literals, local_assign_t_true,
                     local_assign_f_true, depth + 1) ||
               solve(new_cnf_f, literals, local_assign_t_false,
                     local_assign_f_false, depth + 1);
      }
    }

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
    std::set<std::string> literals;
    std::string line;

    while (std::getline(file, line)) {
      cnf.push_back(line);
      std::istringstream iss(line);
      std::string literal;

      while (iss >> literal) {
        if (literal[0] == '~')
          literals.insert(literal.substr(1));
        else
          literals.insert(literal);
      }
    }

    std::cout << "Solving " << filename << "..." << std::endl;

    if (solve(cnf, literals, std::set<std::string>(),
              std::set<std::string>())) {
      std::cout << "\nSATISFIABLE" << std::endl;
      std::cout << "assignment written to output_dpll.txt" << std::endl;
      std::ofstream output("output_dpll.txt");
      {
        std::lock_guard<std::mutex> lock(mtx);
        for (const auto &literal : assign_t)
          output << literal << std::endl;
        for (const auto &literal : assign_f)
          output << "~" << literal << std::endl;
      }
      output.close();
    } else {
      std::cout << "\nUNSATISFIABLE" << std::endl;
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
