#include	<general.h>
#include	<io.h>
#include	<dos.h>
#include	<fcntl.h>
#include	<string.h>
#include	<alloc.h>






DFILE::DFILE (const char near * name, MODE mode)
: XFILE(mode)
{
  switch (mode)
    {
      case REA : Error = _dos_open(name, O_RDONLY | O_DENYNONE, &Handle); break;
      case WRI : Error = _dos_creat(name, FA_ARCH, &Handle); break;
      case UPD : Error = _dos_open(name, O_RDWR | O_DENYALL, &Handle); break;
    }
}







DFILE::~DFILE (void)
{
  Error = _dos_close(Handle);
  Handle = -1;
}














word DFILE::Read (byte far * buf, word len)
{
  if (Mode == WRI || Handle < 0) return 0;
  if (len) Error = _dos_read(Handle, buf, len, &len);
  return len;
}











word DFILE::Write (byte far * buf, word len)
{
  if (len)
    {
      if (Mode == REA || Handle < 0) return 0;
      Error = _dos_write(Handle, buf, len, &len);
    }
  return len;
}







long DFILE::Mark (void)
{
  return tell(Handle);
}






long DFILE::Seek (long pos)
{
  lseek(Handle, pos, SEEK_SET);
  return tell(Handle);
}









long DFILE::Size (void)
{
  if (Handle < 0) return 0;
  return filelength(Handle);
}





Boolean DFILE::Exist (const char * name)
{
  return access(name, 0) == 0;
}
