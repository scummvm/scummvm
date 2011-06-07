#include	<wind.h>




word ShowHMChar (void)
{
  //Wind *W = (Wind *) _BX; int x = _CX, y = _DX;
  //int i = W->Wid*(y)+(x);
  //int c = Image(W)[i];

  asm add  SI,[BX].Body.Near
  asm mov  AL,[SI]

  asm xor  SI,SI
  asm cmp  DX,[BX].(Wind)Y
  asm jne  rtn
  asm cmp  CX,[BX].Hp
  asm jb   capchk
  asm cmp  CX,[BX].(Wind)X
  asm jae  capchk
  asm inc  SI
  asm inc  SI

  capchk:
  asm cmp  AL,'A'
  asm jl   rtn
  asm cmp  AL,'Z'
  asm jg   rtn
  asm inc  SI

  rtn:
  asm mov  AH,[BX+SI].Color+FLD_N
  return _AX;
  //return W->Attr[((i >= W->Vp) && (i < W->Y)) + 2*IsUpper(c)] | c;
}




