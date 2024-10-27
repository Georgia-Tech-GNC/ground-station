#include "tst.h"
#include "database.h"

tstsuite("Factorials") {
  tstcheck(1 == 1);
  tstcheck(1+1 == 2);
  tstcheck(1+5 == 6);
  tstcheck(20+100 == 120);
}