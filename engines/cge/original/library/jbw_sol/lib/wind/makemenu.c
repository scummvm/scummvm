#include	<wind.h>
#include	<string.h>



extern	Wind	*WindStack;




word ShowMenuChar (void)
{
  //Wind *W = (Wind *) _BX; int x = _CX, y = _DX;

  asm xchg CX,DX		// this releases DX for further MUL
  asm mov AL,' '
  // (x < W->Wid) ?
  asm cmp DX,[BX].Wid
  asm jae bar
  // (y < W->Hig-W->Vp+W->Y) ?
  asm mov DX,[BX].Hig
  asm sub DX,[BX].Vp
  asm add DX,[BX].(Wind)Y
  asm cmp CX,DX
  asm jae bar

  asm mov AX,[BX].Vp
  asm sub AX,[BX].(Wind)Y
  asm mov DX,[BX].Wid
  asm mul DX
  asm add SI,AX
  asm add SI,[BX].Body.Near
  asm mov al,[SI]

  bar:
  asm xor SI,SI
  asm cmp CX,[BX].(Wind)Y
  asm jne let
  asm inc  SI
  asm inc SI

  let:
  asm mov  CX,[BX].Flags
  asm test CX,KeyChoiceMask
  asm jz   rtn
  asm cmp  al,'A'
  asm jl   rtn
  asm cmp  al,'Z'
  asm jg   rtn
  asm inc  SI

  rtn:
  asm mov  AH,[BX+SI].Color+FLD_N
  return _AX;
  //c = (x < W->Wid && y < W->Hig-W->Vp+W->Y) ? Image(W)[W->Wid*(y+W->Vp-W->Y)+x] : ' ';
  //return W->Attr[(y==W->Y) + ((W->Flags.KeyChoice) ? 2*IsUpper(c) : 0)] | c;
}




Wind * MakeMenu (int l, int t, /*char *t,*/ ColorBank c, char *mnu, int max, int cur)
{
  if (CreateWind(l, t, l+strlen(mnu)/max+1, t+max+1, c, FALSE, TRUE) == NULL) return NULL;
  WindStack->Body.Near = mnu;
  WindStack->Hig = max;
  WindStack->Y = cur;
  WindStack->Vp = cur;
  WindStack->ShowProc = ShowMenuChar;
  WindStack->ReptProc = RepaintMenuWind;
  return WindStack;
}
