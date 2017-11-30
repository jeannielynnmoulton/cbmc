/*******************************************************************\

Module: Solver which is lazy for String constraints.

Author: Romain Brenguier

\*******************************************************************/

#include "lazy_string_solver.h"

lazy_string_solvert::lazy_string_solvert(string_refinementt::infot &info)
  : string_refinementt(info)
{ }

exprt lazy_string_solvert::get(const exprt &expr) const
{
  return string_refinementt::get(expr);
}

void lazy_string_solvert::set_to(const exprt &expr, bool value)
{
  string_refinementt::set_to(expr, value);
}

decision_proceduret::resultt lazy_string_solvert::dec_solve()
{
  return string_refinementt::dec_solve();
}
