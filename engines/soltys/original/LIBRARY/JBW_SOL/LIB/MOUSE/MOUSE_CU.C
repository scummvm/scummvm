#include	<wind.h>
#include	<dos.h>



void MouseCursor (Boolean on)
{
  _AX = 0x000A; /* set cursor */
  _BX = 0x0000; /* atribute mode */
  _CX = 0xFFFF; /* screen mask */
  _DX = 0x7700; /* cursor mask */
  __int__(0x33);

  _AX = 0x0001 + (on == 0); /* turn cursor on/off */
  __int__(0x33);
}
