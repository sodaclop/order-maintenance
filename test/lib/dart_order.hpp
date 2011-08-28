// A std::map-like container that allows O(1) access to random members
// of the map. The interesting functions with no analogue in std::map
// are get_nth and delete_nth.

#include <map>
#include <vector>
#include <utility>

template<typename T, typename U>
struct dart_order {

protected:
  typedef std::map<T,U> ordered_t;  

public:
  typedef typename ordered_t::iterator ord_iter;
  typedef typename ordered_t::const_iterator ord_const_iter;

protected:
  // The values in order
  ordered_t ordered;
  // The values with no guaranteed order, stored as pointers from
  // unordered into ordered. Note that a function mimicking
  // std::map<T,U>::erase is not possible without also keeping
  // pointers from the values in ordered into unordered
  std::vector<ord_iter> unordered;
  
public:

  // Gets some arbitrary iterator into the map
  ord_iter get_nth(const unsigned n) {
    assert (n < unordered.size());
    return unordered[n];
  }

  // Deletes some arbitrary key-value pair from the map.
  void erase_nth(unsigned n) {
    assert (n < unordered.size());
    ordered.erase(unordered[n]);
    unordered[n] = unordered.back();
    unordered.pop_back();
  }

  // Returns end() if the key k is already present.
  ord_iter insert(const T & k, const U & v) {
    const std::pair<ord_iter, bool> ins_res = ordered.insert(std::make_pair(k,v));
    if (not ins_res.second) {
      return ordered.end();
    }
    unordered.push_back(ins_res.first);
    return ins_res.first;
  }

  unsigned size() const {
    assert (ordered.size() == unordered.size());
    return ordered.size();
  }

  ord_iter begin() {
    return ordered.begin();
  }

  ord_iter end() {
    return ordered.end();
  }



}; 
