#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "zcc/Parse/Grammar.h"

namespace zcc {

// Canonical LR(1) parse table built from a grammar given as names. A name that
// never appears on a left-hand side is a terminal; "$" is the end marker.
// Production 0 must be the augmented rule START -> start.
class LR1Table {
public:
  enum class ActionKind : std::uint8_t { Error, Shift, Reduce, Accept };
  struct Action {
    ActionKind kind = ActionKind::Error;
    int value = 0; // shift: next state; reduce: production index
  };

  // Throws std::logic_error if the grammar is not LR(1).
  explicit LR1Table(const std::vector<Production> &productions);

  // Terminal id for a name, or -1 if the name is not a terminal here.
  int terminal(const std::string &name) const;
  Action action(int state, int terminal) const;
  // State reached after reducing production p from the given state.
  int gotoAfterReduce(int state, int production) const;
  std::size_t rhsSize(int production) const { return rhs_[production].size(); }
  std::size_t stateCount() const { return actions_.size(); }

private:
  std::map<std::string, int> ids_;
  std::vector<bool> isNonterminal_;
  std::vector<int> lhs_;
  std::vector<std::vector<int>> rhs_;
  std::vector<std::map<int, int>> transitions_; // symbol -> state
  std::vector<std::map<int, Action>> actions_;  // terminal -> action
};

} // namespace zcc
