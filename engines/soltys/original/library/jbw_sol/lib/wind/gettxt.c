#include	<wind.h>
#include	<string.h>
#include	<stdlib.h>




static	byte	Mode;




static word ShowGTChar (void)
{
  asm	add	si,[bx].Body.Near
  asm	mov	al,[si]
  asm	mov	ah,Mode
  asm	cmp	ah,1
  asm	jna	set_colr
  asm	test	al,not ' '
  asm	jz	set_colr
  asm	mov	al,ah
  set_colr:
  asm	mov	ah,[bx].Color+FLD_N
  return _AX;
}



char * GetString (char * str, int len, int caps, const char * title, ColorBank c)
#define lmar 1
/*
  M - text offset in window body
  N - current cursor position
*/
{
  Wind *W;
  char *p;
  Keys k;
  Boolean first = TRUE, finis = FALSE;
  int wid, n = 0;

  Mode = caps;
  if (len > 128) len = 128;
  wid = strlen(title);
  if (wid < len) wid = len;
  wid +=2;
  W = MakeWind(0, 0, wid+lmar, 2, c);
  if (W == NULL) return NULL;
  W->ShowProc = ShowGTChar;
  W->Flags.Capital = (caps != 0);
  W->Flags.Movable = TRUE;
  SetTitle(W, title, -1);
  StdMove(W);

  W->Cursor.Shape = (Mono) ? HGC_Cursor : CGA_Cursor;
  wid = strlen(str);
  WriteWindMem(W, lmar, 0, str, min(len, wid) + 1);
  ShowWind(W);
  p = Image(W) + lmar;
  p[len] = '\0';
  while (! finis)
    {
      CursorGoto(W, lmar+n, 0);
      SetRefreshWind(W);
      k = GetKey();
      if (W->Flags.Capital) k = Upper(k);
      switch (k)
	{
	  case TwiceLeft:
		       if (PosToWind(MX, MY) == W) k = Enter;
		       else break;
	  case MouseRight:
	  case Enter :
	  case Esc   : finis = TRUE; break;
	  case BSp   : if (n) Stuff(p, -- n, 1, ""); break;
	  case Home  : n = 0; break;
	  case End   : n = strlen(p); break;
	  case Left  : if (n) -- n; break;
	  case Right : if (n < strlen(p)) ++n; break;
	  case Del   : if (p[n]) Stuff(p, n, 1, ""); break;
	  default    : if ((word) k < 256)
			 {
			   if (first)
			     {
			       memset(p, 0, len);
			       n=0;
			     }
			   if (strlen(p) < len)
			   Stuff(p, n ++, 0, (const char *) &k);
			 }
		       break;
	}
      first = FALSE;
    }
  if (k == Enter) strcpy(str, p);
  CloseWind(W);
  return (k == Enter) ? str : NULL;
}
#undef lmar
