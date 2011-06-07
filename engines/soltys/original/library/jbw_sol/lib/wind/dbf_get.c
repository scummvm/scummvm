#include	<wind.h>
#include	"C:\jbw\lib\dbf\dbf_inc.h"
#include	<mem.h>



extern	int	MaxSaySize;

extern	Boolean	InsMode	= FALSE;

static	Pos = 0, Org = 0;




static void DbfGetHome (void)
{
  Org = (Pos = 0);
}





Boolean WindGetC (Wind *W, int x, int y, char *buff, int l)
{
  Boolean more = TRUE;
  int l1 = min(l, MaxSaySize);

  DbfGetHome();
  W->Cursor.Shape = CurShp(InsMode);
  SetRefreshWind(W);
  if (Pos >= l1) Pos = l1-1;

  while (more)
    {
      CursorGoto(W, x+Pos, y);
      WriteWindMem(W, x, y, buff + Org-Pos, l1);
      switch (GetKey())
	{
	  case Home	: DbfGetHome(); break;
	  case End	: for (Org = l-1; Org && buff[Org] == ' ' ; Org --);
			  if (buff[Org] != ' ' && Org < l-1) ++ Org;
			  Pos = min(MaxSaySize-1, Org); break;
	  case Del	: memcpy(buff+Org, buff+Org+1, l-Org-1);
			  buff[l-1] = ' '; break;
	  case Ins	: InsMode = ! InsMode; W->Cursor.Shape = CurShp(InsMode);
			  SetRefreshWind(W); break;
	  case BSp	: if (Org)
			    {
			      memcpy(buff+Org-1,buff+Org,l-Org);
			      buff[l-1]=' ';
			    }
	  case Left	: if (Org) { --Org; if (Pos) --Pos; } break;
	  case Enter	:
	  case Esc	: more = FALSE; break;
	  default	: if (LastKey & 0xFF00) { more = FALSE; break; }
			  if (InsMode) movmem(buff+Org, buff+Org+1, l-Org-1);
			  buff[Org] = LastKey;
	  case Right	: if (Org<l-1) { ++Org; if (Pos<MaxSaySize-1) ++Pos; }
			  break;
	}
    }
  return TRUE;
}




static int DateRealPos (int i)
{
  int d = DateConvr[i] - '1';
  if (d >= 0 && d <= 8) return d;
  else return -1;
}




Boolean WindGetD (Wind *W, int x, int y, char *buff)
{
  Boolean more = TRUE;
  int len = strlen(DateConvr);
  word shp = W->Cursor.Shape;

  W->Cursor.Shape = CurOvr;
//  SetRefreshWind(W);
  DbfGetHome();

  while (more)
    {
      int i;
      CursorGoto(W, x + Org, y);
      WriteWindText(W, x, y, DbfD2C(buff));
      switch (GetKey())
	{
	  case Home	: DbfGetHome(); break;
	  case End	: Org = len-1; break;
	  case Left	:
	  case BSp	: if (Org == 0) break;
			  do
			    { -- Org; }
			  while (Org && DateRealPos(Org) < 0);
			  if (LastKey != BSp) break;
	  case Del	: for (i = Org; DateRealPos(i+1) >= 0; i ++)
			    buff[DateRealPos(i)] = buff[DateRealPos(i+1)];
			  buff[DateRealPos(i)] = ' '; break;
	  case Enter	:
	  case Esc	: more = FALSE; break;
	  default	: if (LastKey & 0xFF00) { more = FALSE; break; }
			  if (LastKey < '0' || LastKey > '9') break;
			  i = DateRealPos(Org);
			  buff[i] = LastKey;
	  case Right	: if (Org >= len-1) break;
			  do
			    { ++ Org; }
			  while (Org < len-1 && DateRealPos(Org) < 0);
			  break;
	}
    }
  W->Cursor.Shape = shp;
  return TRUE;
}






Boolean WindGetN (Wind *W, int x, int y, char *buff, int l, int d)
{
  int i, td, pp;
  Boolean neg;
  Boolean more = TRUE, virg = TRUE;
  char *p;

  p = strchr(buff, '.');
  td = (p == NULL) ? 0 : l - (p-buff+1);
  W->Cursor.Shape = CurShp(InsMode);
  W->Y = y;
  SetRefreshWind(W);
  DbfGetHome();
  while (TRUE)
    {
      if (virg || !more)
	{
	  while (td > d)
	    {
	      movmem(buff, buff+1, l-1);
	      *buff = ' ';
	      -- td;
	    }
	  while (td < d && *buff == ' ')
	    {
	      memcpy(buff, buff+1, l-1);
	      buff[l-1] = '0';
	      ++ td;
	    }
	  pp = l-td-(td>0);
	  for (i = 0; i < pp; i ++)
	    if (buff[i] == '0' || buff[i] == ' ') buff[i] = ' '; else break;
	  for (i = pp+1; i < l; i ++)
	    if (buff[i] < '0' || buff[i] > '9') buff[i] = '0';

	  neg = FALSE;
	  for (i = 0; i < pp; i ++)
	    {
	      if (buff[i] == '-') { neg = TRUE; buff[i] = ' '; }
	      if (buff[i] == ' ') { movmem(buff, buff+1, i); *buff = ' '; }
	    }

	  if (buff[pp-1] == ' ') buff[pp-1] = '0';
	  for (i = 0; buff[i] == ' '; i ++); if (neg) buff[i-1] = '-';
	  if (td) buff[pp] = '.';
	}
      pp = l - td - (td>0);
      CursorGoto(W, x+Org, y);
      WriteWindMem(W, x, y, buff, l);
      if (! more) break;
      switch (LastKey = GetKey())
	{
	  case Left	: if (Org) { -- Org; if (Org && Org == pp) -- Org; }
			  break;
	  case '.'	: if (td) Org = pp+1; break;
	  case Home	: DbfGetHome(); break;
	  case End	: Org = l-1; break;
	  case BSp	: if (Org == 0) break;
			  -- Org; if (Org && Org == pp) -- Org;
	  case Del	: if (Org < pp)
			    {
			      memcpy(buff+Org, buff+Org+1, pp-Org-1);
			      buff[pp-1] = ' '; break;
			    }
			  else
			    {
			      memcpy(buff+Org, buff+Org+1, l-Org-1);
			      buff[l-1] = '0';
			    }
			  break;
	  case Ins	: InsMode = ! InsMode;
			  W->Cursor.Shape = CurShp(InsMode);
			  SetRefreshWind(W); break;
	  case Enter	:
	  case Esc	: more = FALSE; break;
	  default	: if (LastKey & 0xFF00) { more = FALSE; break; }
			  if (LastKey < '0' || LastKey > '9') break;
	  case '-'	: if (LastKey == '-' && Org > pp) break;
			  if (virg)
			    {
			      for (i = 0; i < pp; i ++) buff[i] = ' ';
			      for (i = pp+1; i < l; i ++) buff[i] = '0';
			    }
			  if (InsMode)
			    {
			      if (Org < pp)
				{
				  movmem(buff+Org, buff+Org+1, pp-Org-1);
				}
			      else
				{
				  movmem(buff+Org, buff+Org+1, l-Org-1);
				}
			    }
			  buff[Org] = LastKey;
	  case Right	: if (Org < l-1) { ++ Org; if (Org == pp) ++ Org; }
			  break;
	}
      virg = FALSE;
    }
  return TRUE;
}






Boolean WindGetL (Wind *W, int x, int y, char *buff)
{
  Boolean more = TRUE;

  CursorGoto(W, x, y);
  W->Cursor.Shape = CurShp(InsMode);
  SetRefreshWind(W);
  while (more)
    {
      WriteWindMem(W, x, y, buff, 1);
      switch (LastKey = GetKey())
	{
	  case Ins	: InsMode = ! InsMode;
			  W->Cursor.Shape = CurShp(InsMode);
			  SetRefreshWind(W); break;
	  case 'f'	:
	  case 'F'	:
	  case 't'	:
	  case 'T'	:
	  case 'n'	:
	  case 'N'	:
	  case 'y'	:
	  case 'Y'	: *buff = LastKey; break;
	  default	: more = FALSE; break;
	}
    }
  return TRUE;
}






Keys WindDbfGet (Wind *W, int x, int y, int fld)
{
  char *p = DbfFldPtr(fld);
  switch (Work.DescPtr[fld].FldTpe)
    {
      case 'D' : WindGetD(W, x, y, p); break;
      case 'N' : WindGetN(W, x, y, p,
				   Work.DescPtr[fld].FldLen,
				   Work.DescPtr[fld].FldDec);
		 break;
      case 'L' : WindGetL(W, x, y, p); break;
      case 'M' : CursorGoto(W, x, y); LastKey = GetKey(); break;
      case 'C' :
      default  : WindGetC(W, x, y, p,
				   Work.DescPtr[fld].FldLen);
		 break;
    }
  return LastKey;
}
