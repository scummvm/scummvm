#include	"dbf_inc.h"







char *DbfFldNam (int n)
{
  return (!Work.Used||n<0||n>=Work.Fields) ? "" : Work.DescPtr[n].FldNme;
}
