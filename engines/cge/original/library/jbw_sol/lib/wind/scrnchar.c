#include	<wind.h>




word ShowScreenChar (void)
{
  asm shl  SI,1
  asm add  SI,[BX].Body.Near
  asm lodsw
  return _AX;
}
