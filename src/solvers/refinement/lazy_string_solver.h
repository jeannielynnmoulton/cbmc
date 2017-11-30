/*******************************************************************\

Module: Solver which is lazy for String constraints.

Author: Romain Brenguier

\*******************************************************************/

#ifndef CPROVER_SOLVERS_REFINEMENT_LAZY_STRING_SOLVER_H
#define CPROVER_SOLVERS_REFINEMENT_LAZY_STRING_SOLVER_H

#include "string_refinement.h"

class lazy_string_solvert final: public string_refinementt
{
public:
  explicit lazy_string_solvert(string_refinementt::infot &info);

  std::string decision_procedure_text() const override
  {
    return
      "lazy string solver with "
      + string_refinementt::decision_procedure_text();
  }
  exprt get(const exprt &expr) const override;
  void set_to(const exprt &expr, bool value) override;
  decision_proceduret::resultt dec_solve() override;

private:
  // If `k -> e` is present in the map then `k` can be evaluated as the
  // result of `e`.
  std::map<exprt, exprt> dependency;

  // Expressions that are required to be passed to the underlying solver
  std::set<exprt> required;

  void add_dependencies_and_requirements(const std::vector &equations);

  void debug_info();
};

#endif // CPROVER_SOLVERS_REFINEMENT_LAZY_STRING_SOLVER_H
