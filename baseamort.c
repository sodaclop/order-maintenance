#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h> // TODO: is this needed any more?
#include <errno.h>

#include "order_maintenance.h"

typedef uint32_t tag_t;
typedef uint16_t count_t;
typedef uint64_t sqtag_t;

/*
struct finger_t;
typedef struct finger_t finger;
*/
struct ordmain_node {
  tag_t tag;
  struct ordmain_node * prev; // only used for delete
  struct ordmain_node * next;
  struct ordmain_node * base;
};

/*
struct finger_t {
  struct ordmain_node * home;
};
*/

static struct ordmain_node * make_base() {
  struct ordmain_node * const h = malloc(sizeof(struct ordmain_node));
  if (NULL == h) {
    return NULL;
  }
  h->tag = 0;
  h->prev = h;
  h->next = h;
  h->base = h;
  return h;
}

struct ordmain_node * 
ordmain_insert_before(struct ordmain_node * x) {
  return ordmain_insert_after(x->prev);
}

// Returns a finger to a struct ordmain_node one past the struct ordmain_node pointed to by the finger xf
struct ordmain_node * 
ordmain_insert_after(struct ordmain_node * x) {
  if (NULL == x) {
    x = make_base();
  }
  if (NULL == x) {
    return NULL;
  }
  assert (NULL != x);
  assert (NULL != x->next);

  // The struct ordmain_node h will be the answer we eventually return
  struct ordmain_node * h = malloc(sizeof(struct ordmain_node));
  if (NULL == h) {
    // malloc has failed
    free(x);
    return NULL;
  }
  // Set up the struct ordmain_node so that it will be valid once its new neighbors point to it:
  h->prev = x;
  h->next = x->next;
  h->base = x->base;

  // if we are inserting into an empty list
  if (x->next == x) {
    x->next = h;
    x->prev = h;
    h->tag = (~0) >> 1;
    return h;
  }

  count_t j = 1;
  struct ordmain_node * xj = x->next;
  assert (xj != x);
  tag_t wj = xj->tag - x->tag;
  assert (0 != wj);
  tag_t j2 = ((tag_t)j) * ((tag_t)j);
  assert (1 == j2);
  while (wj <= j2) {
    ++j;
    /* Since j started as 1, if 0 == j, j has wrapped around, and the
       structure is actually full. */
    assert (0 != j);
    xj = xj->next;
    assert (NULL != xj);
    wj = xj->tag - x->tag;
    if (0 == wj) { // gone around
      wj = ~0; //TODO: actually want ~0 + 1
      break;
    }
    j2 = ((tag_t)j) * ((tag_t)j);
  }

  /* reset the tags of j-1 struct ordmain_nodes by evenly spacing them
   */
  struct ordmain_node * xk = x->next;
  for (count_t k = 1; k < j; ++k) {
    const sqtag_t p = ((sqtag_t)wj) * ((sqtag_t)k);
    const sqtag_t pret = p / ((sqtag_t)j) + (sqtag_t)(x->tag);
    assert (NULL != xk);
    xk->tag = pret & ((sqtag_t)(~((tag_t)0)));
    assert (xk->tag != xk->prev->tag);
    assert (xk->tag != 1+(xk->prev->tag));
    xk = xk->next;
  }
  assert (NULL != x->next);
  const tag_t nt = x->tag + (x->next->tag - x->tag)/2;
  assert (nt != x->tag);
  assert (nt != x->next->tag);
  h->tag = nt;
  x->next->prev = h;
  x->next = h;
  return h;
}

struct ordmain_node * 
ordmain_insert_before(struct ordmain_node * x) {
  return ordmain_insert_after(x->prev);
}

/*
  order(const struct ordmain_node * const x, const struct ordmain_node * const y):

  If x and y are not valid pointers to ordmain_nodes in the same list,
  sets errno to EINVAL and returns 0.

  Returns 1 if ${x} comes before ${y}, -1 if ${y} comes before ${x},
  and 0 is they are the same node.
 */
static int 
order(const struct ordmain_node * const x, const struct ordmain_node * const y) {
  if ((NULL == x)
      || (NULL == y)
      || (x->base != y->base)) {
    errno = EINVAL; // TODO: is this the right errno?
    return -2;
  }
  assert (NULL != x);
  assert (NULL != y);
  assert (NULL != x->base);
  assert (NULL != y->base);
  assert (x->base == y->base);
  
  const tag_t xtag = x->tag - x->base->tag;
  const tag_t ytag = y->tag - y->base->tag;

  if (xtag > ytag) {
    return -1;
  }
  if (xtag < ytag) {
    return 1;
  }
  assert (x == y);
  return 0;
}

bool ordmain_in_order(const struct ordmain_node * x, const struct ordmain_node * y) {
  return 1 == order(x,y);
}


void ordmain_delete(struct ordmain_node * const x) {
  assert (NULL != x);
  assert (NULL != x->prev);
  assert (NULL != x->next);
  assert (x->base != x); // can't delete base
  x->prev->next = x->next;
  x->next->prev = x->prev;
  if (x->base->next == x->base) {
    free(x->base);
  }
  free(x);
}

/*
int main() {
  struct ordmain_node * a = 0;
  a = insert(a);
  for (unsigned i = 0; i < (unsigned)~0; ++i) {
    insert(a);
    if (0 == (i % 10000)) {
      printf("%u\n", i);
    }
  }
}
*/
