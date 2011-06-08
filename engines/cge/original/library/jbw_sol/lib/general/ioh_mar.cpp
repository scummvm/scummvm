#include	<general.h>
#include	<io.h>



long IOHAND::Mark (void)
{
  return (Handle < 0) ? 0 : tell(Handle);
}
