#include	<base.h>
#include	"C:\jbw\lib\dbf\dbf_inc.h"
#include	<mem.h>

#define	Org	(W->Hp)
#define	Pos	(W->X)

extern	int	MaxSaySize;

extern	GetHan	DbfGetHan = { NULL };

extern	Boolean	InsMode	= FALSE;

static	Boolean	Negative;
static	int	TrueDec;
static	int	DotPos;
static	WindHan * WH;
static	EdtDef * ED;



int DateRealPos (int i)
{
  int d = DateConvr[i] - '1';
  if (d >= 0 && d <= 8) return d;
  else return -1;
}





static void SetDbfParms (Wind * W)
{
  int fld;
  char *p = NULL;

  WH = W->AuxPtr;
  ED = &((EdtDef *)(WH->Fmt))[W->Vp];
  fld = ED->Fld;

  DbfGetHan.X = ED->X + strlen(ED->Nam), DbfGetHan.Y = ED->Y;
  DbfGetHan.Tpe = DbfFldTpe(fld);
  DbfGetHan.Ptr = DbfFldPtr(fld);
  DbfGetHan.Len = (DbfGetHan.Tpe == 'D') ? strlen(DateConvr) : DbfFldLen(fld);
  DbfGetHan.Dec = DbfFldDec(fld);
  DbfGetHan.Wid = min(DbfGetHan.Len, MaxSaySize);
  DbfGetHan.Cap = (Base[WH->Sel].Format[fld].Tpe & 0x20) == 0;

  if (DbfGetHan.Tpe == 'N') p = memchr(DbfGetHan.Ptr, '.', DbfGetHan.Len);
  TrueDec = (p == NULL) ? 0 : (DbfGetHan.Len - (p-DbfGetHan.Ptr+1));
}







Boolean DbfGetHome (Wind * W)
{
  Org = (Pos = 0);
  return TRUE;
}




Boolean DbfGetEnd (Wind * W)
{
  for (Org = DbfGetHan.Len-1; Org && DbfGetHan.Ptr[Org] == ' ' ; Org --);
    if (DbfGetHan.Ptr[Org] != ' ' && Org < DbfGetHan.Len-1) ++ Org;
  Pos = min(MaxSaySize-1, Org);
  return TRUE;
}





static void DbfCheckNum (void)
{
  int i;
  while (TrueDec > DbfGetHan.Dec)
    {
      movmem(DbfGetHan.Ptr, DbfGetHan.Ptr+1, DbfGetHan.Len-1);
      *DbfGetHan.Ptr = ' ';
      -- TrueDec;
    }
  while (TrueDec < DbfGetHan.Dec && *DbfGetHan.Ptr == ' ')
    {
      memcpy(DbfGetHan.Ptr, DbfGetHan.Ptr+1, DbfGetHan.Len-1);
      DbfGetHan.Ptr[DbfGetHan.Len-1] = '0';
      ++ TrueDec;
    }
  DotPos = DbfGetHan.Len-TrueDec-(TrueDec>0);
  for (i = 0; i < DotPos; i ++)
    if (DbfGetHan.Ptr[i] == '0' || DbfGetHan.Ptr[i] == ' ') DbfGetHan.Ptr[i] = ' '; else break;
  for (i = DotPos+1; i < DbfGetHan.Len; i ++)
    if (DbfGetHan.Ptr[i] < '0' || DbfGetHan.Ptr[i] > '9') DbfGetHan.Ptr[i] = '0';

  Negative = FALSE;
  for (i = 0; i < DotPos; i ++)
    {
      if (DbfGetHan.Ptr[i] == '-') { Negative = TRUE; DbfGetHan.Ptr[i] = ' '; }
      if (DbfGetHan.Ptr[i] == ' ') { movmem(DbfGetHan.Ptr, DbfGetHan.Ptr+1, i); *DbfGetHan.Ptr = ' '; }
    }

  if (DbfGetHan.Ptr[DotPos-1] == ' ') DbfGetHan.Ptr[DotPos-1] = '0';
  for (i = 0; DbfGetHan.Ptr[i] == ' '; i ++); if (Negative) DbfGetHan.Ptr[i-1] = '-';
  if (TrueDec) DbfGetHan.Ptr[DotPos] = '.';
  ///////DotPos = DbfGetHan.Len - TrueDec - (TrueDec > 0);$$$$$$$$$$$$$
}







void BaseReset (Wind * W)
{
  SetDbfParms(W);
  DbfGetHome(W);
  while (W->Vp < W->Hig-1 && ED->Keyb == NULL)
    {
      W->Vp = ++ W->Y;
      SetDbfParms(W);
    }
  if (DbfGetHan.Tpe == 'N')
    {
      DbfCheckNum();
    }
  //FieldShow(W, W->Y);$$$$$$$$$$$$$$$$$$$$$$$$$
  W->Flags.Touched = FALSE;
}





void FieldShow (Wind * W, int n)
{
  int vp = W->Vp;
  W->Vp = n;

  SetDbfParms(W);

  if (n == vp)
    {
      W->Cursor.Shape = (W->Flags.EditEnable) ? CurShp(InsMode) : OFF_Cursor;
      CursorGoto(W, DbfGetHan.X+Pos, DbfGetHan.Y);
      if (DbfGetHan.Tpe != 'D') DbfGetHan.Ptr += Org-Pos;
    }
  if (DbfGetHan.Tpe == 'D') WriteWindText(W, DbfGetHan.X, DbfGetHan.Y, DbfD2C(DbfGetHan.Ptr));
  else WriteWindMem(W, DbfGetHan.X, DbfGetHan.Y, DbfGetHan.Ptr, DbfGetHan.Wid);
  W->Vp = vp;
}







Boolean DbfGetLeft (Wind * W)
{
  if (Org)
    {
      --Org;
      if (Pos) --Pos;
    }
  return TRUE;
}




Boolean DbfGetUp (Wind * W)
{
  BaseReset(W);
  MenuUp(W);
  return TRUE;
}




Boolean DbfGetDown (Wind * W)
{
  BaseReset(W);
  MenuDown(W);
  return TRUE;
}




Boolean DbfGetRight (Wind * W)
{
  if (Org<DbfGetHan.Len-1)
    {
      ++Org;
      if (Pos<MaxSaySize-1) ++Pos;
    }
  return TRUE;
}



#pragma argsused
Boolean DbfGetIns (Wind * W)
{
  InsMode = ! InsMode;
//  W->Cursor.Shape = CurShp(InsMode);
  return TRUE;
}




Boolean DbfGetDel (Wind * W)
{
  memcpy(DbfGetHan.Ptr+Org, DbfGetHan.Ptr+Org+1, DbfGetHan.Len-Org-1);
  DbfGetHan.Ptr[DbfGetHan.Len-1] = ' ';
  return TRUE;
}




Boolean DbfGetBsp (Wind * W)
{
  if (Org)
    {
      memcpy(DbfGetHan.Ptr+Org-1,DbfGetHan.Ptr+Org,DbfGetHan.Len-Org);
      DbfGetHan.Ptr[DbfGetHan.Len-1]=' ';
      DbfGetLeft(W);
    }
  return TRUE;
}





//--------------------------------------------------------------------------



Boolean WindGetC (Wind *W)
{
  switch (LastKey)
    {
      case Home	: return DbfGetHome(W);
      case End	: return DbfGetEnd(W);
      case Del	: return DbfGetDel(W);
      case BSp	: return DbfGetBsp(W);
      case Left	: return DbfGetLeft(W);
      case Ins	: return DbfGetIns(W);
      default	: if (LastKey & 0xFF00) return FALSE;
		  if (LastKey < 32) return FALSE;
		  if (Org == 0 && DbfGetHan.Cap) LastKey = Upper(LastKey);
		  if (InsMode) movmem(DbfGetHan.Ptr+Org, DbfGetHan.Ptr+Org+1, DbfGetHan.Len-Org-1);
		  DbfGetHan.Ptr[Org] = LastKey;
		  return DbfGetRight(W);
      case Right: return DbfGetRight(W);
    }
}




Boolean WindGetD (Wind *W)
{
  int i;
  switch (LastKey)
    {
      case Home	: return DbfGetHome(W);
      case End	: return DbfGetEnd(W);
      case Left	:
      case BSp	: if (Pos == 0) break;
		      do
			{ -- Pos; }
		      while (Pos && DateRealPos(Pos) < 0);
		      if (LastKey != BSp) break;
      case Del	: for (i = Pos; DateRealPos(i+1) >= 0; i ++)
			DbfGetHan.Ptr[DateRealPos(i)] = DbfGetHan.Ptr[DateRealPos(i+1)];
		      DbfGetHan.Ptr[DateRealPos(i)] = ' '; break;
      default	: if (LastKey & 0xFF00) return FALSE;
		      if (LastKey < '0' || LastKey > '9') return FALSE;
		      i = DateRealPos(Pos);
		      DbfGetHan.Ptr[i] = LastKey;
      case Right: if (Pos >= DbfGetHan.Wid-1) break;
		      do
			{ ++ Pos; }
		      while (Pos < DbfGetHan.Wid-1 && DateRealPos(Pos) < 0);
		      break;
    }
  return TRUE;
}






static Boolean WindGetN (Wind *W)
{
  int i;

  if (! W->Flags.Touched) DbfCheckNum();
  switch (LastKey)
    {
      case Left	: DbfGetLeft(W); if (Org && Org == DotPos) -- Org; break;
      case '.'	: if (TrueDec) Org = DotPos+1; break;
      case Home	: DbfGetHome(W); break;
      case End	: DbfGetEnd(W); break;
      case BSp	: if (Org == 0) break;
		      -- Org; if (Org && Org == DotPos) -- Org;
      case Del	: if (Org < DotPos)
			{
			  memcpy(DbfGetHan.Ptr+Org, DbfGetHan.Ptr+Org+1, DotPos-Org-1);
			  DbfGetHan.Ptr[DotPos-1] = ' '; break;
			}
		      else
			{
			  memcpy(DbfGetHan.Ptr+Org, DbfGetHan.Ptr+Org+1, DbfGetHan.Len-Org-1);
			  DbfGetHan.Ptr[DbfGetHan.Len-1] = '0';
			}
		      break;
      case Ins	: return DbfGetIns(W);
      default	: if (LastKey & 0xFF00) return FALSE;
		      if (LastKey < '0' || LastKey > '9') return FALSE;
      case '-'	: if (LastKey == '-' && Org > DotPos) break;
		      if (! W->Flags.Touched)
			{
			  for (i = 0; i < DotPos; i ++) DbfGetHan.Ptr[i] = ' ';
			  for (i = DotPos+1; i < DbfGetHan.Len; i ++) DbfGetHan.Ptr[i] = '0';
			}
		      if (InsMode)
			{
			  if (Org < DotPos)
			    {
			      movmem(DbfGetHan.Ptr+Org, DbfGetHan.Ptr+Org+1, DotPos-Org-1);
			    }
			  else
			    {
			      movmem(DbfGetHan.Ptr+Org, DbfGetHan.Ptr+Org+1, DbfGetHan.Len-Org-1);
			    }
			}
		      DbfGetHan.Ptr[Org] = LastKey;
      case Right	: DbfGetRight(W); if (Org == DotPos) ++ Org; break;
    }
  Pos = Org;
  return TRUE;
}





#pragma argsused
static Boolean WindGetL (Wind *W)
{
  switch (LastKey = GetKey())
    {
      case 'f'	:
      case 'F'	:
      case 't'	:
      case 'T'	:
      case 'n'	:
      case 'N'	:
      case 'y'	:
      case 'Y'	: *DbfGetHan.Ptr = LastKey; break;
      default	: return FALSE;
    }
  return TRUE;
}






Boolean FieldKeyb (Wind *W)
{
  Boolean ok;

  SetDbfParms(W);

  switch (DbfGetHan.Tpe)
    {
      case 'D' : ok = WindGetD(W); break;
      case 'N' : ok = WindGetN(W); break;
      case 'L' : ok = WindGetL(W); break;
      case 'M' : ok = FALSE; break;
      case 'C' :
      default  : ok = WindGetC(W); break;
    }
  return ok;
}
