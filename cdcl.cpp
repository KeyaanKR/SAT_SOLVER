#include <bits/stdc++.h>

class DPLL {
private:
  std::set<std::string> assign_t, assign_f;
  int n_prop = 0, n_decs = 0;

  std::unordered_map<std::string, double> score_map;
  std::vector<std::string> variable_order;

  int conflicts = 0;
  const double bonus = 1.0;
  const double level_factor = 2.0;
  const int level_interval = 1000;

  // initialize the scores to the frequency of the variables in the clauses
  void initialize_scores(const std::vector<std::string> &cnf) {
    for (const auto &clause : cnf) {
      std::istringstream iss(clause);
      std::string literal;
      while (iss >> literal) {
        std::string var = (literal[0] == '~') ? literal.substr(1) : literal;
        if (score_map.find(var) == score_map.end()) {
          score_map[var] = 0.0;
        }
        score_map[var] += 1.0;
      }
    }
    update_variable_order();
  }

  // arrange the variables in descending order
  void update_variable_order() {
    variable_order.clear();
    for (const auto &entry : score_map) {
      variable_order.push_back(entry.first);
    }
    std::sort(variable_order.begin(), variable_order.end(),
              [this](const std::string &a, const std::string &b) {
                return score_map[a] > score_map[b];
              });
  }

  // choose the variable with maxmium score
  std::string decision(std::set<std::string> &literals) {
    for (const auto &var : variable_order) {
      if (literals.find(var) != literals.end()) {
        return var;
      }
    }
    return *literals.begin();
  }

  // give bonus to the variables in the learned clause, and level the scores
  void update_scores(const std::vector<std::string> &learned_clause) {
    for (const auto &literal : learned_clause) {
      std::string var = (literal[0] == '~') ? literal.substr(1) : literal;
      score_map[var] += bonus;
    }

    conflicts++;

    if (conflicts % level_interval == 0) {
      for (auto &entry : score_map) {
        entry.second /= level_factor;
      }
    }

    update_variable_order();
  }

  void print_cnf(const std::vector<std::string> &cnf) {
    std::string CNF;
    // parse through the cnf
    for (const auto &clause : cnf) {
      // if the clause is not empty, add it to the string
      if (!clause.empty())
        CNF += "(" + clause + ")";
    }
    // if the string is empty, add an empty clause
    if (CNF.empty())
      CNF = "()";
    std::cout << CNF << std::endl;
  }

  bool is_unit_clause(const std::string &clause) {
    std::istringstream iss(clause);
    std::vector<std::string> literals;
    std::string literal;

    while (iss >> literal)
      literals.push_back(literal);

    return literals.size() == 1;
  }

  void removeUnitfromClauses(std::vector<std::string> &cnf,
                             const std::string &unit) {
    for (auto &clause : cnf) {
      std::istringstream iss(clause);
      std::vector<std::string> words;
      std::string word;

      while (iss >> word) {
        if (word != unit) {
          // Only keep the word if it's not the unit
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

  void deleteClause(std::vector<std::string> &cnf, const std::string &unit) {
    cnf.erase(std::remove_if(cnf.begin(), cnf.end(),
                             [&unit](const std::string &clause) {
                               std::istringstream iss(clause);
                               std::string word;
                               while (iss >> word) {
                                 // if the unit is found in the clause, remove
                                 // the clause
                                 if (word == unit) {
                                   return true;
                                 }
                               }
                               return false;
                             }),
              cnf.end());
  }

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
        // to prevent unit clause from being added to pureLiterals
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

  bool solve(std::vector<std::string> cnf, std::set<std::string> literals) {
    std::vector<std::string> new_t, new_f;
    n_decs++;
    std::clog << "\rDecision: " << n_decs << std::flush;

    // remove duplicate clauses
    std::sort(cnf.begin(), cnf.end());
    cnf.erase(std::unique(cnf.begin(), cnf.end()), cnf.end());

    // find unit clauses
    std::vector<std::string> units;
    std::copy_if(
        cnf.begin(), cnf.end(), std::back_inserter(units),
        [this](const std::string &clause) { return is_unit_clause(clause); });

    // find pure literals
    find_pureLiterals(cnf, units);

    // if there are unit clauses
    if (!units.empty()) {
      for (const auto &unit : units) {
        n_prop++;
        // if the unit is a negation
        if (unit[0] == '~') {
          new_f.push_back(unit.substr(1));
          assign_f.insert(unit.substr(1));
          // remove the clauses with the unit
          deleteClause(cnf, unit);
          // remove the negation from clauses
          removeUnitfromClauses(cnf, unit.substr(1));
        } else {
          new_t.push_back(unit);
          assign_t.insert(unit);
          // remove the clauses with the unit
          deleteClause(cnf, unit);
          // remove the unit from the clauses
          removeUnitfromClauses(cnf, '~' + unit);
        }
      }
    }

    // if the cnf is empty, return true
    if (cnf.empty())
      return true;
    // if there is an empty clause, return false
    if (std::any_of(cnf.begin(), cnf.end(),
                    [](const std::string &clause) { return clause.empty(); })) {
      // remove the assignments
      for (std::string literal : new_t)
        assign_t.erase(literal);
      for (std::string literal : new_f)
        assign_f.erase(literal);
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
      std::string literal = decision(literals);
      std::vector<std::string> new_cnf = cnf;
      new_cnf.push_back(literal);

      if (solve(new_cnf, literals))
        return true;

      new_cnf = cnf;
      new_cnf.push_back("~" + literal);
      if (solve(new_cnf, literals))
        return true;

      std::vector<std::string> learned_clause = {
          (literal[0] == '~') ? literal.substr(1) : "~" + literal};
      update_scores(learned_clause);
    }

    for (std::string literal : new_t)
      assign_t.erase(literal);
    for (std::string literal : new_f)
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

    initialize_scores(cnf);

    if (solve(cnf, literals)) {
      std::cout << "\nResult: SATISFIABLE" << std::endl;
      std::cout << "Solution written output_dpll.txt" << std::endl;
      std::ofstream output("output_dpll.txt");
      for (const auto &literal : assign_t)
        output << literal << std::endl;
      for (const auto &literal : assign_f)
        output << "~" << literal << std::endl;
      output.close();
    } else {
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
