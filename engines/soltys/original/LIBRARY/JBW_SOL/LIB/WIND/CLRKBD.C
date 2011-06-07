#include	<wind.h>


#define		BIOS_KEY	0x16
#define		CHECK_KEY	1
#define		TAKE_KEY	0



void ClearKeyboard (void)
{
  more:
  asm	mov	ah,CHECK_KEY
  asm	int	BIOS_KEY
  asm	jz	empty
  asm	mov	ah,TAKE_KEY
  asm	int	BIOS_KEY
  asm	jmp	short more
  empty:
  LastKey = NoKey;
  NextKey = NoKey;
  while (MousePressed(3));
}
