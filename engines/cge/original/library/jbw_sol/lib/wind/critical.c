#include	<wind.h>
#include	<string.h>
#include	<dos.h>
#include	<errno.h>


#pragma argsused
int _loadds  CriticalError (int err, int dev, word dhs, word dho)
{
  int i, cr;
  char txt[80];

  cr = Critic; Critic = TRUE;
  if (dev < 0)
    {
      strcpy(txt, HrdErrDevTxt);
      i = strlen(txt); txt[i++] = ' ';
      Move((char far *)(txt + i), MK_FP(dhs, dho+10), 8);
      i += 8; txt[i --] = '\0';
      do { txt[i] = '\0'; -- i; } while (txt[i] == ' ');
    }
  else
    {
      strcpy(txt, HrdErrDrvTxt);
      strcat(txt, " A");
      txt[strlen(txt)-1] += dev & 0x00FF;
    }
  strcat(txt, ": ");
  strcat(txt, HrdErrInfo[err]);

  do
    { i = Question(WAR, txt, HrdErrQueTxt); }
  while (i < 0 || i > 1);
  Refresh();
  Critic = cr;
  switch (i)
    {
      case 0  : hardresume(1); break;
      default : hardretn(EFAULT); break;
    }
  return 0;
}






