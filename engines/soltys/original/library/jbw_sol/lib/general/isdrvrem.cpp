#include	<general.h>
#include	<dos.h>

int DriveRemote (unsigned drv)
{
  _BX = drv;
  _AX = 0x4409;
  geninterrupt(0x21);
  return (_DH & 0x10) != 0;
}
