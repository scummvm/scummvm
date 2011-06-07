#include	<general.h>
#include	<string.h>
#include	<dir.h>
#include	<dos.h>





const char * ProgName (const char * ext)
{
  static char pn[MAXFILE-1+MAXEXT];

  fnsplit(_argv[0], NULL, NULL, pn, NULL);
  if (ext)
    {
      char tmp[MAXFILE];
      fnmerge(pn, NULL, NULL, strcpy(tmp, pn), ext);
    }
  return pn;
}
