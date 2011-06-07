#include	<general.h>
#include	<dos.h>



IOHAND::~IOHAND (void)
{
  if (Handle != -1)
    {
      Error = _dos_close(Handle);
      Handle = -1;
    }
}
