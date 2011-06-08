#include	<general.h>
#include	<io.h>



long IOHAND::Size (void)
{
  if (Handle < 0) return 0;
  return filelength(Handle);
}

