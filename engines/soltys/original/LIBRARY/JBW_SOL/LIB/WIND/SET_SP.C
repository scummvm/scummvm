#include	<wind.h>



extern	Wind *	(*SaveProc)	(void) = NULL;

void SetSaveProc (Wind * (*sp)())
{
  SaveProc = sp;
}





