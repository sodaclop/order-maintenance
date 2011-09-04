#ifndef ORDER_MAINTENANCE_H
#define ORDER_MAINTENANCE_H

#include <stdbool.h>

/*
opaque
*/
struct ordmain_node;

#ifndef NDEBUG
typedef uint32_t tag_t;
typedef uint16_t count_t;
typedef uint64_t sqtag_t;

struct ordmain_node {
  tag_t tag;
  /*
    ${prev} is used only for delete and ordmain_insert_before.
   */
  struct ordmain_node * prev; 
  struct ordmain_node * next;
  struct ordmain_node * base;
};
#endif

/*
Returns true when x precedes y in the list.
*/
bool ordmain_in_order(const struct ordmain_node * x, const struct ordmain_node * y);

/* 
Returns a pointer to a newly created node placed just after x. Nodes
are members of only one list. If x is NULL, creates a list with a
single node, then returns a pointer to that node.
*/
struct ordmain_node * 
ordmain_insert_after(struct ordmain_node * x);

/*
See ordmain_insert_after.
 */
struct ordmain_node * 
ordmain_insert_before(struct ordmain_node *);

/*
Removes the node x from the list it belongs to, then frees the memory it
was using.
*/
void ordmain_delete(struct ordmain_node * x); 


#endif /* ORDER_MAINTENANCE_H */
