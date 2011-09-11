// A std::map-like container that allows O(1) access to random members
// of the map. The interesting functions with no analogue in std::map
// are get_nth and delete_nth.

#include <map>
#include <vector>
#include <utility>

template<typename T, typename U>
struct dart_order {
  typedef T key_type;
  typedef std::pair<T,U> value_type;

protected:

  struct bival {
    U val;
    int name;
    bival(U v, int n = -1) :
      val(v), name(n) {}
  };

  typedef std::map<T,bival> ordered_t;  


public:
  typedef typename ordered_t::iterator iterator;
  typedef typename ordered_t::const_iterator const_iterator;

protected:
  // The values in order
  ordered_t ordered;
  // The values with no guaranteed order, stored as pointers from
  // unordered into ordered. Note that a function mimicking
  // std::map<T,U>::erase is not possible without also keeping
  // pointers from the values in ordered into unordered
  std::vector<iterator> unordered;

  // struct handle {
  //   typename std::vector<ord_iter>::iterator hood;
  // };
  
  // Gets some arbitrary iterator into the map
  iterator get_nth(const unsigned n) {
    assert (n < unordered.size());
    return unordered[n];
  }

  // Deletes some arbitrary key-value pair from the map.
  void erase_nth(unsigned n) {
    assert (n < unordered.size());
    ordered.erase(unordered[n]);
    unordered[n] = unordered.back();
    unordered[n]->second.name = n;
    unordered.pop_back();
  }

public:

  template<typename F>
  iterator get_random(const F & f) {
    const unsigned i = f() % size();
    return get_nth(i);
  }


  void erase(const iterator i) {
    const unsigned n = find_unordered(i);
    erase_nth(n);
  }

  void erase(const T key) {
    iterator i = ordered.find(key);
    if (i == ordered.end()) {
      return;
    }
    erase(i);
  }

protected:

  unsigned find_unordered(const iterator i) {
    return i->second.name;
  }

public:

  // Returns end() if the key k is already present.
  iterator insert(const std::pair<T,U> & kv) {
    const std::pair<iterator, bool> ins_res = ordered.insert(kv);
    if (not ins_res.second) {
      return ordered.end();
    }
    unordered.push_back(ins_res.first);
    ins_res.first->second.name = unordered.size()-1;
    return ins_res.first;
  }

  unsigned size() const {
    assert (ordered.size() == unordered.size());
    return ordered.size();
  }

  iterator begin() {
    return ordered.begin();
  }

  iterator end() {
    return ordered.end();
  }

  const_iterator begin() const {
    return ordered.begin();
  }

  const_iterator end() const {
    return ordered.end();
  }


  iterator lower_bound(const T & k) {
    return ordered.lower_bound(k);
  }


}; 
