#include "FA.h"
#include <map>
#include <utility>
#include <set>
#include <stack>
#include <iostream>

/*epsilon_closure of T ( of FA ) is set of NFA states reachable from state T on
  epsilon transition.  */
auto
epsilon_closure (fa::state t,
		 const fa::finite_autometa &fa)
{
  auto fa_trans = fa.get_transition_relations ();
  auto fa_states = fa.get_states ();

  auto st = (std::stack <fa::state>) {};
  st.push (t);

  auto e_t = std::set({t});
  while (!st.empty ())
    {
      auto top = st.top ();
      st.pop ();

      for (auto current_state: fa_states)
	{
	  if (fa.move (current_state, fa::epsilon).count (top)
	      && !(e_t.count (current_state)))
	    {
	      e_t.insert (current_state);
	      st.push (current_state);
	    }
	}
    }

  return e_t;
}

/*epsilon_closure of T ( of FA ) is set of NFA states reachable from set of
  state T on epsilon transition.  */
auto
epsilon_closure (std::set <fa::state> t,
		 const fa::finite_autometa &fa)
{
  auto fa_trans = fa.get_transition_relations ();
  auto fa_states = fa.get_states ();

  auto st = (std::stack <fa::state>) {};
  for (auto s: t)
    {
      st.push (s);
    }

  auto e_t = t;
  while (!st.empty ())
    {
      auto top = st.top ();
      st.pop ();

      for (auto current_state: fa_states)
	{
	  if (fa.move (current_state, fa::epsilon).count (top)
	      && !(e_t.count (current_state)))
	    {
	      e_t.insert (current_state);
	      st.push (current_state);
	    }
	}
    }

  return e_t;
}

/*The function take's non detereminisitic finite autometa (NAF) and returns a
  deterministic finite autometa (DFA) that accepts the same language as NFA.  */
auto
convert (const fa::finite_autometa &nfa)
{
  auto s0 =  epsilon_closure (nfa.get_initialstate (),
			      nfa);

  /* the idea is that every dfa state is a set of nfa states.  */
  using dfa_state = std::set <fa::state>;
  auto dfa_states = std::set ({s0});

  auto input_symbols = nfa.get_input_chars ();
  std::map <std::pair <dfa_state, fa::symbol>,
	    dfa_state> dfa_trans;

  auto unmarked_states = dfa_states;

  while (! unmarked_states.empty ())
    {
      auto T = unmarked_states.extract (unmarked_states.begin ());
      for (auto symbol: input_symbols)
	{
	  auto u = epsilon_closure (nfa.move (T.value (),symbol), nfa);
	  if (!dfa_states.count (u))
	    {
	      dfa_states.insert (u);
	      unmarked_states.insert (u);
	    }
	  dfa_trans [{T.value (), symbol}] = u;
	}
    }
  return dfa_trans;
}

auto main() -> int
{
  // TODO: add more tests
  auto nfa_transitions = (fa::transition_table) { {{1,'a'},{1}} ,
						  {{1,'b'},{2}} ,
						  {{2,'a'},{2,1}} ,
						  {{2,'b'},{3}} ,
						  {{3,'a'},{3}} ,
						  {{3,'b'},{3}}};
  auto nfa = fa::finite_autometa ({1,2,3},
				  {'a','b'},
				  {3},
				  1,
				  nfa_transitions);
  auto dfa_trans = convert(nfa);

  /*Print this new transition table on stdout.  This is makeshift for now and
   would be moved in seperate function or would be replaced by automated testing
   of some kind in future versions.  */
  for (const auto& tran : dfa_trans)
    {
      // states
      std::cout <<"{ ";
      for (auto state : tran.first.first)
	{
	  std::cout<<state<<" ";
	}
      std::cout<<"} / ";

      //symbol scanned
      std::cout<<tran.first.second<<" -> ";

      // destination state
      std::cout <<"{ ";
      for (auto state : tran.second)
	{
	  std::cout<<state<<" ";
	}
      std::cout<<"}\n";
    }

  return 0;
}
