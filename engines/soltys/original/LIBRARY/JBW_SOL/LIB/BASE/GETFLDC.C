#include	<base.h>
#include	<stdlib.h>
#include	<string.h>



char * GetFldC (int fld, const char * s, const char * t, int hlp)
{
  static char buf[128];
  Wind * w;
  char * r = NULL;
  Boolean WindGetC(Wind * W);
  int i = strlen(t);

  DbfGetHan.Ptr = buf;
  DbfGetHan.Len = DbfFldLen(fld);
  DbfGetHan.Wid = DbfGetHan.Len;
  DbfGetHan.X = 1;
  DbfGetHan.Y = 0;
  DbfGetHan.Tpe = 'C';

  if ((w = MakeWind(0, 0, max(i, DbfGetHan.Len)+3, 2, STD)) == NULL)
    return NULL;
  w->Flags.Movable = TRUE;
  w->Help = hlp;
  SetTitle(w, t, -1);
  StdMove(w);
  ShowWind(w);
  memset(buf, ' ', sizeof(buf));

  if (s != NULL)
    {
      i = strlen(s);
      memcpy(buf, s, min(i, DbfGetHan.Len));
    }
  while (TopWind() == w)
    {
      w->Cursor.Shape = CurShp(InsMode);
      CursorGoto(w, DbfGetHan.X+w->X, DbfGetHan.Y);
      WriteWindText(w, DbfGetHan.X, DbfGetHan.Y, buf);
      switch (GetKey())
	{
	  case Enter   : r = buf; // ...
	  case Esc     : CloseWind(w); break;
	  default      : WindGetC(w); break;
	}
    }
  return r;
}
