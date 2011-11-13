/*

A general balanced tree (Andersson 1989) is a binary search tree of logarithmic height maintained by partial rebuilding.
Each 

Idea: we can change the depth multiplier when we hit the limit, then globally rebuild?

 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef uint64_t count_t;
// log_t represents the log_2 of a count_t.
// It is always between 0 and b, where b is the number of bits in a
// count_t.
typedef uint8_t log_t;

// ceiling is the number of bits in a count_t
static const log_t ceiling = sizeof(count_t) << 3;

/*
A tree with n leaves has 2n-1 nodes. It must have height < 2^slop *
floor(log_2 n)
 */ 
static const log_t slop = 1;

struct root {
  // The largest size the tree has ever been
  count_t largest;
  // The number of dead nodes
  count_t deletes;
};

struct node {
  count_t tag;
  struct node * left;
  struct node * right;
  struct root * root;
  /* The last bit set when creating this node, or ceiling if no bits
     were set. This value is necessarily between 0 and ceiling. */
  log_t elevation;
  bool deleted;
};

bool
in_order(const struct node * x, const struct node * y) {
  return x->tag < y->tag;
}

log_t
min_log(const log_t x, const log_t y) {
  return (x < y) ? x : y;
}

struct valley {
  log_t min_elevation;
  count_t width_increase;
};

count_t
mask_greater_eq(const log_t pos) {
  assert (pos < ceiling);
  const count_t mask_here = ((count_t)1) << pos;
  const count_t mask_less = mask_here - 1;
  return ~mask_less;
}

count_t
mask_greater(const log_t pos) {
  assert (pos < ceiling);
  const count_t mask_here = ((count_t)1) << pos;
  const count_t mask_less = mask_here - 1;
  const count_t mask_lesseq = (mask_less << 1) + 1;
  return ~mask_lesseq;
}


void 
max_span(const struct node * low, const count_t length, const log_t this_elev, const count_t path_to_root) {

  const struct node * x = low;
  for(count_t i = 0; i < length; ++i) {
    assert (path_to_root == (low->tag & mask_greater(this_elev)));
    low = low->right;
  }
  if (low != NULL) {
    assert (path_to_root != (low->tag & mask_greater(this_elev)));
  }
  low = x->left;
  if (low != NULL) {
    assert (path_to_root != (low->tag & mask_greater(this_elev)));
  }

}
  

/*

on output, *low to *high must be the largest contiguous region sharing
the path to root of any member above elevation.

 */
struct valley
expand_once(struct node ** low, struct node ** high, const log_t elevation) {
  struct valley ans = {ceiling, 0};
  assert (elevation < ceiling);
  const count_t mask = mask_greater(elevation);
  // the high-order bits of every tag in the range we are searching for
  const count_t path_to_root = (*low)->tag & mask;
  while ((*low)->left
	 && path_to_root == (((*low)->left->tag) & mask)) {
    *low = (*low)->left;
    ++ans.width_increase;
    ans.min_elevation = min_log(ans.min_elevation, (*low)->elevation);
  }
  while (*high
	 && path_to_root == (((*high)->tag) & mask)) {

    ++ans.width_increase;
    ans.min_elevation = min_log(ans.min_elevation, (*high)->elevation);
    *high = (*high)->right;
  }
  return ans;
}

void
check_from(const struct node * low, const count_t many) {
#ifndef NDEBUG
  const struct node * x = low;
  for (count_t i = 0; i < many-1; ++i) {
    assert (x->tag < x->right->tag);
    if (!(x->tag & (((count_t)1) << x->elevation))) {
      assert (x->tag + (((count_t)1) << x->elevation) == x->right->tag);
    }
      
    x = x->right;
  }
#endif
}

void
check_until(const struct node * low) {
#ifndef NDEBUG
  const struct node * x = low;
  while (x && x->right) {
    assert (x->tag < x->right->tag);
    if (!(x->tag & (((count_t)1) << x->elevation))) {
      assert (x->tag + (((count_t)1) << x->elevation) == x->right->tag);
    }
    x = x->right;
  }
#endif
}

struct node *
redistribute(struct node * low, const log_t level, const count_t many, const count_t path_to_root) {
  // level is the top level nodes will differ at. It is between 0 and ceiling-1
  assert (many > 1);
  if (2 == many) {
    low->tag = path_to_root;
    low->elevation = level;
    assert (NULL != low->right);
    low = low->right;
    low->tag = path_to_root + (((count_t)1) << level);
    low->elevation = level;
    low = low->right;
    return low;
  } else if (3 == many) {
    struct node * oldlow = low;
    low->tag = path_to_root;
    low->elevation = level;
    assert (NULL != low->right);
    low = low->right;
    struct node * ans = redistribute(low, level-1, 2, path_to_root + (((count_t)1) << level));
    check_from(oldlow, 3);
    return ans;
  } else {
    struct node * middle = redistribute(low, level-1, many/2, path_to_root);
    check_from(low, many/2);
    struct node * ans = redistribute(middle, level-1, many - many/2, path_to_root + (((count_t)1) << level));
    check_from(low, many);
    return ans;
  }
}

void 
distribute(struct node * low, const log_t level, const count_t many) {
  const count_t high_mask = mask_greater_eq(level);
  //const count_t high_mask = ~((((count_t)1) << (level+1)) - 1);
  redistribute(low, level, many, high_mask & low->tag);
  check_from(low,many);
}


void
detach(struct node * x) {
  if (NULL == x) {
    return;
  } else {
    if (NULL != x->left) {
      x->left->right = x->right;
    }
    if (NULL != x->right) {
      x->right->left = x->left;
    }
    free(x);
  }
}


bool 
in_balance(const log_t height, const count_t size) {
  // height <= (2^slop) * log size
  // height / (2^slop) <= log size
  // height >> slop <= log size
  // 2^(height >> slop) <= size
  if (slop <= height) {
    const count_t low = ((count_t)1) << (height >> slop);
    return (low <= size);
  } else {
    return true;
  }
} 

void
rebalance(struct node * x) {
  // take larger and larger contiguous subsets. Once one is too small, rebalance it
  struct node * y = x->right;
  count_t many = 1;
  log_t current_elevation = x->elevation;
  log_t lowest_elevation = x->elevation;
  max_span(x, many, current_elevation-1, x->tag & mask_greater(current_elevation-1));  
  while (current_elevation < ceiling 
	 && in_balance(current_elevation - lowest_elevation + 1, many)) {
    struct valley change = expand_once(&x, &y, current_elevation);
    lowest_elevation = min_log(lowest_elevation, change.min_elevation);
    many += change.width_increase;
    max_span(x, many, current_elevation, x->tag & mask_greater(current_elevation));
    ++current_elevation;
  }
  max_span(x, many, current_elevation-1, x->tag & mask_greater(current_elevation-1));  
  // current elevation is now the lowest level that the followers of x agree on
  if (y) {
    assert (y->tag - x->tag >= (((count_t)1) << (current_elevation)));
    assert (y->left->tag - x->tag < (((count_t)1) << (current_elevation)));
  }
  distribute(x, current_elevation-1, many);
}



void
maybe_rebalance(struct node * x) {
  const log_t height = ceiling - x->elevation;
  if (!in_balance(height, x->root->largest)) {
    rebalance(x);
  }
}

struct node *
insert_after(struct node * x) {
  check_until(x);
  if (NULL != x) {

    x->elevation -= 1;

    const count_t mask = ((count_t)1) << x->elevation;



    struct node * ans = (struct node *)malloc(sizeof(struct node));
    ans->deleted = false;
    ans->tag = x->tag + mask;
    ans->left = x;
    ans->right = x->right;
    ans->root = x->root;
    ans->elevation = x->elevation;

    x->right = ans;
    if (NULL != ans->right) {
      ans->right->left = ans;
    }   
    
    x->root->largest += 1;
    check_until(x);
    maybe_rebalance(ans);
    check_until(x);
    return ans;
  } else {
    struct root * root = (struct root *)malloc(sizeof(struct root));
    root->largest = 1;
    root->deletes = 0;

    struct node * ans = (struct node *)malloc(sizeof(struct node));
    ans->root = root;
    ans->tag = 0;
    ans->left = NULL;
    ans->right = NULL;
    ans->elevation = ceiling-1;
    ans->deleted = false;
    return ans;
  }    
}

struct node *
insert_before(struct node * x) {
  if (NULL == x) {
    return insert_after(x);
  } else {
    check_until(x);
    x->elevation -= 1;

    const count_t mask = ((count_t)1) << x->elevation;



    struct node * ans = (struct node *)malloc(sizeof(struct node));
    ans->tag = x->tag;
    x->tag += mask;
    ans->right = x;
    ans->left = x->left;
    ans->root = x->root;
    ans->elevation = x->elevation;
    ans->deleted = false;

    x->left = ans;
    if (NULL != ans->left) {
      ans->left->right = ans;
    }   
    
    x->root->largest += 1;
    check_until(ans);
    maybe_rebalance(ans);
    check_until(ans);
    return ans;
  }    
}

/*
struct node *
move_left(struct node ** x) {
  if (NULL == (*x)->left) {
    return *x;
  } else {
    struct node * ans = *x;
    *x = (*x)->left;
    return ans;
  }
}

bool
move_right(struct node ** x) {
  if (NULL == (*x)->right) {
    return false;
  } else {
    *x = (*x)->right;
    return true;
  }
}
*/

struct cleared {
  struct node * left;
  struct node * right;
  count_t many;
};

struct cleared 
clear(struct node * x) {
  struct cleared ans = {NULL, NULL, 0};

  struct node * right_start = x;

  while (NULL != x) {
    if (x->deleted) {
      struct node * newx = x->left;
      detach(x);
      x = newx;
    } else {
      ans.left = x;
      if (NULL == ans.right) {
	ans.right = x;
      }
      ++ans.many;
      x = x->left;
    }
  }

  assert(ans.right ? (ans.many > 0) : ((0 == ans.many) && (NULL == ans.left)));

  x = ans.right ? ans.right : right_start;
  x = x->right;

  while (NULL != x) {
    if (x->deleted) {
      struct node * newx = x->right;
      detach(x);
      x = newx;
    } else {
      ++ans.many;
      ans.right = x;
      if (NULL == ans.left) {
	ans.left = x;
      }
      x = x->right;
    }
  }

  return ans;
}

void
rebuild(struct node * x) {
  struct root * oldroot = x->root;
  struct cleared fresh = clear(x);
  if (0 == fresh.many) {
    free(oldroot);
  } else {
    if (fresh.many > 1) {
      fresh.left->tag = 0;
      distribute(fresh.left, ceiling-1, fresh.many);
    } else {
      fresh.left->tag = 0;
      fresh.left->elevation = ceiling-1;
    }
    fresh.left->root->largest = fresh.many;
    fresh.left->root->deletes = 0;
  }
} 
    
void
erase(struct node * x) {
  check_until(x);
  x->deleted = true;
  x->root->deletes += 1;
  if (NULL == x->left
      && NULL == x->right) {
    free(x->root);
    free(x);
  } else {
    if ((x->root->deletes << 1) > x->root->largest) {
      rebuild(x);
    }
  }

}
      

/*
void
rebuild(struct node * x) {
  struct node * left = x;
  while (NULL != left->left) {
    struct node * const temp = left->left;
    if (left->deleted) {
      detach(left);
    }
    left = temp;
  }
  if (left->deleted) {
    struct node * const temp = left->right;
    detach(left);
    left = temp;
  }

  struct node * right = x;
  while (NULL != right->right) {
    struct node * const temp = right->right;
    if (right->deleted) {
      detach(right);
    }
    right = temp;
  }
  if (right->deleted) {
    struct node * const temp = right->left;
    detach(right);
    right = temp;
  }

  
  
  while (NULL != right->right) {
    right = right->right;
  }

*/
