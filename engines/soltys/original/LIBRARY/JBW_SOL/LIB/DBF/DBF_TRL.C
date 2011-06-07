#include	"dbf_inc.h"







int DbfTrmLen (int n)
{
  char * p = Work.DescPtr[n].FldPtr;
  int i;
  if (! Work.Used || n < 0 || n >= Work.Fields) return 0;
  for (i = Work.DescPtr[n].FldLen; i > 0; i --) if (p[i-1] != ' ') break;
  return i;
}
