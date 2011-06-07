#include	<base.h>
#include	<string.h>
#include	<stdlib.h>
#include	<mem.h>

#ifdef	DEBUG
  #include	<stdio.h>
#endif

extern	BaseHan	Base[];

static	int	NrSiz, LnSiz;

void	NoCore	(void);








static void PutText (EditStat * es, char * nr, int lnr)
{
  int siz;
  int wid = DbfFldLen(1);
  char *str = DbfFldPtr(1);
  EditPtrType p, q;

  //---- write edited lines - starting from lnr
  for (p = es->Buff; p < es->Tail; lnr ++)
    {
      q = EditMemChr(p, '\n', (word) (es->Tail-p));
      if (q == NULL)
	{
	  q = es->Tail;
	  if (q - p > wid) q = p + wid;
	}
      siz = (word) (q - p);
      DbfGoBottom();
      DbfReplace(0, nr);
      itom(lnr, DbfFldPtr(0)+NrSiz, NrRadix, LnSiz);
      EditMovMem(p, str, siz);
      while (siz < wid) str[siz ++] = ' ';
      p = q+1;
    }
  es->Updated = FALSE;
}






void WriteText (Wind * W)
{
  EditStat * es = WES(W);
  WindHan * wh = W->AuxPtr;
  int B = wh->Sel;
  char nr[MaxKeyLen+1];

  NrSiz = (wh->Lnk >= 0) ? Base[wh->Lnk].Format[0].Len : -wh->Lnk;
  LnSiz = Base[B].Format[0].Len-NrSiz;

  if (! es->Updated) return;
  BasePush(B);
  ltom(Base[B].Current, nr, NrRadix, NrSiz);
  itom (0, nr+NrSiz, NrRadix, LnSiz);
  nr[NrSiz+LnSiz] = '\0';

  /// erase old text
  DbfLock();
  for (DbfFind(nr); ! DbfEof() && memcmp(DbfFldPtr(0), nr, NrSiz) == 0; DbfFind(nr))
    DbfRemove();

  //---- write edited lines
  PutText(es, nr, 0);
  DbfUnlock();
  BasePop();
}






void AppendText (Wind * W, const char * header)
{
  EditStat * es = WES(W);
  WindHan * wh = W->AuxPtr;
  int B = wh->Sel;
  int i;
  char nr[MaxKeyLen+1];
  char * np;

  NrSiz = Base[wh->Lnk].Format[0].Len;
  LnSiz = Base[B].Format[0].Len-NrSiz;

 #ifdef DEBUG
  if (LnSiz > 4)
    {
      puts("LnSiz > 4");
      abort();
    }
 #endif

  if (es->Tail == es->Buff) es->Updated = FALSE;
  if (! es->Updated) return;
  BasePush(B);
  np = DbfFldPtr(0);
  ltom(Base[B].Current, nr, NrRadix, NrSiz);	//--- main number
  memset(nr+NrSiz, '~', LnSiz);			//--- line number (largest)
  nr[NrSiz+LnSiz] = '\0';
  DbfLock();
  DbfFind(nr);					//--- find bottom
  if (memcmp(nr, np, NrSiz) != 0) DbfSkip(-1);
  i = (memcmp(nr, np, NrSiz) == 0) ? (mtoi(np + NrSiz, NrRadix, LnSiz) + 1) : 0;

  //---- write header
  if (header != NULL)
    {
      DbfGoBottom();
      DbfReplace(0, nr);
      itom(i ++, DbfFldPtr(0)+NrSiz, NrRadix, LnSiz);
      DbfReplace(1, header);
      DbfFldPtr(1)[DbfFldLen(1)-1] = '\0';
    }

  //---- write edited lines
  PutText(es, nr, i);
  DbfUnlock();
  BasePop();
}






void ReadText (Wind * W)
{
  EditStat * es = WES(W);
  WindHan * wh = W->AuxPtr;
  int B = wh->Sel;
  int i, siz;
  char nr[MaxKeyLen+1], *s;
  EditPtrType p;

  BasePush(B);

  WriteText(W);

  if (wh->Lnk >= 0) Base[B].Current = Base[wh->Lnk].Current;
  ltom(Base[B].Current, nr, NrRadix, NrSiz);
  itom(0, nr+NrSiz, NrRadix, LnSiz);
  nr[NrSiz+LnSiz] = '\0';

  s = DbfFldPtr(1);

  p = es->Buff;
  EditMemSet(p, '\n', es->BufSiz);

  DbfLock();

  DbfFind(nr);
  for (i = 0; memcmp(DbfFldPtr(0), nr, NrSiz) == 0; i ++)
    {
      for (siz = DbfFldLen(1)-1; siz >= 0; siz --)
	if (! IsWhite(s[siz]))
	  break;
      if (++ siz)
	{
	  if (p - es->Buff + siz + 1 >= es->BufSiz) break;
	  EditMovMem(s, p, siz);
	  p += siz;
	}
      * (p ++) = '\n';
      DbfSkip(1);
    }
  DbfUnlock();
  BasePop();
  es->Tail = p;
  es->Updated = FALSE;
  es->Valid = FALSE;
  W->Flags.Repaint = TRUE;
}






void KillTextWind (Wind * W)
{
  EditStat * es = WES(W);
  WindHan * wh = W->AuxPtr;
  WriteText(W);
  EditFree(es->Buff);
  BasePush(wh->Sel);
  DbfClose();
  BasePop();
}





void TextRepaint (Wind * W)
{
 if (! WES(W)->Valid) ReadText(W);
  EditReptProc(W);
}





Wind * OpenTextWind (WindHan * wh)
{
  EditStat * es = wh->Fmt;
  BaseHan * bh = &Base[wh->Sel];
  FldDef * fd = bh->Format;
  Wind *w;

  if (wh->Wnd != NULL)
    {
      BringToFront(wh->Wnd);
      return wh->Wnd;
    }
  w = CreateWind(0, 0, wh->W+1, wh->H+1, STD, FALSE, TRUE);
  if (w == NULL) NoCore();
  SetTitle(w, wh->Nam, -1);
  MoveWind(w, wh->X, wh->Y);

  w->Flags.Movable = TRUE;
  w->Flags.HSizeable = TRUE;
  w->Flags.VSizeable = TRUE;
  w->Flags.Zoomable  = TRUE;
  w->ShowProc = ShowEditChar;
  w->Body.Near = es;
  w->AuxPtr = wh;
  w->KillProc = KillTextWind;
  w->ReptProc = TextRepaint;

  if ((es->Buff = EditMalloc(es->BufSiz = 1024 * wh->Mem)) == NULL) NoCore();
  es->BufSiz --;
  es->Tail = es->Buff;
  es->InsMode = TRUE;
  es->Updated = FALSE;
  es->XlatProc = NULL;
  es->Wrap = fd[1].Len+1;
  es->Valid = FALSE;

  return wh->Wnd = w;
}
