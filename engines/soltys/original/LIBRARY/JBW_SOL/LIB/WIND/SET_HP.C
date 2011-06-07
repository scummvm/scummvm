#include	<wind.h>



extern	void	(*HelpProc)	(void) = NULL;

void SetHelpProc (void (*hp)())
{
  HelpProc = hp;
}



