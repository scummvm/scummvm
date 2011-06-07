#include	<general.h>



ALLOC_MODE SetAllocMode (ALLOC_MODE am)
{
  asm	mov	ax,0x5800	// get mode
  asm	int	0x21
  asm	push	ax		// preserve recent mode
  asm	mov	ax,0x5801	// set mode
  asm	mov	bx,am		// desired mode
  asm	int	0x21
  asm	pop	ax		// return old value

  return (ALLOC_MODE) _AX;
}
