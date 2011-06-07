#include	<wind.h>
#include	<dos.h>


MouseFunType *SetMouseFun (MouseFunType *p, word f)
{
  asm {	mov	ax,12
	mov	cx,f
	mov	es,word ptr p+2
	mov	dx,word ptr p
	int	0x33
      }
  return MK_FP(_ES, _DX);
}





