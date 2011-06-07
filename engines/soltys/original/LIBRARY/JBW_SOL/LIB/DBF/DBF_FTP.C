#include	"dbf_inc.h"







int DbfFldTpe (int n)
{
  return (!Work.Used||n<0||n>=Work.Fields) ? 0 : Work.DescPtr[n].FldTpe;
}
