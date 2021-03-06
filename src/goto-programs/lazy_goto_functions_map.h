// Copyright 2016-2017 Diffblue Limited. All Rights Reserved.

/// \file
/// A lazy wrapper for goto_functionst.

#ifndef CPROVER_GOTO_PROGRAMS_LAZY_GOTO_FUNCTIONS_MAP_H
#define CPROVER_GOTO_PROGRAMS_LAZY_GOTO_FUNCTIONS_MAP_H

#include <unordered_set>
#include "goto_functions.h"
#include "goto_convert_functions.h"
#include <util/message.h>
#include <util/language_file.h>


/// Provides a wrapper for a map of lazily loaded goto_functiont.
/// This incrementally builds a goto-functions object, while permitting
/// access to goto programs while they are still under construction.
/// The intended workflow:
/// 1. The front-end registers the functions that are potentially
///   available, probably by use of util/language_files.h
/// 2. The main function registers functions that should be run on
///   each program, in sequence, after it is converted.
/// 3. Analyses will then access functions using the `at` function
/// \tparam bodyt: The type of the function bodies, usually goto_programt
template<typename bodyt>
class lazy_goto_functions_mapt final
{
public:
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef irep_idt key_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef goto_function_templatet<bodyt> &mapped_type;
  /// The type of elements returned by const members
  // NOLINTNEXTLINE(readability/identifiers)  - name matches mapped_type
  typedef const goto_function_templatet<bodyt> &const_mapped_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef std::pair<const key_type, goto_function_templatet<bodyt>> value_type;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef value_type &reference;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef const value_type &const_reference;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef value_type *pointer;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef const value_type *const_pointer;
  // NOLINTNEXTLINE(readability/identifiers)  - name matches those used in STL
  typedef std::size_t size_type;

  typedef
  std::function<void(goto_functionst::goto_functiont &function)>
    post_process_functiont;

private:
  typedef std::map<key_type, goto_function_templatet<bodyt>> underlying_mapt;
  underlying_mapt &goto_functions;
  /// Names of functions that are already fully available in the programt state.
  /// \remarks These functions do not need processing before being returned
  /// whenever they are requested
  mutable std::unordered_set<irep_idt, irep_id_hash> processed_functions;

  language_filest &language_files;
  symbol_tablet &symbol_table;
  // This is mutable because it has internal state that it changes during the
  // course of conversion. Strictly it should make that state mutable or
  // recreate it for each conversion, but it's easier just to store it mutable.
  mutable goto_convert_functionst convert_functions;
  const post_process_functiont post_process_function;

public:
  /// Creates a lazy_goto_functions_mapt.
  lazy_goto_functions_mapt(
    underlying_mapt &goto_functions,
    language_filest &language_files,
    symbol_tablet &symbol_table,
    post_process_functiont post_process_function,
    message_handlert &message_handler)
  : goto_functions(goto_functions),
    language_files(language_files),
    symbol_table(symbol_table),
    convert_functions(symbol_table, message_handler),
    post_process_function(std::move(post_process_function))
  {
  }

  /// Gets the body for a given function.
  /// \param name: The name of the function to search for.
  /// \return The function body corresponding to the given function.
  const_mapped_type at(const key_type &name) const
  {
    return ensure_function_loaded_internal(name).second;
  }

  /// Gets the body for a given function.
  /// \param name: The name of the function to search for.
  /// \return The function body corresponding to the given function.
  mapped_type at(const key_type &name)
  {
    return ensure_function_loaded_internal(name).second;
  }

  void unload(const key_type &name) const { goto_functions.erase(name); }

  void ensure_function_loaded(const key_type &name) const
  {
    ensure_function_loaded_internal(name);
  }

private:
  // This returns a non-const reference, but if you use this method from a
  // const method then you should not return such a reference without making it
  // const first
  reference ensure_function_loaded_internal(const key_type &name) const
  {
    reference named_function=ensure_entry_converted(name);
    mapped_type function=named_function.second;
    if(processed_functions.count(name)==0)
    {
      // Run function-pass conversions
      post_process_function(function);
      // Assign procedure-local location numbers for now
      function.body.compute_location_numbers();
      processed_functions.insert(name);
    }
    return named_function;
  }

  reference ensure_entry_converted(const key_type &name) const
  {
    typename underlying_mapt::iterator it=goto_functions.find(name);
    if(it!=goto_functions.end())
      return *it;
    // Fill in symbol table entry body if not already done
    // If this returns false then it's a stub
    language_files.convert_lazy_method(name, symbol_table);
    // Create goto_functiont
    goto_functionst::goto_functiont function;
    convert_functions.convert_function(name, function);
    // Add to map
    return *goto_functions.emplace(name, std::move(function)).first;
  }
};

#endif  // CPROVER_GOTO_PROGRAMS_LAZY_GOTO_FUNCTIONS_MAP_H
