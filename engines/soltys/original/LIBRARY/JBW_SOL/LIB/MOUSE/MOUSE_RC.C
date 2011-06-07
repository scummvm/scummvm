#include	<wind.h>
#include	<dos.h>



int MouseRCount (int Button)
{
  if (! Mouse) return 0;
  _AX = 0x0006; _BX = Button; __int__(0x33);
  return _BX;
}
