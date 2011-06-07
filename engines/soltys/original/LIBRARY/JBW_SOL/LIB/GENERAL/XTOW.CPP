#include	<general.h>



word xtow (const char * x)
{
  word w = 0;
  if (x)
    {
      while (IsHxDig(*x))
	{
	  register word d = * (x ++);
	  if (d > '9') d -= 'A' - ('9' + 1);
	  w = (w << 4) | (d & 0xF);
	}
    }
  return w;
}
