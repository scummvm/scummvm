#include	"dbf_inc.h"





int DbfFCount (void)
{
  return (Work.Used) ? Work.Fields : 0;
}
