#include	<wind.h>
#include	<io.h>





Boolean IsWritable (const char *fname)
{
  return !access(fname, 2);
}
