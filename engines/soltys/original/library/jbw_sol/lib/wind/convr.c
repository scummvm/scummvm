#include	<wind.h>





extern char (*Convr)(char) = NULL;




void SetConvrProc (char (*cp)(char))
{
  Convr = cp;
}

