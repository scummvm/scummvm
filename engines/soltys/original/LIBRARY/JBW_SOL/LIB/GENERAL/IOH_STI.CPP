#include	<general.h>
#include	<io.h>



void IOHAND::SetTime (ftime t)
{
  setftime(Handle, &t);
}

