#include	<base.h>
#include	<string.h>


extern	void	(*DictBrowseProc)(void) = NULL;


Boolean DictNew (char * d0)
{
  char * s = GetFldC(1, d0, "Dopisz", 2);

  if (s == NULL) return FALSE;
  else
    {
      long n;
      int i;

      if (MemTrmLen(s, DbfFldLen(1)) == 0) return FALSE;
      i = DbfOrder();
      DbfGoBottom();
      SetOrder(0);
      DbfSkip(-1);
      SetOrder(i);
      n = NrVal(0) + 1;
      DbfGoBottom();
      NrStr(0, n);
      DbfReplace(1, s);
      DbfSkip(0);
      return TRUE;
    }
}





Boolean DictKey (Wind * W)
{
  static char SearchKey[MaxKeyLen+1];
  char * dp = DbfFldPtr(1);
  Boolean ok = TRUE;

  switch (LastKey)
    {
      case ShiftF5    : if (DictBrowseProc != NULL) DictBrowseProc(); break;
      case Left       : W->X = 0;
			//if (! DbfBof())
			  {
			    DbfSkip(-1);
			  }
			break;
      case Right      : DbfSkip(+1);
			if (DbfEof()) DbfSkip(-1); else W->X = 0;
			break;
      case Home       : W->X = 0; DbfGoTop(); break;
      case End        : DbfGoBottom(); DbfSkip(-1);
			W->X = 0; break;
      case BSp        : if (W->X) -- W->X;
			SearchKey[W->X] = '\0'; break;
      default         : if ((LastKey & 0xFF00) || LastKey < ' ')
			  {
			    ok = FALSE;
			    break;
			  }
			if (W->X == 0) LastKey = Upper(LastKey);
			if (W->X < MaxKeyLen-1)
			  {
			    RecPos rp = DbfRecNo();
			    //int i;
			    SearchKey[W->X] = LastKey;
			    SearchKey[++W->X] = '\0';
			    DbfFind(SearchKey);
			    if (memicmp(SearchKey, dp, W->X) != 0)
			      {
				int i;
				DbfGoto(rp);
				for (i = 0; i < W->X-1; i ++)
				  SearchKey[i] = dp[i];
				NextKey = End;
				if (! DictNew(SearchKey)) DbfGoto(rp);
				W->X = 0; SearchKey[0] = '\0';
				LastKey = NoKey;
			      }
			    //for (i = 0; i < W->X; i ++)
			    //  if (Upper(SearchKey[i]) != Upper(dp[i])) break;
			    //SearchKey[W->X = i] = '\0';
			 }
		       break;
    }
  return ok;
}







Boolean DictKeyb (Wind * W)
{
  WindHan * wh = W->AuxPtr;
  EdtDef * fd = &((EdtDef *) (wh->Fmt))[W->Vp];
  int Dict = fd->Base;
  int sel = DbfSelected();
  char * nr = DbfFldPtr(fd->Fld);
  char * nrd;
  Boolean ok = TRUE;
  int ord;

  BaseSelect(Dict);
  ord = DbfOrder();

  nrd = DbfFldPtr(0);
  SetOrder(0);
  DbfFind(nr);
  SetOrder(ord);

  ok = DictKey(W);
  BaseSelect(sel);
  if (ok) DbfReplace(fd->Fld, nrd);
  return ok;
}







void DictShow (Wind * W, int n)
{
  WindHan * wh = W->AuxPtr;
  EdtDef * fd = &((EdtDef *) (wh->Fmt))[n];
  int Dict = fd->Base;
  int x = fd->X + strlen(fd->Nam), y = fd->Y;
  int sel = DbfSelected();
  char *nr;
  int ord;

  nr = DbfFldPtr(fd->Fld);
  BaseSelect(Dict);
  ord = DbfOrder();
  SetOrder(0);
  if (! DbfFind(nr)) DbfGoBottom();
  SetOrder(ord);

  if (n == W->Vp)
    {
      W->Cursor.Shape = (W->Flags.EditEnable) ? CurShp(TRUE) : OFF_Cursor;
      CursorGoto(W, x+W->X, y);
    }
  WriteWindMem(W, x, y, DbfFldPtr(1), DbfFldLen(1));
  BaseSelect(sel);
}
