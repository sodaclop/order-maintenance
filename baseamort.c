#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint32_t tag_t;
typedef uint16_t count_t;
typedef uint64_t sqtag_t;

struct finger_t;
typedef struct finger_t finger;
struct record_t;
typedef struct record_t record;

struct record_t {
  tag_t tag;
  record * prev; // only used for delete
  record * next;
  finger * door;
  record * base;
};

struct finger_t {
  record * home;
};

finger * empty() {
  record * h = malloc(sizeof(record));
  if (NULL == h) {
    return NULL;
  }
  finger * ans = malloc(sizeof(finger));
  if (NULL == ans) {
    free(h);
    return NULL;
  }
  ans->home = h;
  h->door = ans;
  h->tag = 0;
  h->prev = h;
  h->next = h;
  h->base = h;
  return ans;
}

// Returns a finger to a record one past the record pointed to by the finger xf
finger * insert(finger * const xf) {
  assert (NULL != xf);
  record * const x = xf->home;
  assert (NULL != x);
  assert (NULL != x->next);

  // The finger y will eventually be the answer we return.
  finger * y = malloc(sizeof(finger));
  if (NULL == y) {
    // malloc has failed
    return NULL;
  }
  // The record h will be pointed to by the answer finger y
  record * h = malloc(sizeof(record));
  if (NULL == h) {
    // malloc has failed
    free(y);
    return NULL;
  }
  // Make the finger and its record point to each other:
  y->home = h;
  h->door = y;
  // Set up the record so that it will be valid once its new neighbors point to it:
  h->prev = x;
  h->next = x->next;
  h->base = x->base;


  if (x->next == x) {
    x->next = h;
    x->prev = h;
    h->tag = (~0) << 1;
    return y;
  }

  count_t j = 1;
  record * xj = x->next;
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
      wj = ~0;
      break;
    }
    j2 = ((tag_t)j) * ((tag_t)j);
  }

  /* reset the tags of j-1 records by evenly spacing them
   */
  record * xk = x->next;
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
  return y;
}

int order(const finger * const x, const finger * const y) {
  assert (NULL != x);
  assert (NULL != y);
  assert (NULL != x->home);
  assert (NULL != y->home);
  assert (NULL != x->home->base);
  assert (NULL != y->home->base);
  assert (x->home->base == y->home->base);
  
  const tag_t xtag = x->home->tag - x->home->base->tag;
  const tag_t ytag = y->home->tag - y->home->base->tag;

  if (xtag > ytag) {
    return -1;
  }
  if (xtag < ytag) {
    return 1;
  }
  assert (x == y);
  return 0;
}

void delete(finger * const x) {
  assert (NULL != x);
  assert (NULL != x->home);
  assert (NULL != x->home->prev);
  assert (NULL != x->home->next);
  assert (x->home->base != x->home); // can't delete base
  x->home->prev->next = x->home->next;
  x->home->next->prev = x->home->prev;
  free(x->home);
  free(x);
}


int main() {
  finger * a = empty();
  for (unsigned i = 0; i < (unsigned)~0; ++i) {
    insert(a);
    if (0 == (i % 10000)) {
      printf("%u\n", i);
    }
  }
}
