#include	<base.h>
#include	<string.h>

extern	char	MaleSign[] = "\xB";
extern	char	FemaleSign[] = "\xC";


void SexShow (Wind * W, int n)
{
  WindHan *wh = W->AuxPtr;
  EdtDef * ed = &((EdtDef *) (wh->Fmt))[n];
  int x = ed->X + strlen(ed->Nam), y = ed->Y;
  char *s = " ";

  if (n == W->Vp)
    {
      W->Cursor.Shape = (W->Flags.EditEnable) ? CurShp(TRUE) : OFF_Cursor;
      CursorGoto(W, x, y);
    }
  switch (*DbfFldPtr(ed->Fld))
    {
      case '0' : s = FemaleSign; break;
      case '1' : s = MaleSign;   break;
    }
  WriteWindText(W, x, y, s);
}






Boolean SexKey (char * s)
{
  Keys k = Upper(LastKey);

  switch (k)
    {
      case Left  : *s = '0'; break;
      case Right : *s = '1'; break;
      case ' '   : if (*s == '1') *s = '0'; else *s = '1'; break;
      default    : if (k & 0xFF00) return FALSE;
		   if (k == Upper(*FemaleSign)) { *s = '0'; break; }
		   if (k == Upper(*MaleSign)) { *s = '1'; break; }
		   return FALSE;
    }
  return TRUE;
}






Boolean SexKeyb (Wind *W)
{
  WindHan *wh = W->AuxPtr;
  EdtDef *fd = &((EdtDef *) (wh->Fmt))[W->Vp];
  return SexKey(DbfFldPtr(fd->Fld));
}
