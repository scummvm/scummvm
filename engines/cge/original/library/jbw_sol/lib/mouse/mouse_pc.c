#include	<wind.h>
#include	<dos.h>



int MousePCount (int Button)
{
  if (! Mouse) return 0;
  _AX = 0x0005; _BX = Button; __int__(0x33);
  return _BX;
}
