#include	<wind.h>
#include	<stddef.h>



extern	int	(*UpperProc)	(int);




int Upper (int c)
{
  return (UpperProc==NULL) ? (IsLower(c) ? (c-('a'-'A')) : c) : UpperProc(c);
}





