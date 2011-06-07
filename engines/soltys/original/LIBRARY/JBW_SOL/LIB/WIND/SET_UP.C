#include	<wind.h>



extern	int	(*UpperProc)	(int)  = NULL;

void SetUpperProc (int (*up)(int))
{
  UpperProc = up;
}





