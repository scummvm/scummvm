#include	<general.h>
#include	<dos.h>
#include	<fcntl.h>


IOHAND::IOHAND (const char near * name, IOMODE mode, CRYPT * crpt)
: XFILE(mode), Crypt(crpt), Seed(SEED)
{
  switch (mode)
    {
      case REA : Error = _dos_open(name, O_RDONLY | O_DENYNONE, &Handle); break;
      case WRI : Error = _dos_creat(name, FA_ARCH, &Handle); break;
      case UPD : Error = _dos_open(name, O_RDWR | O_DENYALL, &Handle); break;
    }
  if (Error) Handle = -1;
}
