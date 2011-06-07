#include	"dbf_inc.h"



DbfFilter * DbfSetFilter (DbfFilter * f)
{
  asm	mov	bx,WorkPtr
  asm	mov	ax,[bx].Filter
  asm	mov	cx,f
  asm	mov	[bx].Filter,cx
  return (DbfFilter *) _AX;
}
