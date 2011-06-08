#include	<general.h>


void StdLog (const char *msg, word w)
{
  static char s[] = "xxxx";
  wtom(w, s, 16, 4);
  StdLog(msg, s);
}
