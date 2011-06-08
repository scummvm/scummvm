#include	<wind.h>



extern	Keys	(*KeyProc)	(Keys) = NULL;

void SetKeyProc (Keys (*kp)())
{
  KeyProc = kp;
}





