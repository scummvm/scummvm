#include	<general.h>



void StdLog (const char *msg, dword d)
{
  static char s[] = "xxxxxxxx";
  dwtom(d, s, 16, 8);
  StdLog(msg, s);
}
