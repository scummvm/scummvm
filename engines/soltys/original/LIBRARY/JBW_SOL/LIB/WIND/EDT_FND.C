#include	<edit.h>
#include	<stdlib.h>
#include	<mem.h>
#include	<string.h>


Boolean	EditRt	(EditStat * es);

Boolean EditFind (Wind * W, const char * s)
{
  EditStat * es = WES(W);
  int i = strlen(s), j;
  Boolean ok = FALSE;
  EditPtrType p = es->LineTab[es->Y]+es->Column;

  while (! ok && EditRt(es))
    {
      ++ p;
      for (j = 0; j < i; j ++)
	if (s[j] != ((es->XlatProc == NULL) ? p[j] : es->XlatProc(p[j]))) break;
      if (j == i) ok = TRUE;
    }
  W->Flags.Repaint = TRUE;
  return ok;
}
