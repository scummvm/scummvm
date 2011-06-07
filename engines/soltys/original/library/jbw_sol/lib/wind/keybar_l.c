#include	<wind.h>
#include	<string.h>



extern	Wind	*KeyBarWind;




void SetKeyLabel (Keys Fn, const char *Lab)
{
  int x;
  char *p = KeyBarWind->Body.Near;

  asm	mov	ax,Fn
  asm	sub	ax,F1
  asm	js	short skl_xit
  asm	cmp	ax,9
  asm	jg	short skl_xit
  asm	mov	cl,3
  asm	shl	ax,cl
  asm	inc	ax
  asm	cmp	ax,72
  asm	jb	skl_do
  asm	inc	ax

  skl_do:
  p += (x = _AX);
  if (memcmp(p, Lab, 6) != 0) WriteWindMem(KeyBarWind, x, 0, Lab, 6);

  skl_xit:
}
