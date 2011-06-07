#include	<fastio.h>
#include	"dbf_inc.h"
#include	<mem.h>


#define		CurCGA		((InsMode)  ? 0x0407 : 0x0607)
#define		CurHGC		((InsMode)  ? 0x080C : 0x0B0C)
#define		CurShp		((IsMono()) ? CurHGC : CurCGA)


extern	int	MaxSaySize;

static	Boolean	InsMode	= FALSE;





void SayC (int x, int y, const char *s, int l)
{
  SayMemD(x, y, s, min(MaxSaySize, l));
}






Boolean DbfSay (int x, int y, int fld)
{
  switch (Work.DescPtr[fld].FldTpe)
    {
      case 'D' : SayC(x,y,DbfD2C(DbfFldPtr(fld)),10); break;
      case 'N' :
      case 'M' :
      case 'L' :
      case 'C' :
      default  : SayC(x,y,DbfFldPtr(fld),Work.DescPtr[fld].FldLen);
		 break;
    }
  return TRUE;
}







Boolean GetC (int x, int y, char *buff, int l)
{
  int org = 0, pos = 0;
  Boolean dat = (l == 0);
  Boolean more = TRUE;
  int color = SetColor(STD+REV), cursor = SetCursor(CurShp);

  if (dat) l = 8;

  while (more)
    {
      SetXY(x+pos+((pos>=4)+(pos>=6))*dat, y);
      SayC(x, y, ((dat) ? DbfD2C(buff) : buff) + org-pos, l+dat+dat);
      switch (LastKey = GetKey())
	{
	  case Home	: org = (pos = 0); break;
	  case End	: for (org = l-1; org && buff[org] == ' ' ;org --);
			  if (buff[org] != ' ' && org < l-1) ++ org;
			  pos = min(MaxSaySize-1, org); break;
	  case Del	: memcpy(buff+org, buff+org+1, l-org-1);
			  buff[l-1] = ' '; break;
	  case Ins	: InsMode = ! InsMode; SetCursor(CurShp); break;
	  case BSp	: if (org)
			    {
			      memcpy(buff+org-1,buff+org,l-org);
			      buff[l-1]=' ';
			    }
	  case Left	: if (org) { --org; if (pos) --pos; } break;
	  case Enter	:
	  case Esc	: more = FALSE; break;
	  default	: if (LastKey > 255) { more = FALSE; break; }
			  if (dat && (LastKey < '0' || LastKey > '9')) break;
			  if (InsMode) movmem(buff+org, buff+org+1, l-org-1);
			  buff[org] = LastKey;
	  case Right	: if (org<l-1) { ++org; if (pos<MaxSaySize-1) ++pos; }
			  break;
	}
    }
  SetColor(color);
  SetCursor(cursor);
  return TRUE;
}






Boolean GetN (int x, int y, char *buff, int l, int d)
{
  int i, td, pp, pos = 0;
  Boolean neg;
  Boolean more = TRUE, virg = TRUE;
  char *p;
  int color = SetColor(STD+REV), cursor = SetCursor(CurShp);

  p = strchr(buff, '.');
  td = (p == NULL) ? 0 : l - (p-buff+1);
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
      SetXY(x+pos, y); SayC(x, y, buff, l);
      if (! more) break;
      switch (LastKey = GetKey())
	{
	  case Left	: if (pos) { -- pos; if (pos && pos == pp) -- pos; }
			  break;
	  case '.'	: if (td) pos = pp+1; break;
	  case Home	: pos = 0; break;
	  case End	: pos = l-1; break;
	  case BSp	: if (pos == 0) break;
			  -- pos; if (pos && pos == pp) -- pos;
	  case Del	: if (pos < pp)
			    {
			      memcpy(buff+pos, buff+pos+1, pp-pos-1);
			      buff[pp-1] = ' '; break;
			    }
			  else
			    {
			      memcpy(buff+pos, buff+pos+1, l-pos-1);
			      buff[l-1] = '0';
			    }
			  break;
	  case Ins	: InsMode = ! InsMode; SetCursor(CurShp); break;
	  case Enter	:
	  case Esc	: more = FALSE; break;
	  default	: if (LastKey > 255) { more = FALSE; break; }
			  if (LastKey < '0' || LastKey > '9') break;
	  case '-'	: if (LastKey == '-' && pos > pp) break;
			  if (virg)
			    {
			      for (i = 0; i < pp; i ++) buff[i] = ' ';
			      for (i = pp+1; i < l; i ++) buff[i] = '0';
			    }
			  if (InsMode)
			    {
			      if (pos < pp)
				{
				  movmem(buff+pos, buff+pos+1, pp-pos-1);
				}
			      else
				{
				  movmem(buff+pos, buff+pos+1, l-pos-1);
				}
			    }
			  buff[pos] = LastKey;
	  case Right	: if (pos < l-1) { ++ pos; if (pos == pp) ++ pos; }
			  break;
	}
      virg = FALSE;
    }
  SetColor(color);
  SetCursor(cursor);
  return TRUE;
}






Boolean GetL (int x, int y, char *buff)
{
  Boolean more = TRUE;
  int color = SetColor(STD+REV), cursor = SetCursor(CurShp);

  SetXY(x, y);
  while (more)
    {
      *ScrAdr(x, y) = Attr | *buff;
      switch (LastKey = GetKey())
	{
	  case Ins	: InsMode = ! InsMode; SetCursor(CurShp); break;
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
  SetColor(color);
  SetCursor(cursor);
  return TRUE;
}






int DbfGet (int x, int y, int fld)
{
  switch (Work.DescPtr[fld].FldTpe)
    {
      case 'D' : GetC(x, y, DbfFldPtr(fld), 0); break;
      case 'N' : GetN(x, y, DbfFldPtr(fld),
			    Work.DescPtr[fld].FldLen,
			    Work.DescPtr[fld].FldDec);
		 break;
      case 'L' : GetL(x, y, DbfFldPtr(fld)); break;
      case 'M' : GotoXY(x, y); LastKey = GetKey(); break;
      case 'C' :
      default  : GetC(x, y, DbfFldPtr(fld),
			    Work.DescPtr[fld].FldLen);
		 break;
    }
  return LastKey;
}
