#include	<general.h>
#include	<io.h>
#include	<dos.h>
#include	<fcntl.h>
#include	<string.h>
#include	<alloc.h>


#define		BUF		((byte far *) buf)



IOHAND::IOHAND (IOMODE mode, CRYPT * crpt)
: XFILE(mode), Handle(-1), Crypt(crpt), Seed(SEED)
{
}







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







IOHAND::~IOHAND (void)
{
  if (Handle != -1)
    {
      Error = _dos_close(Handle);
      Handle = -1;
    }
}






word IOHAND::Read (void far * buf, word len)
{
  if (Mode == WRI || Handle < 0) return 0;
  if (len) Error = _dos_read(Handle, buf, len, &len);
  if (Crypt) Seed = Crypt(buf, len, Seed);
  return len;
}











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







long IOHAND::Mark (void)
{
  return (Handle < 0) ? 0 : tell(Handle);
}






long IOHAND::Seek (long pos)
{
  if (Handle < 0) return 0;
  lseek(Handle, pos, SEEK_SET);
  return tell(Handle);
}









long IOHAND::Size (void)
{
  if (Handle < 0) return 0;
  return filelength(Handle);
}






Boolean IOHAND::Exist (const char * name)
{
  return access(name, 0) == 0;
}
