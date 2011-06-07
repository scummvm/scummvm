#include	<wind.h>
#include	<stddef.h>




extern	Wind	*WindStack;




Wind * MakeWind (int l, int t, int r, int b, /*char *tit,*/ ColorBank c)
{
  if (CreateWind(l, t, r, b, c, TRUE, TRUE) == NULL) return NULL;
  return WindStack;
}
