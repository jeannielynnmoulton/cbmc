/*******************************************************************\

Module: Unit test for dependence_graph.h

Author: Chris Smowton, chris.smowton@diffblue.com

\*******************************************************************/

#include <iostream>

#include <testing-utils/catch.hpp>
#include <analyses/dependence_graph.h>
#include <util/symbol_table.h>
#include <util/std_code.h>
#include <util/c_types.h>
#include <util/arith_tools.h>
#include <goto-programs/goto_convert_functions.h>
#include <langapi/mode.h>
#include <java_bytecode/java_bytecode_language.h>

static symbolt create_void_function_symbol(
  const irep_idt &name,
  const codet &code)
{
  code_typet void_function_type;
  symbolt function;
  function.name = name;
  function.type = void_function_type;
  function.mode = ID_java;
  function.value = code;
  return function;
}

const std::set<goto_programt::const_targett>&
    dependence_graph_test_get_control_deps(const dep_graph_domaint &domain)
{
  return domain.control_deps;
}

const std::set<goto_programt::const_targett>&
    dependence_graph_test_get_data_deps(const dep_graph_domaint &domain)
{
  return domain.data_deps;
}

SCENARIO("dependence_graph", "[core][analyses][dependence_graph]")
{
  GIVEN("A call under a control dependency")
  {
    // Create code like:
    // void __CPROVER__start() {
    //   int x;
    //   if(NONDET(int) == 0) {
    //     b();
    //     x = 1;
    //   }
    // }
    // void b() { }

    register_language(new_java_bytecode_language);

    goto_modelt goto_model;
    namespacet ns(goto_model.symbol_table);

    typet int_type = signed_int_type();

    symbolt x_symbol;
    x_symbol.name = id2string(goto_functionst::entry_point()) + "::x";
    x_symbol.base_name = "x";
    x_symbol.type = int_type;
    x_symbol.is_lvalue = true;
    x_symbol.is_state_var = true;
    x_symbol.is_thread_local = true;
    x_symbol.is_file_local = true;
    goto_model.symbol_table.add(x_symbol);

    code_typet void_function_type;

    code_blockt a_body;
    code_declt declare_x(x_symbol.symbol_expr());
    a_body.move_to_operands(declare_x);

    code_ifthenelset if_block;

    if_block.cond() =
      equal_exprt(
        side_effect_expr_nondett(int_type),
        from_integer(0, int_type));

    code_blockt then_block;
    code_function_callt call;
    call.function() = symbol_exprt("b", void_function_type);
    then_block.move_to_operands(call);
    code_assignt assign_x(
      x_symbol.symbol_expr(), from_integer(1, int_type));
    then_block.move_to_operands(assign_x);

    if_block.then_case() = then_block;

    a_body.move_to_operands(if_block);

    goto_model.symbol_table.add(
      create_void_function_symbol(goto_functionst::entry_point(), a_body));
    goto_model.symbol_table.add(
      create_void_function_symbol("b", code_skipt()));

    stream_message_handlert msg(std::cerr);
    goto_convert(goto_model, msg);

    WHEN("Constructing a dependence graph")
    {
      dependence_grapht dep_graph(ns);
      dep_graph(goto_model.goto_functions, ns);

      THEN("The function call and assignment instructions "
           "should have a control dependency")
      {
        for(std::size_t node_idx = 0; node_idx < dep_graph.size(); ++node_idx)
        {
          const dep_nodet &node = dep_graph[node_idx];
          const dep_graph_domaint &node_domain = dep_graph[node.PC];
          if(node.PC->is_assign() || node.PC->is_function_call())
          {
            REQUIRE(
              dependence_graph_test_get_control_deps(node_domain).size() == 1);
          }
        }
      }

      THEN("The graph's domain and its grapht representation "
           "should be consistent")
      {
        for(std::size_t node_idx = 0; node_idx < dep_graph.size(); ++node_idx)
        {
          const dep_nodet &node = dep_graph[node_idx];
          const dep_graph_domaint &node_domain = dep_graph[node.PC];
          const std::set<goto_programt::const_targett> &control_deps =
            dependence_graph_test_get_control_deps(node_domain);
          const std::set<goto_programt::const_targett> &data_deps =
            dependence_graph_test_get_data_deps(node_domain);

          std::size_t domain_dep_count =
            control_deps.size() + data_deps.size();

          REQUIRE(domain_dep_count == node.in.size());

          for(const auto &dep_edge : node.in)
          {
            if(dep_edge.second.get() == dep_edget::kindt::CTRL)
              REQUIRE(control_deps.count(dep_graph[dep_edge.first].PC));
            else if(dep_edge.second.get() == dep_edget::kindt::DATA)
              REQUIRE(data_deps.count(dep_graph[dep_edge.first].PC));
          }
        }
      }
    }
  }
}
