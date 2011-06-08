#include	"dbf_inc.h"






Boolean DbfIsIndexed (void)
{
  return IxPkP() != NULL;
}
