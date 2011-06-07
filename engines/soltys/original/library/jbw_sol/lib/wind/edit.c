#include	<edit.h>
#include	<stdlib.h>
#include	<mem.h>
#include	<string.h>
#include	<dos.h>		// only for delay()


#define		InWord(c)	(IsAlNum(c) || c > 127)



extern	int	EOL_Ch = 'ú';
extern	int	MRK_Ch = '';


static	int	EHig;
static	int	EWid;





static EditPtrType NextLine (EditPtrType line, EditPtrType end)
{
  EditPtrType p = EditMemChr(line, LF, (int)(end-line));
  return (p == NULL) ? end : p+1;
}





static EditPtrType PrevLine (EditPtrType line, EditPtrType top)
{
  EditPtrType p = line-1;
  while (*(-- p) != LF) if (p < top) return top;
  return p+1;
}






/*
void SetTab (EditStat *es)
{
  EditPtrType * linep = es->LineTab, * lineq = linep+MaxScrHig, p;
  byte * lenp = es->LgthTab;

  for (p = *linep; linep < lineq; linep ++, lenp ++)
    {
      *linep = p;
      p = NextLine(p, es->Tail);
      *lenp = p - *linep;
    }
}
*/

void SetTab (EditStat *es)
{
  int i; EditPtrType p = es->LineTab[0];

  for (i = 0; i < MaxScrHig; i ++)
    {
      es->LineTab[i] = p;
      p = NextLine(p, es->Tail);
      es->LgthTab[i] = p - es->LineTab[i];
    }
}







static char CurChar (EditStat *es)
{
  return es->LineTab[es->Y][es->Column];
}





static void ViewHome (EditStat *es)
{
  es->Line = 0; es->Column = 0;
  es->Y = 0;
  es->Here = NULL;
  es->LineTab[0] = es->Buff;
  SetTab(es);
}






static Boolean RollDn (EditStat *es)
{
  int i; EditPtrType p = es->LineTab[0];

  if (p <= es->Buff) return FALSE;
  for (i = MaxScrHig-1; i > 0; i --)
    {
      es->LineTab[i] = es->LineTab[i-1];
      es->LgthTab[i] = es->LgthTab[i-1];
    }
  es->LineTab[0] = PrevLine(p, es->Buff);
  es->LgthTab[0] = p - es->LineTab[0];
  ++ es->Y; return TRUE;
}





static Boolean RollUp (EditStat *es)
{
  int i; EditPtrType p = NextLine(es->LineTab[MaxScrHig-1], es->Tail);

  if (es->LineTab[EHig-1] >= es->Tail) return FALSE;
  for (i = 1; i < MaxScrHig; i ++)
    {
      es->LineTab[i-1] = es->LineTab[i];
      es->LgthTab[i-1] = es->LgthTab[i];
    }
  es->LineTab[MaxScrHig-1] = p;
  es->LgthTab[MaxScrHig-1] = NextLine(p, es->Tail) - p;
  -- es->Y; return TRUE;
}






void EditHome (EditStat *es)
{
  es->Column = 0; es->S = 0;
}





void EditEnd (EditStat *es)
{
  int i = es->LgthTab[es->Y];
  es->Column = i - (i && es->LineTab[es->Y][i-1] == LF);
}





Boolean EditUp (EditStat *es)
{
  int l;

  if (es->Line <= 0) return FALSE;
  if (es->Y > 0) -- es->Y;
  else
    if (RollDn(es)) -- es->Y;
    else return FALSE;
  -- es->Line; l = es->LgthTab[es->Y];
  if (es->Column >= l) EditEnd(es);
  return TRUE;
}






Boolean EditDn (EditStat *es)
{
  int l;

  if (es->Line >= es->LineCount) return FALSE;
  if (es->Y < EHig-1) ++ es->Y;
  else
    if (RollUp(es)) ++ es->Y;
    else return FALSE;
  ++ es->Line; l = es->LgthTab[es->Y];
  if (es->Column >= l) EditEnd(es);
  return TRUE;
}






Boolean EditLt (EditStat *es)
{
  if (es->Column)
    {
      -- es->Column;
      return TRUE;
    }
  if (EditUp(es))
    {
      EditEnd(es);
      return TRUE;
    }
  return FALSE;
}





Boolean EditRt (EditStat *es)
{
  int i = es->LgthTab[es->Y];
  if (es->Column < es->LgthTab[es->Y]-(i && es->LineTab[es->Y][i-1] == LF))
    {
      ++ es->Column;
      return TRUE;
    }
  if (EditDn(es))
    {
      EditHome(es);
      return TRUE;
    }
  return FALSE;
}





void WordLeft (EditStat *es)
{
  do { if (!EditLt(es)) return; } while (!InWord(CurChar(es))); EditRt(es);
  do { if (!EditLt(es)) return; } while (InWord(CurChar(es))); EditRt(es);
}



void WordRight (EditStat *es)
{
  while (InWord(CurChar(es))) if (!EditRt(es)) return;
  while (!InWord(CurChar(es))) if (!EditRt(es)) return;
}





void EditPgUp (EditStat *es)
{
  int i;
  for (i = 0; i < EHig; i ++) { RollDn(es); EditUp(es); }
}





void EditPgDn (EditStat *es)
{
  int i;
  for (i = 0; i < EHig; i ++) { RollUp(es); EditDn(es); }
}





Boolean ViewUp (EditStat *es)
{
  if (es->Y < EHig/2) RollDn(es);
  return EditUp(es);
}






Boolean ViewDn (EditStat *es)
{
  if (es->Y > EHig/2) RollUp(es);
  return EditDn(es);
}





void DelLine (EditStat *es)
{
  EditPtrType p = es->LineTab[es->Y];
  EditPtrType q = p + es->LgthTab[es->Y];
  if (p != q && *(q-1) == LF) -- es->LineCount;
  Move(p, q, (int)(es->Tail - q)); es->Tail -= (int)(q-p);
  es->Updated = TRUE; SetTab(es); EditHome(es);
}




static Boolean PutKey(EditStat *es, EditPtrType ptr, int k, Boolean ins)
{
  if (k == CR || k == LF)
    {
      k = LF;
      if (! ins)
	{
	  EditEnd(es);
	  return TRUE;
	}
    }
  if (es->Tail-es->Buff >= es->BufSiz-es->LineCount) return FALSE;
  ins = ins || *ptr == LF;
  if (ins)
    {
      EditMovMem(ptr, ptr+1, (int)((es->Tail++)-ptr));
    }
  else
    {
      if (ptr == es->Tail)
	{
	  ++ es->Tail;
	  ++ es->LgthTab[es->Y];
	}
    }
  if (k == LF) ++ es->LineCount;
  *ptr = k;
  if (ins) SetTab(es);
  es->Updated = TRUE;
  return TRUE;
}






Boolean Wrap (EditStat *es)
{
  int i, x = es->Column, y = es->Y;
  EditPtrType p = es->LineTab[y];
  EditPtrType q = p + es->Wrap;

  i = es->LgthTab[y];
  if (i + (i && p[i-1] != LF) <= es->Wrap) return FALSE;
  while (-- q != p)
    {
      if (*q == ' ' /*|| *q == Tab*/) break;
    }
  if (q == p)
    {
      PutKey(es, p + ((x) ? (x ++) : (++ x)), LF, TRUE);
    }
  else
    {
      *q = LF;
      ++ es->LineCount;
      es->Updated = TRUE;
    }
  SetTab(es);
  EditHome(es);
  for (i = 0; i < x; i ++) EditRt(es);
  return TRUE;
}







void EditInit (Wind * W)
{
  EditStat * es = WES(W);
  EditPtrType p;
  int i;

  // count up EOLNs
  for (i = 0, p = es->Buff; p < es->Tail; i ++)
    {
      if ((p = EditMemChr(p, LF, (int)(es->Tail-p))) == NULL) break;
      ++ p;
    }
  es->LineCount = i;
  if (es->Valid)
    {
      EditPtrType p = es->LineTab[0];
      while (p != es->Buff && *(p-1) != '\n') -- p;
      es->LineTab[0] = p;
      SetTab(es);
    }
  else
    {
      ViewHome(es);
      EditHome(es);
      es->Valid = TRUE;
    }
  W->Flags.Repaint = TRUE;
}






void EditReptProc (Wind *W)
{
  EditStat *es = WES(W);

  EHig = WindBodyHig(W);
  EWid = WindBodyWid(W);

  if (! es->Valid) EditInit(W);
  if (W->Flags.EditEnable)
    {
      while (es->Y >= EHig) RollUp(es);
      while (es->Column < es->S) -- es->S;
      while (es->S < 0 || es->Column - es->S >= EWid) ++ es->S;
      if (es->StatProc != NULL) (es->StatProc)(W);
      CursorGoto(W, es->Column-es->S, es->Y);
      W->Cursor.Shape = (Mono) ? HGC_Cursor : CGA_Cursor;
      if (! es->InsMode) W->Cursor.Shape &= 0x00FF;
    }
  else
    {
      es->S = -1;
      W->Cursor.Shape = OFF_Cursor;
    }
  SetRefreshWind(W);
}







static void FollowMouse (void)
{
  Wind *W = TopWind();
  EditStat *es = WES(W);
  int x, y, l;
  extern void (*IdleProc) (void);

  if (PosToWind(MX, MY) != W) return;   // mouse out of window

  while (TRUE)
    {
      y = RelY(W, MY);

      if (W->Flags.EditEnable)
	{
	  while (y < es->Y) if (!EditUp(es) || y <  0   ) break;
	  while (y > es->Y) if (!EditDn(es) || y >= EHig) break;
	  x = RelX(W, MX);
	  l = es->LgthTab[es->Y];
	  x = min(x, l);
	  while (es->Column && x < es->Column - es->S)
	    if (! EditLt(es) || x < 0) break;
	  while (es->Column < l-1 && x > es->Column - es->S)
	    if (! EditRt(es) || x > EWid) break;
	}
      else
	{
	  if (y < es->Y) ViewUp(es);
	  if (y > es->Y) ViewDn(es);
	}
      EditReptProc(W);
      Refresh();
      if (! MousePressed(1)) break;
      if (IdleProc != NULL) IdleProc();
      delay(50);
      MX = MouseX(); MY = MouseY();
    }
}






word ShowEditChar (void)
{
  //int c, i;
  //EditStat *es = W->Body.Near;
  //EditPtrType fp;

  asm		mov	SI,[BX].Body.Near	// SI points to EditStat
  asm		add	CX,[SI].S		// x + shift
  asm		jns	normal

  // (X == 0 && S == -1)
  asm		mov	AL,' '
  asm		cmp	DX,[SI].(EditStat)Y
  asm		jne	atrib
  asm		mov	AX,MRK_Ch
  asm		je	atrib

  normal:
  asm		add	SI,DX			// + y
  asm           mov	AH,[SI].LgthTab         // get line length
  asm		add	SI,DX			// + 2*y

  #ifdef FAR_EDIT
    asm		shl	DX,1			// resize index if FAR
    asm		add	SI,DX
    asm		les	DI,[SI].LineTab		// take line address
    asm		sub	SI,DX
    asm		sub	SI,DX
    asm		shr	DX,1			// restore index
  #else
    asm		mov	DI,[SI].LineTab		// take line address
    asm		sub	SI,DX
    asm		sub	SI,DX
  #endif

  asm		add	DI,CX

//  x += es->S; i = es->LgthTab[y];
//  fp = es->LineTab[y];

  asm		mov	AL,' '
  asm		cmp	CL,AH
  asm		jae	eolch

  #ifdef FAR_EDIT
    asm		mov	AL,ES:[DI]
  #else
    asm		mov	AL,[DI]
  #endif

//  c = (x < i) ? fp[x] : ' ';

  eolch:
  asm		cmp	AL,LF
  asm		jne	atrib
  asm		mov	AL,byte ptr EOL_Ch

//  if (c == '\n') c = EOL_Ch;

  atrib:
  asm		sub	DI,CX
  asm		mov	AH,[BX].Color[FLD_N]	// normal
  asm		mov	CX,[SI].Here		// block origin
  asm		or	CX,CX			// NULL?
  asm		jz	rtn
  asm		cmp	DI,CX			// >= es->Here?
  asm		jl	rtn
  asm		mov	DX,[SI].(Wind)Y		// current line number

  asm		shl	DX,1
  #ifdef FAR_EDIT
  asm		shl	DX,1			// mult by 2 or 4
  #endif

  asm		add	SI,DX			// complete offset
  asm		mov	DX,[SI].LineTab 	// address of current line

  asm		cmp	DI,DX			// <= es->LineTab[es->Y]?
  asm		jg	rtn
  asm		mov	AH,[BX].Color[SEL_N]	// text selected

  rtn:
  return _AX;

//  if ((word) (es->Here) != NULL)
//    {
//      if ((word) fp >= (word) (es->Here) &&
//	    (word) fp <= (word) (es->LineTab[es->Y]))
//	return W->Attr[1] | c;
//    }
//  return W->Attr[0] | c;

}







Boolean EditDown (Wind * W)
{
  Boolean d = EditDn(WES(W));
  W->Flags.Repaint = TRUE;
  return d;
}






Boolean EditKey (Wind *W)
{
  EditPtrType p;
  EditStat *es = WES(W);

  /* window height, width */
  EHig = WindBodyHig(W);
  EWid = WindBodyWid(W);

  if (! es->Valid || es->Line > es->LineCount) EditInit(W);

  if (W->Flags.EditEnable)
    {
      p = es->LineTab[es->Y]+es->Column;
      switch(LastKey)
	{
	  case TwiceLeft   :
	  case MouseLeft   : FollowMouse(); break;
	  case CtrlY       : DelLine(es);   break;
	  case Home        : EditHome(es);  break;
	  case End         : EditEnd(es);   break;
	  case Left        : EditLt(es);    break;
	  case Right       : EditRt(es);    break;
	  case Up          : EditUp(es);    break;
	  case Down        : EditDn(es);    break;
	  case PgUp        : EditPgUp(es);  break;
	  case PgDn        : EditPgDn(es);  break;
	  case CtrlLeft    : WordLeft(es);  break;
	  case CtrlRight   : WordRight(es); break;
	  case CtrlPgUp    : ViewHome(es);  EditHome(es); break;
	  case CtrlPgDn    : while (EditDn(es)); EditEnd(es); break;
	  case Ins	   : es->InsMode = !es->InsMode; break;
	  case BSp	   : if (EditLt(es)) --p; else break;
	  case Del	   : if (p < es->Tail)
			       {
				 if (*p == LF) -- es->LineCount;
				 EditMovMem(p+1, p, (int)((--es->Tail)-p));
				 SetTab(es); es->Updated = TRUE;
				 while (Wrap(es));
			       }
			     break;
	  default          : if (LastKey & 0xFF00) return FALSE;
			     if (LastKey < 32) return FALSE;
	  case Enter       : if (PutKey(es, p, LastKey, es->InsMode))
			       {
				 if (es->InsMode) SetTab(es);
				 Wrap(es);
				 EditRt(es);
			       }
			     break;
	}
    }
  else
    {
      switch(LastKey)
	{
	  case MouseLeft   : FollowMouse(); break;
	  case CtrlPgUp    :
	  case Home        : ViewHome(es); break;
	  case CtrlPgDn    :
	  case End         : while (ViewDn(es)); break;
	  case Up          : ViewUp(es); break;
	  case Down        : ViewDn(es); break;
	  case PgUp        : EditPgUp(es); break;
	  case PgDn        : EditPgDn(es); break;
	  default          : return FALSE;
	}
    }
  W->Flags.Repaint = TRUE;
  return TRUE;
}
