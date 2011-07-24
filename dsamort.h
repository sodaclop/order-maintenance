#ifndef DSAMORT_H
#define DSAMORT_H

/* A finger is a persistent pointer into an order-maintenance
   structure. It remains valid until it is explicitly deleted. */
struct finger_t;
typedef struct finger_t finger;

finger *singleton ();
/* order returns -2 on error, -1 if the first finger follows the
   second, 0 if they are the same, and 1 if the second finger follows
   the first. */
int order (const finger *, const finger *);
finger *insert (finger *);
/* delete also frees the memory used by the finger */
void delete (finger *);


#endif // DSAMORT_H
