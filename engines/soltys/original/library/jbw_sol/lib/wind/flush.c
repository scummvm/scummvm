#include	<wind.h>


int _fastcall FileFlush (int handle)
{
  asm	mov	bx,handle
  asm	mov	ah,0x68	// Flush buffer
  asm	int	0x21
  asm	mov	ax,0	// 0 means OK...
  asm	jnc	xit	// ...if CY=0
  asm	dec	ax	// -1 means error
  xit:
  return _AX;
}
