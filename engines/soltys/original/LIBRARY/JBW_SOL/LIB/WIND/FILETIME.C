#include	<wind.h>
#include	<io.h>
#include	<fcntl.h>






dword DwordFileTime (const char *fn)
{
  union { struct ftime t; dword d; } ft;
  int fh = _open(fn, O_RDONLY | O_DENYNONE);
  if (fh < 0) return 0L;
  getftime(fh, &ft.t);
  _close(fh);
  return ft.d;
}
