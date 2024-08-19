#include <bits/stdc++.h>

class literal {
public:
  int variable;
  bool value;
  bool is_assigned;

  literal(int variable, bool value, bool is_assigned)
      : variable(variable), value(value), is_assigned(is_assigned) {}

  bool is_positive() const { return variable > 0; }

  int get_variable() const { return std::abs(variable); }

  bool evaluate() const {
    if (!is_assigned) {
      throw std::runtime_error("Cannot evaluate unassigned literal");
    }
    return is_positive() ? value : !value;
  }

  literal get_negation() const {
    // Negating both the variable and its value
    return literal(-variable, !value, is_assigned);
  }

  bool operator==(const literal &other) const {
    return variable == other.variable && value == other.value &&
           is_assigned == other.is_assigned;
  }
};

class clause {
public:
  std::vector<literal> literals;
  bool is_satisfied = false;

  clause(std::vector<literal> literals, bool is_satisfied)
      : literals(literals), is_satisfied(is_satisfied) {}

  bool evaluate_clause() {
    for (auto &lit : literals) {
      if (lit.is_assigned && lit.evaluate()) {
        is_satisfied = true;
        return true;
      }
    }
    return false;
  }
};

class formula {
private:
  std::vector<clause> clauses;
  std::vector<literal> assignment;

public:
  formula(std::vector<clause> clauses, std::vector<literal> assignment)
      : clauses(clauses), assignment(assignment) {}

  bool satisfied() {
    for (auto &c : clauses) {
      if (!c.evaluate_clause()) {
        return false;
      }
    }
    return true;
  }

  void unit_propagation() {
    std::vector<literal> unit_literals;

    for (auto &c : clauses) {
      if (c.literals.size() == 1 && !c.literals[0].is_assigned) {
        c.is_satisfied = true;
        c.literals[0].is_assigned = true;
        unit_literals.push_back(c.literals[0]);
      }
    }

    while (!unit_literals.empty()) {
      literal l = unit_literals.back();
      unit_literals.pop_back();

      for (auto &c : clauses) {
        if (!c.is_satisfied) {
          for (auto &lit : c.literals) {
            if (lit.get_variable() == l.get_variable()) {
              if (lit.is_positive() == l.is_positive()) {
                c.is_satisfied = true;
              } else {
                c.literals.erase(
                    std::remove(c.literals.begin(), c.literals.end(), lit),
                    c.literals.end());
                if (c.literals.size() == 1 && !c.literals[0].is_assigned) {
                  unit_literals.push_back(c.literals[0]);
                  c.literals[0].is_assigned = true;
                }
              }
              break;
            }
          }
        }
      }
    }
  }

  bool dpll() {
    unit_propagation();

    if (satisfied()) {
      return true;
    }

    for (const auto &c : clauses) {
      if (c.literals.empty() && !c.is_satisfied) {
        return false;
      }
    }

    int var = choose_variable();
    if (var == 0) {
      return false;
    }

    literal l1(var, true, true);
    if (dpll_recursive(l1)) {
      return true;
    }

    literal l2(var, false, true);
    return dpll_recursive(l2);
  }

private:
  bool dpll_recursive(const literal &lit) {
    auto saved_clauses = clauses;
    auto saved_assignment = assignment;

    assignment.push_back(lit);

    for (auto &c : clauses) {
      if (!c.is_satisfied) {
        auto it = std::find_if(
            c.literals.begin(), c.literals.end(),
            [&lit](const literal &l) { return l.variable == lit.variable; });
        if (it != c.literals.end()) {
          if (it->is_positive() == lit.value) {
            c.is_satisfied = true;
          } else {
            c.literals.erase(it);
          }
        }
      }
    }

    bool result = dpll();

    if (!result) {
      clauses = std::move(saved_clauses);
      assignment = std::move(saved_assignment);
    }

    return result;
  }

  int choose_variable() {
    for (const auto &c : clauses) {
      if (!c.is_satisfied) {
        for (const auto &l : c.literals) {
          if (!l.is_assigned) {
            return l.get_variable();
          }
        }
      }
    }
    return 0;
  }
};

int main() {
  literal l1(1, false, false);  // positive literal for variable 1
  literal l2(2, false, false);  // negative literal for variable 2
  literal l3(3, false, false);  // positive literal for variable 3
  literal l4(4, false, false);  // positive literal for variable 1
  literal l5(-1, false, false); // negative literal for variable 2
  literal l6(-2, false, false); // positive literal for variable 3
  literal l7(-3, false, false); // positive literal for variable 1
  literal l8(-4, false, false); // negative literal for variable 2

  clause c1({l1, l2, l7}, false);
  clause c2({l5, l6}, false);
  clause c3({l3, l4}, false);
  clause c4({l7, l8}, false);
  clause c5({l5, l2, l3, l4}, false);
  clause c6({l1, l6, l7, l8}, false);
  clause c7({l1}, false);
  clause c8({l5}, false);

  formula f({c1, c2, c3, c4, c5, c6}, {});

  bool is_satisfiable = f.dpll();

  std::cout << "Formula is "
            << (is_satisfiable ? "satisfiable" : "unsatisfiable") << std::endl;

  return 0;
}
