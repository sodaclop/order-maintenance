#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


#define SMBC 16
typedef uint16_t smn;
#define TOP UINT16_MAX
typedef uint32_t flat;

#define LOBITS (TOP >> (SMBC/2))
const smn lobits = LOBITS;
const smn hibits = TOP - LOBITS;

typedef struct bign_t {
  smn lo;
  smn hi;
} bign;

flat flatten(const bign x) {
  flat ans = x.lo;
  ans += (flat)(x.hi) << SMBC;
  return ans;
}

bign multup(const smn x, const smn y) {
  const smn xlo = x & lobits;
  const smn xhi = (x & hibits) >> (SMBC/2);
  const smn ylo = y & lobits;
  const smn yhi = (y & hibits) >> (SMBC/2);

  bign ans;
  ans.lo = xlo * ylo;
  ans.hi = xhi * yhi;

  const smn mid1 = xhi * ylo;
  const smn mid2 = yhi * xlo;

  smn oldlo;

  oldlo = ans.lo;
  ans.lo += (mid1 & lobits) << (SMBC/2);
  if (ans.lo < oldlo) {
    ++ans.hi;
  }

  oldlo = ans.lo;
  ans.lo += (mid2 & lobits) << (SMBC/2);
  if (ans.lo < oldlo) {
    ++ans.hi;
  }

  ans.hi += (mid1 & hibits) >> (SMBC/2);
  ans.hi += (mid2 & hibits) >> (SMBC/2);

  return ans;
}

int main() {
  for (unsigned i = 0; i <= TOP; ++i) {
    for (unsigned j = 0; j <= TOP; ++j) {
      const bign attempt = multup(i,j);
      const flat real = (flat)(i) * (flat)(j);
      assert (real == flatten(attempt));
    }
  }
}
