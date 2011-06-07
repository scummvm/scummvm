#include	<general.h>
#include	<dos.h>



word IOHAND::Read (void far * buf, word len)
{
  if (Mode == WRI || Handle < 0) return 0;
  if (len) Error = _dos_read(Handle, buf, len, &len);
  if (Crypt) Seed = Crypt(buf, len, Seed);
  return len;
}
