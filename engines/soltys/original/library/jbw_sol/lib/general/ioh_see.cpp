#include	<general.h>
#include	<io.h>



long IOHAND::Seek (long pos)
{
  if (Handle < 0) return 0;
  lseek(Handle, pos, SEEK_SET);
  return tell(Handle);
}
