#include	<general.h>



word atow (const char * a)
{
  word w = 0;
  if (a)
    {
      while (IsDigit(*a))
	{
	  w = (10 * w) + (*(a ++) & 0xF);
	}
    }
  return w;
}
