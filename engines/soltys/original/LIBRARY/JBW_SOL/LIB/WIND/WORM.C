#include	<wind.h>
#include	<stdlib.h>
#include	<dos.h>
#include	<bios.h>


static struct WormStruct { int X, Y, C; } WormBody[] = { { 0, 0, 'Û' },
							 { 0, 0, '²' },
							 { 0, 0, '±' },
							 { 0, 0, '°' }  };





static void WormStep (Wind *W)
{
  static int dx = 1, dy = 0;
  int x = WormBody[0].X, y = WormBody[0].Y;

  switch (random(8))
    {
      case 0  : if (dy) { dx =  dy; dy = 0; }
		else    { dy =  dx; dx = 0; } break;
      case 1  : if (dy) { dx = -dy; dy = 0; }
		else    { dy = -dx; dx = 0; } break;
    }
  x += dx; y += dy;
  if (x >= 0 && x < MaxScrWid && y >= 0 && y < MaxScrHig)
    {
      register struct WormStruct * wp;
      for (wp = WormBody + ArrayCount(WormBody) - 1; wp > WormBody; wp --)
	{
	  SetRefreshChar(W, wp->X, wp->Y);
	  wp->X = (wp-1)->X;
	  wp->Y = (wp-1)->Y;
	}
      SetRefreshChar(W, WormBody[0].X = x, WormBody[0].Y = y);
      delay(1000/ProtectSpeed);
    }
  W->Flags.Repaint = TRUE;
}





static word ShowWormChar (void)
{
  asm mov AH,[BX].Color+FLD_H
  asm mov AL,' '
  asm mov DH,CL
  asm lea SI,WormBody
  _CX = ArrayCount(WormBody);
  again:
  asm cmp DH,[SI].(struct WormStruct)X
  asm jne nxt
  asm cmp DL,[SI].(struct WormStruct)Y
  asm jne nxt

  asm mov AL,[SI].(struct WormStruct)C
  return _AX;

  nxt:
  _SI += sizeof(*WormBody);
  asm loop again

  asm mov AL,' '
  return _AX;
}








Wind * MakeWorm (void)
{
  Wind *w = CreateWind(0, 0, MaxScrWid-1, MaxScrHig-1, DSK, FALSE, FALSE);
  w->ShowProc = ShowWormChar;
  w->ReptProc = WormStep;
  ShowWind(w);
  w->Flags.Repaint = TRUE;
  return w;
}
