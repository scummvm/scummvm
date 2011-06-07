#include	"dbf_inc.h"





RecPos DbfRecNo (void)
{
  return (Work.Used) ? Work.RecNo : (RecPos) 0;
}
