#include	<wind.h>



extern	void	(*HelpProc)	(void);

void (*GetHelpProc(void))()
{
  return HelpProc;
}



