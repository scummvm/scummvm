#include	<general.h>
#include	<io.h>



Boolean IOHAND::Exist (const char * name)
{
  return access(name, 0) == 0;
}
