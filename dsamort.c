#include "dsamort.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

typedef uint32_t tag_t;
typedef uint16_t count_t;
typedef uint8_t log_t;

typedef 
struct list_t {
  count_t size;
  log_t limit; // The limit of sublist length
  count_t oneup; // The size at which we increase the limit
} list;

struct leaf_t;
typedef struct leaf_t leaf;

typedef
struct sublist_t {
  log_t size;
  tag_t tag;
  list * parent;
  leaf * first_child;
} sublist;

struct leaf_t {
  tag_t tag;
  sublist * parent;
  finger * door;
};

struct finger_t {
  leaf * home;
};


leaf * insert_sublist(leaf *);
int split_sublist(sublist * x);
/* {
  x->parent->limit
  }*/

finger *insert (finger * x) {
  finger * ans = malloc(sizeof(finger));
  if (NULL == ans) {
    return NULL;
  }
  ans->home = insert_sublist(x->home);
  if (NULL == ans->home) {
    free(ans);
    return NULL;
  }
  if (ans->home->parent->size > ans->home->parent->parent->limit) {
    int err = split_sublist(ans->home->parent);
    if (0 != err) {
      free(ans);
      return NULL;
    }
  }
  return ans;
}

int tagorder(const tag_t x, const tag_t y) {
  if (x > y) {
    return -1;
  }
  if (x < y) {
    return 1;
  }
  return 0;
}

int order(const finger * const x, const finger * const y) {
  if ((0 == x) || (0 == y) || (0 == x->home) || (0 == y->home)) {
    return -2;
  }
  
  if (x->home->parent == y->home->parent) {
    return tagorder(x->home->tag, y->home->tag);
  }
  
  if ((0 == x->home->parent) || (0 == y->home->parent) 
      || (x->home->parent->parent != y->home->parent->parent)) {
    return -2;
  }
  return tagorder(x->home->parent->tag, y->home->parent->tag);
}

finger * singleton() {
  list    * top    = NULL;
  sublist * middle = NULL;
  leaf    * bottom = NULL;
  finger  * ans    = NULL;

  top = malloc(sizeof(list));
  if (NULL == top) {
    goto err;
  }
  top->size = 1;
  top->limit = 1;
  
  middle = malloc(sizeof(sublist));
  if (NULL == middle) {
    goto err;
  }
  middle->size = 1;
  middle->tag = 0;
  middle->parent = top;
  
  bottom = malloc(sizeof(leaf));
  if (NULL == bottom) {
    goto err;
  }
  middle->first_child = bottom;
  bottom->tag = 0;
  bottom->parent = middle;
  
  ans = malloc(sizeof(finger));
  if (NULL == ans) {
    goto err;
  }
  bottom->door = ans;
  ans->home = bottom;
  
  return ans;

 err:
  
  free(top);
  free(middle);
  free(bottom);
  free(ans);
  return NULL;

} 


  
/*
count_t two_pow_min_one[] = 
  {
    (1 <<  1) - 1,
    (1 <<  2) - 1,
    (1 <<  3) - 1,
    (1 <<  4) - 1,
    (1 <<  5) - 1,
    (1 <<  6) - 1,
    (1 <<  7) - 1,
    (1 <<  8) - 1,
    (1 <<  9) - 1,
    (1 << 10) - 1,
    (1 << 11) - 1,
    (1 << 12) - 1,
    (1 << 13) - 1,
    (1 << 14) - 1,
    (1 << 15) - 1,
    (1 << 16) - 1
  }
*/
