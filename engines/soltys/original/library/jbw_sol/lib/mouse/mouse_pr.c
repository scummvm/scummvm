#include	<wind.h>
#include	<dos.h>



int MousePressed (int ButtonMask)
{
  if (! Mouse) return 0;
  _AX = 0x0003; __int__(0x33);
  return (_BX & ButtonMask);
}
