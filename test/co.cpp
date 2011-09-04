#include <ctime>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <algorithm>
using namespace std;

#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
using namespace boost;

#include "lib/dart_order.hpp"
extern "C" {
#include "order_maintenance.h"
}

optional<unsigned> parse_input(int argc, char * argv[]) {
  assert (argc > 0); // Otherwise this program has no name
  const optional<unsigned> nothing;
  if (argc < 2) {
    return nothing;
  }
  try {
    return lexical_cast<unsigned>(argv[1]);
  } catch (bad_lexical_cast) {
    return nothing;
  }
}

typedef time_t seed_t;
typedef pair<vector<int>, time_t> rand_vec;

rand_vec mk_rand_vec(const unsigned size) {
  rand_vec ans;
  ans.second = time(NULL);
  ans.first.resize(size);
  generate(ans.first.begin(), ans.first.end(), rand);
  return ans;
}

int main(int argc, char * argv[]) {
  // Check command line parameters
  const optional<unsigned> maybe_size = parse_input(argc, argv);
  if (not maybe_size) {
    cerr << "ERROR" << endl
	 << "USAGE: " << argv[0] << " SIZE" << endl
	 << "SIZE is the number of inputs to test" << endl;
    return 1;
  }
  const unsigned size = *maybe_size;

  // Create the input we will use to test;
  // const rand_vec rv = mk_rand_vec(size);
  // cerr << "seed: " << rv.second << endl;
  // const vector<int> & input = rv.first;
  
  const time_t seed = time(NULL);

  // floating point exception: wasn't checking a.size() > 0 before
  // trying to find a random value in it.

  //const time_t seed = 1314209255;

  // co: baseamort.c:56: ordmain_insert_after: Assertion `((void *)0)
  // != x->next' failed. Assumed last iterator in map was equal to
  // m.end(). m.end() is actually one after the last iterator.

  //  const time_t seed = 1314210100;

  // had changed behavior of in_order computation to match strcmp,
  // forgot to change conversion of result (-2,-1,0,1) to bool.

  //  const time_t seed = 1315098207;

  srand(seed);
  cerr << "seed: " << seed << endl;
  

  typedef dart_order<int,ordmain_node *> dart_t;
  dart_t a;
  while (a.size() < size) {
    const int toinsert = rand();
    //cerr << "to insert: " << toinsert << endl;
    const dart_t::ord_iter place = a.insert(toinsert,NULL);
    if (place == a.end()) {
      continue; // value was already in dartboard
    }
    if (place == a.begin()) { // must insert_before
	dart_t::ord_iter after_place = place;
	++after_place;
      if (after_place == a.end()) { // can't insert_before, this is the first item
	place->second = ordmain_insert_after(NULL);
      } else {
	ordmain_node * const after = after_place->second;
	place->second = ordmain_insert_before(after);
      }
    } else {
      dart_t::ord_iter before_place = place;
      --before_place;
      ordmain_node * const before = before_place->second;
      place->second = ordmain_insert_after(before);
    }

    while ((a.size() > 0) && (0 == rand() % 2)) {
      const unsigned i = rand() % a.size();
      dart_t::ord_iter p = a.get_nth(i);
      ordmain_delete(p->second);
      a.erase_nth(i);
    }
    
    if (0 == a.size()) {
      continue;
    }
    const unsigned i = rand() % a.size();
    const unsigned j = rand() % a.size();

    dart_t::ord_const_iter p = a.get_nth(i);
    dart_t::ord_const_iter q = a.get_nth(j);
    
    //cerr << "to order: " << p->first << ' ' << q->first << endl;

    assert ((p->first < q->first) ==
	    (ordmain_in_order(p->second, q->second)));


  }
}
