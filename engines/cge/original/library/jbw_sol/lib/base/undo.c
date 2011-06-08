#include	<base.h>


void UndoRec (void)
{
  DbfUndo();
  DbfSkip(0);
  if (DbfEof()) DbfSkip(-1);
}
