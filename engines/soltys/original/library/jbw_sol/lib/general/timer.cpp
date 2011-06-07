#include	<general.h>




long Timer (void)
{
  asm	mov	ax,0x40
  asm	mov	es,ax
  asm	mov	cx,es:[0x6C]
  asm	mov	dx,es:[0x6E]
  return  ((long) _DX << 16) | _CX;
}
