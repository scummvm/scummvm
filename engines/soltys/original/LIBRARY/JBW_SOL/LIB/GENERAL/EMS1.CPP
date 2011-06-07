#include	<general.h>




EMS::EMS (int count)
: Handle(-1)
{
  if (Test())
    {
      asm	mov	ah,0x43		// open handle
      asm	mov	bx,count
      asm	int	EMS_INT
      asm	or	ah,ah
      asm	jnz	xit
      Handle = _DX;
      asm	mov	ah,0x47		// save context
      asm	int	EMS_INT
    }
  xit:
}







EMS::~EMS (void)
{
  if (Handle >= 0)
    {
      _DX = Handle;
      asm	mov	ah,0x48		// restore context
      asm	int	EMS_INT
      asm	mov	ah,0x45		// close handle
      asm	int	EMS_INT
    }
}



Boolean EMS::Test (void)
{
  static char e[] = "EMMXXXX0";

  asm	mov	ax,0x3D40
  asm	mov	dx,offset e
  asm	int	0x21
  asm	jc	fail

  asm	push	ax
  asm	mov	bx,ax
  asm	mov	ax,0x4407
  asm	int	0x21

  asm	pop	bx
  asm	push	ax
  asm	mov	ax,0x3E00
  asm	int	0x21
  asm	pop	ax

  asm	cmp	al,0x00
  asm	je	fail

  success:
  return TRUE;
  fail:
  return FALSE;
}





void _seg * EMS::Frame (void)
{
  asm	mov	ah,0x41
  asm	int	EMS_INT
  asm	or	ah,ah
  asm	jnz	fail
  return (void _seg *) _BX;

  fail:
  return NULL;
}





void far * EMS::Take (int from)
{
  _DX = Handle;
  asm	or	dx,dx
  asm	js	fail
  asm	mov	ah,0x44
  asm	mov	al,0
  asm	mov	bx,from
  asm	int	EMS_INT
  asm	or	ah,ah
  asm	jnz	fail
  asm	mov	ah,0x44
  asm	mov	al,1
  asm	inc	bx
  asm	int	EMS_INT
  asm	or	ah,ah
  asm	jnz	fail
  asm	mov	ah,0x44
  asm	mov	al,2
  asm	inc	bx
  asm	int	EMS_INT
  asm	or	ah,ah
  asm	jnz	fail
  asm	mov	ah,0x44
  asm	mov	al,3
  asm	inc	bx
  asm	int	EMS_INT
  asm	or	ah,ah
  asm	jnz	fail
  return Frame();

  fail:
  return NULL;
}

