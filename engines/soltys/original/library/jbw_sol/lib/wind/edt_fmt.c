#include	<edit.h>
#include	<stdlib.h>
#include	<mem.h>
#include	<string.h>


void	SetTab	(EditStat * es);
Boolean	Wrap	(EditStat * es);
Boolean	EditDn	(EditStat * es);
Boolean	EditEnd	(EditStat * es);



Boolean EditFormat (Wind * W)
{
  EditStat * es = WES(W);
  int i, j; EditPtrType p;

  while (TRUE)
    {
      i = es->Y;
      j = es->LgthTab[i];
      if (j < 2) break;
      p = es->LineTab[i];
      if (*(p + j) == LF || (p + j) >= es->Tail) break;
      *(p + j - 1) = ' ';
      es->Updated = TRUE;
      -- es->LineCount;
      SetTab(es);
      if (Wrap(es)) EditDn(es);
    }
  EditEnd(es);
  W->Flags.Repaint = TRUE;
  return TRUE;
}
