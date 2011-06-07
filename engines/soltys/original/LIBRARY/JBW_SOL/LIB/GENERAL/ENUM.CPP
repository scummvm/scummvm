#include	<general.h>
#include	<string.h>


int TakeEnum (const char ** tab, const char * txt)
{
  const char ** e;
  if (txt)
    {
      for (e = tab; *e; e ++)
	{
	  if (stricmp(txt, *e) == 0)
	    {
	      return e - tab;
	    }
	}
    }
  return -1;
}
