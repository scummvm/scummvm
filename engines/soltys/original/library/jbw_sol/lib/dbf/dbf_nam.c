#include	"dbf_inc.h"





char *DbfName (void)
{
  return (Work.Used) ? Work.dBPath : "";
}
