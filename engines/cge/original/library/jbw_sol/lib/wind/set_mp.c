#include	<wind.h>



extern	void	(*MenuProc)	(int)  = NULL;




void SetMenuProc (void (*mp)())
{
  MenuProc = mp;
}
