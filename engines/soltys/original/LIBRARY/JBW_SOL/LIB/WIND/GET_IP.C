#include	<wind.h>



extern	void    (*IdleProc)	(void);

void (*GetIdleProc(void))()
{
  return IdleProc;
}



