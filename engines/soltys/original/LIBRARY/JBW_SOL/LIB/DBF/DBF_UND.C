#include	"dbf_inc.h"




Boolean DbfUndo (void)
{
  if (! WorkPtr->Used || WorkPtr->RdOnly) return FALSE;
  PutMirror();
  return TRUE;
}
