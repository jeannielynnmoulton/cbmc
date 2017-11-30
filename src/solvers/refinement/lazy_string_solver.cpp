/*******************************************************************\

Module: Solver which is lazy for String constraints.

Author: Romain Brenguier

\*******************************************************************/

#include <util/ssa_expr.h>
#include <algorithm>
#include <stack>
#include "lazy_string_solver.h"
#include <util/expr_iterator.h>
#include <java_bytecode/java_types.h>

lazy_string_solvert::lazy_string_solvert(string_refinementt::infot &info)
  : string_refinementt(info)
{ }

exprt lazy_string_solvert::get(const exprt &expr) const
{
  auto it = dependency.find(expr);
  if(it == dependency.end())
    return string_refinementt::get(expr);

  debug() << "get " << from_expr(ns, "", expr) << " of type "
          << from_type(ns, "", expr.type()) << eom;

  if(expr.type().id() == ID_array)
  {
    array_exprt array(to_array_type(expr.type()));
    array.operands().push_back(from_integer('f', java_char_type()));
    array.operands().push_back(from_integer('o', java_char_type()));
    array.operands().push_back(from_integer('o', java_char_type()));
    return array;
  }
  else if(expr.type().id() == ID_signedbv || expr.type().id() == ID_signedbv)
    return from_integer(0, expr.type());
  else
    return string_refinementt::get(expr);
}

void lazy_string_solvert::set_to(const exprt &expr, bool value)
{
  if(expr.id() != ID_equal)
    required.insert(expr);
  string_refinementt::set_to(expr, value);
}

static bool can_be_lazy(const equal_exprt &e)
{
  if(const auto app =
    expr_try_dynamic_cast<function_application_exprt>(e.rhs()))
  {
    const auto &name=app->function();
    const irep_idt &id=
      is_ssa_expr(name)
      ?to_ssa_expr(name).get_object_name()
      :to_symbol_expr(name).get_identifier();

    return id==ID_cprover_string_concat_func;
  }
  else
    return e.rhs().id() == ID_symbol;
}

static bool has_subexpr(const exprt &e1, const exprt &e2)
{
  const auto it = std::find(e1.depth_begin(), e1.depth_end(), e2);
  return it != e1.depth_end();
}

static std::vector<equal_exprt> filter_out_equations_captured_by_union_find(
  const std::vector &equations,
  const union_find_replacet &ufr)
{
  std::vector<equal_exprt> filtered;
  std::copy_if(
    equations.begin(),
    equations.end(),
    std::back_inserter(filtered),
    [&](const equal_exprt &e)
    {
      return ufr.find(e.lhs()) != ufr.find(e.rhs());
    });
  return filtered;
}

void lazy_string_solvert::add_dependencies_and_requirements(
  const std::vector &equations)
{
  for(const auto &e : equations)
  {
    if(has_char_array_subexpr(e, ns))
    {
      auto it = dependency.insert(std::make_pair(e.lhs(), e.rhs()));
      if(!it.second)
      {
        debug() << "Warning: " << from_expr(ns, "", e.lhs())
                << "already present\n";
        required.insert(e.lhs());
      }
      else if(!can_be_lazy(e))
        required.insert(e.lhs());
    }
  }
}

/// Propagate requirements according to dependencies
static void propagate_requirements(
  const std::map<exprt, exprt> &dependency, std::set<exprt> &required)
{
  std::vector<exprt> stack(required.begin(), required.end());

  while(!stack.empty())
  {
    const exprt e = stack.back();
    stack.pop_back();

    for(const auto &pair : dependency)
    {
      if(has_subexpr(pair.second, e))
      {
        required.insert(pair.first);
        stack.push_back(pair.first);
      }
    }
  }
}

/// Remove non required equations
static void remove_non_required(
  std::vector<equal_exprt> &equations, const std::set<exprt> &required)
{
  auto new_equations_size = std::remove_if(
    equations.begin(),
    equations.end(),
    [&](const equal_exprt &e)
    {
      return required.find(e.lhs()) == required.end();
    });
  equations.erase(new_equations_size, equations.end());
}

/// Print debug information about the content of the class
void lazy_string_solvert::debug_info()
{
  debug() << "Dependencies:\n";
  for(const auto &pair : dependency)
  {
    debug() << "  * " << from_expr(ns, "", pair.first)
            << " -> " << from_expr(ns, "", pair.second)
            << pair.second.id() << "\n";
  }

  debug() << "Requirements:\n";
  for(const auto &e : required)
  {
    debug() << "  < " << from_expr(ns, "", e) << eom;
  }
}

decision_proceduret::resultt lazy_string_solvert::dec_solve()
{
  symbol_resolve =
    generate_symbol_resolution_from_equations(equations, ns, debug());

  std::vector<equal_exprt> filtered_equations =
    filter_out_equations_captured_by_union_find(equations, symbol_resolve);

  replace_symbols_in_equations(symbol_resolve, filtered_equations);

  add_dependencies_and_requirements(filtered_equations);

  debug_info();

  propagate_requirements(dependency, required);

  remove_non_required(filtered_equations, required);

  // Give filtered_equations to the solver
  filtered_equations.swap(equations);
  output_equations(debug(), equations, ns);
  const auto result = string_refinementt::dec_solve();

  // Re-establish initial content of equations
  equations = std::move(filtered_equations);
  return result;
}
