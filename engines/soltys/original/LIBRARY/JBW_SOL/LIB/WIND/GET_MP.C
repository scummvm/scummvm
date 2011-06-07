#include	<wind.h>



extern	void	(*MenuProc)	(int);




void (*GetMenuProc (void))()
{
  return MenuProc;
}
