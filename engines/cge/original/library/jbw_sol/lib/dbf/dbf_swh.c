#include	"dbf_inc.h"

void DbfSetWriteHook (void (*wh)(void))
{
  WorkPtr->WriteHook = wh;
}
