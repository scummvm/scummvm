#include	<wind.h>
#include	<stddef.h>



static word ShowRainbowChar (void)
{
  static char frm[8] = "ÚÄÄ¿ÀÄÄÙ";

  // compute color row number
  asm	mov	AX,CX
  asm	mov	CH,3
  asm	div     CH

  // blank row? column?
  asm	test	DL,0x01
  asm	jz	blk_row
  asm	or	AH,AH
  asm	jz	blk_col

  // assemble attribute
  asm	and	DL,0x0E
  asm	shl	DL,1
  asm	shl	DL,1
  asm	shl	DL,1
  asm	or	DL,AL

  // determine foreground character
  asm	mov	AL,'('
  asm	test	AH,1
  asm	jnz	retn
  asm	mov	AL,')'

  retn:
  asm	mov	AH,DL
  return _AX;

  blk_row:
  asm	lea	si,frm
  asm	shr	DL,1
  asm	sub	DL,[BX].Vp
  asm	jz	hfra
  asm	dec	DL
  asm	jnz	blank
  asm	add	si,4

  hfra:
  asm	mov	AL,[BX].Hp
  asm	shl	AL,1		// * 2
  asm	add	AL,[BX].Hp	// * 3
  asm	sub	CL,AL
  asm	jc	blank
  asm	cmp	CL,3
  asm	ja	blank
  asm	xor	CH,CH
  asm	add	SI,CX
  asm	mov	AL,[SI]
  asm	jmp	mono_ret

  // blank column
  blk_col:
  asm	shr	DL,1
  asm	cmp	DL,[BX].Vp
  asm	jne	blank
  asm	cmp	AL,[BX].Hp
  asm	je	vbar
  asm	dec	AL
  asm	cmp	AL,[BX].Hp
  asm	jne	blank

  // vertical bar
  vbar:
  asm	mov	AL,179
  asm	je	mono_ret	// jmp

  // blank
  blank:
  asm	mov	AL,' '

  // add attribute, then return
  mono_ret:
  asm	mov	AH,[BX].Color+FRM_N
  return _AX;
}





Boolean SetColor (byte *c)
{
  Wind *PAL = CreateWind(0, 0, 50, 18, DSK, FALSE, TRUE);

  if (PAL == NULL) return FALSE;
  PAL->ShowProc = ShowRainbowChar;
  PAL->Flags.Movable = TRUE;
  CenterWind(PAL);

  BringToFront(PAL);
  ShowWind(PAL);

  actual:
  c = ((byte *) (Colors[Mono]))+i; oldc = *c;
  x = oldc % 16; y = oldc / 16;
  PAL->Hp = x;
  PAL->Vp = y;
  oldm = Mono;

  while (k != Esc && k != Enter)
    {
      k = GetKey();
      if (Mono != oldm) goto actual;
      switch (k)
	{
	  case TwiceLeft:
	  case MouseLeft:
		       if (PosToWind(MX, MY) == PAL)
			 {
			   x = RelX(PAL, MX);
			   y = RelY(PAL, MY);
			   if (x < 0 || y < 0) break;
			   if (x % 3 == 0 || y % 2 == 0) break;
			   x /= 3; y /= 2;
			   if (x > 15 || y > 7) break;
			   PAL->Hp = x; PAL->Vp = y;
			   if (k == TwiceLeft) k = Enter;
			 }
		       break;
	  case Up    : PAL->Vp = (PAL->Vp - 1) &  7; break;
	  case Down  : PAL->Vp = (PAL->Vp + 1) &  7; break;
	  case Left  : PAL->Hp = (PAL->Hp - 1) & 15; break;
	  case Right : PAL->Hp = (PAL->Hp + 1) & 15; break;
	}
      *c = (k == Esc) ? oldc : (16 * PAL->Vp + PAL->Hp);
      SetRefreshDesk();
    }
  HideWind(PAL);
  return TRUE
}






Boolean ColorChoice (int x, int y, char *names)
{
  static int cc = 0;
  static Boolean active = FALSE;
  Wind *CM;
  int i = -1;

  if (active) return FALSE;
  active = TRUE;

  CM = MakeMenu(0, 0, MNU, names, ColorBanks*ColorRegs, cc);
  if (CM == NULL) return FALSE;
  CM->Bot = CM->Top+11;
  CM->Flags.Movable = TRUE;
  if (x < 0) CenterWindHorz(CM); else MoveWind(CM, x, CM->Top);
  if (y < 0) CenterWindVert(CM); else MoveWind(CM, CM->Lft, y);
  ShowWind(CM);

  while (TRUE)
    {
      byte *c, oldc;
      Boolean oldm;
      int x, y;
      Keys k = NoKey;

      BringToFront(CM);
      i = MenuChoice(CM);
      if (i >= 0)
	{

	}
      else if (LastKey == Esc) break;
    }
  CloseWind(CM);
  active = FALSE;
  if (i < 0) return FALSE;
  cc = i;
  return TRUE;
}
