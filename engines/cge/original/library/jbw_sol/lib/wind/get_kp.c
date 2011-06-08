#include	<wind.h>



extern	Keys	(*KeyProc)	(Keys);

Keys (*GetKeyProc (void))()
{
  return KeyProc;
}





