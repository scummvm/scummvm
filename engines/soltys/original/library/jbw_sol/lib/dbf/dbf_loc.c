#include	"dbf_inc.h"


int DbfLocked (void)
{
  return (WorkPtr->Used) ? WorkPtr->Locked : 0;
}
