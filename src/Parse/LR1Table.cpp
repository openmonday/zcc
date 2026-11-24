#include "zcc/Parse/LR1Table.h"

#include <set>
#include <stdexcept>
#include <tuple>

namespace zcc {
namespace {

// (production, dot position, lookahead terminal)
using Item = std::tuple<int, int, int>;
using ItemSet = std::set<Item>;

} // namespace

LR1Table::LR1Table(const std::vector<Production> &productions) {
  auto intern = [this](const std::string &name) {
    auto it = ids_.find(name);
    if (it != ids_.end()) return it->second;
    int id = static_cast<int>(ids_.size());
    ids_.emplace(name, id);
    return id;
  };

  const int dollar = intern("$");
  for (const Production &p : productions) {
    lhs_.push_back(intern(p.lhs));
    std::vector<int> rhs;
    for (const std::string &s : p.rhs) rhs.push_back(intern(s));
    rhs_.push_back(std::move(rhs));
  }

  const std::size_t nsym = ids_.size();
  isNonterminal_.assign(nsym, false);
  for (int a : lhs_) isNonterminal_[a] = true;

  // nullable and FIRST sets, by fixed-point iteration.
  std::vector<bool> nullable(nsym, false);
  std::vector<std::set<int>> first(nsym);
  for (bool changed = true; changed;) {
    changed = false;
    for (std::size_t p = 0; p < rhs_.size(); ++p) {
      const int a = lhs_[p];
      bool allNullable = true;
      for (int x : rhs_[p]) {
        if (!isNonterminal_[x]) {
          if (first[a].insert(x).second) changed = true;
          allNullable = false;
          break;
        }
        for (int t : first[x])
          if (first[a].insert(t).second) changed = true;
        if (!nullable[x]) {
          allNullable = false;
          break;
        }
      }
      if (allNullable && !nullable[a]) {
        nullable[a] = true;
        changed = true;
      }
    }
  }

  // FIRST(rhs[from..] followed by lookahead la).
  auto firstOfTail = [&](const std::vector<int> &rhs, std::size_t from,
                         int la) {
    std::set<int> out;
    for (std::size_t i = from; i < rhs.size(); ++i) {
      const int x = rhs[i];
      if (!isNonterminal_[x]) {
        out.insert(x);
        return out;
      }
      out.insert(first[x].begin(), first[x].end());
      if (!nullable[x]) return out;
    }
    out.insert(la);
    return out;
  };

  auto closure = [&](ItemSet items) {
    std::vector<Item> work(items.begin(), items.end());
    while (!work.empty()) {
      const auto [p, dot, la] = work.back();
      work.pop_back();
      const std::vector<int> &rhs = rhs_[p];
      if (dot >= static_cast<int>(rhs.size()) || !isNonterminal_[rhs[dot]])
        continue;
      const std::set<int> lookaheads = firstOfTail(rhs, dot + 1, la);
      for (std::size_t q = 0; q < rhs_.size(); ++q) {
        if (lhs_[q] != rhs[dot]) continue;
        for (int b : lookaheads) {
          Item next{static_cast<int>(q), 0, b};
          if (items.insert(next).second) work.push_back(next);
        }
      }
    }
    return items;
  };

  // Canonical collection of LR(1) item sets.
  std::vector<ItemSet> states;
  std::map<ItemSet, int> index;
  states.push_back(closure(ItemSet{Item{0, 0, dollar}}));
  index[states[0]] = 0;
  transitions_.emplace_back();

  for (std::size_t i = 0; i < states.size(); ++i) {
    std::map<int, ItemSet> kernels;
    for (const auto [p, dot, la] : states[i]) {
      if (dot < static_cast<int>(rhs_[p].size()))
        kernels[rhs_[p][dot]].insert(Item{p, dot + 1, la});
    }
    for (const auto &[symbol, kernel] : kernels) {
      ItemSet target = closure(kernel);
      auto found = index.find(target);
      int to;
      if (found == index.end()) {
        to = static_cast<int>(states.size());
        index[target] = to;
        states.push_back(std::move(target));
        transitions_.emplace_back();
      } else {
        to = found->second;
      }
      transitions_[i][symbol] = to;
    }
  }

  // Fill ACTION; any clash means the grammar is not LR(1).
  actions_.assign(states.size(), {});
  auto setAction = [this](int state, int terminal, Action act) {
    auto [it, inserted] = actions_[state].emplace(terminal, act);
    if (!inserted && (it->second.kind != act.kind || it->second.value != act.value))
      throw std::logic_error("LR(1) conflict in state " + std::to_string(state));
  };
  for (std::size_t i = 0; i < states.size(); ++i) {
    for (const auto &[symbol, to] : transitions_[i])
      if (!isNonterminal_[symbol])
        setAction(static_cast<int>(i), symbol, {ActionKind::Shift, to});
    for (const auto [p, dot, la] : states[i]) {
      if (dot != static_cast<int>(rhs_[p].size())) continue;
      if (p == 0 && la == dollar)
        setAction(static_cast<int>(i), la, {ActionKind::Accept, 0});
      else
        setAction(static_cast<int>(i), la, {ActionKind::Reduce, p});
    }
  }
}

int LR1Table::terminal(const std::string &name) const {
  auto it = ids_.find(name);
  if (it == ids_.end() || isNonterminal_[it->second]) return -1;
  return it->second;
}

LR1Table::Action LR1Table::action(int state, int terminal) const {
  auto it = actions_[state].find(terminal);
  return it == actions_[state].end() ? Action{} : it->second;
}

int LR1Table::gotoAfterReduce(int state, int production) const {
  auto it = transitions_[state].find(lhs_[production]);
  return it == transitions_[state].end() ? -1 : it->second;
}

} // namespace zcc
