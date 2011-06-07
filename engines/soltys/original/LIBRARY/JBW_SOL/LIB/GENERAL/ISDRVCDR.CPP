#include	<general.h>
#include	<dos.h>


int DriveCD (unsigned drv)
{
  char copr_name[38];
  //--- Installation Check
  _BX = 0;
  _AX = 0x1500;
  geninterrupt(0x2F);
  if (_AL != 0xFF) return 0;
  //--- driver installed, check drv
  if (drv == 0) _dos_getdrive(&drv);
  _ES = FP_SEG(copr_name);
  _BX = FP_OFF(copr_name);
  _CX = drv-1;
  _AX = 0x1502;		// CD-ROM Get Copyright File Name
  geninterrupt(0x2F);
  return (_FLAGS & 1) ^ 1;
}
