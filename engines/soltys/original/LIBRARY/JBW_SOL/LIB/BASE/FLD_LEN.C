#include	<base.h>
#include	<string.h>




int FormatFieldLen (WindHan *wh, int F)
{
  EdtDef * ed = wh->Fmt;
  FldDef * fd = Base[wh->Sel].Format;

  int i = ed[F].Fld;
  if (i < 0) return -i;
  if (fd[i].Tpe == 'D') return strlen(DateConvr);
    {
      i = fd[i].Len;
      if (i > MaxSaySize) i = MaxSaySize;
    }
  return i;
}
