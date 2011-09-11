#include <vector>
using std::vector;

#include <boost/optional.hpp>
using boost::optional;

#include "dart_order.hpp"

template<typename T>
struct neighborhood {
  bool present;
  optional<T> before;
  optional<T> after;

  bool operator==(neighborhood<T> that) const {
    return present == that.present
      && before == that.before
      && after == that.after;
  }
  
  template<typename V, typename U>
  static neighborhood 
  from_lower_bound(const V & group, U lb, T key) {
    neighborhood ans;
    static const optional<T> nothing;
    
    if (lb == group.end()) {
      ans.present = false;
      ans.after = nothing;
      
      if (lb != group.begin()) {
	--lb;
	ans.before = lb->first;
      } else {
	ans.before = nothing;
      }
    } else {
      if (lb == group.begin()) {
	ans.before = nothing;
      } else {
	U lb2 = lb;
	--lb2;
	ans.before = lb2->first;
      }
      if (key == lb->first) {
	ans.present = true;
	++lb;
	if (lb != group.end()) {
	  ans.after = lb->first;
	} else {
	  ans.after = nothing;
	}
      } else {
	ans.present = false;
	ans.after = lb->first;
      }
    }
    return ans;
  }
};

template<typename T>
struct pred_op {
  enum OP_TYPE {
    INSERT,
    DELETE,
    LOOKUP
  } type;
  T key;
};

template<typename T>
struct pred_trace {
  vector<pred_op<T> > ops;
  vector<neighborhood<T> > lookups;
  struct index {
    unsigned ops_idx;
    unsigned lookups_idx;
    index() : ops_idx(0), lookups_idx(0) {}
  };
  pred_trace() :
    ops(), lookups() {}
};

#include <iostream>
using std::cerr;
using std::endl;

template<typename T, typename K>
//typename pred_trace<typename T::key_type>::index 
void
check_trace(const pred_trace<K> & trace, T & x) {
  //typedef typename T::key_type K;
  typename pred_trace<K>::index i;
  for(;i.ops_idx < trace.ops.size(); ++i.ops_idx) {
    
    pred_op<K> op = trace.ops[i.ops_idx];
       
    switch (op.type) {
    case pred_op<K>::INSERT:
      //cerr << "INSERT " << op.key << endl;
      x.insert(typename T::value_type(op.key, op.key)); 
      break;
    case pred_op<K>::DELETE:
      //cerr << "ERASE " << op.key << endl;
      x.erase(op.key);
      break;
    case pred_op<K>::LOOKUP:
    default:
      const neighborhood<K> around = 
	neighborhood<K>::from_lower_bound(x, x.lower_bound(op.key), op.key);
      const neighborhood<K> & expected = trace.lookups[i.lookups_idx];
      if (around.before || around.after) {
	//assert(false); // TODO: this should trigger sometimes
      }
      if (not (around == expected)) {
	cerr << i.ops_idx - i.lookups_idx << endl;
	throw i;
      }
      ++i.lookups_idx;
    }
  } 
}
 
const double lookup_prob = 0.90;
const double insert_prob = 0.09;

const double lookup_hit_prob = 0.5;
const double insert_dupe_prob = 0.01;
const double delete_hit_prob = 0.01;

const int mrand_max = RAND_MAX;

int mrand() {
  return rand() % mrand_max;
}

template<typename T>
pred_trace<typename T::key_type>
make_trace(const unsigned n) {
  typedef typename T::key_type K;
  pred_trace<K> ans;
  T x;
  double top = mrand_max;


  for (unsigned i = 0; i < n; ++i) {
    pred_op<K> op;
    const int op_type = mrand();
    if (op_type < lookup_prob * top) {
      op.type = pred_op<K>::LOOKUP;
      const int lookup_hit = mrand();
      if (x.size() > 0 
	  && lookup_hit < lookup_hit_prob * top) {
	op.key = x.get_random(mrand)->first;
      } else {
	op.key = mrand();
      }
      ans.ops.push_back(op);
      ans.lookups.push_back(neighborhood<K>::from_lower_bound(x, x.lower_bound(op.key), op.key));
    } else if (op_type < (lookup_prob + insert_prob) * top) {
      op.type = pred_op<K>::INSERT;
      const int insert_dupe = mrand();
      if (x.size() > 0
	  && insert_dupe < insert_dupe_prob * top) {
	op.key = x.get_random(mrand)->first;
      } else {
	op.key = mrand();
      }
      ans.ops.push_back(op);
      x.insert(make_pair(op.key,op.key));
    } else {
      op.type = pred_op<K>::DELETE;
      const int delete_hit = mrand();
      if (x.size() > 0
	  && delete_hit < delete_hit_prob * top) {
	op.key = x.get_random(mrand)->first;
      } else {
	op.key = mrand();
      }
      ans.ops.push_back(op);
      x.erase(op.key);
    }
  }
  return ans;
}

#include <map>
#include "yfasttrie.hpp"

#include <boost/date_time.hpp>
using namespace boost::posix_time;



void 
spec_trace(const pred_trace<int> & x
	   , std::map<nobits<int>, nobits<int> > & y) {
  check_trace<std::map<nobits<int>, nobits<int> > >(x,y);
}

int 
main() {
  

  time_t seed = time(0);
  // seed = 1315621550;
  //1315622546
  //1315622565
  //seed = 1315622590; /100 or RAND_MAX

  /*

RAND_MAX
seed: 1315632843
4228

seed: 1315632877
2325

seed: 1315632908
139

10000
seed: 1315633022
pred_bench.exe: ../../y-fast-trie/yfasttrie.hpp:253: yfasttrie<KEY, VAL, HASH>::insert_res yfasttrie<KEY, VAL, HASH>::insert_or_set(KEY, const VAL&) [with KEY = int, VAL = int, HASH = void]: Assertion `false' failed.
fixed!
   */
  //seed = 1315633022;
  //seed = 1315632908;  
  cerr << "seed: " << seed << endl;
  srand(seed);
  typedef dart_order<int,int> map_t;
  pred_trace<int> trace = make_trace<map_t>(3000000);
  //  typedef std::map<int,int> test_t;
  typedef yfasttrie<int,int> test_t;
  typedef std::map<int,int> test_2;


  ptime start = microsec_clock::universal_time();
  test_t empty;
  check_trace(trace, empty);
  ptime finish = microsec_clock::universal_time();
  cerr << finish-start << endl;

  start = microsec_clock::universal_time();
  test_2 empty2;
  check_trace(trace, empty2);
  finish = microsec_clock::universal_time();
  cerr << finish-start << endl;


  
  typedef std::map<nobits<int>,nobits<int> > test_3;
  start = microsec_clock::universal_time();
  test_3 empty3;
  check_trace(trace, empty3);
  finish = microsec_clock::universal_time();
  cerr << finish-start << endl;
  
}
