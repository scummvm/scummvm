#include	"dbf_inc.h"





int DbfRecLen (void)
{
  return (Work.Used) ? Work.HeadPtr->RecLength : 0;
}
