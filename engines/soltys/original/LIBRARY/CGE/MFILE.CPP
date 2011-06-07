#include	"mfile.h"
#include	<dos.h>
#include	<fcntl.h>
#include	<string.h>
#include	<alloc.h>






MFILE::MFILE (byte far * adr, long siz, MODE mode)
: XFILE(mode), Adr(adr), Ptr(0), Lim((byte far *)((long)adr+siz))
{
}







MFILE::~MFILE (void)
{
}














word MFILE::Read (byte far * buf, word len)
{
  if (Mode == WRI) return 0;
  if (Ptr + len > Lim) len = (word) (Lim - Ptr);
  _fmemcpy(buf, Ptr, len);
  Ptr += len;
  return len;
}


word MFILE::Write (byte far * buf, word len)
{
  if (len)
    {
      if (Mode == REA) return 0;
      if (Ptr + len > Lim) len = (word) (Lim - Ptr);
      _fmemcpy(Ptr, buf, len);
      Ptr += len;
    }
  return len;
}







long MFILE::Mark (void)
{
  return Ptr - Adr;
}






long MFILE::Seek (long pos)
{
  long n = Size();
  if (pos > n) pos = n;
  Ptr = (byte far *) ((long) Adr + pos);
  return pos;
}









long MFILE::Size (void)
{
  return Lim - Adr;
}
