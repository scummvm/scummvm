#include	<wind.h>
#include	<string.h>



extern	Wind	*KeyBarWind = NULL;




static word ShowKeyBarChar (void)
{
  asm add si,[bx].Body.Near
  asm mov al,[si]
  asm xor si,si
  asm cmp cl,80
  asm jnl rtn
  asm cmp cl,73
  asm jnl mod8
  asm inc cl

  mod8:
  asm and cl,7
  asm cmp cl,1
  asm jng rtn
  asm inc si

  rtn:
  asm mov ah,[bx+si].Color+SEL_N
  return _AX;
}





Wind * MakeKeyBar (const char *bar)
{
  KeyBarWind = CreateWind(0, ZoomBot, strlen(bar)-1, ZoomBot, DSK, FALSE, FALSE);
  if (KeyBarWind != NULL)
    {
      KeyBarWind->Body.Near = bar;
      KeyBarWind->ShowProc = ShowKeyBarChar;
      ShowWind(KeyBarWind);
    }
  -- ZoomBot;
  return KeyBarWind;
}
