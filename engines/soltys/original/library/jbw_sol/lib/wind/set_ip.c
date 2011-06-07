#include	<wind.h>



extern	void    (*IdleProc)	(void) = NULL;

void SetIdleProc (void (*ip)())
{
  IdleProc = ip;
}


