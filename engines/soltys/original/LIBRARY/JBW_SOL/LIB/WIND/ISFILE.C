#include	<wind.h>
#include	<io.h>





Boolean IsFile (const char *fname)
{
  return !access(fname, 0);
}
