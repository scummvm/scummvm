#include	<general.h>
#include	<dos.h>


word IOHAND::Write (void far * buf, word len)
{
  if (len)
    {
      if (Mode == REA || Handle < 0) return 0;
      if (Crypt) Seed = Crypt(buf, len, Seed);
      Error = _dos_write(Handle, buf, len, &len);
      if (Crypt) Seed = Crypt(buf, len, Seed); //------$$$$$$$
    }
  return len;
}
