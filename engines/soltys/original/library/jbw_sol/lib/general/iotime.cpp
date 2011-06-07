#include	<general.h>
#include	<io.h>






ftime IOHAND::Time (void)
{
  ftime t;
  getftime(Handle, &t);
  return t;
}









void IOHAND::SetTime (ftime t)
{
  setftime(Handle, &t);
}

