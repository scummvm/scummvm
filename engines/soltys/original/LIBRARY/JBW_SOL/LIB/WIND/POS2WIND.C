#include	<wind.h>



extern	Wind *		WindStack;


Wind *PosToWind (int x, int y)
{
  _BX = (word) WindStack;
  chk0:
  _CX = ZoomTop;
  _DX = ZoomBot;
  asm    or	bx,bx
  asm	 je	fail
  asm	 mov	ax,[bx].Flags
  asm	 test	ax,AppearMask
  asm	 jz	next
  asm    test   ax,ZoomedMask
  asm    jnz    chkY
  asm	 mov	cx,[bx].Top
  asm	 mov	dx,[bx].Bot
  asm	 mov	ax,x
  asm	 cmp	ax,[bx].Lft
  asm	 jl	next
  asm	 cmp	ax,[bx].Rgt
  asm	 jg	next
  chkY:
  asm	 mov	ax,y
  asm	 cmp	ax,cx
  asm	 jl	next
  asm	 cmp	ax,dx
  asm	 jng	inside
  next:
  asm	 mov	bx,[bx].Next
  asm	 jmp	chk0
  fail:
  asm	 xor	bx,bx
  inside:
  return (Wind *) _BX;
}






