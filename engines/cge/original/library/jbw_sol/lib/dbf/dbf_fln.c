#include	"dbf_inc.h"







int DbfFldLen (int n)
{
  return (!Work.Used||n<0||n>=Work.Fields) ? 0 : Work.DescPtr[n].FldLen;
}
