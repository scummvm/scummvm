#include	<general.h>
#include	<dos.h>


int DriveFixed (unsigned drv)
{
  _BX =	drv;
  _AX = 0x4408;
  geninterrupt(0x21);
  return _AX;
}
